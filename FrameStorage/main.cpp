#include "FrameStorage.h"

int main() {
	FrameStorage inputfStorage;
	std::string inputFileName = "C:\\Users\\Max\\source\\repos\\FrameStorage\\FrameStorage\\test_video\\30_Seconds.mp4";
	inputfStorage.openForRead(inputFileName);

	CompressionType cType = lossless;
	FrameStorage outputfStorage;
	std::string outputFileName = "C:\\Users\\Max\\source\\repos\\FrameStorage\\FrameStorage\\test_video\\output.h264";
	outputfStorage.openForWrite(outputFileName, inputfStorage.getWidth(), inputfStorage.getHeight(), cType);

	//int frameCount = inputfStorage.getNumFrames();
	cv::Mat cvFrame = cv::Mat(inputfStorage.getHeight(), inputfStorage.getWidth(), CV_8UC3);
	//printf("frameCount = %d\n", frameCount);
	//for (int i = 0; i < frameCount; i += 1) {
	//	inputfStorage.getFrameByIndex(i, cvFrame);
	//	outputfStorage.storeFrame(cvFrame, i);
		//cv::imshow("", cvFrame);
 		//cv::waitKey();
	//}	

	//outputfStorage.storeTheRest();
	//outputfStorage.closeFile();
	//int retCode = system("C:\\Users\\Max\\source\\repos\\FrameStorage\\FrameStorage\\test_video\\ffmpeg.exe -framerate 30 -i C:\\Users\\Max\\source\\repos\\FrameStorage\\FrameStorage\\test_video\\output.h264 -c copy C:\\Users\\Max\\source\\repos\\FrameStorage\\FrameStorage\\test_video\\output.mp4");
	
	/*
	srand(time(NULL));
	
	while (true) {
	int frameCount = inputfStorage.getNumFrames();
		int frameNumber = rand() % frameCount;
		inputfStorage.getFrameByIndex(frameNumber, cvFrame);
		cv::imshow("", cvFrame);
		cv::waitKey();
	}	
	*/

	

	//std::vector<int> frameIDs = { 1, 10, 5, 11, 20, 12, 70, 40, 3 };
	std::vector<int> frameIDs = { 1, 10, 855, 311, 720, 512, 70, 140, 3 };
	for (auto fID: frameIDs) {
		inputfStorage.getFrameByIndexStable(fID, cvFrame);
		cv::imshow("", cvFrame);
		cv::waitKey();
	}	
	return 0;
}