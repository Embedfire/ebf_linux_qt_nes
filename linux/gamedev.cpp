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
#include "gamedev.h"

static int _gamedev_dev_init(gamedev_t* gamedev);
static void _gamedev_dev_destroy(gamedev_t* gamedev);
static void *_gamedev_process(void *dev);

static char _gamedev_dev_num = 0;
static gamedev_t* _gamedev_head;
static gamedev_path_t _gamedev[MAX_GAMEDEV];

static pthread_mutex_t _mutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  _convar = PTHREAD_COND_INITIALIZER;

static int _gamedev_dev_init(gamedev_t* gamedev)
{
    gamedev->gamedev_fd = open(gamedev->dev, O_RDONLY);
    if (-1 == gamedev->gamedev_fd) {
        printf("%s dev not found \r\n", gamedev->dev);
        return -1;
    }

    return 0;
}

static void _gamedev_dev_destroy(gamedev_t* gamedev)
{
    if (gamedev->gamedev_fd)
        close(gamedev->gamedev_fd);
}

static void *_gamedev_process(void *dev)
{
    int rc;
    gamedev_t* gamedev;
    gamedev = (gamedev_t*)dev;

    printf("%s:%d %s()\n", __FILE__, __LINE__, __FUNCTION__);

    while (1) {
        //因为有阻塞所以没有输入时是休眠
        if (-1 == (rc = gamedev->gamedev_get(gamedev)))
            gamedev->gamedev_destroy(gamedev);
        else
            *(gamedev->gamedevx) = (long unsigned int)rc;
        
        //有数据时唤醒
        pthread_mutex_lock(&_mutex);
        /*  唤醒主线程 */
        pthread_cond_signal(&_convar);
        pthread_mutex_unlock(&_mutex);
    }
}

char gamedev_find(char *dev)
{
    int i;
    usb_dev_handler_t handler[MAX_GAMEDEV];
    memset(handler, 0, sizeof(handler));

    usb_dev_search();
    usb_dev_parsing(dev, handler, _gamedev_dev_num, MAX_GAMEDEV);

    for (i = _gamedev_dev_num; i < MAX_GAMEDEV; i++) {
        if (strlen(handler[i].handler)) {
            _gamedev_dev_num ++;
            snprintf(_gamedev[i].gamedev_path, sizeof(_gamedev[i].gamedev_path), "/dev/input/%s", handler[i].handler);
            printf("find %d num gamedev_path,\n_gamedev[%d].gamedev_path:%s, len: %ld!\n", _gamedev_dev_num, i, _gamedev[i].gamedev_path, strlen(_gamedev[i].gamedev_path));
        }
    }

    return _gamedev_dev_num;
}


char *get_gamedev_path(int num)
{
    if (num >= _gamedev_dev_num)
        return NULL;
    
    return _gamedev[num].gamedev_path;
}


int gamedev_register(gamedev_t* gamedev)
{
    gamedev_t* tmp;

    if ((NULL == gamedev->gamedev_get) || (NULL == gamedev->gamedevx) || (NULL == gamedev->dev))
        return -1;

    if (NULL == gamedev->gamedev_init)
        gamedev->gamedev_init = _gamedev_dev_init;
    
    if (NULL == gamedev->gamedev_destroy)
        gamedev->gamedev_destroy = _gamedev_dev_destroy;
    
    if (gamedev->gamedev_init(gamedev) < 0)
        return -1;

    pthread_create(&gamedev->thread_id, NULL, _gamedev_process, (void*)gamedev);
    if(! _gamedev_head)
        _gamedev_head = gamedev;
    else {
        tmp = _gamedev_head;
        while(tmp->next) {
            tmp = tmp->next;
        }
        
        tmp->next = gamedev;
    }

    gamedev->next = NULL;
    return 0;
}

void gamedev_get_event(void)
{
    /* 休眠 */
    pthread_mutex_lock(&_mutex);
    pthread_cond_wait(&_convar, &_mutex);
    pthread_mutex_unlock(&_mutex);
}

