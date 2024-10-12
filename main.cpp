#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}
   void list_encoders() {
       const AVCodec *codec = nullptr;
       void *i = 0;
       while ((codec = av_codec_iterate(&i))) {
           if (av_codec_is_encoder(codec)) {
               std::cout << "Encoder: " << codec->name << std::endl;
           }
       }
   }
int main() {
    const std::string input_rtsp_url    = "rtsp://input_rtsp_url";
    // const std::string input_rtsp_url    = "rtsp://username:password@ip:554/Streaming/Channels/1";// 海康摄像头RTSP格式
    const std::string output_rtsp_url   = "rtsp://output_rtsp_url";

    // av_register_all();
    avformat_network_init();

    AVFormatContext *input_format_ctx = nullptr;
    AVDictionary *options = nullptr;
    av_dict_set(&options, "buffer_size", "1024000", 0); // 设置缓冲区大小
    av_dict_set(&options, "max_delay", "500000", 0); // 设置最大延迟
    av_dict_set(&options, "rtsp_transport", "tcp", 0); // 使用TCP传输
    if (avformat_open_input(&input_format_ctx, input_rtsp_url.c_str(), nullptr, &options) != 0) {
        std::cerr << "Could not open input stream." << std::endl;
        return -1;
    }
    av_dict_free(&options);


    if (avformat_find_stream_info(input_format_ctx, nullptr) < 0) {
        std::cerr << "Could not find stream information." << std::endl;
        return -1;
    }
    const AVCodec *decoder = avcodec_find_decoder_by_name("h264_rkmpp");

    
    int video_stream_index = av_find_best_stream(input_format_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
    if (video_stream_index < 0) {
        std::cerr << "Could not find video stream in the input." << std::endl;
        return -1;
    }
    AVCodecContext *decoder_ctx = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(decoder_ctx, input_format_ctx->streams[video_stream_index]->codecpar);
    if (avcodec_open2(decoder_ctx, decoder, nullptr) < 0) {
        std::cerr << "Could not open decoder." << std::endl;
        return -1;
    }

    AVFormatContext *output_format_ctx = nullptr;
    avformat_alloc_output_context2(&output_format_ctx, nullptr, "rtsp", output_rtsp_url.c_str());
    if (!output_format_ctx) {
        std::cerr << "Could not create output context." << std::endl;
        return -1;
    }
    // list_encoders();
    // AVCodec *encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
    const AVCodec *encoder = avcodec_find_encoder_by_name("h264_rkmpp");
    if (!encoder) {
        std::cerr << "H264 encoder not found." << std::endl;
        return -1;
    }
    std::cout << "Decoder: " << decoder->name << std::endl;
    std::cout << "Encoder: " << encoder->name << std::endl;
    AVStream *out_stream = avformat_new_stream(output_format_ctx, encoder);
    if (!out_stream) {
        std::cerr << "Failed to allocate output stream." << std::endl;
        return -1;
    }

    AVCodecContext *encoder_ctx = avcodec_alloc_context3(encoder);
    encoder_ctx->height = decoder_ctx->height;
    encoder_ctx->width = decoder_ctx->width;
    encoder_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    encoder_ctx->time_base = {1, 25};
    // encoder_ctx->profile = FF_PROFILE_H264_LOW;
    // Set the target bitrate (e.g., 400000 for 400 kbps)
    encoder_ctx->bit_rate = 4000000;

    // Optional: Set maximum and minimum bit rate
    encoder_ctx->rc_max_rate = 5000000; // Maximum bit rate
    encoder_ctx->rc_min_rate = 3000000; // Minimum bit rate

    // Optional: Set buffer size
    encoder_ctx->rc_buffer_size = 1024000/2; // Buffer size

    if (output_format_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        encoder_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (avcodec_open2(encoder_ctx, encoder, nullptr) < 0) {
        std::cerr << "Could not open encoder." << std::endl;
        return -1;
    }

    avcodec_parameters_from_context(out_stream->codecpar, encoder_ctx);

    if (!(output_format_ctx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&output_format_ctx->pb, output_rtsp_url.c_str(), AVIO_FLAG_WRITE) < 0) {
            std::cerr << "Could not open output URL." << std::endl;
            return -1;
        }
    }

    if (avformat_write_header(output_format_ctx, nullptr) < 0) {
        std::cerr << "Error occurred when opening output URL." << std::endl;
        return -1;
    }

    int64_t frame_index = 0; // Initialize frame index

    AVPacket packet;
    while (av_read_frame(input_format_ctx, &packet) >= 0) {
    if (packet.stream_index == video_stream_index) {
        if (avcodec_send_packet(decoder_ctx, &packet) == 0) {
            AVFrame *frame = av_frame_alloc();
            if (avcodec_receive_frame(decoder_ctx, frame) == 0) {
                // Set the PTS for the frame
                frame->pts = frame_index++;

                // Encode the frame
                if (avcodec_send_frame(encoder_ctx, frame) == 0) {
                    AVPacket *enc_pkt = av_packet_alloc();
                    if (enc_pkt && avcodec_receive_packet(encoder_ctx, enc_pkt) == 0) {
                        // Rescale PTS and DTS to the output stream's time base
                        enc_pkt->pts = av_rescale_q(enc_pkt->pts, encoder_ctx->time_base, out_stream->time_base);
                        enc_pkt->dts = av_rescale_q(enc_pkt->dts, encoder_ctx->time_base, out_stream->time_base);
                        enc_pkt->duration = av_rescale_q(enc_pkt->duration, encoder_ctx->time_base, out_stream->time_base);
                        enc_pkt->stream_index = out_stream->index;

                        av_interleaved_write_frame(output_format_ctx, enc_pkt);
                        av_packet_unref(enc_pkt);
                    }
                    av_packet_free(&enc_pkt);
                }
                av_frame_free(&frame);
            }
        }
        av_packet_unref(&packet);
    }
}

    av_write_trailer(output_format_ctx);

    avcodec_free_context(&decoder_ctx);
    avcodec_free_context(&encoder_ctx);
    avformat_close_input(&input_format_ctx);
    if (!(output_format_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&output_format_ctx->pb);
    avformat_free_context(output_format_ctx);

    avformat_network_deinit();

    return 0;
}