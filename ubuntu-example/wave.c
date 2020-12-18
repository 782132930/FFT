/*
 *  自制简易版示波器,图像输出到fb0
 */
#include <stdlib.h>
#include <string.h>
#include "wave.h"
#include "fbmap.h"

WAVE_COLOR

Wave_Struct *wave_init(int xOffset, int yOffset, int width, int height)
{
    int i;
    Wave_Struct *ws = (Wave_Struct *)calloc(1, sizeof(Wave_Struct));
    ws->xOffset = xOffset;
    ws->yOffset = yOffset;
    ws->width = width;
    ws->height = height;
    ws->height_half = height / 2;
    ws->map_size = width * height * 3;
    ws->map = (unsigned char *)calloc(ws->map_size, sizeof(char));
    for (i = 0; i < WAVE_CHN; i++)
        ws->chn[i] = (short *)calloc(width, sizeof(short));
    return ws;
}

void wave_release(Wave_Struct **ws)
{
    int i;
    if (!ws)
        return;
    if (*ws)
    {
        if ((*ws)->map)
            free((*ws)->map);
        for (i = 0; i < WAVE_CHN; i++)
        {
            if ((*ws)->chn[i])
                free((*ws)->chn[i]);
        }
        free(*ws);
        *ws = NULL;
    }
}

void wave_line(
    int xStart, int yStart,
    int xEnd, int yEnd,
    int width, unsigned char *map, char *rgb)
{
    int offset;
    unsigned short t;
    int xerr = 0, yerr = 0;
    int delta_x, delta_y;
    int distance;
    int incx, incy, xCount, yCount;
    //计算坐标增量
    delta_x = xEnd - xStart;
    delta_y = yEnd - yStart;
    xCount = xStart;
    yCount = yStart;
    //
    if (delta_x > 0)
        incx = 1; //设置单步方向
    else if (delta_x == 0)
        incx = 0; //垂直线
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }
    //
    if (delta_y > 0)
        incy = 1;
    else if (delta_y == 0)
        incy = 0; //水平线
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }
    //选取基本增量坐标轴
    if (delta_x > delta_y)
        distance = delta_x;
    else
        distance = delta_y;
    //画线输出
    for (t = 0; t <= distance + 1; t++)
    {
        offset = (yCount * width + xCount) * 3;
        //线和线之间半透明叠加
        map[offset + 0] = (map[offset + 0] + rgb[0]) >> 1;
        map[offset + 1] = (map[offset + 1] + rgb[1]) >> 1;
        map[offset + 2] = (map[offset + 2] + rgb[2]) >> 1;
        //
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance)
        {
            xerr -= distance;
            xCount += incx;
        }
        if (yerr > distance)
        {
            yerr -= distance;
            yCount += incy;
        }
    }
}

void wave_load(Wave_Struct *ws, int chn, short value)
{
    if (!ws || chn < 0 || chn >= WAVE_CHN)
        return;
    ws->chn[chn][ws->output_count] = value;
}

void wave_output(Wave_Struct *ws)
{
    int i, j;
    int x, y;           //新点
    int ox = 0, oy = 0; //旧点
    if (!ws)
        return;
    //清屏
    memset(ws->map, 0, ws->map_size);
    //画基线
    memset(ws->map + ws->height_half * ws->width * 3, 0xFF, ws->width * 3);
    //画各通道曲线
    for (i = 0; i < WAVE_CHN; i++)
    {
        ox = oy = 0;
        //遍历该通道的点
        for (j = 0; j <= ws->output_count; j++)
        {
            //计算当前点在屏幕中的位置(x,y)
            y = ws->height_half - ws->chn[i][j] * ws->height_half / 32768;
            if (y < 0)
                y = 0;
            else if (y >= ws->height)
                y = ws->height - 1;
            x = j;
            //用新点和旧点(ox,oy)连线
            wave_line(ox, oy, x, y, ws->width, ws->map, (char *)wave_color[i]);
            //更新旧点
            ox = x;
            oy = y;
        }
    }
    //输出到屏幕
    fb_output(ws->map, ws->xOffset, ws->yOffset, ws->width, ws->height);
    //屏幕已经画满了?
    ws->output_count += 1;
    if (ws->output_count >= ws->width)
    {
        //遍历各通道
        for (i = 0; i < WAVE_CHN; i++)
            //全体往左移动,丢弃掉第一个数据
            for (j = 0; j < ws->width - 1; j++)
                ws->chn[i][j] = ws->chn[i][j + 1];
        //又空出了一个点的位置
        ws->output_count -= 1;
    }
}
