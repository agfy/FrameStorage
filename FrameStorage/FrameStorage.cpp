#include "FrameStorage.h"
#include <iostream>

FrameStorage::FrameStorage(CompressionType type) {
	cType = type;

	av_register_all();
	frame = av_frame_alloc();
	framergb = av_frame_alloc();
	format_context = NULL;
}

int64_t FrameToPts(AVStream* pavStream, int frame) {
	return (int64_t(frame) * pavStream->r_frame_rate.den *  pavStream-> time_base.den) /(int64_t(pavStream->r_frame_rate.num)*pavStream->time_base.num);
}

int FrameStorage::getNumFrames() {
	if (format_context->streams[video_stream]->nb_frames != 0) {
		return format_context->streams[video_stream]->nb_frames;
	}
	else {
		int dur = format_context->duration / AV_TIME_BASE;
		int videoFPS = av_q2d(format_context->streams[video_stream]->r_frame_rate);
		return dur * videoFPS;
	}
}

cv::Mat FrameStorage::getFrameByIndex(int frameIndex) {
	avcodec_flush_buffers(codec_context);

	std::cout << "Getting " + std::to_string(frameIndex) + " frame\n";
	if (!format_context) {
		fprintf(stderr, "ffmpeg: empty format_context\n");
		return cv::Mat{};
	}

	int64_t seekTarget = FrameToPts(format_context->streams[video_stream], frameIndex);
	std::cout << "wanted " << seekTarget << std::endl;
	int64_t curSeekTarget = seekTarget;
	int64_t curPacketPts = seekTarget;
	
	//seek until we have packet before target pts
	while (curPacketPts >= seekTarget) {
		if (av_seek_frame(format_context, video_stream, curSeekTarget, 0) < 0) {
			fprintf(stderr, "ffmpeg: av_seek_frame failed\n");
			return cv::Mat{};
		}	
	
		while (av_read_frame(format_context, &packet) >= 0) {
			if (packet.stream_index == video_stream) {
				if (packet.pts > seekTarget) {
					frameIndex -= 500;
					if (frameIndex < 0) {
						frameIndex = 0;
					}
					curSeekTarget = FrameToPts(format_context->streams[video_stream], frameIndex);
					break;
				}
				else {
					curPacketPts = packet.pts;
					break;
				}
			}
		}
	}

	//decode until we have packet after target pts
	while (av_read_frame(format_context, &packet) >= 0) {
		if (packet.stream_index == video_stream) {
			// Video stream packet
			int frame_finished;
			// Decode AVPacket
			if (avcodec_decode_video2(codec_context, frame, &frame_finished, &packet) < 0) {
				fprintf(stderr, "ffmpeg: avcodec_decode_video2 failed\n");
				return cv::Mat{};
			}
			// frame_finished is positive if AVFrame completely decoded. AVFrame can be stored in multiple AVPackets
			if (frame_finished) {
				if (frame->pts < seekTarget) {
					continue;
				}
				std::cout << "got frame " << frame->pts << std::endl;
				if (sws_scale(img_convert_context, frame->data, frame->linesize, 0, codec_context->height, framergb->data, framergb->linesize) < 0) {
					fprintf(stderr, "ffmpeg: sws_scale failed\n");
					return cv::Mat{};
				}

				cv::Mat mat(codec_context->height, codec_context->width, CV_8UC3, framergb->data[0], framergb->linesize[0]);
				cv::imshow("frame", mat);
				int k = cv::waitKey(0);

				return mat;
			}
		}
	}
}

void FrameStorage::storeFrame(cv::Mat&) {

}

void FrameStorage::openForRead(std::string& filename) {
	// Read file header and store info in AVFormatContext struct
	err = avformat_open_input(&format_context, filename.c_str(), NULL, NULL);
	if (err < 0) {
		fprintf(stderr, "ffmpeg: Unable to open input file\n");
		return;
	}

	// Now we have existing streams in format_context->streams and number of streams format_context->nb_streams
	err = avformat_find_stream_info(format_context, NULL);
	if (err < 0) {
		fprintf(stderr, "ffmpeg: Unable to find stream info\n");
		return;
	}

	// Print information about file and all streams onto standard error
	av_dump_format(format_context, 0, filename.c_str(), 0);

	// Find the first video stream
	for (video_stream = 0; video_stream < format_context->nb_streams; ++video_stream) {
		if (format_context->streams[video_stream]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			break;
		}
	}
	if (video_stream == format_context->nb_streams) {
		fprintf(stderr, "ffmpeg: Unable to find video stream\n");
		return;
	}

	codec_context = format_context->streams[video_stream]->codec;
	codec = avcodec_find_decoder(codec_context->codec_id);
	// Open codec
	err = avcodec_open2(codec_context, codec, NULL);
	if (err < 0) {
		fprintf(stderr, "ffmpeg: Unable to open codec\n");
		return;
	}

	img_convert_context = sws_getCachedContext(NULL, codec_context->width, codec_context->height, codec_context->pix_fmt,
		codec_context->width, codec_context->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
	if (img_convert_context == NULL) {
		fprintf(stderr, "Cannot initialize the conversion context\n");
		return;
	}

	int bytes = avpicture_get_size(AV_PIX_FMT_BGR24, codec_context->width, codec_context->height);
	uint8_t* buffer = (uint8_t *)av_malloc(bytes * sizeof(uint8_t));
	avpicture_fill((AVPicture *)framergb, buffer, AV_PIX_FMT_BGR24, codec_context->width, codec_context->height);

	timeBase = (int64_t(codec_context->time_base.num) * AV_TIME_BASE) / int64_t(codec_context->time_base.den);
}

void FrameStorage::openForWrite(std::string&) {

}