/*
 *  自制简易版示波器,图像输出到fb0
 */
#ifndef _WAVE_H_
#define _WAVE_H_

//通道数
#define WAVE_CHN 12
//各通道RGB颜色定义
#define WAVE_COLOR                         \
    const char wave_color[WAVE_CHN][3] = { \
        {0xFF, 0x00, 0x00},                \
        {0x00, 0xFF, 0x00},                \
        {0x00, 0x00, 0xFF},                \
        {0xFF, 0xFF, 0x00},                \
        {0x00, 0xFF, 0xFF},                \
        {0xFF, 0x00, 0xFF},                \
        {0xFF, 0x80, 0x00},                \
        {0x00, 0xFF, 0x80},                \
        {0x80, 0x00, 0xFF},                \
        {0x80, 0x40, 0x40},                \
        {0x40, 0x80, 0x40},                \
        {0x40, 0x40, 0x80},                \
    };

typedef struct
{
    int xOffset, yOffset;
    int width, height;
    //半屏高度
    int height_half;
    //每个通道横向已经输出了多少个点
    int output_count;
    //RGB图像矩阵
    unsigned char *map;
    int map_size;
    //各通道原始数据
    short *chn[WAVE_CHN];
} Wave_Struct;

Wave_Struct *wave_init(int xOffset, int yOffset, int width, int height);
void wave_release(Wave_Struct **ws);
void wave_load(Wave_Struct *ws, int chn, short value);
void wave_output(Wave_Struct *ws);

#endif