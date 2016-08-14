/**
* 名称：视频解码器
* 功能：解码h264文件，hevc，vp8，vp9文件
*
* 作者：刘文
* 单位：华侨大学
* 日期：2015年7月28日
*
* 说明：该解码程序采用FFMPEG完成，编码采用X264，X265,libvpx完成
*/

#include <stdio.h>
#include <Windows.h>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
};
#endif
#endif

double h26xdecoderMain(char* in_file, char* out_file, int codecID);