/**
* ���ƣ�����X264����Ƶ������
*
* ���ߣ� ����
* ��λ�� ���ȴ�ѧ
* ���ڣ�2015/8/3
*
* ���ܣ� ��Ҫ���ڲ���x264�����������ܡ�
* �������ã�������ģʽ���в��ԣ�1.�㶨����ģʽ 2.�̶���������ģʽ
* 1.�㶨����ģʽ
*	��ģʽ�£�ֻ�������²�����֡��֡�ߣ�֡���ͣ�֡����֡�ʣ���־ģʽ��
*  ���ʿ���ģʽ=X264_RC_CRF�����ʣ�i_bitrate�� = ** kbps�����������Ĭ�ϲ���
*
* 2.�̶���������ģʽ
*  ��ģʽ�£� ֻ�������²�����֡��֡�ߣ�֡���ͣ�֡����֡�ʣ���־ģʽ��
*  ���ʿ���ģʽ=X264_RC_CQP��QP��i_qp_constant�� = **,���������Ĭ�ϲ���
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