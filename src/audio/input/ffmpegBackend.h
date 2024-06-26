/*
 *  Copyright (C) 2006-2023 Leandro Nini
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef FFMPEG_H
#define FFMPEG_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

extern "C" {
#ifdef HAVE_FFMPEG
#  include <libavcodec/avcodec.h>
#  include <libavformat/avformat.h>
#  if LIBAVCODEC_VERSION_INT <= (58<<16)
#    error LIBAVCODEC too old
#  endif
#  if LIBAVFORMAT_VERSION_INT <= (58<<16 | 12<<8)
#    error LIBAVFORMAT too old
#  endif
#endif
}

#include "input.h"
#include "inputConfig.h"

#include "AutoDLL.h"

#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

/*****************************************************************/

#include "configFrame.h"

class ffmpegConfigFrame : public configFrame
{
private:
    ffmpegConfigFrame() {}
    ffmpegConfigFrame(const ffmpegConfigFrame&) = delete;
    ffmpegConfigFrame& operator=(const ffmpegConfigFrame&) = delete;

public:
    ffmpegConfigFrame(QWidget* win);
    ~ffmpegConfigFrame() override = default;
};

/*****************************************************************/

class ffmpegConfig : public inputConfig
{
public:
    ffmpegConfig(const char name[], const unsigned char* iconType, unsigned int iconLen) :
        inputConfig(name, iconType, iconLen)
    {}

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new ffmpegConfigFrame(win); }
};

/*****************************************************************/

class ffmpegBackend : public input
{
private:
    AVStream *m_audioStream;
    AVFormatContext *m_formatContext;
    AVCodecContext *m_codecContext;
    int m_audioStreamIndex;

    int m_planar;
    int m_sampleSize;
    sample_t m_precision;

    AVFrame *m_frame;
    AVPacket m_packet;

    bool m_needData;

    size_t m_decodeBufOffset;
    unsigned char m_decodeBuf[MAX_AUDIO_FRAME_SIZE*2];

    static const AutoDLL avformatDll;
    static const AutoDLL avcodecDll;
    static const AutoDLL avutilDll;

    static QStringList m_ext;

    ffmpegConfig m_config;

private:
    static int (*dl_avformat_open_input)(AVFormatContext**, const char*, AVInputFormat*, AVDictionary**);
    static void (*dl_avformat_close_input)(AVFormatContext **);
    static int (*dl_avformat_find_stream_info)(AVFormatContext*, AVDictionary**);
    static int (*dl_avcodec_open2)(AVCodecContext*, const AVCodec*, AVDictionary**);
    static int (*dl_avcodec_send_packet)(AVCodecContext*, const AVPacket*);
    static int (*dl_avcodec_receive_frame)(AVCodecContext*, const AVFrame*);
    static AVFrame* (*dl_av_frame_alloc)();
    static void (*dl_av_frame_free)(AVFrame**);
    static int (*dl_av_sample_fmt_is_planar)(enum AVSampleFormat);
    static int (*dl_av_read_frame)(AVFormatContext*, AVPacket*);
    static int (*dl_av_seek_frame)(AVFormatContext*, int, int64_t, int);
    static AVDictionaryEntry* (*dl_av_dict_get)(AVDictionary*, const char*, const AVDictionaryEntry*, int);
    static AVCodec* (*dl_avcodec_find_decoder)(enum AVCodecID);
    static void (*dl_av_packet_unref)(AVPacket*);
    static void (*dl_avcodec_flush_buffers)(AVCodecContext*);
    static AVCodecContext* (*dl_avcodec_alloc_context3)(const AVCodec *codec);
    static void (*dl_avcodec_free_context)(AVCodecContext **avctx);
    static int (*dl_av_find_best_stream)(AVFormatContext *ic, enum AVMediaType type, int wanted_stream_nb,
                                         int related_stream, AVCodec **decoder_ret, int flags);
    static int (*dl_avcodec_parameters_to_context)(AVCodecContext *codec, const AVCodecParameters *par);
    static int64_t (*dl_av_rescale_q)(int64_t, AVRational, AVRational);
    static AVRational (*dl_av_get_time_base_q)();

private:
    ffmpegBackend(const QString& fileName);

    /// Open selected stream
    void openStream();

    QString getMetadata(const char* type);

public:
    ~ffmpegBackend() override;

    static const char name[];

    static bool init();

    /// Factory method
    static input* factory(const QString& fileName) { return new ffmpegBackend(fileName); }
    static inputConfig* cFactory();

    /// Get supported extension
    static QStringList ext();

    /// Seek support
    bool seekable() const override { return true; }

    /// Seek specified position
    bool seek(double pos) override;

    /// Get samplerate
    unsigned int samplerate() const override { return m_audioStream->codecpar->sample_rate; }

    /// Get channels
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(59, 24, 100)
    unsigned int channels() const override { return m_audioStream->codecpar->channels; }
#else
    unsigned int channels() const override { return m_audioStream->codecpar->ch_layout.nb_channels; }
#endif
    /// Get precision
    sample_t precision() const override { return m_precision; }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize) override;
};

#endif
