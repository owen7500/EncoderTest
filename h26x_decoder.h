/**
* ���ƣ���Ƶ������
* ���ܣ�����h264�ļ���hevc��vp8��vp9�ļ�
*
* ���ߣ�����
* ��λ�����ȴ�ѧ
* ���ڣ�2015��7��28��
*
* ˵�����ý���������FFMPEG��ɣ��������X264��X265,libvpx���
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