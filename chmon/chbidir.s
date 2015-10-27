
.export send_byte
.export get_byte
.export check_data_available

datasize = 256
bufsize = datasize+4

usbrecbuf = $07e8-(2*bufsize)
usbsndbuf = $07e8-(bufsize)

USBReadData         = usbrecbuf + 0
USBReadSequence     = usbrecbuf + datasize + 0
USBReadBytesLeft    = usbrecbuf + datasize + 1
USBReadStart        = usbrecbuf + datasize + 2

USBWriteData        = usbsndbuf + 0
USBWriteSequence    = usbsndbuf + datasize + 0
USBWriteBytesLeft   = usbsndbuf + datasize + 1
USBWriteStart       = usbsndbuf + datasize + 2

;-------------------------------------------------------------------------------
init_usb:
                lda #0
                sta USBReadStart
                sta USBReadData
                sta USBReadSequence
                sta USBReadBytesLeft
                sta USBWriteStart
                sta USBWriteData
                sta USBWriteSequence
                sta USBWriteBytesLeft
                rts

;-------------------------------------------------------------------------------
; checks if the pc wants to send something
check_data_available:
                clc
                lda USBReadBytesLeft
                adc #$ff ; set carry if A!=0
                lda USBReadBytesLeft
                rts

;-------------------------------------------------------------------------------
; gets one byte from usb
get_byte:
                stx xs1+1       ; save X

                ; get current sequence number
                ldx USBReadSequence

                ; signal PC to start sending
                lda #$ff
                sta USBReadStart
waitbyte:
                ; wait until byte has arrived
                cpx USBReadSequence
                beq waitbyte

xs1:            ldx #0  ; restore X
                ; get the byte
                lda USBReadData
                ; flags must be set according to read byte
                rts

;-------------------------------------------------------------------------------
                .export _get_block
_get_block:
get_block:
                stx xs3+1       ; save X

                ; get current sequence number
                ldx USBReadSequence

                ; signal PC to start sending
                lda #$ff
                sta USBReadStart
waitblock:
                ; wait until block has arrived
                cpx USBReadSequence
                beq waitblock

xs3:            ldx #0          ; restore X
                rts
;-------------------------------------------------------------------------------

send_byte:
                stx xs2+1
                pha

                ; get current sequence number
;                ldx USBWriteSequence

                sta USBWriteData        ; send back the byte

;                lda #1
;                sta USBWriteBytesLeft

                ; signal PC to start recieving
                lda #$ff
                sta USBWriteStart
waitbyte2:
                lda USBWriteStart
                bne waitbyte2

;                dec USBWriteBytesLeft

xs2:            ldx #0
                pla
                rts

;-------------------------------------------------------------------------------

send_block:
                stx xs4+1
                pha

                ; get current sequence number
;                ldx USBWriteSequence
;                lda #1                  ; FIXME!
;                sta USBWriteBytesLeft

                ; signal PC to start recieving
                lda #$ff
                sta USBWriteStart
waitblock2:
                lda USBWriteStart
                bne waitblock2

;                lda #0
;                sta USBWriteBytesLeft

xs4:            ldx #0
                pla
                rts

