/*
 * GCR buffer contain all physical half tracks, including 0.0 / 0.5 which might
 * not be accessable.
 */
#define GCRMAXTRACKLEN 0x2000
#define GCRMAXHALFTRACKS   (84+2)

extern unsigned char gcrbuffer[GCRMAXHALFTRACKS][GCRMAXTRACKLEN];
extern unsigned char gcrspdbuffer[GCRMAXHALFTRACKS][4];
extern unsigned int gcrlenbuffer[GCRMAXHALFTRACKS];

