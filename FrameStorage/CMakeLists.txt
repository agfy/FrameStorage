cmake_minimum_required(VERSION 3.12)

project ( FrameStorage )

# PACKAGES
set(OpenCV_DIR $ENV{OPENCV_HOME})
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs imgproc highgui calib3d video)

# INCLUDES
include_directories($ENV{FFMPEG_HOME}/include)
include_directories(${OpenCV_INCLUDE_DIRS})
# include_directories($ENV{OPENCV_HOME}/include)
# include_directories(./)

# LIB DIRS
IF (WIN32)
	# link_directories($ENV{OPENCV_HOME}/x64/vc14/lib/)
	link_directories($ENV{FFMPEG_HOME}/lib)
ELSEIF(APPLE)
ELSEIF(UNIX AND NOT APPLE)
ELSE()
ENDIF()

# OPTIONS
set(CMAKE_CXX_STANDARD 17)

# PROJECT
file(GLOB SRCS "./*.h*" "./*.c*")
add_executable(FrameStorage ${SRCS})

# LINK
IF (WIN32)
	target_link_libraries(FrameStorage PRIVATE ${OpenCV_LIBS})
	target_link_libraries(FrameStorage PRIVATE avformat avcodec avutil avfilter swscale swresample)
ELSEIF(UNIX)
ELSEIF(APPLE)
ELSEIF(UNIX AND NOT APPLE)
ELSE()
ENDIF()
