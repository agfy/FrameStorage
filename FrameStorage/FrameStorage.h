#pragma once
#include "string"
#include <opencv2/core/mat.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

enum CompressionType {
	lossless = 0, 
	compressed 
};

class FrameStorage{
public:
	FrameStorage(CompressionType type);
	int getNumFrames();
	cv::Mat getFrameByIndex(int);
	void storeFrame(cv::Mat&);
	void openForRead(std::string&);
	void openForWrite(std::string&);

private:
	int err;
	int prevFrameIndex;
	CompressionType cType;
	AVFormatContext* format_context;
	AVCodecContext* codec_context; // AVCodecContext - information about the codec
	AVCodec* codec; // AVCodec - codec itself
	AVPacket packet;
	AVFrame* frame;
	AVFrame* framergb;
	SwsContext* img_convert_context;
	int video_stream;
	int64_t timeBase;
};