#ifndef _gamedev_INPUT_H_
#define _gamedev_INPUT_H_

extern DWORD	dwKeyPad1;
extern DWORD	dwKeyPad2;

#define MAX_GAMEDEV     2
#define DEV_PATH_LEN    20

typedef struct gamedev_path{
    char gamedev_path[DEV_PATH_LEN];
} gamedev_path_t;

typedef struct gamedev{
    int (*gamedev_get)(struct gamedev*);
    long unsigned int *gamedevx;
    int (*gamedev_init) (struct gamedev*);
    void (*gamedev_destroy) (struct gamedev*);
    char *dev;
    int gamedev_fd;
    char gamedev_event;
    struct gamedev *next;
    pthread_t thread_id;
} gamedev_t;

struct gamedev_event {		
    struct timeval  time;      /* event timestamp in milliseconds */
    __u16           type;     /* value */
    __u16           code;      /* event type */
    __s32           value;    /* axis/button number */
};

char gamedev_find(char *dev);
char *get_gamedev_path(int num);
void gamedev_get_event(void);
int gamedev_register(gamedev_t* gamedev);

#endif
