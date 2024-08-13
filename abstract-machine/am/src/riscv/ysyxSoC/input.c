#include <am.h>
#include <ysyxsoc.h>

static uint8_t scan2ascii[][2];

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint8_t scan_code = inb( (uintptr_t)(PS2_KEYBOARD) );

  kbd->keydown = 0;
  kbd->keycode = (int)scan2ascii[scan_code];
}

scan2ascii[][2] = {
{0x76,0x1b},
{0x05,0x70},
{0x06,0x71},
{0x04,0x72},
{0x0C,0x73},
{0x03,0x74},
{0x0B,0x75},
{0x83,0x76},
{0x0A,0x77},
{0x01,0x78},
{0x09,0x79},
{0x78,0x7a},
{0x07,0x7b},
{0x0E,0xc0},
{0x16,0x61},
{0x1E,0x62},
{0x26,0x63},
{0x25,0x64},
{0x2E,0x65},
{0x36,0x66},
{0x3D,0x67},
{0x3E,0x68},
{0x46,0x69},
{0x45,0x60},
{0x4E,0x6d},
{0x55,0xbb},
{0x66,0x8},
{0x0D,0x9},
{0x15,0x51},
{0x1D,0x57},
{0x24,0x45},
{0x2D,0x52},
{0x2C,0x54},
{0x35,0x59},
{0x3C,0x55},
{0x43,0x49},
{0x44,0x4f},
{0x4D,0x5a},
{0x54,0xdb},
{0x5B,0xdd},
{0x5D,0xdc},
{0x58,0x14},
{0x1C,0x41},
{0x1B,0x53},
{0x23,0x44},
{0x2B,0x46},
{0x34,0x47},
{0x33,0x48},
{0x3B,0x4a},
{0x42,0x4b},
{0x4B,0x4c},
{0x4C,0xba},
{0x52,0xb6},
{0x5A,0xd},
{0x12,0x10},
{0x1A,0x5a},
{0x22,0x58},
{0x21,0x43},
{0x2A,0x56},
{0x32,0x42},
{0x31,0x4e},
{0x3A,0x4d},
{0x41,0xbc},
{0x49,0xbe},
{0x4A,0xbf},
{0x59,0x10},
{0x14,0x11},
{0x11,0x12},
{0x29,0x20},
};


