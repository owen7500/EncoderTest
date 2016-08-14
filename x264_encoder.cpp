#include "x264_encoder.h"
#include <Windows.h>

/*x264 encoder function
*#param1 src_fileName   the name of the video to be encoded
*#param2 dst_fileName	the name of encoded video
*#param3 width			the width of the frame
*#param4 height			the height of the frame
*#param5 control_mode	bit rate control mode ,1 implies ABR,2 implies CQP,3 implies CRF
*#param6 mode_data      the value of bit rate or cq level
*/
double x264Main(char* src_fileName, char* dst_fileName,int width,int height, int control_mode, float mode_data,int FPS)
{

	int ret;
	int y_size;
	int i, j;
	//int64_t startTime, endTime;	//编码的起始时间
	double startTime, endTime, costTime;

	//char* src_fileName = "E:\\1VideoSequences\\1080p_1920x1080\\Basketball_1920x1080.yuv";           //源数据YUV文件
	//char* dst_fileName = "E:\\2BitrateFile_H264\\basketball\\Basketbal_avgrate4000.h264";//目的码流文件h264
	int frame_num = 0;							//编码的帧数,如果设置为0，表示编码所有帧
	int csp = X264_CSP_I420;					//I420格式
//	int width = 1920, height = 1080;             //帧宽与帧高

	int iNal = 0;							//每个inal包含有的图像个数
	x264_nal_t* pNals = NULL;				//压缩后的图像流
	x264_t* pHandle = NULL;
	x264_picture_t* pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));
	x264_picture_t* pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));
	x264_param_t* pParam = (x264_param_t*)malloc(sizeof(x264_param_t));


	FILE* fp_src = fopen(src_fileName, "rb");  //打开源数据
	if (!fp_src)
	{
		fprintf(stderr, "could not open %s\n", src_fileName);
		return -1;
	}

	FILE* fp_dst = fopen(dst_fileName, "wb");    //打开目的文件
	if (!fp_dst)
	{
		fprintf(stderr, "could not open %s\n", dst_fileName);
		return -1;
	}

	print_version_info();

	x264_param_default(pParam);				//设置默认参数
	//x264_param_default_preset(pParam, "medium", "psnr");	//设置preset
	pParam->i_width = width;				//设置编码的帧高
	pParam->i_height = height;				//设置编码图像的帧宽

	//Param
	pParam->i_log_level = X264_LOG_INFO;			//日志模式
	//pParam->i_threads = 1;							//同时编码图像数
	pParam->i_frame_total = frame_num;				//总的帧数

	pParam->b_open_gop  = 1;						//是否允许图像组编码
	pParam->rc.i_qp_max=51;
	pParam->rc.i_qp_min=10;

	pParam->i_fps_den = 1;
	pParam->i_fps_num = FPS;
	pParam->i_timebase_den = pParam->i_fps_num;		//帧率
	pParam->i_timebase_num = pParam->i_fps_den;

	pParam->i_keyint_max = 10;						  //最大关键帧数，即I帧的数目
	pParam->i_bframe  = 3;							  //总的B帧数目
	pParam->i_bframe_pyramid = 1;
	pParam->i_bframe_adaptive = X264_B_ADAPT_FAST;

	//码率控制方式
	switch (control_mode)
	{
	case 1:
		////平均码率模式
		pParam->rc.i_rc_method = X264_RC_ABR;			//平均码率模式
		pParam->rc.i_bitrate = mode_data;				//平均码率   单位：kbps
		//pParam->rc.f_rate_tolerance = 4;				//允许误差
		break;
	case 2:
		//固定QP模式
		pParam->rc.i_rc_method = X264_RC_CQP;			//恒定QP编码
		pParam->rc.i_qp_constant = mode_data;				//QP = QPVALUE
		break;
	case 3:
		//固定码率模式
		pParam->rc.i_rc_method = X264_RC_CRF;			//恒定码率模式
		pParam->rc.f_rf_constant = mode_data;           //CRF value
		pParam->rc.i_bitrate = 4000;					//码率
		break;
	default:
		printf("invalid control mode(1-3)\n");
		return -1;
	}

	pParam->i_csp = csp;									//设置像素格式（I420）
	x264_param_apply_profile(pParam,"high");


	pHandle = x264_encoder_open(pParam);

	//为相关结构体分配内存
	x264_picture_init(pPic_out);
	x264_picture_alloc(pPic_in, csp, pParam->i_width, pParam->i_height);

	//ret = x264_encoder_headers(pHandle, &pNals, &iNal);

	y_size = pParam->i_width * pParam->i_height;

	startTime = GetTickCount();	//获取当前系统时间

	//detect frame number
	if (frame_num == 0){
		_fseeki64(fp_src, 0, SEEK_END);
		switch (csp){
		case X264_CSP_I444:frame_num = _ftelli64(fp_src) / (y_size * 3); break;
		case X264_CSP_I420:frame_num = _ftelli64(fp_src) / (y_size * 3 / 2); break;
		default:printf("Colorspace Not Support.\n"); return -1;
		}
		printf("frame number: %d\n", frame_num);
		_fseeki64(fp_src, 0, SEEK_SET);
	}

	//Loop to Encode
	for (i = 0; i<frame_num; i++){
		switch (csp){
		case X264_CSP_I444:{
							   fread(pPic_in->img.plane[0], y_size, 1, fp_src);	//Y
							   fread(pPic_in->img.plane[1], y_size, 1, fp_src);	//U
							   fread(pPic_in->img.plane[2], y_size, 1, fp_src);	//V
							   break; }
		case X264_CSP_I420:{
							   fread(pPic_in->img.plane[0], y_size, 1, fp_src);	//Y
							   fread(pPic_in->img.plane[1], y_size / 4, 1, fp_src);	//U
							   fread(pPic_in->img.plane[2], y_size / 4, 1, fp_src);	//V
							   break; }
		default:{
					printf("Colorspace Not Support.\n");
					return -1; }
		}
		pPic_in->i_pts = i;
		Sleep(500);

		ret = x264_encoder_encode(pHandle, &pNals, &iNal, pPic_in, pPic_out);
		if (ret< 0){
			printf("Error.\n");
			return -1;
		}

		printf("Succeed encode frame: %5d\n", pPic_in->i_pts);

		for (j = 0; j < iNal; ++j){
			fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_dst);
		}
	}
	//i=0;
	//pPic_in->i_pts++;			//时间戳

	//flush encoder
	while (1){
		ret = x264_encoder_encode(pHandle, &pNals, &iNal, NULL, pPic_out);
		if (ret == 0){
			break;
		}
		printf("Flush 1 frame.\n");
		for (j = 0; j < iNal; ++j){
			fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_dst);
		}
		i++;
	}

	endTime = GetTickCount();				//获取结束时间

	costTime = endTime - startTime;		//计算耗时

	x264_picture_clean(pPic_in);
	x264_encoder_close(pHandle);
	pHandle = NULL;

	free(pPic_in);
	free(pPic_out);
	free(pParam);

	fclose(fp_src);
	fclose(fp_dst);

	printf("x264 encoder cost time :%f ms\n", costTime);
	return costTime;
}

static void print_version_info(void)
{
#ifdef X264_POINTVER
	printf("x264 "X264_POINTVER"\n");
#else
	printf("x264 0.%d.X\n", X264_BUILD);
#endif
#if HAVE_SWSCALE
	printf("(libswscale %d.%d.%d)\n", LIBSWSCALE_VERSION_MAJOR, LIBSWSCALE_VERSION_MINOR, LIBSWSCALE_VERSION_MICRO);
#endif
#if HAVE_LAVF
	printf("(libavformat %d.%d.%d)\n", LIBAVFORMAT_VERSION_MAJOR, LIBAVFORMAT_VERSION_MINOR, LIBAVFORMAT_VERSION_MICRO);
#endif
#if HAVE_FFMS
	printf("(ffmpegsource %d.%d.%d.%d)\n", FFMS_VERSION >> 24, (FFMS_VERSION & 0xff0000) >> 16, (FFMS_VERSION & 0xff00) >> 8, FFMS_VERSION & 0xff);
#endif
	printf("built on " __DATE__ ", ");
#ifdef __INTEL_COMPILER
	printf("intel: %.2f (%d)\n", __INTEL_COMPILER / 100.f, __INTEL_COMPILER_BUILD_DATE);
#elif defined(__GNUC__)
	printf("gcc: " __VERSION__ "\n");
#elif defined(_MSC_FULL_VER)
	printf("msvc: %.2f (%u)\n", _MSC_VER / 100.f, _MSC_FULL_VER);
#else
	printf("using an unknown compiler\n");
#endif
	//printf("x264 configuration: --bit-depth=%d --chroma-format=%s\n", X264_BIT_DEPTH, chroma_format_names[X264_CHROMA_FORMAT]);
	//printf("libx264 configuration: --bit-depth=%d --chroma-format=%s\n", x264_bit_depth, chroma_format_names[x264_chroma_format]);
	//printf("x264 license: ");
#if HAVE_GPL
	printf("GPL version 2 or later\n");
#else
	printf("Non-GPL commercial\n");
#endif
#if HAVE_SWSCALE
	const char *license = swscale_license();
	printf("libswscale%s%s license: %s\n", HAVE_LAVF ? "/libavformat" : "", HAVE_FFMS ? "/ffmpegsource" : "", license);
	if (!strcmp(license, "nonfree and unredistributable") ||
		(!HAVE_GPL && (!strcmp(license, "GPL version 2 or later")
		|| !strcmp(license, "GPL version 3 or later"))))
		printf("WARNING: This binary is unredistributable!\n");
#endif
}