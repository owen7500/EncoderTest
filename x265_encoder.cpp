#include "x265_encoder.h"
#include <Windows.h>
/*x265 encoder function
*#param1 src_fileName   the name of the video to be encoded
*#param2 dst_fileName	the name of encoded video
*#param3 control_mode	bit rate control mode ,1 implies ABR,2 implies CQP,3 implies CRF
*#param4 mode_data      the value of bit rate or cq level
*/
//#define __USE_FILE_OFFSET64
//#define _LARGEFILE64_SOURCE
//#define _FILE_OFFSET_BITS 64


double x265Main(char* src_fileName, char* dst_fileName,int width,int height, int control_mode, float mode_data,int FPS)
{
	int i, j;
	FILE *fp_src = NULL;
	FILE *fp_dst = NULL;
	int y_size;
	int buff_size;
	char *buff = NULL;
	int ret;
	x265_nal *pNals = NULL;
	uint32_t iNal = 0;
	double startTime, endTime, costTime;	//编码的起始时间

	x265_param* pParam = NULL;
	x265_encoder* pHandle = NULL;
	x265_picture *pPic_in = NULL;

	//Encode 250 frames,if set 0, encode all frames
	int frame_num = 0;
	int csp = X265_CSP_I420;
	//int FPS = 0;
	//int width = 1920, height = 1080;

	//打开源文件
	fp_src = fopen(src_fileName, "rb");
	if (fp_src == NULL)//检测
	{
		fprintf(fp_src, "Could not open source file:%s", src_fileName);
		return -1;
	}
	//打开目的文件
	fp_dst = fopen(dst_fileName, "wb");
	if (fp_dst == NULL){//检测
		fprintf(fp_dst, "Could not open source file:%s", dst_fileName);
		fclose(fp_src);
		return -1;
	}

	pParam = x265_param_alloc();
	//x265_param_default(pParam);
	x265_param_default_preset(pParam, "medium", NULL);		//预设参数设置,预设包含：ultrafast,veryfast,superfast,faster,fast,medium,slow,slower,veryslow

	pParam->totalFrames = frame_num;
	pParam->internalCsp = csp;				//帧格式 I420
	pParam->bRepeatHeaders = 1;				//write sps,pps before keyframe
	pParam->sourceWidth = width;			//帧宽
	pParam->sourceHeight = height;			//帧高
	pParam->fpsNum = FPS;					//帧率
	pParam->fpsDenom = 1;

	pParam->logLevel = X265_LOG_INFO;		//日志级别
	pParam->keyframeMax = 10;				//最大关键帧
	pParam->bOpenGOP = 1;
	//pParam->frameNumThreads = 1;			//并发编码帧数
	pParam->bFrameAdaptive = 1;
	pParam->bframes = 3;
	pParam->bBPyramid = 1;

	//码率控制模式 X265_RC_METHODS
	switch (control_mode)
	{
	case 1:
		////平均码率模式
		pParam->rc.rateControlMode = X265_RC_ABR;			//平均码率模式
		pParam->rc.bitrate = mode_data;						//平均码率   单位：kbps
		break;
	case 2:
		//固定QP模式
		pParam->rc.rateControlMode = X265_RC_CQP;			//恒定QP编码
		pParam->rc.qp = mode_data;							//QP = QPVALUE
		break;
	case 3:
		//固定码率模式
		pParam->rc.rateControlMode = X265_RC_CRF;			//恒定码率模式
		pParam->rc.rfConstant = mode_data;                  //恒定码率模式
		break;
		//pParam->rc.bStrictCbr = 1;
	default:
		printf("invalid control mode(1-3)\n");
		x265_param_free(pParam);
		fclose(fp_src);
		fclose(fp_dst);
		return -1;
	}

	//Init
	pHandle = x265_encoder_open(pParam);
	if (pHandle == NULL){
		printf("x265_encoder_open err\n");
		x265_param_free(pParam);
		fclose(fp_src);
		fclose(fp_dst);
		return 0;
	}

	y_size = pParam->sourceWidth * pParam->sourceHeight;

	pPic_in = x265_picture_alloc();
	x265_picture_init(pParam, pPic_in);
	switch (csp){
	case X265_CSP_I444:{
						   buff = (char *)malloc(y_size * 3);
						   pPic_in->planes[0] = buff;
						   pPic_in->planes[1] = buff + y_size;
						   pPic_in->planes[2] = buff + y_size * 2;
						   pPic_in->stride[0] = width;
						   pPic_in->stride[1] = width;
						   pPic_in->stride[2] = width;
						   break;
	}
	case X265_CSP_I420:{
						   buff = (char *)malloc(y_size * 3 / 2);
						   pPic_in->planes[0] = buff;
						   pPic_in->planes[1] = buff + y_size;
						   pPic_in->planes[2] = buff + y_size * 5 / 4;
						   pPic_in->stride[0] = width;
						   pPic_in->stride[1] = width / 2;
						   pPic_in->stride[2] = width / 2;
						   break;
	}
	default:{
				printf("Colorspace Not Support.\n");
				x265_encoder_close(pHandle);
				x265_picture_free(pPic_in);
				x265_param_free(pParam);
				fclose(fp_src);
				fclose(fp_dst);
				return -1;
	}
	}

	//detect frame number
	if (frame_num == 0){
		fseek(fp_src, 0, SEEK_SET);
		//fseek(fp_src,0L,SEEK_END);
		_fseeki64(fp_src, 0, SEEK_END);
		switch (csp){
		case X265_CSP_I444:frame_num = _ftelli64(fp_src) / (y_size * 3); break;
		case X265_CSP_I420:frame_num = _ftelli64(fp_src) / (y_size * 3 / 2); break;
		default:printf("Colorspace Not Support.\n"); return -1;
		}
		_fseeki64(fp_src, 0, SEEK_SET);
	}

	/**************************编码开始*******************************/
	/***************************************************************/
	startTime = GetTickCount();			//获取起始时间
	//Loop to Encode
	for (i = 0; i<frame_num; i++){
		switch (csp){
		case X265_CSP_I444:{
							   fread(pPic_in->planes[0], 1, y_size, fp_src);		//Y
							   fread(pPic_in->planes[1], 1, y_size, fp_src);		//U
							   fread(pPic_in->planes[2], 1, y_size, fp_src);		//V
							   break; }
		case X265_CSP_I420:{
							   fread(pPic_in->planes[0], 1, y_size, fp_src);		//Y
							   fread(pPic_in->planes[1], 1, y_size / 4, fp_src);	//U
							   fread(pPic_in->planes[2], 1, y_size / 4, fp_src);	//V
							   break; }
		default:{
					printf("Colorspace Not Support.\n");
					x265_encoder_close(pHandle);
					x265_picture_free(pPic_in);
					x265_param_free(pParam);
					free(buff);
					fclose(fp_src);
					fclose(fp_dst);
					return -1; }
		}

		ret = x265_encoder_encode(pHandle, &pNals, &iNal, pPic_in, NULL);

		printf("Succeed encode %5d frames\n", i);

		for (j = 0; j<iNal; j++){
			fwrite(pNals[j].payload, 1, pNals[j].sizeBytes, fp_dst);
		}
	}
	//Flush Decoder
	while (1){
		ret = x265_encoder_encode(pHandle, &pNals, &iNal, NULL, NULL);
		if (ret == 0){
			break;
		}
		printf("Flush 1 frame.\n");

		for (j = 0; j<iNal; j++){
			fwrite(pNals[j].payload, 1, pNals[j].sizeBytes, fp_dst);
		}
	}
	endTime = GetTickCount();			//获取结束时间
	costTime = endTime - startTime;
	x265_encoder_close(pHandle);
	x265_picture_free(pPic_in);
	x265_param_free(pParam);
	free(buff);
	fclose(fp_src);
	fclose(fp_dst);

	printf("x265 encoder cost time: %f ms\n", costTime);
	return costTime;
}
