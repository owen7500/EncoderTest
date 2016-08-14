/**
* 名称：基于X265的视频编码器
*
* 作者： 刘文
* 单位： 华侨大学
*
* 功能： 主要用于测试x265编码器的性能。
* 参数设置：分两种模式进行测试：1.恒定码率模式 2.固定量化参数模式
* 1.恒定码率模式
*	该模式下，只设置以下参数：帧宽，帧高，帧类型，帧数，帧率，日志模式；
*  码率控制模式=X265_RC_CRF，码率（bitrate） = ** kbps，其余均采用默认参数
*
* 2.固定量化参数模式
*  该模式下， 只设置以下参数：帧宽，帧高，帧类型，帧数，帧率，日志模式；
*  码率控制模式=X265_RC_CQP，QP（qp） = **,其余均采用默认参数
*
*/

#include <stdio.h>
#include <Windows.h>
#include <stdlib.h>

#if defined ( __cplusplus)
extern "C"
{
#include "x265.h"
};
#else
#include "x265.h"
#endif

double x265Main(char* src_fileName, char* dst_fileName, int width, int height, int control_mode, float mode_data, int FPS);