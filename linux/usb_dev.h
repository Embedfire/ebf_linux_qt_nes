#ifndef _USB_DEV_H_
#define _USB_DEV_H_

#define DEVICES_NUM 10
#define MAX_LENTH 256
#define NAME_LEN 40
#define HANDLER_LEN 7

#define USB_DEV_PATH "/proc/bus/input/devices"

typedef struct usb_dev {
    char name[NAME_LEN];
    char handler[HANDLER_LEN];
} usb_dev_t;

typedef struct usb_dev_handler {
    char handler[HANDLER_LEN];
} usb_dev_handler_t;


int usb_dev_search(void);
int usb_dev_parsing(char *str, usb_dev_handler_t *handler, int num, int max);


#endif
