/*
 *  fb矩阵输出
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

#define FB_PATH "/dev/fb0"

typedef struct
{
    int fd;
    unsigned char *fb;
    size_t fbSize;
    struct fb_var_screeninfo fbInfo;
    //bytes per point
    int bpp;
    //bytes width, height
    int bw, bh;
} FbMap;

static FbMap *fbmap = NULL;
int fb_width = 1024, fb_height = 600;

void fb_release(void)
{
    if (!fbmap)
        return;
    if (fbmap->fb)
        munmap(fbmap->fb, fbmap->fbSize);
    if (fbmap->fd > 0)
        close(fbmap->fd);
    free(fbmap);
    fbmap = NULL;
}

//返回0正常
int fb_init(void)
{
    if (fbmap)
        return 0;

    fbmap = (FbMap *)calloc(1, sizeof(FbMap));

    fbmap->fd = open(FB_PATH, O_RDWR);
    if (fbmap->fd < 1)
    {
        fprintf(stderr, "fb_init: open %s err \r\n", FB_PATH);
        fb_release();
        return -1;
    }

    if (ioctl(fbmap->fd, FBIOGET_VSCREENINFO, &fbmap->fbInfo) < 0)
    {
        fprintf(stderr, "fb_init: ioctl FBIOGET_VSCREENINFO err \r\n");
        fb_release();
        return -1;
    }
    printf("frameBuffer: %s, %d x %d, %dbytes / %dbpp\r\n",
           FB_PATH, fbmap->fbInfo.xres, fbmap->fbInfo.yres, fbmap->fbInfo.bits_per_pixel / 8, fbmap->fbInfo.bits_per_pixel);

    fbmap->bpp = fbmap->fbInfo.bits_per_pixel / 8;
    fbmap->bw = fbmap->bpp * fbmap->fbInfo.xres;
    fbmap->bh = fbmap->bpp * fbmap->fbInfo.yres;
    fbmap->fbSize = fbmap->fbInfo.xres * fbmap->fbInfo.yres * fbmap->bpp;

    fb_width = fbmap->fbInfo.xres;
    fb_height = fbmap->fbInfo.yres;

    fbmap->fb = (unsigned char *)mmap(0, fbmap->fbSize, PROT_READ | PROT_WRITE, MAP_SHARED, fbmap->fd, 0);
    if (!fbmap->fb)
    {
        fprintf(stderr, "fb_init: mmap size %d err \r\n", (int)fbmap->fbSize);
        fb_release();
        return -1;
    }

    return 0;
}

/*
 *  屏幕输出
 *  data: 图像数组,数据长度必须为 width*height*3, RGB格式
 *  offsetX, offsetY: 屏幕起始位置
 *  width, height: 图像宽高
 */
void fb_output(unsigned char *data, int offsetX, int offsetY, int width, int height)
{
    int x, y, offset;
    //初始化检查
    if (fb_init())
        return;
    if (!data)
        return;
    //起始坐标限制
    if (offsetX < 0)
        offsetX = 0;
    else if (offsetX >= fbmap->fbInfo.xres)
        return;
    if (offsetY < 0)
        offsetY = 0;
    else if (offsetY >= fbmap->fbInfo.yres)
        return;
    //范围限制
    if (width < 1)
        return;
    else if (offsetX + width - 1 >= fbmap->fbInfo.xres)
        width = fbmap->fbInfo.xres - offsetX;
    if (height < 1)
        return;
    else if (offsetY + height - 1 >= fbmap->fbInfo.yres)
        height = fbmap->fbInfo.yres - offsetY;
    //覆盖画图
    for (y = 0; y < height; y++)
    {
        //当前行在fb数据的偏移
        offset = (y + offsetY) * fbmap->bw + (0 + offsetX) * fbmap->bpp;
        //画data中的一行数据
        for (x = 0; x < width; x++)
        {
            if (fbmap->bpp == 4)
                fbmap->fb[offset + 3] = 0x00; //A
            fbmap->fb[offset + 2] = *data++;  //R
            fbmap->fb[offset + 1] = *data++;  //G
            fbmap->fb[offset + 0] = *data++;  //B
            offset += fbmap->bpp;
        }
    }
}
