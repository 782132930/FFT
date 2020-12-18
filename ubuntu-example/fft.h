/*
 *  参数:
 *      inReal[N]: 实数部分数组
 *      inImag[N]: 虚数部分数组 <不用可以置NULL>
 *      N: 采样数据个数,必须为2的x次方,如2,4,8...256,512,1024
 * 
 *  输出:
 *      outReal[N]: 实数部分数组 <不用可以置NULL>
 *      outImag[N]: 虚数部分数组 <不用可以置NULL>
 *      outAF[N]: 输出 幅-频曲线(amplitude-frequency) <不用可以置NULL>
 *      outPF[N]: 输出 相-频曲线(phase-frequency) <不用可以置NULL>
 *
 *  原文链接: https://zhuanlan.zhihu.com/p/135259438
 *
 *  (以下为改版代码)
 */
#ifndef _FFT_H_
#define _FFT_H_

void FFT(
    float inReal[], float inImag[],
    float outReal[], float outImag[],
    float outAF[], float outPF[],
    unsigned int N);   /*复数FFT快速计算*/
void FFTR(
    float inReal[], float inImag[],
    float outReal[], float outImag[],
    float outAF[], float outPF[],
    unsigned int N);  /*实数FFT快速计算*/
void IFFT(
    float inReal[], float inImag[],
    float outReal[], float outImag[],
    unsigned int N);  /*复数IFFT快速计算*/
void IFFTR(
    float inReal[], float inImag[],
    float outReal[], float outImag[],
    unsigned int N); /*实数IFFT快速计算*/

#endif
