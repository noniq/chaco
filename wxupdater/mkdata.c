
// this is a hack. do not look further if you like your sanity

#include <stdio.h>
#include <stdlib.h>

FILE *in1, *in2, *in3, *out;

int outhex(FILE *in, FILE *out)
{
    int c, n = 0;
    while(!feof(in)) {
        c = fgetc(in);
        if(feof(in)) break;
        fprintf(out, "0x%02x,", c);
        n++;
        if (n % 16 == 0) {
            fprintf(out, "\n");
        }
    }
}

int main(int argc, char *argv[])
{
    in1 = fopen(argv[1], "rb");
    in2 = fopen(argv[2], "rb");
    in3 = fopen(argv[3], "rb");
    out = fopen(argv[4], "wb");

    printf("converting core data from '%s'\n", argv[1]);
    fprintf(out, "unsigned char coredata[] = {\n");
    outhex(in1, out);
    fprintf(out, "};\n");

    printf("converting core data from '%s'\n", argv[2]);
    fprintf(out, "unsigned char coredatav2[] = {\n");
    outhex(in2, out);
    fprintf(out, "};\n");

    printf("converting rom data from '%s'\n", argv[3]);
    fprintf(out, "unsigned char romdata[] = {\n");
    outhex(in3, out);
    fprintf(out, "};\n");

    fclose(in1);
    fclose(in2);
    fclose(in3);
    fclose(out);

    return 0;
}
