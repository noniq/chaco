/*
 *
 * (c) 2013 Thomas Giesel
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
 * Thomas Giesel skoe@directbox.com
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "chacolib.h"
#define C64_RAM_SIZE 0x10000
extern int shutdown(int n);

#include "ef3xfer.h"
#include "ef3xfer_internal.h"
#include "str_to_key.h"

void execute_run(void)
{
    unsigned char buf[4];
    /* put RUN into keyboard buffer */
    buf[0] = 0;
    if (chameleon_writememory(buf, 1, 198) < 0) {
        ef3xfer_log_printf("error writing to chameleon memory.\n");
        exit(shutdown(-1));
    }
    buf[0] = 82;
    buf[1] = 85;
    buf[2] = 78;
    buf[3] = 13;
    if (chameleon_writememory(buf, 4, 631) < 0) {
        ef3xfer_log_printf("error writing to chameleon memory.\n");
        exit(shutdown(-1));
    }
    buf[0] = 4;
    if (chameleon_writememory(buf, 1, 198) < 0) {
        ef3xfer_log_printf("error writing to chameleon memory.\n");
        exit(shutdown(-1));
    }
}

/*****************************************************************************/
int ef3xfer_transfer_prg(const char* p_filename)
{
    FILE *f;
    size_t addr, len;
    unsigned char *buffer;

    ef3xfer_log_printf("Send PRG\n");

    /* write .prg file to memory + RUN */
    if ((f = fopen(p_filename, "rb")) == NULL) {
        ef3xfer_log_printf("error opening: '%s'\n", p_filename);
        exit(shutdown(-1));
    }
    addr = fgetc(f);
    addr += ((int)fgetc(f) << 8);

    buffer = malloc(C64_RAM_SIZE);
    len = fread(buffer, 1, C64_RAM_SIZE - addr, f);
    fclose(f);
    ef3xfer_log_printf("sending '%s' ($%04x bytes to $%04x.)...\n", p_filename, len, addr);
    if (chameleon_writememory(buffer, len, addr) < 0) {
        ef3xfer_log_printf("error writing to chameleon memory.\n");
        free(buffer);
        return 0;
    }
    free(buffer);
    execute_run();
    return 1;
}


/*****************************************************************************/
int ef3xfer_transfer_prg_mem(const unsigned char* p_prg, int len)
{
    size_t addr;

    ef3xfer_log_printf("Send PRG\n");

    addr = p_prg[0];
    addr += ((int)p_prg[1] << 8);

    ef3xfer_log_printf("sending $%04x bytes to $%04x...\n", len, addr);
    if (chameleon_writememory((unsigned char*)&p_prg[2], len, addr) < 0) {
        ef3xfer_log_printf("error writing to chameleon memory.\n");
        return 0;
    }
    execute_run();
    return 1;
}

