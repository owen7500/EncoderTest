#include "h26x_decoder.h"
#include <Windows.h>

/*h26x decoder function
*#param1 in_file    the name of the video to be decoded
*#param2 out_file	the name of decoded video
*#param3 codecID	codec id ,0 implies H264,1 implies H265
*/
double h26xdecoderMain(char* in_file,char* out_file,int codecID)
{
	AVCodec *pCodec;												//�������CODEC
	AVCodecContext *pCodecCtx = NULL;								//��������������Ļ���
	AVCodecParserContext *pCodecParserCtx = NULL;					//�������������Ļ���

	FILE *fp_in;													//�����ļ�
	FILE *fp_out;													//����ļ�
	AVFrame	*pFrame;												//������ͼ��֡�ռ�
	AVCodecID codec_id;												//������ID
	const char* codecName = NULL;												//���������ƣ���������ӡ

	const int in_buffer_size = 40960;								//��������С
	uint8_t in_buffer[in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE] = { 0 };    //�����뻺��������ռ䣬����ʼ��Ϊ0
	uint8_t *cur_ptr;
	int cur_size;
	AVPacket packet;
	int ret, got_picture;											//ret��ʾ���뺯���ķ���ֵ��got_picture��ʾ�Ƿ����һ֡ͼ��
	double startTime, endTime, costTime;								//��¼�����ʱ

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

	pCodec = avcodec_find_decoder(codec_id);							//Ѱ�Ҷ�ӦID�ı������
	if (!pCodec) {
		printf("Codec not found\n");
		return -1;
	}
	/*��������������Ļ���,����CODEC�ṹ����ʼ��CODECĬ�ϲ���*/
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx){
		printf("Could not allocate video codec context\n");
		return -1;
	}

	pCodecParserCtx = av_parser_init(codec_id);			//��ʼ��AVCodecParserContext
	if (!pCodecParserCtx){
		printf("Could not allocate video parser context\n");
		return -1;
	}

	//if(pCodec->capabilities&CODEC_CAP_TRUNCATED)
	//    pCodecCtx->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */

	//�򿪽�����
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		printf("Could not open codec\n");
		return -1;
	}
	//Input File
	fp_in = fopen(in_file, "rb");              //�������ļ�
	if (!fp_in) {
		printf("Could not open input stream\n");
		return -1;
	}
	//Output File
	fp_out = fopen(out_file, "wb");				//������ļ�
	if (!fp_out) {
		printf("Could not open output YUV file\n");
		return -1;
	}

	startTime = GetTickCount();					//��ȡ��ʼʱ��

	pFrame = av_frame_alloc();					//ΪAVFrame����ռ䣬����ʼ��
	av_init_packet(&packet);					//��ʼ��һ��������������������

	while (1) {

		cur_size = fread(in_buffer, 1, in_buffer_size, fp_in);
		if (cur_size == 0)
			break;
		cur_ptr = in_buffer;

		while (cur_size>0)
		{
			/*�������һ��Packet,ʹ��AVCodecParser��������������з����һ֡һ֡��ѹ����������*/
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

			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);	//��packet�н���һ֡ͼ��,����ɹ�����1�����򷵻�0
			if (ret < 0) {
				printf("Decode Error.\n");
				return ret;
			}
			if (got_picture)
			{//�Ƿ����һ֡ͼ��
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

	while (1){//��ֹ�����ڽ������е�ͼ��û�б���ȫ���룬���½����֡
		/*�������һ��Packet,ʹ��AVCodecParser��������������з����һ֡һ֡��ѹ����������*/
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

	endTime = GetTickCount();				//��ȡ��ֹʱ��
	costTime = endTime - startTime;			//���ĵ���ʱ��
	printf("%s Decoder cost time: %f ms\n", codecName, costTime);
	fclose(fp_in);
	fclose(fp_out);

	av_parser_close(pCodecParserCtx);

	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);

	return costTime;
}