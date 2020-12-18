/*
 *  参数:
 *      inReal[N]: 实数部分数组
 *      inImag[N]: 虚数部分数组 <不用可以置NULL>
 *      N: 采样数据个数,必须为2的x次方,如2,4,8...256,512,1024
 * 
 *  输出:
 *      outAF[N]: 输出 幅-频曲线(amplitude-frequency) <不用可以置NULL>
 *      outPF[N]: 输出 相-频曲线(phase-frequency) <不用可以置NULL>
 *
 *  原文链接: https://zhuanlan.zhihu.com/p/135259438
 *
 *  (以下为改版代码)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define FFT_PI 3.1415926535897

//重复代码,这里抠出来共用
#define FFT_BASE()                                \
	unsigned int M = (int)log2(N);                \
	float _inReal[N], _inImag[N];                 \
	if (inReal)                                   \
		memcpy(_inReal, inReal, sizeof(_inReal)); \
	else                                          \
		memset(_inReal, 0, sizeof(_inReal));      \
	if (inImag)                                   \
		memcpy(_inImag, inImag, sizeof(_inImag)); \
	else                                          \
		memset(_inImag, 0, sizeof(_inImag));

//倒序
static void _reversal(float _inReal[], float _inImag[], unsigned int N, unsigned int M)
{
	unsigned int k;
	unsigned int I, J, K, F0, F1, m, n;
	float temp;

	//倒序
	for (I = 0; I < N; I++) //根据规律四，需要对数组元素执行码间倒序
	{
		/*获取下标I的反序J的数值*/
		J = 0;
		for (k = 0; k < (M / 2 + 0.5); k++) //k表示操作
		{
			//*反序操作*/
			m = 1;							 //m是最低位为1的二进制数
			n = (unsigned int)pow(2, M - 1); //n是第M位为1的二进制数
			m <<= k;						 //对m左移k位
			n >>= k;						 //对n右移k位
			F0 = I & n;						 //I与n按位与提取出前半部分第k位
			F1 = I & m;						 //I与m按位与提取出F0对应的后半部分的低位
			if (F0)
				J = J | m; //J与m按位或使F0对应低位为1
			if (F1)
				J = J | n; //J与n按位或使F1对应高位为1
		}
		//printf("I为: ");printBin(I,M);printf(" %d\n",I);
		//printf("J为: ");printBin(J,M);printf(" %d\n\n",J);

		if (I < J)
		{
			temp = _inReal[I];
			_inReal[I] = _inReal[J];
			_inReal[J] = temp;
			//补充虚数部分交换:
			temp = _inImag[I];
			_inImag[I] = _inImag[J];
			_inImag[J] = temp;
		}
	}
}

//执行FFT
static void _FFT(float _inReal[], float _inImag[], unsigned int N, unsigned int M)
{
	int i, j, k, r;
	int p, L, B;
	float Tr, Ti;

	//进行FFT
	for (L = 1; L <= M; L++) //FFT蝶形级数L从1--M
	{
		/*第L级的运算:
		然后对于第L级，我们在问题五种提到，蝶形运算的种类数目等于间隔B:有多少种蝶形运算就要需要循环多少次;
		随着循环的不同，旋转指数P也不同，P的增量为k=2^(M-L)*/
		//先计算一下间隔 B = 2^(L-1);
		B = 1;
		B = (int)pow(2, L - 1);
		for (j = 0; j <= B - 1; j++)
		//j = 0,1,2,...,2^(L-1) - 1
		{ /*同种蝶形运算*/
			//先计算增量k=2^(M-L)
			k = 1;
			k = (int)pow(2, M - L);
			//计算旋转指数p，增量为k时，则P=j*k
			p = 1;
			p = j * k;
			/*接下来，由问题六我们可以知道同种蝶形运算的次数刚好为增量k=2^(M-L)；
			同种蝶形的运算次数即为蝶形运算的次数*/
			for (i = 0; i <= k - 1; i++)
			{
				//数组下标定为r
				r = 1;
				r = j + 2 * B * i;
				Tr = _inReal[r + B] * cos(2.0 * FFT_PI * p / N) + _inImag[r + B] * sin(2.0 * FFT_PI * p / N);
				Ti = _inImag[r + B] * cos(2.0 * FFT_PI * p / N) - _inReal[r + B] * sin(2.0 * FFT_PI * p / N);
				_inReal[r + B] = _inReal[r] - Tr;
				_inImag[r + B] = _inImag[r] - Ti;
				_inReal[r] = _inReal[r] + Tr;
				_inImag[r] = _inImag[r] + Ti;
			}
		}
	}
}

//复数FFT
void FFT(float inReal[], float inImag[], float outAF[], float outPF[], unsigned int N)
{
	unsigned int i;

	FFT_BASE();

	//倒序
	_reversal(_inReal, _inImag, N, M);

	_FFT(_inReal, _inImag, N, M);

	//计算幅频
	if (outAF)
	{
		for (i = 0; i < N; i++)
			outAF[i] = sqrt(_inReal[i] * _inReal[i] + _inImag[i] * _inImag[i]) / (N / 2);
	}
	//计算相频
	if (outPF)
	{
		for (i = 0; i < N; i++)
			outPF[i] = atan2(_inImag[i], _inReal[i]);
	}
}

//实数FFT
void FFTR(float inReal[], float inImag[], float outAF[], float outPF[], unsigned int N)
{
	int i, k;
	int j, r, p, B;
	float Tr, Ti;
	float yR[N / 2], yI[N / 2];
	float x1R[N / 2], x2R[N / 2], x1I[N / 2], x2I[N / 2], xR[N], xI[N];

	FFT_BASE();

	for (i = 0; i < N / 2; i++)
	{
		yR[i] = _inReal[2 * i];
		yI[i] = _inReal[2 * i + 1];
	}

	//倒序
	_reversal(yR, yI, N / 2, M - 1);

	_FFT(yR, yI, N / 2, M - 1);

	//求X1(k)和X2(k)
	for (k = 0; k < N / 2; k++)
	{
		if (k == 0)
		{
			x1R[k] = yR[k];
			x1I[k] = yI[k];
			x2R[k] = yI[k];
			x2I[k] = -yR[k];
		}
		else
		{
			x1R[k] = (yR[k] + yR[N / 2 - k]) / 2;
			x1I[k] = (yI[k] - yI[N / 2 - k]) / 2;
			x2R[k] = (yI[k] + yI[N / 2 - k]) / 2;
			x2I[k] = (yR[N / 2 - k] - yR[k]) / 2;
		}
	}

	//第M级的蝶形运算
	B = 1;
	B = (int)pow(2, M - 1);
	for (j = 0; j <= N / 2 - 1; j++)
	//j = 0,1,2,...,2^(L-1) - 1
	{
		p = 1;
		p = j;
		//数组下标定为r
		r = 1;
		r = j;
		int k = j;
		Tr = x2R[r] * cos(2.0 * FFT_PI * p / N) + x2I[r] * sin(2.0 * FFT_PI * p / N);
		Ti = x2I[r] * cos(2.0 * FFT_PI * p / N) - x2R[r] * sin(2.0 * FFT_PI * p / N);
		xR[r] = x1R[r] + Tr;
		xI[r] = x1I[r] + Ti;

		if (r == 0)
		{
			xR[N / 2] = x1R[0] - x2R[0];
			xI[N / 2] = x1I[0] - x2I[0];
		}
		else
		{
			xR[N - r] = xR[r];
			xI[N - r] = -xI[r];
		}
	}

	for (i = 0; i < N; i++)
	{
		_inReal[i] = xR[i];
		_inImag[i] = xI[i];
	}

	//计算幅频
	if (outAF)
	{
		for (i = 0; i < N; i++)
			outAF[i] = sqrt(_inReal[i] * _inReal[i] + _inImag[i] * _inImag[i]) / (N / 2);
	}
	//计算相频
	if (outPF)
	{
		for (i = 0; i < N; i++)
			outPF[i] = atan2(_inImag[i], _inReal[i]);
	}
}

//执行IFFT
static void _IFFT(float _inReal[], float _inImag[], unsigned int N, unsigned int M)
{
	int i, j, k, r;
	int p, L, B;
	float Tr, Ti;

	//进行IFFT
	for (L = 1; L <= M; L++) //FFT蝶形级数L从1--M
	{
		/*第L级的运算:
		然后对于第L级，我们在问题五种提到，蝶形运算的种类数目等于间隔B:有多少种蝶形运算就要需要循环多少次;
		随着循环的不同，旋转指数P也不同，P的增量为k=2^(M-L)*/
		//先计算一下间隔 B = 2^(L-1);
		B = 1;
		B = (int)pow(2, L - 1);
		for (j = 0; j <= B - 1; j++)
		//j = 0,1,2,...,2^(L-1) - 1
		{ /*同种蝶形运算*/
			//先计算增量k=2^(M-L)
			k = 1;
			k = (int)pow(2, M - L);
			//计算旋转指数p，增量为k时，则P=j*k
			p = 1;
			p = j * k;
			/*接下来，由问题六我们可以知道同种蝶形运算的次数刚好为增量k=2^(M-L)；
			同种蝶形的运算次数即为蝶形运算的次数*/
			for (i = 0; i <= k - 1; i++)
			{
				//数组下标定为r
				r = 1;
				r = j + 2 * B * i;
				Tr = _inReal[r + B] * cos(2.0 * FFT_PI * p / N) - _inImag[r + B] * sin(2.0 * FFT_PI * p / N);
				Ti = _inImag[r + B] * cos(2.0 * FFT_PI * p / N) + _inReal[r + B] * sin(2.0 * FFT_PI * p / N);
				_inReal[r + B] = _inReal[r] - Tr;
				_inReal[r + B] = _inReal[r + B] / 2;
				_inImag[r + B] = _inImag[r] - Ti;
				_inImag[r + B] = _inImag[r + B] / 2;
				_inReal[r] = _inReal[r] + Tr;
				_inReal[r] = _inReal[r] / 2;
				_inImag[r] = _inImag[r] + Ti;
				_inImag[r] = _inImag[r] / 2;
			}
		}
	}
}

//复数IFFT
void IFFT(float inReal[], float inImag[], float outAF[], float outPF[], unsigned int N)
{
	unsigned int i;

	FFT_BASE();

	//倒序
	_reversal(_inReal, _inImag, N, M);

	_IFFT(_inReal, _inImag, N, M);

	//计算幅频
	if (outAF)
	{
		for (i = 0; i < N; i++)
			outAF[i] = sqrt(_inReal[i] * _inReal[i] + _inImag[i] * _inImag[i]) / (N / 2);
	}
	//计算相频
	if (outPF)
	{
		for (i = 0; i < N; i++)
			outPF[i] = atan2(_inImag[i], _inReal[i]);
	}
}

//实数IFFT
void IFFTR(float inReal[], float inImag[], float outAF[], float outPF[], unsigned int N)
{
	int i, k;
	int j, r, p, B;
	float Tr, Ti;
	float yR[N / 2], yI[N / 2];
	float x1R[N / 2], x2R[N / 2], x1I[N / 2], x2I[N / 2];

	FFT_BASE();

	for (i = 0; i < N / 2; i++)
	{
		yR[i] = _inReal[2 * i];
		yI[i] = _inReal[2 * i + 1];
	}

	//倒序
	_reversal(yR, yI, N / 2, M - 1);

	_IFFT(yR, yI, N / 2, M - 1);

	//求X1(k)和X2(k)
	for (k = 0; k < N / 2; k++)
	{
		if (k == 0)
		{
			x1R[k] = yR[k];
			x1I[k] = yI[k];
			x2R[k] = yI[k];
			x2I[k] = -yR[k];
		}
		else
		{
			x1R[k] = (yR[k] + yR[N / 2 - k]) / 2;
			x1I[k] = (yI[k] - yI[N / 2 - k]) / 2;
			x2R[k] = (yI[k] + yI[N / 2 - k]) / 2;
			x2I[k] = (yR[N / 2 - k] - yR[k]) / 2;
		}
	}

	//第M级的蝶形运算
	B = 1;
	B = (int)pow(2, M - 1);
	for (j = 0; j <= B - 1; j++)
	//j = 0,1,2,...,2^(L-1) - 1
	{
		p = 1;
		p = j;
		//数组下标定为r
		r = 1;
		r = j;
		Tr = x2R[r] * cos(2.0 * FFT_PI * p / N) - x2I[r] * sin(2.0 * FFT_PI * p / N);
		Ti = x2I[r] * cos(2.0 * FFT_PI * p / N) + x2R[r] * sin(2.0 * FFT_PI * p / N);
		_inReal[r] = x1R[r] + Tr;
		_inReal[r] = _inReal[r] / 2;
		_inImag[r] = x1I[r] + Ti;
		_inImag[r] = _inImag[r] / 2;

		if (r == 0)
		{
			_inReal[N / 2] = x1R[0] - x2R[0];
			_inReal[N / 2] = _inReal[N / 2] / 2;
			_inImag[N / 2] = x1I[0] - x2I[0];
			_inImag[N / 2] = _inImag[N / 2] / 2;
		}
		else
		{
			_inReal[N - r] = _inReal[r];
			_inImag[N - r] = -_inImag[r];
		}
	}

	//计算幅频
	if (outAF)
	{
		for (i = 0; i < N; i++)
			outAF[i] = sqrt(_inReal[i] * _inReal[i] + _inImag[i] * _inImag[i]) / (N / 2);
	}
	//计算相频
	if (outPF)
	{
		for (i = 0; i < N; i++)
			outPF[i] = atan2(_inImag[i], _inReal[i]);
	}
}
