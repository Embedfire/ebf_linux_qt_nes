#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usb_dev.h"

static usb_dev_t _usb_devices[DEVICES_NUM];

int usb_dev_search(void)
{
    int num = 0;
    int usb_dev_num = 0;
    char *start;
    char buffer[MAX_LENTH];
    
    memset(_usb_devices,0,sizeof(_usb_devices));

    FILE *pf = fopen(USB_DEV_PATH,"r");
    if (pf == NULL) {
        printf("%s file open file\n", USB_DEV_PATH);
    }

    while ((fgets(buffer, MAX_LENTH, pf) != NULL)) {
        if ((start = strstr(buffer, "Name=")) != NULL) {
            if(strlen(_usb_devices[usb_dev_num].name) == 0) {
                memcpy(_usb_devices[usb_dev_num].name, start+5, NAME_LEN-1);
                _usb_devices[usb_dev_num].handler[NAME_LEN-1] = '\0';
                printf("_usb_devices[%d].name = %s", usb_dev_num, _usb_devices[usb_dev_num].name);
            }
        }

        if ((start = strstr(buffer, "Handlers=")) != NULL) {
            if ((start = strstr(buffer, "event")) != NULL) {
                if(strlen(_usb_devices[usb_dev_num].handler) == 0) {
                    memcpy(_usb_devices[usb_dev_num].handler, start, HANDLER_LEN-1);
                    _usb_devices[usb_dev_num].handler[HANDLER_LEN-1] = '\0';
                    printf("_usb_devices[%d].handler = %s\n", usb_dev_num, _usb_devices[usb_dev_num].handler);
                }
            }
        }
        if((strlen(_usb_devices[usb_dev_num].name) != 0) && (strlen(_usb_devices[usb_dev_num].handler) != 0)) {
            usb_dev_num++;
            if (usb_dev_num > DEVICES_NUM) {
                printf("usb_dev_num > DEVICES_NUM");
                goto ret;
            }
        }
        num++;
    }

ret:
    printf("close %s\n", USB_DEV_PATH);
    fclose(pf);
    return num;
}

int usb_dev_parsing(char *str, usb_dev_handler_t *handler, int num)
{
    int i, h = num;
    for (i = num; i < DEVICES_NUM; i++) {
        printf("%s", _usb_devices[i].name);
        if (strstr(_usb_devices[i].name, str) != NULL) {
            memcpy(handler[h].handler, _usb_devices[i].handler,sizeof(_usb_devices[i].handler));
            printf("handler[%d].handler = %s, len: %ld\n", h, handler[h].handler, sizeof(_usb_devices[i].handler));
            h++;
            if(h >= DEVICES_NUM)
                return h;
        }
    }
    return h;
}

