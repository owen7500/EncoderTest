/* ���ƣ�VPX�������
*  ���ߣ�����
*  ��λ�����ȴ�ѧ
*  ���ܣ�����VP8��VP9��VP10ϵ�б�������YUV��ʽ����Ƶ����ΪIVF��ʽ����Ƶ�ļ�
*  ���룺�������ļ���YUV�����������ļ���(ivf)
*  �����������ivf�ļ�
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

double vpxMain(char* infile_arg, char* outfile_arg, int encoderID, int width, int height, int control_mode, float mode_data, int FPS);
