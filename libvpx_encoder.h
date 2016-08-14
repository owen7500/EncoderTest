/* 名称：VPX编码程序
*  作者：刘文
*  单位：华侨大学
*  功能：采用VP8、VP9、VP10系列编码器将YUV格式的视频编码为IVF格式的视频文件
*  输入：待编码文件（YUV），编码后的文件名(ivf)
*  输出：编码后的ivf文件
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

double vpxMain(char* infile_arg, char* outfile_arg, int encoderID, int width, int height, int control_mode, float mode_data, int FPS);
