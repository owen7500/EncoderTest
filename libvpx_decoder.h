/* 名称：VPX解码程序
*  作者：刘文
*  单位：华侨大学
*  日期：2015年8月16日
*  功能：解码ivf格式的文件
*  对采用VP8、VP9、VP10系列编码器编码的视频文件进行解码
*  输入：待解码文件（ivf），解码后的文件名(yuv)
*  输出：解码后的YUV文件
*/

#include "vpx/vpx_decoder.h"

double libvpx_decoder_main(char* in_file, char* out_file);