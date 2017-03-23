/*
 *  Copyright (C) 2006-2017 Leandro Nini
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
#ifdef HAVE_LIBAVFORMAT_AVFORMAT_H
#  include <libavformat/avformat.h>
#  if LIBAVFORMAT_VERSION_INT <= (56<<16 | 40<<8)
#    error LIBAVFORMAT too old
#  endif
#endif
}
#include "inputBackend.h"
#include "AutoDLL.h"

#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio


/*****************************************************************/

#include "configFrame.h"

class ffmpegConfig : public configFrame
{
    Q_OBJECT

private:
    ffmpegConfig() {}
    ffmpegConfig(const ffmpegConfig&);
    ffmpegConfig& operator=(const ffmpegConfig&);

public:
    ffmpegConfig(QWidget* win);
    virtual ~ffmpegConfig() {}
};

/*****************************************************************/

class ffmpegBackend : public inputBackend
{
private:
    AVStream *_audioStream;
    AVFormatContext *_formatContext;
    int _audioStreamIndex;
    unsigned int _decodeBufOffset;
    unsigned char _decodeBuf[MAX_AUDIO_FRAME_SIZE*2];
    AVPacket _packet;
    int _packetOffset;
    sample_t _precision;

    int _planar;
    int _sampleSize;

    static const AutoDLL _avformat;
    static const AutoDLL _avcodec;
    static const AutoDLL _avutil;

    static QStringList _ext;

private:
    static int (*dl_avformat_open_input)(AVFormatContext**, const char*, AVInputFormat*, AVDictionary**);
    static void (*dl_avformat_close_input)(AVFormatContext **);
    static int (*dl_avformat_find_stream_info)(AVFormatContext*, AVDictionary**);
    static int (*dl_avcodec_open2)(AVCodecContext*, const AVCodec*, AVDictionary**);
    static int (*dl_avcodec_decode_audio4)(AVCodecContext*, AVFrame*, int*, const AVPacket*);
    static AVFrame* (*dl_av_frame_alloc)();
    static void (*dl_av_frame_free)(AVFrame**);
    static int (*dl_av_sample_fmt_is_planar)(enum AVSampleFormat);
    static int (*dl_av_samples_get_buffer_size)(int*, int, int, enum AVSampleFormat, int);
    static int (*dl_av_read_frame)(AVFormatContext*, AVPacket*);
    static int (*dl_av_seek_frame)(AVFormatContext*, int, int64_t, int);
    static AVDictionaryEntry* (*dl_av_dict_get)(AVDictionary*, const char*, const AVDictionaryEntry*, int);
    static AVCodec* (*dl_avcodec_find_decoder)(enum AVCodecID);
    static void (*dl_av_init_packet)(AVPacket*);
    static void (*dl_avcodec_flush_buffers)(AVCodecContext*);
    static int (*dl_avcodec_close)(AVCodecContext*);
    static void (*dl_av_free_packet)(AVPacket*);
    //static int64_t (*dl_av_rescale_q)(int64_t, AVRational, AVRational);

private:
    ffmpegBackend();

    /// Open selected stream
    bool openStream(AVFormatContext* fc, const int streamIndex);

    QString getMetadata(const char* type);

public:
    ~ffmpegBackend();

    static const char name[];

    static bool init();

    /// Factory method
    static input* factory() { return new ffmpegBackend(); }

    /// Check if we support ext
    static bool supports(const QString& fileName);

    /// Get supported extension
    QStringList ext() const;

    /// Open file
    bool open(const QString& fileName);

    /// Close file
    void close();

    /// Seek specified position
    bool seek(const int pos);

    /// Get samplerate
    unsigned int samplerate() const
    {
        return _audioStream ?
            _audioStream->codec->sample_rate // TODO replace codec with codecpar
            : 0;
    }

    /// Get channels
    unsigned int channels() const
    {
        return _audioStream ?
            _audioStream->codec->channels
            : 0;
    }

    /// Get precision
    sample_t precision() const { return _precision; }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds);

    /// Open config dialog
    QWidget* config(QWidget* win) { return new ffmpegConfig(win); }
};

#endif
