#ifndef _APPLICATION_H
#define _APPLICATION_H

#ifndef VERSION
#define VERSION "vdev"
#endif

#ifndef LED_STRIP_COUNT
#define LED_STRIP_COUNT 144
#endif

#ifndef LED_STRIP_TYPE
#define LED_STRIP_TYPE 4
#endif

#ifndef LED_STRIP_SWAP_RG
#define LED_STRIP_SWAP_RG 0
#endif

#include <bcl.h>

char *colors[] = {"modra", "cervena", "zluta", "zelena", "ruzova", "bila", "cerna", "oranzova", "fialova", "hneda"};
uint32_t colorsRGBA[] = {0x0000ff00, 0xff000000, 0xffff0000, 0x00ff0000, 0xff149300, 0xffffff00, 0x00000000, 0xff450000, 0x80008000, 0x8B451300};

typedef struct
{
    uint8_t channel;
    float value;
    bc_tick_t next_pub;

} event_param_t;

#endif // _APPLICATION_H