#include "FrameStorage.h"
#include <iostream>

FrameStorage::FrameStorage() {
	formatContext = NULL;
	prevFrameIndex = 0;
}

int64_t FrameStorage::frameToPts(int frame) {
	AVStream* stream = formatContext->streams[videoStream];
	return (int64_t(frame) * stream->r_frame_rate.den * stream-> time_base.den)/(int64_t(stream->r_frame_rate.num) * stream->time_base.num);
}

int FrameStorage::getNumFrames() {
	if (formatContext->streams[videoStream]->nb_frames != 0) {
		return formatContext->streams[videoStream]->nb_frames;
	}
	else {
		int dur = formatContext->duration / AV_TIME_BASE;
		int videoFPS = av_q2d(formatContext->streams[videoStream]->r_frame_rate);
		return dur * videoFPS;
	}
}

void FrameStorage::getFrameByIndex(int frameIndex, cv::Mat& cvFrame) {
	//avcodec_flush_buffers(codecContext);
	//char errStr[100];

	std::cout << "Getting " + std::to_string(frameIndex) + " frame\n";
	if (!formatContext) {
		fprintf(stderr, "ffmpeg: empty format_context\n");
		return;
	}

	int64_t seekTarget = frameToPts(frameIndex);
	std::cout << "want " << seekTarget << std::endl;
	int64_t curSeekTarget;

	//av_seek_frame fails if we want to seek to last 10-20 frame. 
	//In this case we seek to 50 frames before and then decode untill we get the right frame.
	int frameCount = getNumFrames();
	if (frameIndex > frameCount - 50){
		curSeekTarget = frameToPts(MAX(0, frameCount - 50));
	}
	else {
		curSeekTarget = seekTarget;
	}

	int err = 0;
	/*
	if (prevFrameIndex <= frameIndex) {
		err = av_seek_frame(formatContext, videoStream, curSeekTarget, 0);
	}
	else {
		err = av_seek_frame(formatContext, videoStream, curSeekTarget, AVSEEK_FLAG_BACKWARD);
	}	
	*/

	if (err < 0) {
		//printf("%s", av_strerror(err, errStr, 100));
		fprintf(stderr, "ffmpeg: av_seek_frame failed");
		return;
	}
	prevFrameIndex = frameIndex;

	//seek until we have packet before target pts
	while (true) {
		bool firstFrame = true;
		//decode until we have packet after target pts
		while (av_read_frame(formatContext, packet) >= 0) {
			if (packet->stream_index == videoStream) {
				// Video stream packet
				int frame_finished;
				// Decode AVPacket
				if (avcodec_decode_video2(codecContext, frame, &frame_finished, packet) < 0) {
					fprintf(stderr, "ffmpeg: avcodec_decode_video2 failed\n");
					return;
				}

				// frame_finished is positive if AVFrame completely decoded. AVFrame can be stored in multiple AVPackets
				if (frame_finished) {
					if (firstFrame && frame->pts > seekTarget) {
						break;
					}
					if (firstFrame) {
						firstFrame = false;
					}
					if (frame->pts < seekTarget) {
						continue;
					}
					std::cout << "got frame " << frame->pts << std::endl;
					if (sws_scale(imgConvertContext, frame->data, frame->linesize, 0, codecContext->height, framergb->data, framergb->linesize) < 0) {
						fprintf(stderr, "ffmpeg: sws_scale failed\n");
						return;
					}

					memcpy(cvFrame.data, framergb->data[0], 3 * frame->width * frame->height);

					return;
				}
			}
		}

		if (frameIndex == 0) {
			fprintf(stderr, "ffmpeg: frameIndex alredy zero\n");
			return;
		}

		frameIndex -= 50;
		if (frameIndex < 0) {
			frameIndex = 0;
		}
		curSeekTarget = frameToPts(frameIndex);
		err = av_seek_frame(formatContext, videoStream, curSeekTarget, AVSEEK_FLAG_BACKWARD);
		if (err < 0) {
			fprintf(stderr, "ffmpeg: av_seek_frame failed\n");
			return;
		}
	}
}

void FrameStorage::getFrameByIndexStable(int frameIndex, cv::Mat& cvFrame) {
	//avcodec_flush_buffers(codecContext);
	//char errStr[100];
	std::cout << "Getting " + std::to_string(frameIndex) + " frame\n";
	if (!formatContext) {
		fprintf(stderr, "ffmpeg: empty format_context\n");
		return;
	}

	int err = av_seek_frame(formatContext, videoStream, 0, AVSEEK_FLAG_BACKWARD);
	if (err < 0) {
		fprintf(stderr, "ffmpeg: av_seek_frame failed");
		return;
	}

	int curFrame = 0;
	int frame_finished;
	while(curFrame < frameIndex) {
		if (av_read_frame(formatContext, packet) < 0) {
			fprintf(stderr, "ffmpeg: av_read_frame failed");
			return;
		}
		if (packet->stream_index == videoStream) {
			if (frameIndex - curFrame < 100) {
				if (avcodec_decode_video2(codecContext, frame, &frame_finished, packet) < 0) {
					fprintf(stderr, "ffmpeg: avcodec_decode_video2 failed\n");
					return;
				}
			}
			curFrame++;
		}
	}
	if (sws_scale(imgConvertContext, frame->data, frame->linesize, 0, codecContext->height, framergb->data, framergb->linesize) < 0) {
		fprintf(stderr, "ffmpeg: sws_scale failed\n");
		return;
	}
	memcpy(cvFrame.data, framergb->data[0], 3 * frame->width * frame->height);
	return;
}

void FrameStorage::storeFrame(cv::Mat& cvFrame, int64_t pts) {
	if (sws_scale(imgConvertContext, &cvFrame.data, rgbLinesize, 0, codecContext->height, frame->data, frame->linesize) < 0) {
		fprintf(stderr, "ffmpeg: sws_scale failed\n");
	}
	frame->pts = pts;

	int ret;
	ret = avcodec_send_frame(codecContext, frame);
	if (ret < 0) {
		fprintf(stderr, "Error sending a frame for encoding\n");
		exit(1);
	}

	while (ret >= 0) {
		ret = avcodec_receive_packet(codecContext, packet);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0) {
			fprintf(stderr, "Error during encoding\n");
			exit(1);
		}

  		fwrite(packet->data, 1, packet->size, file);
		av_packet_unref(packet);
	}
}

void FrameStorage::storeTheRest() {
	int ret;
	ret = avcodec_send_frame(codecContext, NULL);
	if (ret < 0) {
		fprintf(stderr, "Error sending a frame for encoding\n");
		exit(1);
	}

	while (ret >= 0) {
		ret = avcodec_receive_packet(codecContext, packet);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0) {
			fprintf(stderr, "Error during encoding\n");
			exit(1);
		}

		fwrite(packet->data, 1, packet->size, file);
		av_packet_unref(packet);
	}
}

void FrameStorage::openForRead(std::string& filename) {
	packet = av_packet_alloc();
	if (!packet)
		return;

	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		return;
	}

	framergb = av_frame_alloc();
	if (!framergb) {
		fprintf(stderr, "Could not allocate video framergb\n");
		return;
	}

	// Read file header and store info in AVFormatContext struct
	err = avformat_open_input(&formatContext, filename.c_str(), NULL, NULL);
	if (err < 0) {
		fprintf(stderr, "ffmpeg: Unable to open input file\n");
		return;
	}

	// Now we have existing streams in format_context->streams and number of streams format_context->nb_streams
	err = avformat_find_stream_info(formatContext, NULL);
	if (err < 0) {
		fprintf(stderr, "ffmpeg: Unable to find stream info\n");
		return;
	}

	// Print information about file and all streams onto standard error
	av_dump_format(formatContext, 0, filename.c_str(), 0);

	// Find the first video stream
	for (videoStream = 0; videoStream < formatContext->nb_streams; ++videoStream) {
		if (formatContext->streams[videoStream]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			break;
		}
	}
	if (videoStream == formatContext->nb_streams) {
		fprintf(stderr, "ffmpeg: Unable to find video stream\n");
		return;
	}

	codecContext = formatContext->streams[videoStream]->codec;
	codec = avcodec_find_decoder(codecContext->codec_id);
	// Open codec
	err = avcodec_open2(codecContext, codec, NULL);
	if (err < 0) {
		fprintf(stderr, "ffmpeg: Unable to open codec\n");
		return;
	}

	imgConvertContext = sws_getCachedContext(NULL, codecContext->width, codecContext->height, codecContext->pix_fmt,
		codecContext->width, codecContext->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
	if (imgConvertContext == NULL) {
		fprintf(stderr, "Cannot initialize the conversion context\n");
		return;
	}

	int bytes = avpicture_get_size(AV_PIX_FMT_BGR24, codecContext->width, codecContext->height);
	uint8_t* buffer = (uint8_t *)av_malloc(bytes * sizeof(uint8_t));
	avpicture_fill((AVPicture *)framergb, buffer, AV_PIX_FMT_BGR24, codecContext->width, codecContext->height);	

	timeBase = (int64_t(codecContext->time_base.num) * AV_TIME_BASE) / int64_t(codecContext->time_base.den);
}

void FrameStorage::openForWrite(std::string& filename, int width, int height, CompressionType ctype) {
	rgbLinesize[0] = 3 * width;
	for (int i = 1; i < 8; i++) {
		rgbLinesize[i] = 0;
	}

	codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec) {
		fprintf(stderr, "codec not found\n");
		return;
	}

	codecContext = avcodec_alloc_context3(codec);
	if (!codecContext) {
		fprintf(stderr, "Could not allocate video codec context\n");
		return;
	}

	packet = av_packet_alloc();
	if (!packet)
		exit(1);

	codecContext->width = width;
	codecContext->height = height;
	codecContext->time_base.den = 30;
	codecContext->time_base.num = 1;
	codecContext->framerate.den = 30;
	codecContext->framerate.num = 1;
	codecContext->gop_size = 10;
	codecContext->max_b_frames = 0;
	codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
	//av_opt_set(codec_context->priv_data, "preset", "slow", 0);
	AVDictionary *param = NULL;

	if (ctype == lossless) {
		av_dict_set(&param, "crf", "0", 0);
	}
	else {
		av_dict_set(&param, "crf", "18", 0);
	}

	if (avcodec_open2(codecContext, codec, &param) < 0) {
		fprintf(stderr, "Could not open codec: %s\n");
		return;
	}

	file = fopen(filename.c_str(), "wb");
	if (!file) {
		fprintf(stderr, "Could not open %s\n", filename);
		return;
	}

	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}
	frame->format = codecContext->pix_fmt;
	frame->width = codecContext->width;
	frame->height = codecContext->height;

	framergb = av_frame_alloc();
	if (!framergb) {
		fprintf(stderr, "Could not allocate video framergb\n");
		exit(1);
	}

	if (av_frame_get_buffer(frame, 32) < 0) {
		fprintf(stderr, "Could not allocate the video frame data\n");
		return;
	}

	imgConvertContext = sws_getCachedContext(NULL, width, height, AV_PIX_FMT_BGR24, width, height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	if (imgConvertContext == NULL) {
		fprintf(stderr, "Cannot initialize the conversion context\n");
		return;
	}
}

int FrameStorage::getWidth() {
	return codecContext->width;
}

int FrameStorage::getHeight() {
	return codecContext->height;
}

void FrameStorage::closeFile() {
	fclose(file);
}