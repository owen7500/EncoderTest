/* 名称：VPX解码程序
*  作者：刘文
*  单位：华侨大学
*  功能：解码ivf格式的文件
*  对采用VP8、VP9、VP10系列编码器编码的视频文件进行解码
*  输入：待解码文件（ivf），解码后的文件名(yuv)
*  输出：解码后的YUV文件
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include "libvpx_decoder.h"

#include "tools_common.h"
#include "video_reader.h"
#include "vpx_config.h"

static const char *exec_name;

void usage_exit(void) {
	fprintf(stderr, "Usage: %s <infile> <outfile>\n", exec_name);
	exit(EXIT_FAILURE);
}

/*vpx decoder function
*#param1 in_file   the name of the video to be decoded
*#param2 out_file	the name of decoded video
*/
double libvpx_decoder_main(char* in_file, char* out_file) {
	int frame_cnt = 0;
	FILE *outfile = NULL;
	vpx_codec_ctx_t codec;
	VpxVideoReader *reader = NULL;
	const VpxInterface *decoder = NULL;
	const VpxVideoInfo *info = NULL;
	//const char* in_file = "E:\\2BitrateFile_H264\\ducks_vp9.ivf";
	//const char* out_file = "E:\\3decodedFile_H264\\ducks_vp9.yuv";
	double start_time, end_time, cost_time;		//起止时间，总时间


	reader = vpx_video_reader_open(in_file);
	if (!reader)
	{
		printf("Failed to open %s for reading.", in_file);
		return -1;
	}
	outfile = fopen(out_file, "wb");
	if (outfile == NULL)
	{
		printf("can not open dst file\n");
		return -1;
	}

	info = vpx_video_reader_get_info(reader);

	decoder = get_vpx_decoder_by_fourcc(info->codec_fourcc);
	if (!decoder)
		die("Unknown input codec.");

	printf("Using %s\n", vpx_codec_iface_name(decoder->codec_interface()));

	if (vpx_codec_dec_init(&codec, decoder->codec_interface(), NULL, 0))
		die_codec(&codec, "Failed to initialize decoder.");

	start_time = GetTickCount();					//起始时间
	while (vpx_video_reader_read_frame(reader)) {
		vpx_codec_iter_t iter = NULL;
		vpx_image_t *img = NULL;
		size_t frame_size = 0;
		const unsigned char *frame = vpx_video_reader_get_frame(reader,
			&frame_size);
		if (vpx_codec_decode(&codec, frame, (unsigned int)frame_size, NULL, 0))
			die_codec(&codec, "Failed to decode frame.");

		while ((img = vpx_codec_get_frame(&codec, &iter)) != NULL) {
			vpx_img_write(img, outfile);
			++frame_cnt;
		}
	}
	end_time = GetTickCount();						//结束时间
	cost_time = end_time - start_time;

	printf("Processed %d frames.\n", frame_cnt);
	if (vpx_codec_destroy(&codec))
		die_codec(&codec, "Failed to destroy codec");

	printf("Play: ffplay -f rawvideo -pix_fmt yuv420p -s %dx%d %s\n",
		info->frame_width, info->frame_height, out_file);
	printf("%s \ncost time:%f ms\n",vpx_codec_iface_name(decoder->codec_interface()),cost_time);
	vpx_video_reader_close(reader);

	fclose(outfile);
	return cost_time;
}
