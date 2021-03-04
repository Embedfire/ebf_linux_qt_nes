/*===================================================================*/
/*                                                                   */
/*  InfoNES_System_Linux.cpp : Linux specific File                   */
/*                                                                   */
/*  2001/05/18  InfoNES Project ( Sound is based on DarcNES )        */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "alsa/asoundlib.h"

#include "../InfoNES.h"
#include "../InfoNES_System.h"
#include "../InfoNES_pAPU.h"

//bool define
#define TRUE 1
#define FALSE 0

/* lcd 操作相关 头文件 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <termios.h>

#include <fcntl.h>

#include "joypad.h"
#include "keyboard.h"
#include "gamedev.h"

static gamedev_t gamedev1;
static gamedev_t gamedev2;

char joypad_name[] = "USB Gamepad";
char keyboard_name[] = "\"USB USB Keyboard\"";

static snd_pcm_t *playback_handle;

static int fb_fd = -1;
static unsigned char *fb_mem;
static int px_width;
static int line_width;
static int screen_width;
static int lcd_width;
static int lcd_height;
static struct fb_var_screeninfo var;
static struct fb_fix_screeninfo finfo;

static int *zoom_x_tab;
static int *zoom_y_tab;


// char s_pchKeyboard[100] = "/dev/input/by-path/platform-ci_hdrc.1-usb-0:1.3:1.0-event-kbd";

static int lcd_fb_display_px(WORD color, int x, int y)
{
    unsigned char  *pen8;
    unsigned short *pen16;
    pen8 = (unsigned char *)(fb_mem + y*line_width + x*px_width);
    pen16 = (unsigned short *)pen8;
    *pen16 = color;

    return 0;
}

static int lcd_fb_init()
{
    //如果使用 mmap 打开方式 必须是 读定方式
    fb_fd = open("/dev/fb0", O_RDWR);
    if(-1 == fb_fd)
    {
        printf("cat't open /dev/fb0 \n");
        return -1;
    }
    if(-1==ioctl(fb_fd,FBIOGET_FSCREENINFO,&finfo))
    {
        printf("cat't ioctl /dev/fb0 \n");
    }
    //获取屏幕参数
    if(-1 == ioctl(fb_fd, FBIOGET_VSCREENINFO, &var))
    {
        close(fb_fd);
        printf("cat't ioctl /dev/fb0 \n");
        return -1;
    }

    //计算参数
    px_width     = var.bits_per_pixel / 8;
    //line_width   = var.xres * px_width; 6ul和157获取出来的参数不同，最终造成花屏，修复如下
    line_width   = finfo.line_length;
    screen_width = var.yres * line_width;
    lcd_width    = var.xres;
    lcd_height   = var.yres;

    printf("fb width:%d height:%d \n", lcd_width, lcd_height);
    printf("px_width:%d line_width:%d  screen_width:%d\n", px_width, line_width,screen_width);
    printf("var.bits_per_pixel:%d var.xres:%d  var.yres:%d \n", var.bits_per_pixel, var.xres,var.yres);
    printf("finfo line_length:%d \n", finfo.line_length);

    fb_mem = (unsigned char *)mmap(NULL, screen_width, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if(fb_mem == (void *)-1)
    {
        close(fb_fd);
        printf("cat't mmap /dev/fb0 \n");
        return -1;
    }
    //清屏
    memset(fb_mem, 0 , screen_width);
    return 0;
}

/**
 * 生成zoom 缩放表
 */
int make_zoom_tab()
{
    int i;
    zoom_x_tab = (int *)malloc(sizeof(int) * lcd_width);

    if(NULL == zoom_x_tab)
    {
        printf("make zoom_x_tab error\n");
        return -1;
    }
    for(i=0; i<lcd_width; i++)
    {
        zoom_x_tab[i] = i*NES_DISP_WIDTH/lcd_width;
    }
    zoom_y_tab = (int *)malloc(sizeof(int) * lcd_height);
    if(NULL == zoom_y_tab)
    {
        printf("make zoom_y_tab error\n");
        return -1;
    }
    for(i=0; i<lcd_height; i++)
    {
        zoom_y_tab[i] = i*NES_DISP_HEIGHT/lcd_height;
    }
    return 1;
}

int gamedev_init(void)
{
    char rc;
    int err = -1;

    rc = gamedev_find(joypad_name);
    if (0 == rc) {
        rc = gamedev_find(keyboard_name);
        if (0 == rc) 
            goto exit;
        else if (1 == rc) {
            gamedev1.dev = get_gamedev_path(0);
            gamedev1.gamedevx = &dwKeyPad1;
            gamedev1.gamedev_get = keyboard_get_event;
            err = gamedev_register(&gamedev1);
            if (-1 == err)
                goto exit;
        } else if (2 == rc) {
            gamedev1.dev = get_gamedev_path(0);
            gamedev2.dev = get_gamedev_path(1);
            gamedev1.gamedevx = &dwKeyPad1;
            gamedev2.gamedevx = &dwKeyPad2;
            gamedev1.gamedev_get = keyboard_get_event;
            gamedev2.gamedev_get = keyboard_get_event;
            err = gamedev_register(&gamedev1);
            if (-1 == err)
                goto exit;
            err = gamedev_register(&gamedev2);
            if (-1 == err)
                goto exit;
        }
    } else if (1 == rc) {
        gamedev1.dev = get_gamedev_path(0);
        gamedev1.gamedevx = &dwKeyPad1;
        gamedev1.gamedev_get = joypad_get_event;
        err = gamedev_register(&gamedev1);
        if (-1 == err)
            goto exit;
        
        rc = gamedev_find(keyboard_name);
        
        if (0 != rc) {
            gamedev2.dev = get_gamedev_path(1);
            gamedev2.gamedevx = &dwKeyPad2;
            gamedev2.gamedev_get = keyboard_get_event;
            err = gamedev_register(&gamedev2);
            if (-1 == err)
                goto exit;
        }
    } else if (2 == rc) {
        gamedev1.dev = get_gamedev_path(0);
        gamedev2.dev = get_gamedev_path(1);
        gamedev1.gamedevx = &dwKeyPad1;
        gamedev2.gamedevx = &dwKeyPad2;
        gamedev1.gamedev_get = joypad_get_event;
        gamedev2.gamedev_get = joypad_get_event;
        err = gamedev_register(&gamedev1);
        if (-1 == err)
            goto exit;
        err = gamedev_register(&gamedev2);
        if (-1 == err)
            goto exit;
    }
exit:
    return err;
}

/*-------------------------------------------------------------------*/
/*  ROM image file information                                       */
/*-------------------------------------------------------------------*/

char	szRomName[256];
char	szSaveName[256];
int	nSRAM_SaveFlag;

/*-------------------------------------------------------------------*/
/*  Constants ( Linux specific )                                     */
/*-------------------------------------------------------------------*/

#define VBOX_SIZE	7
#define SOUND_DEVICE	"/dev/dsp"
#define VERSION		"InfoNES v0.91J"

/*-------------------------------------------------------------------*/
/*  Global Variables ( Linux specific )                              */
/*-------------------------------------------------------------------*/

/* Emulation thread */
pthread_t  emulation_tid;
int bThread;

/* Pad state */
DWORD	dwKeyPad1;
DWORD	dwKeyPad2;
DWORD	dwKeySystem;

/* For Sound Emulation */
BYTE	final_wave[2048];
int	waveptr;
int	wavflag;
int	sound_fd;

/*-------------------------------------------------------------------*/
/*  Function prototypes ( Linux specific )                           */
/*-------------------------------------------------------------------*/

void *emulation_thread( void *args );


void start_application( char *filename );


int LoadSRAM();


int SaveSRAM();


/* Palette data */
WORD NesPalette[ 64 ] =
{ //RGB565
  0x738e,0x20d1,0x0015,0x4013,0x880e,0xa802,0xa000,0x7840,
  0x4140,0x0200,0x0280,0x01c2,0x19cb,0x0000,0x0000,0x0000,
  0xbdd7,0x039d,0x21dd,0x801e,0xb817,0xe00b,0xd940,0xca41,
  0x8b80,0x0480,0x0540,0x0487,0x0411,0x0000,0x0000,0x0000,
  0xffdf,0x3ddf,0x5c9f,0x445f,0xf3df,0xfb96,0xfb8c,0xfcc7,
  0xf5c7,0x8682,0x4ec9,0x5fd3,0x075b,0x0000,0x0000,0x0000,
  0xffdf,0xaf1f,0xc69f,0xd65f,0xfe1f,0xfe1b,0xfdd6,0xfed5,
  0xff14,0xe7d4,0xaf97,0xb7d9,0x9fde,0x0000,0x0000,0x0000,
};


/*===================================================================*/
/*                                                                   */
/*                main() : Application main                          */
/*                                                                   */
/*===================================================================*/

/* Application main */
int main( int argc, char **argv )
{
    char cmd;

    /*-------------------------------------------------------------------*/
    /*  Pad Control                                                      */
    /*-------------------------------------------------------------------*/

    /* Initialize a pad state */
    dwKeyPad1	= 0;
    dwKeyPad2	= 0;
    dwKeySystem = 0;

    /*-------------------------------------------------------------------*/
    /*  Load Cassette & Create Thread                                    */
    /*-------------------------------------------------------------------*/

    /* Initialize thread state */
    bThread = FALSE;

    int i;

    gamedev_init();

    lcd_fb_init();

    //初始化 zoom 缩放表
    make_zoom_tab();

    if (NULL != argv[1])
        start_application( argv[1] );

    //主循环中处理输入事件 声音播放
    while(1)
    {
        gamedev_get_event();

        //主线程休息一下 让子线程用一下 CPU
        usleep(300);
    }
    return(0);
}


/*===================================================================*/
/*                                                                   */
/*           emulation_thread() : Thread Hooking Routine             */
/*                                                                   */
/*===================================================================*/

void *emulation_thread( void *args )
{
    (void* )args;
    InfoNES_Main();
    return 0;
}

/*===================================================================*/
/*                                                                   */
/*     start_application() : Start NES Hardware                      */
/*                                                                   */
/*===================================================================*/
void start_application( char *filename )
{
    /* Set a ROM image name */
    strcpy( szRomName, filename );

    /* Load cassette */
    if ( InfoNES_Load( szRomName ) == 0 )
    {
        /* Load SRAM */
        LoadSRAM();

        /* Create Emulation Thread */
        bThread = TRUE;
        pthread_create( &emulation_tid, NULL, emulation_thread, NULL );
    }
}


/*===================================================================*/
/*                                                                   */
/*           LoadSRAM() : Load a SRAM                                */
/*                                                                   */
/*===================================================================*/
int LoadSRAM()
{
    /*
 *  Load a SRAM
 *
 *  Return values
 *     0 : Normally
 *    -1 : SRAM data couldn't be read
 */

    FILE		*fp;
    unsigned char	pSrcBuf[SRAM_SIZE];
    unsigned char	chData;
    unsigned char	chTag;
    int		nRunLen;
    int		nDecoded;
    int		nDecLen;
    int		nIdx;

    /* It doesn't need to save it */
    nSRAM_SaveFlag = 0;

    /* It is finished if the ROM doesn't have SRAM */
    if ( !ROM_SRAM )
        return(0);

    /* There is necessity to save it */
    nSRAM_SaveFlag = 1;

    /* The preparation of the SRAM file name */
    strcpy( szSaveName, szRomName );
    strcpy( strrchr( szSaveName, '.' ) + 1, "srm" );

    /*-------------------------------------------------------------------*/
    /*  Read a SRAM data                                                 */
    /*-------------------------------------------------------------------*/

    /* Open SRAM file */
    fp = fopen( szSaveName, "rb" );
    if ( fp == NULL )
        return(-1);

    /* Read SRAM data */
    fread( pSrcBuf, SRAM_SIZE, 1, fp );

    /* Close SRAM file */
    fclose( fp );

    /*-------------------------------------------------------------------*/
    /*  Extract a SRAM data                                              */
    /*-------------------------------------------------------------------*/

    nDecoded	= 0;
    nDecLen		= 0;

    chTag = pSrcBuf[nDecoded++];

    while ( nDecLen < 8192 )
    {
        chData = pSrcBuf[nDecoded++];

        if ( chData == chTag )
        {
            chData	= pSrcBuf[nDecoded++];
            nRunLen = pSrcBuf[nDecoded++];
            for ( nIdx = 0; nIdx < nRunLen + 1; ++nIdx )
            {
                SRAM[nDecLen++] = chData;
            }
        }else  {
            SRAM[nDecLen++] = chData;
        }
    }

    /* Successful */
    return(0);
}


/*===================================================================*/
/*                                                                   */
/*           SaveSRAM() : Save a SRAM                                */
/*                                                                   */
/*===================================================================*/
int SaveSRAM()
{
    /*
 *  Save a SRAM
 *
 *  Return values
 *     0 : Normally
 *    -1 : SRAM data couldn't be written
 */

    FILE		*fp;
    int		nUsedTable[256];
    unsigned char	chData;
    unsigned char	chPrevData;
    unsigned char	chTag;
    int		nIdx;
    int		nEncoded;
    int		nEncLen;
    int		nRunLen;
    unsigned char	pDstBuf[SRAM_SIZE];

    if ( !nSRAM_SaveFlag )
        return(0);  /* It doesn't need to save it */

    /*-------------------------------------------------------------------*/
    /*  Compress a SRAM data                                             */
    /*-------------------------------------------------------------------*/

    memset( nUsedTable, 0, sizeof nUsedTable );

    for ( nIdx = 0; nIdx < SRAM_SIZE; ++nIdx )
    {
        ++nUsedTable[SRAM[nIdx++]];
    }
    for ( nIdx = 1, chTag = 0; nIdx < 256; ++nIdx )
    {
        if ( nUsedTable[nIdx] < nUsedTable[chTag] )
            chTag = nIdx;
    }

    nEncoded	= 0;
    nEncLen		= 0;
    nRunLen		= 1;

    pDstBuf[nEncLen++] = chTag;

    chPrevData = SRAM[nEncoded++];

    while ( nEncoded < SRAM_SIZE && nEncLen < SRAM_SIZE - 133 )
    {
        chData = SRAM[nEncoded++];

        if ( chPrevData == chData && nRunLen < 256 )
            ++nRunLen;
        else{
            if ( nRunLen >= 4 || chPrevData == chTag )
            {
                pDstBuf[nEncLen++]	= chTag;
                pDstBuf[nEncLen++]	= chPrevData;
                pDstBuf[nEncLen++]	= nRunLen - 1;
            }else  {
                for ( nIdx = 0; nIdx < nRunLen; ++nIdx )
                    pDstBuf[nEncLen++] = chPrevData;
            }

            chPrevData	= chData;
            nRunLen		= 1;
        }
    }
    if ( nRunLen >= 4 || chPrevData == chTag )
    {
        pDstBuf[nEncLen++]	= chTag;
        pDstBuf[nEncLen++]	= chPrevData;
        pDstBuf[nEncLen++]	= nRunLen - 1;
    }else  {
        for ( nIdx = 0; nIdx < nRunLen; ++nIdx )
            pDstBuf[nEncLen++] = chPrevData;
    }

    /*-------------------------------------------------------------------*/
    /*  Write a SRAM data                                                */
    /*-------------------------------------------------------------------*/

    /* Open SRAM file */
    fp = fopen( szSaveName, "wb" );
    if ( fp == NULL )
        return(-1);

    /* Write SRAM data */
    fwrite( pDstBuf, nEncLen, 1, fp );

    /* Close SRAM file */
    fclose( fp );

    /* Successful */
    return(0);
}


/*===================================================================*/
/*                                                                   */
/*                  InfoNES_Menu() : Menu screen                     */
/*                                                                   */
/*===================================================================*/
int InfoNES_Menu()
{
    /*
 *  Menu screen
 *
 *  Return values
 *     0 : Normally
 *    -1 : Exit InfoNES
 */

    /* If terminated */
    if ( bThread == FALSE )
    {
        return(-1);
    }

    /* Nothing to do here */
    return(0);
}


/*===================================================================*/
/*                                                                   */
/*               InfoNES_ReadRom() : Read ROM image file             */
/*                                                                   */
/*===================================================================*/
int InfoNES_ReadRom( const char *pszFileName )
{
    /*
 *  Read ROM image file
 *
 *  Parameters
 *    const char *pszFileName          (Read)
 *
 *  Return values
 *     0 : Normally
 *    -1 : Error
 */

    FILE *fp;

    /* Open ROM file */
    fp = fopen( pszFileName, "rb" );
    if ( fp == NULL )
        return(-1);

    /* Read ROM Header */
    fread( &NesHeader, sizeof NesHeader, 1, fp );
    if ( memcmp( NesHeader.byID, "NES\x1a", 4 ) != 0 )
    {
        /* not .nes file */
        fclose( fp );
        return(-1);
    }

    /* Clear SRAM */
    memset( SRAM, 0, SRAM_SIZE );

    /* If trainer presents Read Triner at 0x7000-0x71ff */
    if ( NesHeader.byInfo1 & 4 )
    {
        fread( &SRAM[0x1000], 512, 1, fp );
    }

    /* Allocate Memory for ROM Image */
    ROM = (BYTE *) malloc( NesHeader.byRomSize * 0x4000 );

    /* Read ROM Image */
    fread( ROM, 0x4000, NesHeader.byRomSize, fp );

    if ( NesHeader.byVRomSize > 0 )
    {
        /* Allocate Memory for VROM Image */
        VROM = (BYTE *) malloc( NesHeader.byVRomSize * 0x2000 );

        /* Read VROM Image */
        fread( VROM, 0x2000, NesHeader.byVRomSize, fp );
    }

    /* File close */
    fclose( fp );

    /* Successful */
    return(0);
}


/*===================================================================*/
/*                                                                   */
/*           InfoNES_ReleaseRom() : Release a memory for ROM         */
/*                                                                   */
/*===================================================================*/
void InfoNES_ReleaseRom()
{
    /*
 *  Release a memory for ROM
 *
 */

    if ( ROM )
    {
        free( ROM );
        ROM = NULL;
    }

    if ( VROM )
    {
        free( VROM );
        VROM = NULL;
    }
}


/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemoryCopy() : memcpy                         */
/*                                                                   */
/*===================================================================*/
void *InfoNES_MemoryCopy( void *dest, const void *src, int count )
{
    /*
 *  memcpy
 *
 *  Parameters
 *    void *dest                       (Write)
 *      Points to the starting address of the copied block's destination
 *
 *    const void *src                  (Read)
 *      Points to the starting address of the block of memory to copy
 *
 *    int count                        (Read)
 *      Specifies the size, in bytes, of the block of memory to copy
 *
 *  Return values
 *    Pointer of destination
 */

    memcpy( dest, src, count );
    return(dest);
}


/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemorySet() : memset                          */
/*                                                                   */
/*===================================================================*/
void *InfoNES_MemorySet( void *dest, int c, int count )
{
    /*
 *  memset
 *
 *  Parameters
 *    void *dest                       (Write)
 *      Points to the starting address of the block of memory to fill
 *
 *    int c                            (Read)
 *      Specifies the byte value with which to fill the memory block
 *
 *    int count                        (Read)
 *      Specifies the size, in bytes, of the block of memory to fill
 *
 *  Return values
 *    Pointer of destination
 */

    memset( dest, c, count );
    return(dest);
}


/*===================================================================*/
/*                                                                   */
/*      InfoNES_LoadFrame() :                                        */
/*           Transfer the contents of work frame on the screen       */
/*                                                                   */
/*===================================================================*/
void InfoNES_LoadFrame()
{
    int x,y;
    int line_width;
    WORD wColor;

    //修正 即便没有 LCD 也可以出声
    if(0 < fb_fd)
    {
        for (y = 0; y < lcd_height; y++ )
        {
            line_width = zoom_y_tab[y] * NES_DISP_WIDTH;
            for (x = 0; x < lcd_width; x++ )
            {
                wColor = WorkFrame[line_width  + zoom_x_tab[x]];
                lcd_fb_display_px(wColor, x, y);
            }
        }
    }
}


/*===================================================================*/
/*                                                                   */
/*             InfoNES_PadState() : Get a joypad state               */
/*                                                                   */
/*===================================================================*/
void InfoNES_PadState( DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem )
{
    /*
 *  Get a joypad state
 *
 *  Parameters
 *    DWORD *pdwPad1                   (Write)
 *      Joypad 1 State
 *
 *    DWORD *pdwPad2                   (Write)
 *      Joypad 2 State
 *
 *    DWORD *pdwSystem                 (Write)
 *      Input for InfoNES
 *
 */

    /* Transfer joypad state */
    *pdwPad1	= dwKeyPad1;
    *pdwPad2	= dwKeyPad2;
    *pdwSystem	= dwKeySystem;

    //取消重置手柄 在 输入函数中自行处理
    //dwKeyPad1 = 0;
}


/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundInit() : Sound Emulation Initialize           */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundInit( void )
{

}


/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundOpen() : Sound Open                           */
/*                                                                   */
/*===================================================================*/
int InfoNES_SoundOpen( int samples_per_sync, int sample_rate )
{
#if 1
    //sample_rate 采样率 44100
    //samples_per_sync  735
    unsigned int rate      = sample_rate;
    snd_pcm_hw_params_t *hw_params;

    if(0 > snd_pcm_open(&playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0))
    {
        printf("snd_pcm_open err\n");
        return -1;
    }

    if(0 > snd_pcm_hw_params_malloc(&hw_params))
    {
        printf("snd_pcm_hw_params_malloc err\n");
        return -1;
    }

    if(0 > snd_pcm_hw_params_any(playback_handle, hw_params))
    {
        printf("snd_pcm_hw_params_any err\n");
        return -1;
    }
    if(0 > snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED))
    {
        printf("snd_pcm_hw_params_any err\n");
        return -1;
    }

    //16bit PCM 数据
    if(0 > snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_U8))
    {
        printf("snd_pcm_hw_params_set_format err\n");
        return -1;
    }

    if(0 > snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &rate, 0))
    {
        printf("snd_pcm_hw_params_set_rate_near err\n");
        return -1;
    }

    //单声道 非立体声
    if(0 > snd_pcm_hw_params_set_channels(playback_handle, hw_params, 1))
    {
        printf("snd_pcm_hw_params_set_channels err\n");
        return -1;
    }

    if(0 > snd_pcm_hw_params(playback_handle, hw_params))
    {
        printf("snd_pcm_hw_params err\n");
        return -1;
    }

    snd_pcm_hw_params_free(hw_params);

    if(0 > snd_pcm_prepare(playback_handle))
    {
        printf("snd_pcm_prepare err\n");
        return -1;
    }
#endif
    return 1;
}


/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundClose() : Sound Close                         */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundClose( void )
{
    snd_pcm_close(playback_handle);
}


/*===================================================================*/
/*                                                                   */
/*            InfoNES_SoundOutput() : Sound Output 5 Waves           */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundOutput( int samples, BYTE *wave1, BYTE *wave2, BYTE *wave3, BYTE *wave4, BYTE *wave5 )
{
#if 1
    int i;
    int ret;
    unsigned char wav;
    unsigned char *pcmBuf = (unsigned char *)malloc(samples);

    for (i=0; i <samples; i++)
    {
        wav = (wave1[i] + wave2[i] + wave3[i] + wave4[i] + wave5[i]) / 5;
        //单声道 8位数据
        pcmBuf[i] = wav;
    }
    ret = snd_pcm_writei(playback_handle, pcmBuf, samples);
    if(-EPIPE == ret)
    {
        snd_pcm_prepare(playback_handle);
    }
    free(pcmBuf);
    return ;
#endif
}


/*===================================================================*/
/*                                                                   */
/*            InfoNES_Wait() : Wait Emulation if required            */
/*                                                                   */
/*===================================================================*/
void InfoNES_Wait()
{
}


/*===================================================================*/
/*                                                                   */
/*            InfoNES_MessageBox() : Print System Message            */
/*                                                                   */
/*===================================================================*/
void InfoNES_MessageBox( char *pszMsg, ... )
{
    printf( "MessageBox: %s \n", pszMsg );
}


/*
 * End of InfoNES_System_Linux.cpp
 */
