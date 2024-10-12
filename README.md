# RTSP Stream Processor

This project is an RTSP stream processor that decodes and re-encodes video streams using FFmpeg and OpenCV. It is designed to handle video streams from RTSP sources, such as IP cameras, and output them to another RTSP endpoint. Tested in RK3588 debian11.

## Features

- Decode video streams using the `h264_rkmpp` decoder.
- Encode video streams using the `h264_rkmpp` encoder.
- Supports custom RTSP input and output URLs.
- Configurable bitrate and buffer size.

## Prerequisites

- **FFmpeg** with `rkmpp` support [ffmpeg-rockchip](https://github.com/nyanmisaka/ffmpeg-rockchip)
  > **NOTE:** To [Compilation and install](https://github.com/nyanmisaka/ffmpeg-rockchip/wiki/Compilation) libs like `avcoder.so`,please adding `--enable-shared` in ffmepg configure command `./configure`, like `./configure --prefix=/usr --enable-gpl --enable-version3 --enable-libdrm --enable-rkmpp --enable-rkrga --enable-shared --enable-ffplay`
- **OpenCV**
- **CMake** (version 3.10 or higher)
- **PkgConfig**

Ensure that FFmpeg is compiled with `rkmpp` support and that all necessary libraries are installed on your system.

## Building the Project

1. **Clone the repository**:
   ```bash
   git clone https://github.com/yourusername/rtsp-stream-processor.git
   cd rtsp-stream-processor
   ```

2. **Create a build directory**:
   ```bash
   mkdir build
   cd build
   ```

3. **Run CMake**:
   ```bash
   cmake ..
   ```

4. **Compile the project**:
   ```bash
   make
   ```

## Usage
1. Dowd load and run [mediamtx](https://github.com/bluenviron/mediamtx/releases/download/v1.9.2/mediamtx_v1.9.2_linux_arm64v8.tar.gz) rtsp server in localhost.
   
1. Edit the `main.cpp` file to set your specific RTSP input and output URLs:
```bash
const std::string input_rtsp_url = "rtsp://your_input_url";
const std::string output_rtsp_url = "rtsp://your_output_url";// mediamtx RTSP server "rtsp://127.0.0.1:8554/test"
```
2. build code.
3. Run the compiled executable with the desired RTSP input and output URLs:
```bash
./RTSPStreamProcessor
```
5. using vlc to pull RTSP url.

## Code Overview

- **main.cpp**: Contains the main logic for decoding and encoding the RTSP stream.
- **CMakeLists.txt**: CMake configuration file for building the project.
## Troubleshooting
- firewall in 8554 port


## Refrance

