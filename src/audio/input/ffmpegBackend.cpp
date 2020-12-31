/*
 *  Copyright (C) 2006-2020 Leandro Nini
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

#include "ffmpegBackend.h"

#include "settings.h"
#include "loadsym.h"

#include <QDebug>

extern const unsigned char iconFfmpeg[86] =
{
    0x47,0x49,0x46,0x38,0x39,0x61,0x10,0x00,0x10,0x00,0xa1,0x02,0x00,0x4c,0xad,0x4c,
    0x4c,0xae,0x4c,0x01,0x01,0x01,0x01,0x01,0x01,0x21,0xf9,0x04,0x01,0x0a,0x00,0x02,
    0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,0x10,0x00,0x00,0x02,0x27,0x8c,0x6f,0x22,
    0xa9,0x8b,0x0f,0xa3,0x74,0x12,0xb5,0x7a,0x1a,0x0d,0x00,0xae,0x5b,0x39,0x20,0x16,
    0x8c,0x98,0x19,0x6e,0x46,0xe7,0x69,0x59,0xe4,0xb6,0x99,0x02,0xab,0xa5,0x6a,0x33,
    0xda,0x7e,0x09,0x05,0x00,0x3b
};

#define CREDITS "ffmpeg"
#define LINK    "http://www.ffmpeg.org/"

const char ffmpegBackend::name[] = "Ffmpeg";

#ifdef _WIN32
#  define AVFORMATLIB "avformat-" AV_STRINGIFY(LIBAVFORMAT_VERSION_MAJOR) ".dll"
#  define AVCODECLIB  "avcodec-" AV_STRINGIFY(LIBAVCODEC_VERSION_MAJOR) ".dll"
#  define AVUTILLIB   "avutil-" AV_STRINGIFY(LIBAVUTIL_VERSION_MAJOR) ".dll"
#else
#  define AVFORMATLIB "libavformat.so." AV_STRINGIFY(LIBAVFORMAT_VERSION_MAJOR)
#  define AVCODECLIB  "libavcodec.so." AV_STRINGIFY(LIBAVCODEC_VERSION_MAJOR)
#  define AVUTILLIB   "libavutil.so." AV_STRINGIFY(LIBAVUTIL_VERSION_MAJOR)
#endif

const AutoDLL ffmpegBackend::avformatDll(AVFORMATLIB);
const AutoDLL ffmpegBackend::avcodecDll(AVCODECLIB);
const AutoDLL ffmpegBackend::avutilDll(AVUTILLIB);

//#if LIBAVCODEC_VERSION_MAJOR < xx

int (*ffmpegBackend::dl_avformat_open_input) (AVFormatContext**, const char*, AVInputFormat*, AVDictionary**)=0;
void (*ffmpegBackend::dl_avformat_close_input)(AVFormatContext**)=0;
int (*ffmpegBackend::dl_avformat_find_stream_info)(AVFormatContext*, AVDictionary**)=0;
int (*ffmpegBackend::dl_avcodec_open2)(AVCodecContext*, const AVCodec*, AVDictionary**)=0;
int (*ffmpegBackend::dl_avcodec_decode_audio4)(AVCodecContext*, AVFrame*, int*, const AVPacket*)=0;
AVFrame* (*ffmpegBackend::dl_av_frame_alloc)();
void (*ffmpegBackend::dl_av_frame_free)(AVFrame**)=0;
int (*ffmpegBackend::dl_av_sample_fmt_is_planar)(enum AVSampleFormat)=0;
int (*ffmpegBackend::dl_av_samples_get_buffer_size)(int*, int, int, enum AVSampleFormat, int)=0;
int (*ffmpegBackend::dl_av_read_frame)(AVFormatContext*, AVPacket*)=0;
int (*ffmpegBackend::dl_av_seek_frame)(AVFormatContext*, int, int64_t, int)=0;
AVDictionaryEntry* (*ffmpegBackend::dl_av_dict_get)(AVDictionary*, const char*, const AVDictionaryEntry*, int)=0;
AVCodec* (*ffmpegBackend::dl_avcodec_find_decoder)(enum AVCodecID)=0;
void (*ffmpegBackend::dl_av_init_packet)(AVPacket*)=0;
void (*ffmpegBackend::dl_avcodec_flush_buffers)(AVCodecContext*)=0;
void (*ffmpegBackend::dl_av_free_packet)(AVPacket*)=0;
// int64_t (*ffmpegBackend::dl_av_rescale_q)(int64_t, AVRational, AVRational)=0;
AVCodecContext* (*ffmpegBackend::dl_avcodec_alloc_context3)(const AVCodec *codec)=0;
void (*ffmpegBackend::dl_avcodec_free_context)(AVCodecContext **avctx)=0;

QStringList ffmpegBackend::_ext;

/*****************************************************************/

size_t ffmpegBackend::fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds)
{
    size_t n = decodeBufOffset;
    unsigned char* buf = decodeBuf;

    while (n <= bufferSize)
    {
        while (!packet.data)
        {
            if (dl_av_read_frame(formatContext, &packet) < 0)
            {
                qDebug() << "Last frame: " << static_cast<int>(n);
                memcpy(buffer, buf, n);
                decodeBufOffset = 0;
                return n;
            }
            if (packet.stream_index != audioStreamIndex)
            {
                dl_av_free_packet(&packet);
                packet.data = 0;
            }
            else
                packetOffset = 0;
        }
        int frame_size = MAX_AUDIO_FRAME_SIZE*2 - n;

        AVPacket avpkt;
        dl_av_init_packet(&avpkt);
        avpkt.data = packet.data+packetOffset;
        avpkt.size = packet.size-packetOffset;

        AVFrame *frame = dl_av_frame_alloc();
        int got_frame = 0;
        const int used = dl_avcodec_decode_audio4(codecContext, frame, &got_frame, &avpkt);
        if ((used >= 0) && got_frame)
        {
            int plane_size;
            int data_size = dl_av_samples_get_buffer_size(&plane_size, audioStream->codecpar->channels,
                    frame->nb_samples,
                    (AVSampleFormat)audioStream->codecpar->format, 1);
            if (frame_size < data_size)
            {
                qDebug() << "output buffer size is too small for the current frame: ("
                    << frame_size << " < " << data_size << ")";
                return 0;
            }

            if (_planar && (audioStream->codecpar->channels > 1))
            {
                // Interleave channels
                uint8_t *out = (uint8_t*)(buf+n);
                int idx = 0;
                const int samples = plane_size/_sampleSize;

                for (int j=0; j<samples; j++)
                {
                    for (int ch=0; ch<audioStream->codecpar->channels; ch++)
                    {
                        memcpy(out, frame->extended_data[ch]+idx, _sampleSize);
                        out += _sampleSize;
                    }
                    idx += _sampleSize;
                }
            } else {
                    memcpy(buf+n, frame->extended_data[0], plane_size);
            }
            frame_size = data_size;
        }
        else
        {
            frame_size = 0;
        }
        dl_av_frame_free(&frame);

        //qDebug() << "frame_size: " << frame_size;
        if (used >= 0)
        {
            n += frame_size;
            packetOffset += used;
        }
        if ((used < 0) || (packet.size <= packetOffset))
        {
            dl_av_free_packet(&packet);
            packet.data = 0;
        }
    }

    memcpy(buffer, buf, bufferSize);
    decodeBufOffset = n-bufferSize;
    memcpy(buf, buf+bufferSize, decodeBufOffset);

    return bufferSize;
}

/*****************************************************************/

bool ffmpegBackend::init()
{
    if (!avformatDll.loaded() || !avcodecDll.loaded() || !avutilDll.loaded())
            return false;

    LOADSYM(avcodecDll, avcodec_open2, int(*)(AVCodecContext*, const AVCodec*, AVDictionary**))
    LOADSYM(avcodecDll, avcodec_decode_audio4, int(*)(AVCodecContext*, AVFrame*, int*, const AVPacket*))
    LOADSYM(avutilDll, av_frame_alloc, AVFrame*(*)())
    LOADSYM(avutilDll, av_frame_free, void(*)(AVFrame**))
    LOADSYM(avutilDll, av_sample_fmt_is_planar, int(*)(enum AVSampleFormat))
    LOADSYM(avutilDll, av_samples_get_buffer_size, int(*)(int*, int, int, enum AVSampleFormat, int))
    LOADSYM(avformatDll, avformat_open_input, int(*)(AVFormatContext **ps, const char *filename, AVInputFormat *fmt, AVDictionary **options))
    LOADSYM(avformatDll, avformat_close_input, void(*)(AVFormatContext**))
    LOADSYM(avformatDll, avformat_find_stream_info, int(*)(AVFormatContext*, AVDictionary**))
    LOADSYM(avformatDll, av_read_frame, int(*)(AVFormatContext*, AVPacket*))
    LOADSYM(avformatDll, av_seek_frame, int(*)(AVFormatContext*, int, int64_t, int))
    LOADSYM(avformatDll, avcodec_alloc_context3, AVCodecContext*(*)(const AVCodec *codec))
    LOADSYM(avformatDll, avcodec_free_context, void(*)(AVCodecContext **avctx))
    LOADSYM(avutilDll, av_dict_get, AVDictionaryEntry*(*)(AVDictionary*, const char*, const AVDictionaryEntry*, int))
    LOADSYM(avcodecDll, av_init_packet, void(*)(AVPacket*))
    LOADSYM(avcodecDll, avcodec_find_decoder, AVCodec*(*)(enum AVCodecID))
    LOADSYM(avcodecDll, avcodec_flush_buffers, void(*)(AVCodecContext*))
    LOADSYM(avcodecDll, av_free_packet, void(*)(AVPacket*))
    //LOADSYM(avcodecDll, av_rescale_q, int64_t (*)(int64_t, AVRational, AVRational)) // FIXME in libavutil now?

    AVInputFormat *(*dl_av_find_input_format)(const char*);
    LOADSYM(avformatDll, av_find_input_format, AVInputFormat*(*)(const char*))

    void (*dl_av_register_all)(void);
    LOADSYM(avformatDll, av_register_all, void(*)(void))

    // avcodec_register_all() is implicitly called by av_register_all()
    dl_av_register_all();

    // register supported extensions
    if (dl_av_find_input_format("aac"))
        _ext << "aac";
    if (dl_av_find_input_format("ape"))
        _ext << "ape" << "apl" << "mac";
    if (dl_av_find_input_format("asf"))
        _ext << "wma";
    if (dl_av_find_input_format("au"))
        _ext << "au";
    if (dl_av_find_input_format("flac"))
        _ext << "flac";
    if (dl_av_find_input_format("mp3"))
        _ext << "mp2" << "mp3" << "m2a";
    if (dl_av_find_input_format("mp4"))
        _ext << "mp4" << "m4a";
    if (dl_av_find_input_format("mpc"))
        _ext << "mpc";
    if (dl_av_find_input_format("ogg"))
        _ext << "ogg" << "oga";
    if (dl_av_find_input_format("rm"))
        _ext << "rm" << "ra" << "ram";
    if (dl_av_find_input_format("tta"))
        _ext << "tta";
    if (dl_av_find_input_format("voc"))
        _ext << "voc";
    if (dl_av_find_input_format("wav"))
        _ext << "wav";
    if (dl_av_find_input_format("wv"))
        _ext << "wv";

    qDebug() << "Ffmpeg extensions supported: " << _ext;
    return true;
}

bool ffmpegBackend::supports(const QString& fileName)
{
    QString ext = _ext.join("|");
    ext.prepend(".*\\.(").append(")");
    qDebug() << "ffmpegBackend::supports: " << ext;

    QRegExp rx(ext);
    return rx.exactMatch(fileName);
}

inline QStringList ffmpegBackend::ext() const { return _ext; }

ffmpegBackend::ffmpegBackend() :
    inputBackend(name, iconFfmpeg, 86),
    audioStream(nullptr),
    formatContext(nullptr) {}

ffmpegBackend::~ffmpegBackend()
{
    close();
}

QString ffmpegBackend::getMetadata(const char* type)
{
    AVDictionaryEntry* mTag = nullptr;
    QString info;
    for (;;)
    {
        mTag = dl_av_dict_get(formatContext->metadata, type, mTag, 0);
        if (!mTag)
            break;
        if (!info.isEmpty())
            info.append(", ");
        info.append(QString::fromUtf8(mTag->value).trimmed());
    }

    return info;
}

bool ffmpegBackend::open(const QString& fileName)
{
    close();

    AVFormatContext *fc = nullptr;
    if (dl_avformat_open_input(&fc, fileName.toUtf8().constData(), 0, 0) != 0)
    {
        qWarning() << "Cannot open input";
        return false;
    }

    if (dl_avformat_find_stream_info(fc, 0) < 0)
    {
        qWarning() << "Cannot find stream info";
        goto error;
    }

    for (int i=0; i<fc->nb_streams; i++)
    {
        if (fc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStreamIndex = i;
            break;
        }
    }

    if (!openStream(fc, audioStreamIndex))
        goto error;

    formatContext = fc;
    audioStream = fc->streams[audioStreamIndex];
    decodeBufOffset = 0;
    packet.data = 0;

    switch(audioStream->codecpar->format)
    {
    case AV_SAMPLE_FMT_U8:
    case AV_SAMPLE_FMT_U8P:
        _sampleSize = 1;
        _precision = sample_t::U8;
        break;
    case AV_SAMPLE_FMT_S16:
    case AV_SAMPLE_FMT_S16P:
        _sampleSize = 2;
        _precision = sample_t::S16;
        break;
    case AV_SAMPLE_FMT_S32:
    case AV_SAMPLE_FMT_S32P:
        _sampleSize = 4;
        _precision = sample_t::S32;
        break;
    case AV_SAMPLE_FMT_FLT:
    case AV_SAMPLE_FMT_FLTP:
        _sampleSize = 4;
        _precision = sample_t::SAMPLE_FLOAT;
        break;
    default:
        qWarning() << "Unrecognized fomat " << audioStream->codecpar->format;
        goto error;
    }

    _planar = dl_av_sample_fmt_is_planar((AVSampleFormat)audioStream->codecpar->format);

    _metaData.addInfo(metaData::TITLE, getMetadata("title"));
    _metaData.addInfo(metaData::ARTIST, getMetadata("artist"));
    _metaData.addInfo(metaData::ALBUM, getMetadata("album"));
    _metaData.addInfo(metaData::GENRE, getMetadata("genre"));
    _metaData.addInfo(gettext("copyright"), getMetadata("copyright"));
    _metaData.addInfo(metaData::YEAR, getMetadata("date"));
    _metaData.addInfo(metaData::TRACK, getMetadata("track"));
    _metaData.addInfo(metaData::COMMENT, getMetadata("comment"));

    time(fc->duration/1000000);

    songLoaded(fileName);
    return true;

error:
    dl_avcodec_free_context(&codecContext);
    dl_avformat_close_input(&fc);

    return false;
}

void ffmpegBackend::close()
{
    if (songLoaded().isEmpty())
        return;

    if (packet.data)
        dl_av_free_packet(&packet);

    dl_avcodec_free_context(&codecContext);
    dl_avformat_close_input(&formatContext);

    audioStream = 0;
    formatContext = 0;

    songLoaded(QString());
}

bool ffmpegBackend::seek(const int pos)
{
    if (dl_av_seek_frame(formatContext,
        audioStreamIndex,
        //dl_av_rescale_q(pos, AV_TIME_BASE_Q, audioStream->time_base),
        0,
        AVSEEK_FLAG_ANY) < 0)
        return false;

    decodeBufOffset = 0;
    if (packet.data)
    {
        dl_av_free_packet(&packet);
        packet.data = 0;
    }

    dl_avcodec_flush_buffers(codecContext);

    return true;
}

bool ffmpegBackend::openStream(AVFormatContext* fc, const int streamIndex)
{
    if ((streamIndex < 0) || (streamIndex >= fc->nb_streams))
    {
        qWarning() << "Invalid stream index " << streamIndex;
        return false;
    }

    AVCodecID codecId = fc->streams[streamIndex]->codecpar->codec_id;
    AVCodec* codec = dl_avcodec_find_decoder(codecId);
    if (codec == nullptr)
    {
        qWarning() << "Cannot find ecoder";
        return false;
    }

    codecContext = dl_avcodec_alloc_context3(codec);
    if (codecContext == nullptr)
    {
        qWarning() << "Cannot alloc context";
        return false;
    }

    if (dl_avcodec_open2(codecContext, codec, 0) < 0)
    {
        qWarning() << "Cannot open stream";
        return false;
    }

    return true;
}

/*****************************************************************/

ffmpegConfig::ffmpegConfig(QWidget* win) :
    configFrame(win, ffmpegBackend::name, CREDITS, LINK)
{
    matrix()->addWidget(new QLabel(tr("No settings available"), this));
}
