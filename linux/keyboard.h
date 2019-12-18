#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "usb_dev.h"
#include "gamedev.h"

#define KEYBOARD_VALUE_PRESS    0x01
#define KEYBOARD_VALUE_RELEASE  0x00
#define KEYBOARD_LEFT 30
#define KEYBOARD_DOWN 31
#define KEYBOARD_RIGHT 32
#define KEYBOARD_UP 17

#define KEYBOARD_A 22
#define KEYBOARD_B 23
#define KEYBOARD_C 36
#define KEYBOARD_D 37

int keyboard_get_event(gamedev_t* gamedev);

#endif

