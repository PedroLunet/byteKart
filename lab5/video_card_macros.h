#ifndef _VIDEO_CARD_MACROS_
#define _VIDEO_CARD_MACROS_

#define INDEXED 0x105
#define DIRECT_640_480 0x110
#define DIRECT_800_600 0x115
#define DIRECT_1280_1024 0x11A
#define DIRECT 1152_864 0x14C

#define LINEAR_FRAME_BUF BIT(14) // Bit should be set, in order to set the linear frame buffer model
#define SET_VBE_MODE 0x02
#define VBE_CONTROLLER_INFO 0x00

#endif
