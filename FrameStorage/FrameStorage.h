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
	FrameStorage();
	int getNumFrames();
	void getFrameByIndex(int, cv::Mat&);
	void storeFrame(cv::Mat&, int64_t);
	void storeTheRest();
	void openForRead(std::string&);
	void openForWrite(std::string&, int, int, CompressionType);
	int getWidth();
	int getHeight();
	void closeFile();
	int64_t frameToPts(int);

private:
	int err;
	int prevFrameIndex;
	AVFormatContext* formatContext;
	AVCodecContext* codecContext; // AVCodecContext - information about the codec
	AVCodec* codec; // AVCodec - codec itself
	AVPacket* packet;
	AVFrame* frame;
	AVFrame* framergb;
	SwsContext* imgConvertContext;
	int videoStream;
	int64_t timeBase;
	int rgbLinesize[8];

	FILE *file;
};