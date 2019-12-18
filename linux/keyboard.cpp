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
#include "keyboard.h"

int keyboard_get_event(gamedev_t* gamedev)
{
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
        
        if (KEYBOARD_VALUE_PRESS == value) {
            if (KEYBOARD_UP == code) {
                gamedev->gamedev_event |= 1 << 4;
            }

            if (KEYBOARD_DOWN == code) {
                gamedev->gamedev_event |= 1 << 5;
            }

            if (KEYBOARD_LEFT == code) {
                gamedev->gamedev_event |= 1 << 6;
            }

            if (KEYBOARD_RIGHT == code) {
                gamedev->gamedev_event |= 1 << 7;
            }

            if (KEYBOARD_A == code) {
                gamedev->gamedev_event |= 1 << 2;
            }

            if (KEYBOARD_B == code) {
                gamedev->gamedev_event |= 1 << 3;
            }

            if (KEYBOARD_C == code) {
                gamedev->gamedev_event |= 1 << 0;
            }

            if (KEYBOARD_D == code) {
                gamedev->gamedev_event |= 1 << 1;
            }
        } else if (KEYBOARD_VALUE_RELEASE == value) {
            if (KEYBOARD_UP == code) {  
                gamedev->gamedev_event &= ~(1 << 4);
            }

            if (KEYBOARD_DOWN == code) {
                gamedev->gamedev_event &= ~(1 << 5);
            }

            if (KEYBOARD_LEFT == code) {
                gamedev->gamedev_event &= ~(1 << 6);
            }

            if (KEYBOARD_RIGHT == code) {
                gamedev->gamedev_event &= ~(1 << 7);
            }

            if (KEYBOARD_C == code) {
                gamedev->gamedev_event &= ~(1 << 0);
            }

            if (KEYBOARD_D == code) {
                gamedev->gamedev_event &= ~(1 << 1);
            }

            if (KEYBOARD_A == code) {
                gamedev->gamedev_event &= ~(1 << 2);
            }

            if (KEYBOARD_B == code) {
                gamedev->gamedev_event &= ~(1 << 3);
            }
        }

        return gamedev->gamedev_event;

    } else {
        printf("%s:%d %s()| gamedev fd = %d\n", __FILE__, __LINE__, __FUNCTION__, gamedev->gamedev_fd);
        printf("%s:%d %s()| read (%d, %p, %d)...errno = %d\n", __FILE__, __LINE__, __FUNCTION__, gamedev->gamedev_fd, &e, sizeof(e), errno);
    }

    return -1;
}
