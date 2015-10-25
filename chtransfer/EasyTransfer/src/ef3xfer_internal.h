/*
 * ef3xfer_internal.h
 *
 *  Created on: 02.02.2012
 *      Author: skoe
 */

#ifndef EF3XFER_INTERNAL_H_
#define EF3XFER_INTERNAL_H_

void ef3xfer_log_printf(const char* p_str_format, ...);
void ef3xfer_log_progress(int percent, int b_gui_only);

void ef3xfer_disconnect_ftdi(void);

int ef3xfer_do_handshake(const char* p_str_type);

int ef3xfer_read_from_ftdi(void* p_buffer, int size);

int ef3xfer_write_to_ftdi(const void* p_buffer, int size);

int usb_write_block(unsigned char *buf, int len);


#endif /* EF3XFER_INTERNAL_H_ */
