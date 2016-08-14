/**
* 名称：基于X264的视频编码器
*
* 作者： 刘文
* 单位： 华侨大学
* 日期：2015/8/3
*
* 功能： 主要用于测试x264编码器的性能。
* 参数设置：分两种模式进行测试：1.恒定码率模式 2.固定量化参数模式
* 1.恒定码率模式
*	该模式下，只设置以下参数：帧宽，帧高，帧类型，帧数，帧率，日志模式；
*  码率控制模式=X264_RC_CRF，码率（i_bitrate） = ** kbps，其余均采用默认参数
*
* 2.固定量化参数模式
*  该模式下， 只设置以下参数：帧宽，帧高，帧类型，帧数，帧率，日志模式；
*  码率控制模式=X264_RC_CQP，QP（i_qp_constant） = **,其余均采用默认参数
*
*/
#include <stdio.h>
#include <Windows.h>
#include <stdlib.h>

#include "stdint.h"

#if defined ( __cplusplus)
extern "C"
{
#include "x264.h"
};
#else
#include "x264.h"
#endif

double x264Main(char* src_fileName, char* dst_fileName, int width, int height, int control_mode, float mode_data, int FPS);

static void print_version_info(void);