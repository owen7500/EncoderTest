#include "h26x_decoder.h"
#include <Windows.h>

/*h26x decoder function
*#param1 in_file    the name of the video to be decoded
*#param2 out_file	the name of decoded video
*#param3 codecID	codec id ,0 implies H264,1 implies H265
*/
double h26xdecoderMain(char* in_file,char* out_file,int codecID)
{
	AVCodec *pCodec;												//编解码器CODEC
	AVCodecContext *pCodecCtx = NULL;								//编解码器的上下文环境
	AVCodecParserContext *pCodecParserCtx = NULL;					//解析器的上下文环境

	FILE *fp_in;													//输入文件
	FILE *fp_out;													//输出文件
	AVFrame	*pFrame;												//解码后的图像帧空间
	AVCodecID codec_id;												//编码器ID
	const char* codecName = NULL;												//编码器名称，用于最后打印

	const int in_buffer_size = 40960;								//缓冲区大小
	uint8_t in_buffer[in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE] = { 0 };    //给输入缓冲区分配空间，并初始化为0
	uint8_t *cur_ptr;
	int cur_size;
	AVPacket packet;
	int ret, got_picture;											//ret表示解码函数的返回值，got_picture表示是否解码一帧图像
	double startTime, endTime, costTime;								//记录解码耗时

	switch (codecID)
	{
	case 0:
		codec_id = AV_CODEC_ID_H264;
		codecName = avcodec_get_name(codec_id);
		break;
	case 1:
		codec_id = AV_CODEC_ID_HEVC;
		codecName = avcodec_get_name(codec_id);
		break;
	default:
		printf("Codec isn't supported!\n");
		return -1;
	}

	av_log_set_level(AV_LOG_DEBUG);
	avcodec_register_all();

	pCodec = avcodec_find_decoder(codec_id);							//寻找对应ID的编解码器
	if (!pCodec) {
		printf("Codec not found\n");
		return -1;
	}
	/*编解码器的上下文环境,分配CODEC结构，初始化CODEC默认参数*/
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx){
		printf("Could not allocate video codec context\n");
		return -1;
	}

	pCodecParserCtx = av_parser_init(codec_id);			//初始化AVCodecParserContext
	if (!pCodecParserCtx){
		printf("Could not allocate video parser context\n");
		return -1;
	}

	//if(pCodec->capabilities&CODEC_CAP_TRUNCATED)
	//    pCodecCtx->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */

	//打开解码器
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		printf("Could not open codec\n");
		return -1;
	}
	//Input File
	fp_in = fopen(in_file, "rb");              //打开输入文件
	if (!fp_in) {
		printf("Could not open input stream\n");
		return -1;
	}
	//Output File
	fp_out = fopen(out_file, "wb");				//打开输出文件
	if (!fp_out) {
		printf("Could not open output YUV file\n");
		return -1;
	}

	startTime = GetTickCount();					//获取起始时间

	pFrame = av_frame_alloc();					//为AVFrame分配空间，并初始化
	av_init_packet(&packet);					//初始化一个解析包，不包含数据

	while (1) {

		cur_size = fread(in_buffer, 1, in_buffer_size, fp_in);
		if (cur_size == 0)
			break;
		cur_ptr = in_buffer;

		while (cur_size>0)
		{
			/*解析获得一个Packet,使用AVCodecParser从输入的数据流中分离出一帧一帧的压缩编码数据*/
			int len = av_parser_parse2(
				pCodecParserCtx, pCodecCtx,
				&packet.data, &packet.size,
				cur_ptr, cur_size,
				AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

			cur_ptr += len;
			cur_size -= len;

			if (packet.size == 0)
				continue;

			//Some Info from AVCodecParserContext
			/*printf("[Packet]Size:%6d\t", packet.size);
			switch (pCodecParserCtx->pict_type){
			case AV_PICTURE_TYPE_I: printf("Type:I\t"); break;
			case AV_PICTURE_TYPE_P: printf("Type:P\t"); break;
			case AV_PICTURE_TYPE_B: printf("Type:B\t"); break;
			default: printf("Type:Other\t"); break;
			}
			printf("Number:%4d\n", pCodecParserCtx->output_picture_number);*/

			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);	//从packet中解码一帧图像,解码成功返回1，否则返回0
			if (ret < 0) {
				printf("Decode Error.\n");
				return ret;
			}
			if (got_picture)
			{//是否解码一帧图像
				int i = 0;
				unsigned char* tempptr = NULL;
				tempptr = pFrame->data[0];
				for (i = 0; i<pFrame->height; i++){
					fwrite(tempptr, 1, pFrame->width, fp_out);     //Y 
					tempptr += pFrame->linesize[0];
				}
				tempptr = pFrame->data[1];
				for (i = 0; i<pFrame->height / 2; i++){
					fwrite(tempptr, 1, pFrame->width / 2, fp_out);   //U
					tempptr += pFrame->linesize[1];
				}
				tempptr = pFrame->data[2];
				for (i = 0; i<pFrame->height / 2; i++){
					fwrite(tempptr, 1, pFrame->width / 2, fp_out);   //V
					tempptr += pFrame->linesize[2];
				}
				/*printf("Succeed to decode 1 frame!\n");*/
			}
		}

	}

	while (1){//防止残留在解码器中的图像没有被完全解码，导致解码掉帧
		/*解析获得一个Packet,使用AVCodecParser从输入的数据流中分离出一帧一帧的压缩编码数据*/
		int len = av_parser_parse2(
			pCodecParserCtx, pCodecCtx,
			&packet.data, &packet.size,
			cur_ptr, cur_size,
			AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
		cur_ptr += len;
		cur_size -= len;

		//if (packet.size == 0)
		//	break;
		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);

		if (ret < 0) {
			printf("Decode Error.\n");
			return ret;
		}
		if (!got_picture){
			break;
		}
		if (got_picture)
		{
			int i = 0;
			unsigned char* tempptr = NULL;
			tempptr = pFrame->data[0];
			for (i = 0; i<pFrame->height; i++){
				fwrite(tempptr, 1, pFrame->width, fp_out);     //Y 
				tempptr += pFrame->linesize[0];
			}
			tempptr = pFrame->data[1];
			for (i = 0; i<pFrame->height / 2; i++){
				fwrite(tempptr, 1, pFrame->width / 2, fp_out);   //U
				tempptr += pFrame->linesize[1];
			}
			tempptr = pFrame->data[2];
			for (i = 0; i<pFrame->height / 2; i++){
				fwrite(tempptr, 1, pFrame->width / 2, fp_out);   //V
				tempptr += pFrame->linesize[2];
			}
			/*printf("Flush Decoder: Succeed to decode 1 frame!\n");*/
		}
	}
	packet.data = NULL;
	packet.size = 0;

	endTime = GetTickCount();				//获取终止时间
	costTime = endTime - startTime;			//消耗的总时间
	printf("%s Decoder cost time: %f ms\n", codecName, costTime);
	fclose(fp_in);
	fclose(fp_out);

	av_parser_close(pCodecParserCtx);

	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);

	return costTime;
}