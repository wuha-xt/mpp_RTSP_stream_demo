cmake_minimum_required(VERSION 3.10)
project(RTSPStreamProcessor)

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(AVCODEC REQUIRED libavcodec)
pkg_check_modules(AVUTIL REQUIRED libavutil)
pkg_check_modules(SWSCALE REQUIRED libswscale)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${AVFORMAT_INCLUDE_DIRS}
    ${AVCODEC_INCLUDE_DIRS}
    ${AVUTIL_INCLUDE_DIRS}
    ${SWSCALE_INCLUDE_DIRS}
)

add_executable(RTSPStreamProcessor main.cpp)

target_link_libraries(RTSPStreamProcessor
    ${OpenCV_LIBS}
    ${AVFORMAT_LIBRARIES}
    ${AVCODEC_LIBRARIES}
    ${AVUTIL_LIBRARIES}
    ${SWSCALE_LIBRARIES}
)