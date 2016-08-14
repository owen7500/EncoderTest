#include <stdio.h>
#include <string.h>
#include <iostream>
#include <Windows.h>

#include "x264_encoder.h"
#include "x265_encoder.h"
#include "h26x_decoder.h"
#include "libvpx_encoder.h"
#include "libvpx_decoder.h"

static 	char src_path[100] = "E:\\1VideoSequences\\";

void printWelcome();																//welcome tips
int printMenu();																	//print menu
void getInOutFileName(char* in_file,char* out_file);								//get file name 

void changeSrcPath();				//change source path,default is E:\\1VideoSequences\\1080p_1920x1080\\

/*control mode and it's data
*# param1	control_mode		1 indicates ABR/VBR	,2 indicates CQP,3 indicates CRF
*# param2	mode_data			bitrate or qp level or CRF value
*/
void getControlMode(int &control_mode, float &mode_data);							//get control mode and it's data

/*get the frame size(width and height)*/
int getFrameSize(int &width, int &height,int &FPS);

/*the following 8 functions are aim to complete the file name (add path and suffix)*/
int x264encoder_strcat(char* in_file, char* out_file, int control_mode, float mode_data);
int x265encoder_strcat(char* in_file, char* out_file, int control_mode, float mode_data);
int libvp8encoder_strcat(char* in_file, char* out_file, int control_mode, float mode_data);
int libvp9encoder_strcat(char* in_file, char* out_file, int control_mode, float mode_data);
int h264decoder_strcat(char* in_file, char* out_file);
int h265decoder_strcat(char* in_file, char* out_file);
int libvp8decoder_strcat(char* in_file, char* out_file);
int libvp9decoder_strcat(char* in_file, char* out_file);

/**file writer
*#param1 in_file input video file
*#param2 out_file output video file
*#param3 control_mode   encoder bit rate control mode ,if decoder ,set -1
*#param4 mode_data      the data of the control mode,if decoder, set -1
*#param5 cost_time      the process time(encode or decode)
*#return if success,return 0,if failed return minus
*/
int fileWriter(char* in_file,char* out_file,int contol_mode,float mode_data,float cost_time);



int main()
{
	int choice;	//选择
	double ret,ret1;
	char in_file[100];
	char out_file[100];
	char x264_in_file[100], x265_in_file[100], vp8_in_file[100], vp9_in_file[100];
	char x264_out_file[100], x265_out_file[100], vp8_out_file[100], vp9_out_file[100];
	char x264_dst_file[100], x265_dst_file[100], vp8_dst_file[100], vp9_dst_file[100];
	int  control_mode = 0; 
	float mode_data = 0;			//控制方式及数值
	int width, height;			//frame size
	int FPS=0;
	char* in_suffix = NULL;
	char* out_suffix = NULL;
	char* out_path = NULL;
	char ctrlMode[5], modeData[10];
	printWelcome();

CHOICEAPI:
	choice = printMenu();
	switch (choice)
	{
	case 1:
		printf("\nYou selected x264 encoder\n");
		getInOutFileName(in_file,out_file);
		getControlMode(control_mode,mode_data);
		getFrameSize(width, height, FPS);
		x264encoder_strcat(in_file, out_file,control_mode, mode_data);
		ret = x264Main(in_file, out_file, width, height, control_mode, mode_data, FPS);
		if (ret > 0){
			fileWriter(in_file,out_file,control_mode,mode_data,ret);
			printf("succeed!\n");
		}
		system("pause");
		break;
	case 2:
		printf("\nYou selected h264 decoder(by ffmpeg)\n");
		getInOutFileName(in_file,out_file);
		h264decoder_strcat(in_file, out_file);
		ret = h26xdecoderMain(in_file,out_file,0);
		if (ret > 0){
			fileWriter(in_file, out_file, -1,-1 , ret);
			printf("succeed!\n");
		}
		system("pause");
		break;
	case 3:
		printf("\nYou selected x265 encoder\n");
		getInOutFileName(in_file, out_file);
		getControlMode(control_mode,mode_data);
		getFrameSize(width, height, FPS);
		x265encoder_strcat(in_file, out_file, control_mode, mode_data);
		ret = x265Main(in_file, out_file, width, height, control_mode, mode_data, FPS);
		if (ret > 0){
			fileWriter(in_file, out_file, control_mode, mode_data, ret);
			printf("succeed!\n");
		}
		system("pause");
		break;
	case 4:
		printf("\nYou selected h265 decoder(by ffmpeg)\n");
		getInOutFileName(in_file, out_file);
		h265decoder_strcat(in_file, out_file);
		ret = h26xdecoderMain(in_file, out_file, 1);
		if (ret > 0){
			fileWriter(in_file, out_file, -1,-1, ret);
			printf("succeed!\n");
		}
		system("pause");
		break;
	case 5:
		printf("\nYou selected vp8 encoder\n");
		getInOutFileName(in_file, out_file);
		getControlMode(control_mode, mode_data);
		getFrameSize(width, height, FPS);
		libvp8encoder_strcat(in_file, out_file, control_mode, mode_data);
		ret = vpxMain(in_file, out_file, 0, width, height, control_mode, mode_data, FPS);
		if (ret > 0){
			fileWriter(in_file, out_file, control_mode, mode_data, ret);
			printf("succeed!\n");
		}
		system("pause");
		break;
	case 6:
		printf("\nYou selected vp8 decoder\n");
		getInOutFileName(in_file, out_file);
		libvp8decoder_strcat(in_file, out_file);
		ret = libvpx_decoder_main(out_file, out_file);
		if (ret > 0){
			fileWriter(in_file, out_file, -1, -1, ret);
			printf("succeed!\n");
		}
		system("pause");
		break;
	case 7:
		printf("\nYou selected vp9 encoder\n");
		getInOutFileName(in_file, out_file);
		getControlMode(control_mode, mode_data);
		getFrameSize(width, height, FPS);
		libvp9encoder_strcat(in_file, out_file, control_mode, mode_data);
		ret = vpxMain(in_file, out_file, 1, width, height, control_mode, mode_data, FPS);
		if (ret > 0){
			fileWriter(in_file, out_file, control_mode, mode_data, ret);
			printf("succeed!\n");
		}
		system("pause");
		break;
	case 8:
		printf("\nYou selected vp9 decoder\n");
		getInOutFileName(in_file, out_file);
		libvp9decoder_strcat(in_file, out_file);
		ret = libvpx_decoder_main(in_file, out_file);
		if (ret > 0){
			fileWriter(in_file, out_file, -1, -1, ret);
			printf("succeed!\n");
		}
		system("pause");
		break;
	case 9:
		printf("\nYou selected x264,x265,vp8,vp9 encoders\n");
		getInOutFileName(in_file, out_file);
		getControlMode(control_mode, mode_data);
		getFrameSize(width, height, FPS);
		/*************x264 encode and decode ********/
		strcpy(x264_in_file,in_file);
		strcpy(x264_out_file,out_file);
		x264encoder_strcat(x264_in_file, x264_out_file, control_mode, mode_data);				//拼接X264地址
		ret = x264Main(x264_in_file, x264_out_file, width, height, control_mode, mode_data, FPS);		//x264编码
		if (ret > 0){
			fileWriter(in_file, out_file, control_mode, mode_data, ret);
		}
		strcpy(x264_dst_file, in_file);
		strcpy(x264_out_file, out_file);
		sprintf(ctrlMode,"%d",control_mode);
		sprintf(modeData,"%d",(int)mode_data);
		strcat(x264_dst_file,ctrlMode);
		strcat(x264_dst_file,modeData);
		strcat(x264_out_file, ctrlMode);
		strcat(x264_out_file, modeData);
		h264decoder_strcat(x264_out_file, x264_dst_file);				//拼接X264地址
		ret1 = h26xdecoderMain(x264_out_file, x264_dst_file, 0);							//H264解码，使用ffmpeg完成
		if (ret1 > 0){
			fileWriter(in_file, out_file, -1, -1, ret1);
			printf("succeed!\n");
		}
		/*******************************************/

		/*************x265 encode and decode********/
		strcpy(x265_in_file, in_file);
		strcpy(x265_out_file, out_file);
		x265encoder_strcat(x265_in_file, x265_out_file, control_mode, mode_data);							//拼接X265地址
		ret = x265Main(x265_in_file, x265_out_file, width, height, control_mode, mode_data,FPS);		//x265编码
		if (ret > 0){
			fileWriter(in_file, out_file, control_mode, mode_data, ret);
		}
		strcpy(x265_dst_file, in_file);
		strcpy(x265_out_file, out_file);
		sprintf(ctrlMode, "%d", control_mode);
		sprintf(modeData, "%d", (int)mode_data);
		strcat(x265_dst_file, ctrlMode);
		strcat(x265_dst_file, modeData);
		strcat(x265_out_file, ctrlMode);
		strcat(x265_out_file, modeData);
		h265decoder_strcat(x265_out_file, x265_dst_file);							//拼接X265地址
		ret1 = h26xdecoderMain(x265_out_file, x265_dst_file, 1);							//H265解码，使用ffmpeg完成
		if (ret1 > 0){
			fileWriter(in_file, out_file, -1,-1, ret1);
			printf("succeed!\n");
		}
		/*****************************************/

		/*******************vp8 encode and decode *********************/
		strcpy(vp8_in_file, in_file);
		strcpy(vp8_out_file, out_file);
		libvp8encoder_strcat(vp8_in_file, vp8_out_file,control_mode,mode_data);							//拼接vp8地址
		ret = vpxMain(vp8_in_file, vp8_out_file, 0, width, height, control_mode, mode_data, FPS);	//VP8编码
		if (ret > 0){
			fileWriter(in_file, out_file, control_mode, mode_data, ret);
		}
		strcpy(vp8_dst_file, in_file);
		strcpy(vp8_out_file, out_file);
		sprintf(ctrlMode, "%d", control_mode);
		sprintf(modeData, "%d", (int)mode_data);
		strcat(vp8_dst_file, ctrlMode);
		strcat(vp8_dst_file, modeData);
		strcat(vp8_out_file,ctrlMode);
		strcat(vp8_out_file,modeData);
		libvp8decoder_strcat(vp8_out_file, vp8_dst_file);							//拼接vp8地址
		ret1 = libvpx_decoder_main(vp8_out_file, vp8_dst_file);							//libvpx 解码VP8
		if (ret1 > 0){
			fileWriter(in_file, out_file, -1,-1, ret1);
			printf("succeed!\n");
		}
		/************************************************************/

		/******************vp9 encode and decode********************/
		strcpy(vp9_in_file, in_file);
		strcpy(vp9_out_file, out_file);
		libvp9encoder_strcat(vp9_in_file, vp9_out_file,control_mode,mode_data);							//拼接vp9地址
		ret = vpxMain(vp9_in_file, vp9_out_file, 1, width, height, control_mode, mode_data,FPS);	//VP9编码
		if (ret > 0){
			fileWriter(in_file, out_file, control_mode, mode_data, ret);
		}
		strcpy(vp9_dst_file, in_file);
		strcpy(vp9_out_file, out_file);
		sprintf(ctrlMode, "%d", control_mode);
		sprintf(modeData, "%d", (int)mode_data);
		strcat(vp9_dst_file, ctrlMode);
		strcat(vp9_dst_file, modeData);
		strcat(vp9_out_file, ctrlMode);
		strcat(vp9_out_file, modeData);
		libvp9decoder_strcat(vp9_out_file, vp9_dst_file);							//拼接vp9地址
		ret1 = libvpx_decoder_main(vp9_out_file, vp9_dst_file);							//libvpx 解码VP9
		if (ret1 > 0){
			fileWriter(in_file, out_file, -1,-1, ret1);
			printf("succeed!\n");
		}
		/**********************************************************/

		system("pause");
		break;
	case 10:
		changeSrcPath();
		break;
	case 11:
		break;
	case 12:
		exit(1);
		break;
	default:
		printf("\nError choice\nMust between 1 to 12\n");
		system("pause");
		break;
	}
	system("cls");
	goto CHOICEAPI;
	system("pause");
}

void printWelcome()
{
	printf("*******************Welcome Use Encoder Test System*******************\n");
	printf("*********************************************************************\n");
	printf("\tThis system includes four open source encoders-x264,\n\t x265,VP8,VP9.");
	printf("they are the latest copy until 2015\\8\\15,\n\twe are only intend to test which encoder is better\n");
	printf("****************************author:Liu Wen***************************\n");
}

int printMenu()
{
	int choice;
	printf("\n\n");
	printf("********************************Menu**************************************\n");
	printf("  1.H264(encoder)    2.H264(decoder)    3.H265(encoder)    4.H265(decoder)\n");
	printf("  5.vp8(encoder)     6.vp8(decoder)     7.vp9(encoder)     8.vp9(decoder)\n");
	printf("  9.All              10.new src folder  11.return          12.exit\n");
	printf("**************************************************************************\n");
	printf("current source path:%s\n",src_path);
	printf("**************************************************************************\n");
	scanf("%d",&choice);
	fflush(stdin);
	return choice;
}

void getInOutFileName(char* in_file, char* out_file)
{
	printf("Please input the source file name and the destination file name\n");
	printf("source file(no suffix,enter to finish):");
	gets(in_file);
	printf("destination file(no suffix,enter to finish):");
	gets(out_file);
}
int getFrameSize(int &width, int &height,int &FPS)
{
	int chioce;
	printf("*************************frame size*****************************\n");
	printf("\t1.1080P\t\t2.720P\t\t3.4CIF\n\t4.CIF\t\t5.QCIF\t\t6.2160P\n\t7.user_define\n");
	scanf("%d",&chioce);
	fflush(stdin);
	switch (chioce)
	{
	case 1:
		width = 1920;
		height = 1080;
		FPS = 25;
		break;
	case 2:
		width = 1280;
		height = 720;
		FPS = 25;
		break;
	case 3:
		width = 704;
		height = 576;
		FPS = 25;
		break;
	case 4:
		width = 352;
		height = 288;
		FPS = 25;
		break;
	case 5:
		width = 176;
		height = 144;
		FPS = 25;
		break;
	case 6:
		width = 3840;
		height = 2160;
		FPS = 50;
		break;
	case 7:
		printf("width and height must be even number\n");
		printf("width=");
		scanf("%d",&width);
		printf("height=");
		scanf("%d",&height);
		if ((width % 2) || (height % 2))
		{
			printf("Invalid inputs,width and height must be even number\n");
			return -1;
		}
		printf("FPS=");
		scanf("%d",&FPS);
		break;
	default:
		printf("Invalid choice\n");
		return -1;
	}
	fflush(stdin);
	return 0;
}
void getControlMode(int &control_mode, float &mode_data)
{
	printf("*************************control mode****************************\n");
	printf("\t1.ABR\\VBR(平均码率)\t2.CQP\\Q(固定质量)\t3.CRF(固定码率)\n");
	scanf("%d",&control_mode);
	fflush(stdin);
	switch (control_mode)
	{
	case 1:
		printf("bitrate(kbps):");
		scanf("%f", &mode_data);
		break;
	case 2:
		printf("cp level:");
		scanf("%f", &mode_data);
		break;
	case 3:
		printf("crf:");
		scanf("%f", &mode_data);
		break;
	default:
		printf("Invalid choice\n");
		break;
	}
	fflush(stdin);
	printf("\n");
}

int x264encoder_strcat(char* in_file, char* out_file, int control_mode, float mode_data)
{
	char in_suffix[10] = ".yuv";
	char out_suffix[10] = ".h264";
	char out_path[100] = "E:\\2BitrateFile_H264\\";
	char temp[100];
	char ctrMode[5],modeData[10];

	sprintf(ctrMode,"%d",control_mode);
	sprintf(modeData,"%d",(int)mode_data);

	strcpy(temp,src_path);
	strcat(in_file, in_suffix);
	strcat(temp, in_file);
	strcpy(in_file,temp);
	strcat(out_file,ctrMode);
	strcat(out_file,modeData);
	strcat(out_path, out_file);
	strcat(out_path, out_suffix);
	strcpy(out_file,out_path);
	return 0;
}
int x265encoder_strcat(char* in_file, char* out_file, int control_mode, float mode_data)
{
	char in_suffix[10] = ".yuv";
	char out_suffix[10] = ".hevc";
	char out_path[100] = "E:\\2BitrateFile_H265\\";
	char temp[100];
	char ctrMode[5], modeData[10];

	sprintf(ctrMode, "%d", control_mode);
	sprintf(modeData, "%d", (int)mode_data);

	strcpy(temp, src_path);
	strcat(in_file, in_suffix);
	strcat(temp, in_file);
	strcpy(in_file, temp);
	strcat(out_file, ctrMode);
	strcat(out_file, modeData);
	strcat(out_path, out_file);
	strcat(out_path, out_suffix);
	strcpy(out_file, out_path);
	return 0;
}
int libvp8encoder_strcat(char* in_file, char* out_file, int control_mode, float mode_data)
{
	char in_suffix[10] = ".yuv";
	char out_suffix[10] = ".ivf";
	char out_path[100] = "E:\\2BitrateFile_vp8\\";
	char temp[100];
	char ctrMode[5], modeData[10];

	sprintf(ctrMode, "%d", control_mode);
	sprintf(modeData, "%d", (int)mode_data);

	strcpy(temp, src_path);
	strcat(in_file, in_suffix);
	strcat(temp, in_file);
	strcpy(in_file, temp);
	strcat(out_file, ctrMode);
	strcat(out_file, modeData);
	strcat(out_path, out_file);
	strcat(out_path, out_suffix);
	strcpy(out_file, out_path);
	return 0;
}
int libvp9encoder_strcat(char* in_file, char* out_file, int control_mode, float mode_data)
{
	char in_suffix[10] = ".yuv";
	char out_suffix[10] = ".ivf";
	char out_path[100] = "E:\\2BitrateFile_vp9\\";
	char temp[100];
	char ctrMode[5], modeData[10];

	sprintf(ctrMode, "%d", control_mode);
	sprintf(modeData, "%d", (int)mode_data);

	strcpy(temp, src_path);
	strcat(in_file, in_suffix);
	strcat(temp, in_file);
	strcpy(in_file, temp);
	strcat(out_file, ctrMode);
	strcat(out_file, modeData);
	strcat(out_path, out_file);
	strcat(out_path, out_suffix);
	strcpy(out_file, out_path);
	return 0;
}
int h264decoder_strcat(char* in_file, char* out_file)
{
	char in_suffix[10] = ".h264";
	char out_suffix[10] = ".yuv";
	char dst_path[100] = "E:\\3decodedFile_H264\\";
	char in_path[100] = "E:\\2BitrateFile_H264\\";

	strcat(in_file, in_suffix);
	strcat(in_path, in_file);
	strcpy(in_file, in_path);
	strcat(dst_path, out_file);
	strcat(dst_path, out_suffix);
	strcpy(out_file, dst_path);
	return 0;
}
int h265decoder_strcat(char* in_file, char* out_file)
{
	char in_suffix[10] = ".hevc";
	char out_suffix[10] = ".yuv";
	char dst_path[100] = "E:\\3decodedFileh265\\";
	char in_path[100] = "E:\\2BitrateFile_H265\\";

	strcat(in_file, in_suffix);
	strcat(in_path, in_file);
	strcpy(in_file, in_path);
	strcat(dst_path, out_file);
	strcat(dst_path, out_suffix);
	strcpy(out_file, dst_path);
	return 0;
}
int libvp8decoder_strcat(char* in_file, char* out_file)
{
	char in_suffix[10] = ".ivf";
	char out_suffix[10] = ".yuv";
	char dst_path[100] = "E:\\3decodedFile_vp8\\";
	char in_path[100] = "E:\\2BitrateFile_vp8\\";

	strcat(in_file, in_suffix);
	strcat(in_path, in_file);
	strcpy(in_file, in_path);
	strcat(dst_path, out_file);
	strcat(dst_path, out_suffix);
	strcpy(out_file, dst_path);
	return 0;
}
int libvp9decoder_strcat(char* in_file, char* out_file)
{
	char in_suffix[10] = ".ivf";
	char out_suffix[10] = ".yuv";
	char dst_path[100] = "E:\\3decodedFile_vp9\\";
	char in_path[100] = "E:\\2BitrateFile_vp9\\";

	strcat(in_file, in_suffix);
	strcat(in_path, in_file);
	strcpy(in_file, in_path);
	strcat(dst_path, out_file);
	strcat(dst_path, out_suffix);
	strcpy(out_file, dst_path);
	return 0;
}

int fileWriter(char* in_file, char* out_file, int contol_mode, float mode_data, float cost_time)
{
	FILE* dst_file = NULL;
	char temp_name[100];
	strcpy(temp_name,out_file);
	strcat(temp_name, ".edt");
	dst_file = fopen(temp_name, "a");
	if (!dst_file)
	{
		printf("can't open file:%s\n",out_file);
		return -1;
	}
	fprintf(dst_file, "%s\t %s\t %2d %8d %10d\n", in_file, out_file, contol_mode, (int)mode_data, (int)cost_time);
	fclose(dst_file);
	return 0;
}

void changeSrcPath()
{
	char newPath[100];
	printf("new path:");
	gets(newPath);						//get the new path 
	strcpy(src_path,newPath);			//change the path 
}