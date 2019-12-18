#ifndef _JOYPAD_INPUT_H_
#define _JOYPAD_INPUT_H_

#include "gamedev.h"

#define JP_ARROW_TYPE                   0x03
// 上
#define JP_UP_PRESS_VALUE1              0xffffffff
#define JP_UP_PRESS_VALUE2              0x00
// 下
#define JP_DOWN_PRESS_VALUE1            0x01
#define JP_DOWN_PRESS_VALUE2            0xff
// 左
#define JP_LEFT_PRESS_VALUE1            0xffffffff
#define JP_LEFT_PRESS_VALUE2            0x00
// 右
#define JP_RIGHT_PRESS_VALUE1           0x01
#define JP_RIGHT_PRESS_VALUE2           0xff

// 模式 UD LR CODE
#define JP_ARROW_UD_CODE1               0x11
#define JP_ARROW_UD_CODE2               0x01
#define JP_ARROW_LR_CODE1               0x10
#define JP_ARROW_LR_CODE2               0x00

// 松开
#define JP_ARROW_RELEASE_VALUE1         0x00
#define JP_ARROW_RELEASE_VALUE2         0x7f

// 选择
#define JP_FUNCTION_TYPE                0x01

#define JP_SELECT_CODE                  0x128
#define JP_SELECT_PRESS_VALUE           0x01
#define JP_SELECT_RELEASE_VALUE         0x00

// 开始
#define JP_START_CODE                   0x129
#define JP_START_PRESS_VALUE            0x01
#define JP_START_RELEASE_VALUE          0x00

// A
#define JP_A_CODE                       0x120
#define JP_A_PRESS_VALUE                0x01
#define JP_A_RELEASE_VALUE              0x00

// B
#define JP_B_CODE                       0x121
#define JP_B_PRESS_VALUE                0x01
#define JP_B_RELEASE_VALUE              0x00

// X
#define JP_X_CODE                       0x122
#define JP_X_PRESS_VALUE                0x01
#define JP_X_RELEASE_VALUE              0x00

// Y
#define JP_Y_CODE                       0x123
#define JP_Y_PRESS_VALUE                0x01
#define JP_Y_RELEASE_VALUE              0x00


int joypad_get_event(gamedev_t* gamedev);

#endif
