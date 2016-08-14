#include <Windows.h>

#include "libvpx_encoder.h"
#include "vpx/vpx_encoder.h"
#include "vpx/vp8cx.h"
#include "vpx/vpx_codec.h"

#include "tools_common.h"
#include "video_writer.h"

static int encode_frame(vpx_codec_ctx_t *codec,
	vpx_image_t *img,
	int frame_index,
	int flags,
	unsigned  long quality_control,
	VpxVideoWriter *writer);

/*vpx encoder function
*#param1 infile_arg     the name of the video to be encoded
*#param2 outfile_arg    the name of encoded video
*#param3 encoderID      the codec id,0 implies vp8,1 implies vp9
*#param4 width			the width of the frame
*#param5 height			the height of the frame
*#param6 control_mode	bit rate control mode ,1 implies VBR,2 implies CQ,3 implies CBR
*#param7 mode_data      the value of bit rate or cq level
*/
double vpxMain(char* infile_arg, char* outfile_arg,int encoderID,int width,int height,int control_mode,float mode_data,int FPS) 
{
	FILE *infile = NULL;
	vpx_codec_ctx_t codec;
	vpx_codec_enc_cfg_t cfg;
	int frame_count = 0;
	vpx_image_t raw;
	vpx_codec_err_t res;
	VpxVideoInfo info = { 0 };
	VpxVideoWriter *writer = NULL;
	const VpxInterface *encoder = NULL;
	//const int fps = 25;        // TODO(dkovalev) add command line argument
	const int bitrate = 1000;   // kbit/s TODO(dkovalev) add command line argument
	int keyframe_interval = 0;
	double start_time, end_time, cost_time;		//起止时间和总时间
	char* codecID = NULL;						//编码器ID

	int rate_control_flag = control_mode;				//码率控制方式，0表示VPX_VBR, 1表示VPX_CBR,2表示VPX_CQ,3表示VPX_Q
	unsigned long quality_control;			//质量控制，VPX_DL_GOOD_QUALITY，VPX_DL_REALTIME，VPX_DL_BEST_QUALITY
	int control_data;

	//const char* infile_arg = "E:\\1VideoSequences\\1080p_1920x1080\\Ducks_1920x1080.yuv";


	switch (encoderID)
	{
	case 0:
		codecID = "vp8";
		break;
	case 1:
		codecID = "vp9";
		break;
	default:
		printf("error codec id(0,1)\n");
		return -1;
	}
	encoder = get_vpx_encoder_by_name(codecID);			//编码器名称
	if (!encoder)
	{
		printf("Unsupported codec.");
		return -1;
	}

	info.codec_fourcc = encoder->fourcc;
	info.frame_width = width;							//宽
	info.frame_height = height;							//高
	info.time_base.numerator = 1;						//分子
	info.time_base.denominator = FPS;					//帧率
	keyframe_interval = 10;								//关键帧数量


	//判断帧格式是否正确，帧宽帧高必须是双数
	if (info.frame_width <= 0 ||
		info.frame_height <= 0 ||
		(info.frame_width % 2) != 0 ||
		(info.frame_height % 2) != 0) {
		printf("Invalid frame size: %dx%d", info.frame_width, info.frame_height);
		return -1;
	}

	if (!vpx_img_alloc(&raw, VPX_IMG_FMT_I420, info.frame_width,
		info.frame_height, 1)) {
		printf("Failed to allocate image.\n");
		return -1;
	}
	writer = vpx_video_writer_open(outfile_arg, kContainerIVF, &info);
	if (!writer){
		printf("Failed to open %s for writing.", outfile_arg);
		return -1;
	}

	if (!(infile = fopen(infile_arg, "rb"))){
		printf("Failed to open %s for reading.", infile_arg);
		return -1;
	}
	if (keyframe_interval < 0){
		printf("Invalid keyframe interval value.\n");
		return -1;
	}

	printf("Using %s\n", vpx_codec_iface_name(encoder->codec_interface()));

	res = vpx_codec_enc_config_default(encoder->codec_interface(), &cfg, 0);
	if (res)
	{
		printf("Failed to get default codec config.\n");
		return -1;
	}

	cfg.g_w = info.frame_width;
	cfg.g_h = info.frame_height;
//	cfg.g_threads = 1;										//单线程

	//cfg.rc_dropframe_thresh = 0;
	cfg.rc_resize_allowed = 0;
	cfg.rc_min_quantizer = 10;
	cfg.rc_max_quantizer = 51;
	//cfg.rc_undershoot_pct = 100;
	//cfg.rc_overshoot_pct = 15;
	//cfg.rc_buf_initial_sz = 500;
	//cfg.rc_buf_optimal_sz = 600;
	//cfg.rc_buf_sz = 1000;
	//cfg.g_error_resilient = 1;              /* Enable error resilient mode */
	cfg.g_lag_in_frames = 3;
	cfg.kf_mode = VPX_KF_AUTO;
	cfg.kf_min_dist = 10;
	cfg.kf_max_dist = 10;
	//cfg.g_pass = VPX_RC_ONE_PASS;
	cfg.g_timebase.num = info.time_base.numerator;
	cfg.g_timebase.den = info.time_base.denominator;

	switch (rate_control_flag)
	{
	case 1:
		cfg.rc_end_usage = VPX_VBR;
		cfg.rc_target_bitrate = mode_data;
		quality_control = VPX_DL_GOOD_QUALITY;
		control_data = 3;							//CPU速度
		break;
	case 2:
		cfg.rc_end_usage = VPX_Q;
		quality_control = VPX_DL_GOOD_QUALITY;
		control_data = (int)mode_data;					//QP 值
		break;
	case 3:
		cfg.rc_end_usage = VPX_CBR;
		cfg.rc_target_bitrate = mode_data;
		quality_control = VPX_DL_GOOD_QUALITY;
		control_data = 3;							//CPU速度
		break;
	case 4:
		cfg.rc_end_usage = VPX_CQ;
		quality_control = VPX_DL_GOOD_QUALITY;
		control_data = (int)mode_data;							//QP值
		break;
	default:
		printf("invalid control mode(0-3)\n");
		return -1;
	}

	if (vpx_codec_enc_init(&codec, encoder->codec_interface(), &cfg, 0)){
		printf("Failed to initialize encoder");
		return -1;
	}

	switch (rate_control_flag)
	{
	case 0:
		vpx_codec_control(&codec, VP8E_SET_CPUUSED, control_data);
		break;
	case 1:
		vpx_codec_control(&codec, VP8E_SET_CPUUSED, control_data); break;
	case 2:
		vpx_codec_control(&codec, VP8E_SET_CQ_LEVEL, (unsigned int)control_data); break;
	case 3:
		vpx_codec_control(&codec, VP8E_SET_CQ_LEVEL, (unsigned int)control_data); break;
	default:
		vpx_codec_control(&codec, VP8E_SET_CPUUSED, control_data);
		break;
	}

	/*************************************编码开始*********************************/
	/****************************************************************************/

	start_time = GetTickCount();								//起始时间
	// Encode frames.
	while (vpx_img_read(&raw, infile)) {
		int flags = 0;
		if (keyframe_interval > 0 && frame_count % keyframe_interval == 0)
			flags |= VPX_EFLAG_FORCE_KF;
		encode_frame(&codec, &raw, frame_count++, flags, quality_control, writer);
	}

	// Flush encoder.
	while (encode_frame(&codec, NULL, -1, 0, quality_control, writer)) {};

	end_time = GetTickCount();								//结束时间
	cost_time = end_time - start_time;						//总时间

	printf("\n");
	fclose(infile);
	printf("Processed %d frames.\n", frame_count);
	printf("%s cost time:%f ms\n", codecID, cost_time);

	vpx_img_free(&raw);
	if (vpx_codec_destroy(&codec)){
		printf("Failed to destroy codec.");
		return -1;
	}

	vpx_video_writer_close(writer);
	return cost_time;
}

static int encode_frame(vpx_codec_ctx_t *codec,
	vpx_image_t *img,
	int frame_index,
	int flags,
	unsigned  long quality_control,
	VpxVideoWriter *writer) {
	int got_pkts = 0;
	vpx_codec_iter_t iter = NULL;
	const vpx_codec_cx_pkt_t *pkt = NULL;
	const vpx_codec_err_t res = vpx_codec_encode(codec, img, frame_index, 1,
		flags, quality_control);		//VPX编码档次的选择，同时配合cpu-used
	if (res != VPX_CODEC_OK){
		printf("Failed to encode frame\n");
		return -1;
	}

	while ((pkt = vpx_codec_get_cx_data(codec, &iter)) != NULL) {
		got_pkts = 1;

		if (pkt->kind == VPX_CODEC_CX_FRAME_PKT) {
			const int keyframe = (pkt->data.frame.flags & VPX_FRAME_IS_KEY) != 0;
			if (!vpx_video_writer_write_frame(writer,
				(const uint8_t*)pkt->data.frame.buf,
				pkt->data.frame.sz,
				pkt->data.frame.pts)) {
				printf("Failed to write compressed frame");
				return -1;
			}
			printf(keyframe ? "K" : ".");
			fflush(stdout);
		}
	}

	return got_pkts;
}