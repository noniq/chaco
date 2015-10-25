
.import get_byte

.importzp   ptr1, ptr2, ptr3, ptr4
.importzp   tmp1, tmp2, tmp3, tmp4
.import     popa, popax


size_zp      = ptr1
xfer_size_zp = ptr2
p_buff_zp    = ptr3
m_size_hi    = tmp1

; =============================================================================
;
; uint16_t __fastcall__ ef3usb_receive_data(void* buffer, uint16_t size);
;
; Reads exactly size bytes from USB to buffer. Do not return before the
; all data has been received. Return size.
;
; =============================================================================
.proc   _ef3usb_receive_data
.export _ef3usb_receive_data
_ef3usb_receive_data:
        sta size_zp
        sta xfer_size_zp
        stx size_zp + 1         ; Save size
        stx xfer_size_zp + 1

        jsr popax
        sta p_buff_zp
        stx p_buff_zp + 1       ; Save buffer address

        lda xfer_size_zp
        ldy xfer_size_zp + 1

        ; here: A = xfer_size low, Y = xfer_size high
        eor #$ff
        tax
        tya
        eor #$ff
        sta m_size_hi           ; calc -size - 1
        ldy #0
        jmp @incCounter         ; inc to get -size
@getBytes:
        ; xy contains number of bytes to be xfered (x = low byte)
;        wait_usb_rx_ok
;        lda USB_DATA
        jsr get_byte
        sta (p_buff_zp), y
        iny
        bne @incCounter
        inc p_buff_zp + 1
@incCounter:
        inx
        bne @getBytes
        inc m_size_hi
        bne @getBytes
end:
        lda xfer_size_zp
        ldx xfer_size_zp + 1            ; return number of bytes transfered
        rts
.endproc
