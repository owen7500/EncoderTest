/**
* ���ƣ�����X265����Ƶ������
*
* ���ߣ� ����
* ��λ�� ���ȴ�ѧ
*
* ���ܣ� ��Ҫ���ڲ���x265�����������ܡ�
* �������ã�������ģʽ���в��ԣ�1.�㶨����ģʽ 2.�̶���������ģʽ
* 1.�㶨����ģʽ
*	��ģʽ�£�ֻ�������²�����֡��֡�ߣ�֡���ͣ�֡����֡�ʣ���־ģʽ��
*  ���ʿ���ģʽ=X265_RC_CRF�����ʣ�bitrate�� = ** kbps�����������Ĭ�ϲ���
*
* 2.�̶���������ģʽ
*  ��ģʽ�£� ֻ�������²�����֡��֡�ߣ�֡���ͣ�֡����֡�ʣ���־ģʽ��
*  ���ʿ���ģʽ=X265_RC_CQP��QP��qp�� = **,���������Ĭ�ϲ���
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