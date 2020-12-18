#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "fft.h"
#include "wave.h"
#include "fbmap.h"

//采样点数,必须为2^X次方,如2,4,8,,,256,512,1024
#define FFT_N 128

//示波器横向放大点数(最少为1)
#define ZOOM_X 5

//理论上 math.h 里面定义了M_PI
#ifndef M_PI
#define M_PI 3.1415926535897
#endif

/*
 *  创建sin/cos曲线
 *  参数:
 *      line: float数组
 *      len: float数据个数
 *      isSin: 是否使用isSin,默认cos的好处是第一个点不为0
 *      div: 采样点跳跃量,例如:用10个点采完一个sin/cos周期,则div=2*M_PI/10,单位:rad
 *      xOffset: sin/cos相位偏移量,单位:rad
 *      yOffset: y轴上下平移量,单位:点
 *      top: 幅值,为峰峰值的一半
 */
void line_create(float line[], unsigned int len, char isSin, float div, float xOffset, float yOffset, float top)
{
    int i;
    if (isSin)
    {
        for (i = 0; i < len; i++)
            line[i] = sin(i * div + xOffset) * top + yOffset;
    }
    else
    {
        for (i = 0; i < len; i++)
            line[i] = cos(i * div + xOffset) * top + yOffset;
    }
}

/*
 *  创建方波
 *  参数:
 *      line: float数组
 *      len: float数据个数
 *      halfDp: 半周期点数,必须大于1
 *      xOffset: x轴平移量,单位:点
 *      yOffset: y轴上下平移量,单位:点
 *      top: 幅值,为峰峰值的一半
 */
void line_create2(float line[], unsigned int len, unsigned int halfDp, int xOffset, int yOffset, float top)
{
    int i, p;
    char isLow = 1;
    for (i = 0, p = xOffset; i < len; i++, p++)
    {
        if (p % halfDp == 0)
            isLow = isLow ? 0 : 1;
        if (isLow)
            line[i] = yOffset - top;
        else
            line[i] = yOffset + top;
    }
}

/*
 *  曲线相加
 */
void line_sum(float line1[], float line2[], int len, float lineRet[])
{
    int i;
    for (i = 0; i < len; i++)
        lineRet[i] = line1[i] + line2[i];
}

int main(void)
{
    int i = 0, j = 0;
    Wave_Struct *ws;

    float line1[FFT_N] = {0};
    float line2[FFT_N] = {0};
    float line3[FFT_N] = {0};

    float inReal[FFT_N] = {0}; //输入实部
    float inImag[FFT_N] = {0}; //输入虚部

    float outAF[FFT_N]; //输出幅频曲线
    float outPF[FFT_N]; //输出相频曲线

    //初始化一次fb设备,获得屏幕宽高后,初始化示波器
    fb_output(NULL, 0, 0, 0, 0);
    ws = wave_init(0, 0, fb_width, fb_height);

#if 1
    //幅度/相位测试曲线
    line_create(
        line1, FFT_N,
        0,                //cos曲线
        2 * M_PI / 16,    //频率 16点/周期
        -M_PI + 0.001, 0, //相位 -pi (+0.001为避免临界跳变成正pi)
        1500              //幅值 1500
    );
    line_create(
        line2, FFT_N,
        0,            //cos曲线
        2 * M_PI / 8, //频率 8点/周期
        0, 0,         //相位 0
        3000          //幅值 3000
    );
    line_create(
        line3, FFT_N,
        0,               //cos曲线
        2 * M_PI / 4,    //频率 4点/周期
        M_PI - 0.001, 0, //相位 pi (-0.001为避免临界跳变成-pi)
        6000             //幅值 6000
    );
#else
    //方波测试
    line_create2(line1, FFT_N, 2, 0, 0, 1500);
    line_create2(line2, FFT_N, 4, 0, 0, 3000);
    line_create2(line3, FFT_N, 8, 0, 0, 6000);
#endif

    //曲线相加到 inReal[] 数组
    line_sum(line1, inReal, FFT_N, inReal);
    line_sum(line2, inReal, FFT_N, inReal);
    line_sum(line3, inReal, FFT_N, inReal);

    //快速傅立叶变换,得到 幅频/相频 曲线
    FFT(inReal, inImag, outAF, outPF, FFT_N);

    while (1)
    {
        //横向放大倍数(就是横向把一个点打印多次)
        for (j = 0; j < ZOOM_X; j++)
        {
            //画输入曲线
            wave_load(ws, 0, (short)line1[i] + 30000);
            wave_load(ws, 1, (short)line2[i] + 23000);
            wave_load(ws, 2, (short)line3[i] + 15000);
            wave_load(ws, 3, (short)inReal[i]);

            //画相频曲线
            wave_load(ws, 4, -15000); //基准线
            wave_load(ws, 5, (short)(outPF[i] * 1000 - 15000)); //相位范围(-pi, pi),这里放大1000倍
            wave_load(ws, 6, (short)(M_PI * 1000 - 15000)); //pi相位参考线
            wave_load(ws, 7, (short)(-M_PI * 1000 - 15000)); //-pi相位参考线

            //画幅频曲线
            wave_load(ws, 8, -30000); //基准线
            wave_load(ws, 9, (short)(outAF[i] - 30000)); //幅值
            wave_load(ws, 10, 3000 - 30000); //3000幅值参考线

            wave_output(ws);
            usleep(5000);
        }

        if (++i >= FFT_N)
            break;
    }
    return 0;
}
