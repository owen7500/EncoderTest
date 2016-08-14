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
	//int64_t startTime, endTime;	//�������ʼʱ��
	double startTime, endTime, costTime;

	//char* src_fileName = "E:\\1VideoSequences\\1080p_1920x1080\\Basketball_1920x1080.yuv";           //Դ����YUV�ļ�
	//char* dst_fileName = "E:\\2BitrateFile_H264\\basketball\\Basketbal_avgrate4000.h264";//Ŀ�������ļ�h264
	int frame_num = 0;							//�����֡��,�������Ϊ0����ʾ��������֡
	int csp = X264_CSP_I420;					//I420��ʽ
//	int width = 1920, height = 1080;             //֡����֡��

	int iNal = 0;							//ÿ��inal�����е�ͼ�����
	x264_nal_t* pNals = NULL;				//ѹ�����ͼ����
	x264_t* pHandle = NULL;
	x264_picture_t* pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));
	x264_picture_t* pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));
	x264_param_t* pParam = (x264_param_t*)malloc(sizeof(x264_param_t));


	FILE* fp_src = fopen(src_fileName, "rb");  //��Դ����
	if (!fp_src)
	{
		fprintf(stderr, "could not open %s\n", src_fileName);
		return -1;
	}

	FILE* fp_dst = fopen(dst_fileName, "wb");    //��Ŀ���ļ�
	if (!fp_dst)
	{
		fprintf(stderr, "could not open %s\n", dst_fileName);
		return -1;
	}

	print_version_info();

	x264_param_default(pParam);				//����Ĭ�ϲ���
	//x264_param_default_preset(pParam, "medium", "psnr");	//����preset
	pParam->i_width = width;				//���ñ����֡��
	pParam->i_height = height;				//���ñ���ͼ���֡��

	//Param
	pParam->i_log_level = X264_LOG_INFO;			//��־ģʽ
	//pParam->i_threads = 1;							//ͬʱ����ͼ����
	pParam->i_frame_total = frame_num;				//�ܵ�֡��

	pParam->b_open_gop  = 1;						//�Ƿ�����ͼ�������
	pParam->rc.i_qp_max=51;
	pParam->rc.i_qp_min=10;

	pParam->i_fps_den = 1;
	pParam->i_fps_num = FPS;
	pParam->i_timebase_den = pParam->i_fps_num;		//֡��
	pParam->i_timebase_num = pParam->i_fps_den;

	pParam->i_keyint_max = 10;						  //���ؼ�֡������I֡����Ŀ
	pParam->i_bframe  = 3;							  //�ܵ�B֡��Ŀ
	pParam->i_bframe_pyramid = 1;
	pParam->i_bframe_adaptive = X264_B_ADAPT_FAST;

	//���ʿ��Ʒ�ʽ
	switch (control_mode)
	{
	case 1:
		////ƽ������ģʽ
		pParam->rc.i_rc_method = X264_RC_ABR;			//ƽ������ģʽ
		pParam->rc.i_bitrate = mode_data;				//ƽ������   ��λ��kbps
		//pParam->rc.f_rate_tolerance = 4;				//�������
		break;
	case 2:
		//�̶�QPģʽ
		pParam->rc.i_rc_method = X264_RC_CQP;			//�㶨QP����
		pParam->rc.i_qp_constant = mode_data;				//QP = QPVALUE
		break;
	case 3:
		//�̶�����ģʽ
		pParam->rc.i_rc_method = X264_RC_CRF;			//�㶨����ģʽ
		pParam->rc.f_rf_constant = mode_data;           //CRF value
		pParam->rc.i_bitrate = 4000;					//����
		break;
	default:
		printf("invalid control mode(1-3)\n");
		return -1;
	}

	pParam->i_csp = csp;									//�������ظ�ʽ��I420��
	x264_param_apply_profile(pParam,"high");


	pHandle = x264_encoder_open(pParam);

	//Ϊ��ؽṹ������ڴ�
	x264_picture_init(pPic_out);
	x264_picture_alloc(pPic_in, csp, pParam->i_width, pParam->i_height);

	//ret = x264_encoder_headers(pHandle, &pNals, &iNal);

	y_size = pParam->i_width * pParam->i_height;

	startTime = GetTickCount();	//��ȡ��ǰϵͳʱ��

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
	//pPic_in->i_pts++;			//ʱ���

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

	endTime = GetTickCount();				//��ȡ����ʱ��

	costTime = endTime - startTime;		//�����ʱ

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