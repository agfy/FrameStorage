#include "FrameStorage.h"

int main() {
	CompressionType cType = compressed;
	FrameStorage fStorage(cType);
	//std::string fileName = "C:\\Users\\Max\\Downloads\\ffmpeg-4.2-win64-static\\bin\\stefan264.mp4";
	//std::string fileName = "C:\\Users\\Max\\Downloads\\Frozen II. (2019).mkv";
	//std::string fileName = "../test_video/tst_1.mp4"
	std::string fileName = "../test_video/tst_r1_264_txt.mp4";
	fStorage.openForRead(fileName);
	srand(time(NULL));
	
	//while (true) {
	//	int frameCount = fStorage.getNumFrames();
	//	int frameNumber = rand() % frameCount;
	//	auto frame = fStorage.getFrameByIndex(frameNumber);
	//}	

	
	int frameCount = fStorage.getNumFrames();
	printf("frameCount = %d\n", frameCount);
	for (int i = 1; i < frameCount; i+=1) {
		auto frame = fStorage.getFrameByIndex(i);
		cv::imshow("", frame);
 		cv::waitKey();
	}	
	
	//std::vector<int> frameIDs = { 1, 10, 5, 11, 20, 12, 70, 40, 3 };
	//for (auto fID: frameIDs) {
	//	auto frame = fStorage.getFrameByIndex(fID);
	//	cv::imshow("", frame);
	//	cv::waitKey();
	//}

	
	return 0;
}