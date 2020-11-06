#include "FrameStorage.h"

int main() {
	FrameStorage inputfStorage;
	std::string inputFileName = "C:\\Users\\Max\\source\\repos\\FrameStorage\\FrameStorage\\test_video\\30_Seconds.mp4";
	inputfStorage.openForRead(inputFileName);

	CompressionType cType = lossless;
	FrameStorage outputfStorage;
	std::string outputFileName = "C:\\Users\\Max\\source\\repos\\FrameStorage\\FrameStorage\\test_video\\output.h264";
	outputfStorage.openForWrite(outputFileName, inputfStorage.getWidth(), inputfStorage.getHeight(), cType);

	int frameCount = inputfStorage.getNumFrames();
	cv::Mat cvFrame = cv::Mat(inputfStorage.getHeight(), inputfStorage.getWidth(), CV_8UC3);
	printf("frameCount = %d\n", frameCount);
	for (int i = 0; i < frameCount; i += 1) {
		inputfStorage.getFrameByIndex(i, cvFrame);
		outputfStorage.storeFrame(cvFrame, i);
		//cv::imshow("", cvFrame);
 		//cv::waitKey();
	}	

	outputfStorage.storeTheRest();
	outputfStorage.closeFile();
	
	/*
	srand(time(NULL));
	
	while (true) {
	int frameCount = fStorage.getNumFrames();
		int frameNumber = rand() % frameCount;
		auto frame = fStorage.getFrameByIndex(frameNumber);
	}		
	*/

	//std::vector<int> frameIDs = { 1, 10, 5, 11, 20, 12, 70, 40, 3 };
	//for (auto fID: frameIDs) {
	//	auto frame = fStorage.getFrameByIndex(fID);
	//	cv::imshow("", frame);
	//	cv::waitKey();
	//}	
	return 0;
}