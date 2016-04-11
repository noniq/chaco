
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gcr.h"

static unsigned char g64hdr[0x0c];
static unsigned int maxtracklen;
static unsigned int maxtracks;

int loadg64(char *name)
{
    FILE *out;
    unsigned int t = 0;
    unsigned int offset, tracklen;

    out = fopen(name, "rb");

    if (fread(g64hdr, 1, 0x0c, out) != 0x0c) {
            goto end;
    }
    maxtracks = g64hdr[9];
    maxtracklen = g64hdr[10];
    maxtracklen += (g64hdr[11] << 8);

    memset(gcrbuffer, 0, 0x2000 * (42*2));

    for (t = 0; t < maxtracks; t++) {
        fseek(out, 0x0c + (t * 4), SEEK_SET);
        offset = fgetc(out);
        offset += ((unsigned int)fgetc(out)) << 8;
        offset += ((unsigned int)fgetc(out)) << 16;
        offset += ((unsigned int)fgetc(out)) << 24;

        if (offset) {
            fseek(out, offset, SEEK_SET);
            tracklen = fgetc(out);
            tracklen += ((unsigned int)fgetc(out)) << 8;
            if (fread(&gcrbuffer[t + 2][0], 1, tracklen, out) != tracklen) {
                    goto end;
            }
            /* put lenght of gcr track into last two bytes, MSB first */
            gcrbuffer[t + 2][0x1ffe] = (tracklen >> 8) & 0xff;
            gcrbuffer[t + 2][0x1fff] = tracklen & 0xff;
        }
    }

end:
    fclose(out);
    return t;
}


