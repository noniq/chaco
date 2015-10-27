;changelog:
;05.03.07:	sys,ip-addr, changed start address to $c000 !
;28.02.07: 	call returns reg values
;28.02.07:	rewritten for ca65
;---------------------------------------
;netmon
;
;rr-net monitoring server
;oct.06 hannenz@freenet.de
;based on udpslave by magervalp
;assemble with turbomacropro/style.
;---------------------------------------

;packet_page	= $de02
;packet_data	= $de04
;rxtx_data	= $de08
;tx_cmd		= $de0c
;tx_len		= $de0e

ptr		= $2d
len		= $57
;ip_cksum	= $fb
;ip_len		= $fd
packet_len	= $ff

BUF_SIZE	= 256
;---------------------------------------
.code

	.word *+2
;---------------------------------------
;here we go...
        jmp init

        .include "chbidir.s"

init:
        lda #1
        sta 646
	lda #<mssg
	ldy #>mssg
	jsr $ab1e

	lda #0
	sta ptr

	lda #8     ;default device
	sta dev
	lda #$37   ;access rom/io
	sta access
	sta 1

	jsr init_usb

;---------------------------------------
;main loop: wait for incoming packets
;and react upon
;---------------------------------------

main:
	tsx
	stx stack

	lda #0
        sta $d020
        sta $d021
        lda #1
        sta 646

@wait:
	;user break...?! (stop)

	lda $91
	cmp #$7f
	bne @sk
	jmp $a84b    ;basic break
@sk:
        jsr check_data_available
        bcc @wait

        jsr get_byte
        sta packet_len

;        jsr get_byte
;        sta $0428
;        sta data

	;read data and store in
	;data buffer

 	ldy #0
:
;       jsr read_word
        jsr get_byte
	sta data,y
;	txa
;	sta data+1,y
;	iny
	iny
	cpy packet_len
	bne :-

;now we have the data in data buf. and we know that the data is for us!
;so - let's rock!
        inc $d020

	;act upon first data byte:
	lda data
;	sta $0450
	asl
	tax
	lda cmdtab+1,x
	pha
	lda cmdtab,x
	pha

	rts

@tomain:
	jmp main

;cr:	lda #13
;	.byte $2c
;dot:	lda #'.';
;	.byte $2c
;colon:	lda #':'
;	.byte $2c
;space:	lda #' '
;	jmp $ffd2
;
;decnum:	lda #0
;	jmp $bdcd
;---------------------------------------
;send string to current dev, read
;command channel and send output back
;1 = len
;2... string

setlfs	= $fe00
setnam	= $fdf9
open	= $f34a
close	= $f291
chkin	= $f20e
ckout	= $f250
clrch	= $f333

bsout	= $f1ca
chrin	= $f157


diskcommand:
	lda #$37
	sta 1
	jsr $f32f  	;clall

	lda #15    	;command channel
	tay
	ldx dev
	jsr setlfs
	lda #0     	;no filename
	jsr setnam
	jsr open	;open

	lda data+1
	beq @skip

	ldx #15
	jsr ckout

	ldy #0
	sty temp
:	ldy temp
	lda data+2,y
	jsr bsout
	inc temp
	lda temp
	cmp data+1
	bne :-

	jsr clrch

@skip:
	lda #0
	sta datalen
	ldx #15
	jsr chkin

:	lda $90
	bne :+
	jsr chrin
	jsr putbyte
	jmp :-

:	lda #0     ;terminate with 0
	jsr putbyte
	jsr clrch
	lda #15
	jsr close
	jsr senddata
	jmp main

;---------------------------------------
;set access type (ram, rom, reu)

setram:
	lda data+1
	sta access
	jmp main

;---------------------------------------

fill:
	lda data+1
	ldx data+2
	ldy data+3
	stx ptr
	sty ptr+1
	ldy #0
:	sta (ptr),y
	inc ptr
	bne :+
	inc ptr+1
:	ldx ptr
	cpx data+4
	bne :--
	ldx ptr+1
	cpx data+5
	bne :--
	jmp main
;---------------------------------------
poke:
	ldx data+2
	ldy data+3
	stx ptr
	sty ptr+1
	ldy #0
:	lda data+4,y
	sta (ptr),y
	iny
	cpy data+1
	bne :-
	jmp main
;---------------------------------------
telldev:
	lda dev
	sta data
	lda #2
	sta datalen
	jsr senddata
	jmp main

;---------------------------------------
setdev:
	lda data+1
	sta dev
	jmp main
;---------------------------------------
;open write file and reply ok or fail
;1:  fnlen
;2.. string
write:
	ldx dev
	lda #1
	tay
	jsr $ffba
	lda data+1
	ldx #<(data+2)
	ldy #>(data+2)
	jsr $ffbd
	jsr $ffc0
	lda $90
	bne :+

	jsr sendok
	jmp main

:	jsr sendfail
	jmp main
;---------------------------------------
;write data to writefile
;1:  number of bytes to write        
;2.. data
;reply ok or fail

xwrite:
	ldx #1
	jsr $ffc9
	bcc :+

	jsr sendfail
	jmp main

:	ldx #0
:	lda data+2,x
	jsr $ffd2
	inx
	cpx data+1
	bne :-

	jsr $ffcc

	jsr sendok
	jmp main
;---------------------------------------
;close write file, reply ok or fail
xdone:
	jsr $ffcc
	lda #1
	jsr $ffc3
	jmp main
;---------------------------------------
;reply ok or fail
sendfail:
	jsr $ffcc   ;close file
	lda #1
	jsr $ffc3
	lda #$ff    ;and send "fail"
	.byte $2c
sendok:	
	lda #0      ;send "ok"
	sta data
	lda #2
	sta datalen
	jmp senddata

;---------------------------------------
;read from current device and send data
read:
	lda #0
	sta datalen

	;open file

	lda #8
	tay
	ldx dev
	jsr $ffba
	lda data+1
	ldx #<(data+2)
	ldy #>(data+2)
	jsr $ffbd
	jsr $ffc0
	ldx #8
	jsr $ffc6

	;send data

:	lda $90
	bne :+
	jsr $ffcf
	jsr putbyte
	jmp :-

	;close file

:	jsr $ffcc
	lda #8
	jsr $ffc3
	jsr senddata
	jmp main
;---------------------------------------
;send directory of current dev
dir:
	lda #0
	sta datalen

	lda data+1	;length of filename
	sta $b7
	lda #<(data+2)
	sta $bb
	lda #>(data+2)
	sta $bc

	lda dev
	sta $ba
	lda #$60	;sec addr.
	sta $b9
	jsr $f3d5
	lda $ba
	jsr $ffb4  ;dev: talk
	lda #$b9
	jsr $ff96

	lda #0
	sta $90
	ldy #3

@loop:	sty ptr
	jsr $ffa5
	sta ptr+1
	ldy $90
	bne @close
	jsr $ffa5
	ldy $90
	bne @close
	ldy ptr
	dey
	bne @loop

	ldy ptr+1
	jsr $b391
	jsr $bddd

	ldx #0
	stx temp
:	ldx temp
	lda $0100,x
	beq :+
	jsr putbyte
	inc temp
	jmp :-

:	lda #$20
	jsr putbyte

:	jsr $ffa5
	ldx $90
	bne @close
	tax
	beq @cr
	jsr putbyte
	jmp :-

@cr:	lda #13
	jsr putbyte
	lda #10
	jsr putbyte
;        jsr senddata

	ldy #2
	bne @loop

@close:
        lda #0
        jsr putbyte

	jsr $f642
	jsr senddata
	jmp main
;---------------------------------------
;put a byte to output buffer and
;send over rr-net if buffer is full
;(max. 128 bytes)

putbyte:
	ldx datalen
	sta data,x
;	sta $0428,x
	inc datalen
	bpl :+
	jsr senddata
	lda #0
	sta datalen
:	rts

;---------------------------------------
;transfer memory peek to rr-net
;1:   length
;2-5: address (might be reu)
;6:   data
get:
	lda data+1
	sta datalen

	dec $d020

	bit access
	bmi @getreu

	sei
	lda 1
	pha
	lda access
	sta $01

	lda data+3
	sta ptr
	lda data+4
	sta ptr+1

	ldy #0
:	lda (ptr),y
	sta data,y
	iny
	cpy datalen
	bne :-

	pla
	sta $01
	cli

@out:
	jsr senddata
	inc $d020
	jmp main

	;get data from reu

@getreu:
	lda #<data
	ldx #>data
	jsr setupreu
	lda #253      ;fetch
	sta $df01
	bne @out

;---------------------------------------
;setup reu registers for data at .a/.x
;and reu address at data offset 3-5 and
;length in data offset 1

setupreu:
	sta $df02
	stx $df03
	lda data+3
	sta $df04
	lda data+4
	sta $df05
	lda data+5
	sta $df06
	lda data+1
	sta $df07
	lda #0
	sta $df08
	rts

;---------------------------------------
;send ip-header,udp-header and data
;--> send one frame
;number of data bytes to send must be in
;datalen

senddata:

	;make length even
;	lda datalen
;	lsr a
;	bcc :+
;	inc datalen

	;send headers

	lda datalen
	beq nodata
	;jsr send_ip_udp
	jsr send_byte

	;send data

	ldy #0
:	lda data,y
        jsr send_byte
;	ldx data+1,y
;	jsr send_word
;	iny
	iny
	cpy datalen
	bne :-
nodata:
	rts
;---------------------------------------
;read data and store in ram
;1:   len
;3-5: address (might be reu!)
;6    data
put:
	dec $d020
	bit access
	bpl @putram    ;to c64 ram

	;put to reu

	lda #<(data+6)
	ldx #>(data+6)
	jsr setupreu
	lda #252      ;stash
	sta $df01
	jmp @out

	;load to ram

@putram:
	lda data+3
	sta ptr
	lda data+4
	sta ptr+1
	ldy #0
:	lda data+6,y
	sta (ptr),y
	iny
	cpy data+1
	bne :-

@out:
	jsr sendok
	inc $d020
	jmp main
;---------------------------------------
basic:
	ldx stack
	txs
	ldx #$80
	jmp ($0300)

;---------------------------------------
initialize:
	jmp init

;---------------------------------------
status:

inc $d021

	lda access
	sta data
	lda dev
	sta data+1
	lda #2
	sta datalen
	jsr senddata
	jmp main
;---------------------------------------
;perfrom "run"
run:
	lda #$37
	sta 1
	lda $ae
	sta $2d
	lda $af
	sta $2e
	jsr $a533
	jsr $a659
	jsr $a68e
	ldx #$f6
	txs
	jmp $a7ae

;---------------------------------------
;call:
;1-2: address
;3:   akku
;4:   x-reg
;5:   yreg
;6:   status reg

call:
	lda #>(aftercall-1) ;return to main
	pha            ;after call...
	lda #<(aftercall-1)
	pha
	lda data+6   ;sr
	pha
	lda data+3   ;.a
	ldx data+4   ;.x
	ldy data+5   ;.y
	plp          ;set sr
jump:
	jmp (data+1)

aftercall:
	php
	sta data
	stx data+1
	sty data+2
	pla
	sta data+3
	lda #4
	sta datalen
	jsr senddata
	jmp main

;---------------------------------------
;just for debugging...
prhex:
	pha
	lsr
	lsr
	lsr
	lsr
	jsr :+
	pla
	and #$0f
:	clc
	adc #$30
	cmp #$3a
	bcc :+
	adc #6
:	jmp $ffd2
;---------------------------------------
;.rodata
;---------------------------------------
.rodata
mssg:		.byte $93,"chserv v0.3",13,0
dollar:		.byte '$'
cmdtab:	
		.addr initialize-1
		.addr put-1
		.addr get-1
		.addr read-1
		.addr write-1
		.addr xwrite-1
		.addr xdone-1
		.addr run-1
		.addr jump-1
		.addr call-1
		.addr basic-1
		.addr dir-1
		.addr diskcommand-1
		.addr setram-1
		.addr poke-1
		.addr telldev-1
		.addr setdev-1
		.addr fill-1
		.addr status-1
;---------------------------------------
;.bss
;---------------------------------------
.bss
stack:		.res 1
datalen:	.res 1
temp:		.res 1
dev:		.res 1
access:		.res 1
data:		.res BUF_SIZE
;---------------------------------------
