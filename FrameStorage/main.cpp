#include "FrameStorage.h"

int main() {
	CompressionType cType = compressed;
	FrameStorage fStorage(cType);
	//std::string fileName = "C:\\Users\\Max\\Downloads\\ffmpeg-4.2-win64-static\\bin\\stefan264.mp4";
	std::string fileName = "C:\\Users\\Max\\Downloads\\Frozen II. (2019).mkv";
	fStorage.openForRead(fileName);
	srand(time(NULL));
	/*
	while (true) {
		int frameCount = fStorage.getNumFrames();
		int frameNumber = rand() % frameCount;
		auto frame = fStorage.getFrameByIndex(frameNumber);
	}	
	*/
	int frameCount = fStorage.getNumFrames();
	for (int i = 0; i < frameCount; i+=1000) {
		auto frame = fStorage.getFrameByIndex(i);
	}
	
	return 0;
}