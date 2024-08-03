#include <am.h>
#include <ysyxsoc.h>

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
 // char key = inb( (uintptr_t)(PS2_KEYBOARD) );
  kbd->keydown = 0;
  kbd->keycode = AM_KEY_NONE;
}


