#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/input.h>
#include <cstring>
#include <dirent.h>
#include <errno.h>

#include "../InfoNES.h"
#include "usb_dev.h"
#include "joypad.h"


int joypad_get_event(gamedev_t* gamedev)
{
    /**
         * FC手柄 bit 键位对应关系 真实手柄中有一个定时器，处理 连A  连B
         * 0  1   2       3       4    5      6     7
         * A  B   Select  Start  Up   Down   Left  Right
         */
    //因为 USB 手柄每次只能读到一位键值 所以要有静态变量保存上一次的值
    // static unsigned char gamedev_event = 0;
    struct gamedev_event e;
    unsigned short type, code;
    signed int value;
    if(0 < read (gamedev->gamedev_fd, &e, sizeof(e)))
    {
        type = e.type;
        code = e.code;
        value = e.value;
        // printf("-----------------------------------------------------------------------------------\n");
        // printf("%s: type = 0x%x -- code = 0x%x, value= 0x%x \n", gamedev->dev, type, code, value);
        // printf("-----------------------------------------------------------------------------------\n\n\n");
        if(JP_ARROW_TYPE == type)
        {
            if ((JP_ARROW_UD_CODE1 == code && JP_UP_PRESS_VALUE1 == value) || (JP_ARROW_UD_CODE2 == code && JP_UP_PRESS_VALUE2 == value)) {
                gamedev->gamedev_event |= 1<<4;
            }

            if ((JP_ARROW_UD_CODE1 == code && JP_DOWN_PRESS_VALUE1 == value) || (JP_ARROW_UD_CODE2 == code && JP_DOWN_PRESS_VALUE2 == value)) {
                gamedev->gamedev_event |= 1<<5;
            }

            if ((JP_ARROW_UD_CODE1 == code && JP_ARROW_RELEASE_VALUE1 == value) || (JP_ARROW_UD_CODE2 == code && JP_ARROW_RELEASE_VALUE2 == value))
            {
                gamedev->gamedev_event &= ~(1<<4 | 1<<5);
            }

            if ((JP_ARROW_LR_CODE1 == code && JP_LEFT_PRESS_VALUE1 == value) || (JP_ARROW_LR_CODE2 == code && JP_LEFT_PRESS_VALUE2 == value)) {
                gamedev->gamedev_event |= 1<<6;
            }

            if ((JP_ARROW_LR_CODE1 == code && JP_RIGHT_PRESS_VALUE1 == value) || (JP_ARROW_LR_CODE2 == code && JP_RIGHT_PRESS_VALUE2 == value)) {
                gamedev->gamedev_event |= 1<<7;
            }

            if ((JP_ARROW_LR_CODE1 == code && JP_ARROW_RELEASE_VALUE1 == value) || (JP_ARROW_LR_CODE2 == code && JP_ARROW_RELEASE_VALUE2 == value)) {
                gamedev->gamedev_event &= ~(1<<6 | 1<<7);
            }
        }

        if(JP_FUNCTION_TYPE == type)
        {
            if(JP_SELECT_CODE == code && (JP_SELECT_PRESS_VALUE == value)) {
                gamedev->gamedev_event |= 1<<2;
            }

            if(JP_SELECT_CODE == code && (JP_SELECT_RELEASE_VALUE == value)) {
                gamedev->gamedev_event &= ~(1<<2);
            }

            if(JP_START_CODE == code && (JP_START_PRESS_VALUE == value)) {
                gamedev->gamedev_event |= 1<<3;
            }
            
            if(JP_START_CODE == code && (JP_START_RELEASE_VALUE == value)) {
                gamedev->gamedev_event &= ~(1<<3);
            }


            if(JP_A_CODE == code && JP_A_PRESS_VALUE == value) {
                gamedev->gamedev_event |= 1<<0;
            }
            
            if(JP_A_CODE == code && JP_A_RELEASE_VALUE == value) {
                gamedev->gamedev_event &= ~(1<<0);
            }

            if(JP_B_CODE == code && JP_B_PRESS_VALUE == value) {
                gamedev->gamedev_event |= 1<<1;
            }
            
            if(JP_B_CODE == code && JP_B_RELEASE_VALUE == value) {
                gamedev->gamedev_event &= ~(1<<1);
            }

            if(JP_X_CODE == code && JP_X_PRESS_VALUE == value) {
                gamedev->gamedev_event |= 1<<0;
            }

            if(JP_X_CODE == code && JP_X_RELEASE_VALUE == value) {
                gamedev->gamedev_event &= ~(1<<0);
            }

            if(JP_Y_CODE == code && JP_Y_PRESS_VALUE == value) {
                gamedev->gamedev_event |= 1<<1;
            }
            if(JP_Y_CODE == code && JP_Y_RELEASE_VALUE == value) {
                gamedev->gamedev_event &= ~(1<<1);
            }
        }
        return gamedev->gamedev_event;

    } else {
        printf("%s:%d %s()| gamedev fd = %d\n", __FILE__, __LINE__, __FUNCTION__, gamedev->gamedev_fd);
        printf("%s:%d %s()| read (%d, %p, %d)...errno = %d\n", __FILE__, __LINE__, __FUNCTION__, gamedev->gamedev_fd, &e, sizeof(e), errno);
    }

    return -1;
}






