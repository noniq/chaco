/*
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 */

#include <conio.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <ef3usb.h>

unsigned char scrollbuffer[41];
unsigned char a_data_buffer[0x10];
unsigned int n = 0;

int main(void)
{

    clrscr();
    gotoxy(0,7);chline(40);gotoxy(0,0);
    cputs("USB Loopback started");
    memset(scrollbuffer,' ', 40);

    for (;;) {
        ef3usb_receive_data(a_data_buffer, 1);
        n++;
        scrollbuffer[40]=a_data_buffer[0];
        memcpy(scrollbuffer,&scrollbuffer[1],40);
        memcpy((char*)0x450,scrollbuffer,40);
        gotoxy(0,3);cprintf("%04x:%02x", n, a_data_buffer[0]);
        ef3usb_send_data(a_data_buffer, 1);
        if(n == (100 * (8 + 8))) {
            gotoxy(0,5);cputs("done.");
            gotoxy(0,10);
            break;
        }
    }
    asm("jmp $fce2");
    return 0;
}
