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

const AutoDLL ffmpegBackend::_avformat(AVFORMATLIB);
const AutoDLL ffmpegBackend::_avcodec(AVCODECLIB);
const AutoDLL ffmpegBackend::_avutil(AVUTILLIB);

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
int (*ffmpegBackend::dl_avcodec_close)(AVCodecContext*)=0;
void (*ffmpegBackend::dl_av_free_packet)(AVPacket*)=0;
// int64_t (*ffmpegBackend::dl_av_rescale_q)(int64_t, AVRational, AVRational)=0;

QStringList ffmpegBackend::_ext;

// Based on code from GNUsound 0.7.4 - Copyright (C) 2002-2004, Pascal Haakmat
// http://www.gnu.org/software/gnusound/
// and on info from Martin Böhme's article "Using libavformat and libavcodec"
// http://www.inb.uni-luebeck.de/~boehme/using_libavcodec.html

/*****************************************************************/

size_t ffmpegBackend::fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds)
{
    size_t n = _decodeBufOffset;
    unsigned char* buf = _decodeBuf;

    while (n <= bufferSize)
    {
        while (!_packet.data)
        {
            if (dl_av_read_frame(_formatContext, &_packet) < 0)
            {
                qDebug() << "Last frame: " << static_cast<int>(n);
                memcpy(buffer, buf, n);
                _decodeBufOffset = 0;
                return n;
            }
            if (_packet.stream_index != _audioStreamIndex)
            {
                dl_av_free_packet(&_packet);
                _packet.data = 0;
            }
            else
                _packetOffset = 0;
        }
        int frame_size = MAX_AUDIO_FRAME_SIZE*2 - n;

        AVPacket avpkt;
        dl_av_init_packet(&avpkt);
        avpkt.data = _packet.data+_packetOffset;
        avpkt.size = _packet.size-_packetOffset;

        AVFrame *frame = dl_av_frame_alloc();
        int got_frame = 0;
        const int used = dl_avcodec_decode_audio4(_audioStream->codec, frame, &got_frame, &avpkt);
        if ((used >= 0) && got_frame)
        {
            int plane_size;
            int data_size = dl_av_samples_get_buffer_size(&plane_size, _audioStream->codec->channels,
                    frame->nb_samples,
                    _audioStream->codec->sample_fmt, 1);
            if (frame_size < data_size)
            {
                qDebug() << "output buffer size is too small for the current frame: ("
                    << frame_size << " < " << data_size << ")";
                return 0;
            }

            if (_planar && (_audioStream->codec->channels > 1))
            {
                // Interleave channels
                uint8_t *out = (uint8_t*)(buf+n);
                int idx = 0;
                const int samples = plane_size/_sampleSize;

                for (int j=0; j<samples; j++)
                {
                    for (int ch=0; ch<_audioStream->codec->channels; ch++)
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

        qDebug() << "frame_size: " << frame_size;
        if (used >= 0)
        {
            n += frame_size;
            _packetOffset += used;
        }
        if ((used < 0) || (_packet.size <= _packetOffset))
        {
            dl_av_free_packet(&_packet);
            _packet.data = 0;
        }
    }

    memcpy(buffer, buf, bufferSize);
    _decodeBufOffset = n-bufferSize;
    memcpy(buf, buf+bufferSize, _decodeBufOffset);

    return bufferSize;
}

/*****************************************************************/

bool ffmpegBackend::init()
{
    if (!_avformat.loaded() || !_avcodec.loaded() || !_avutil.loaded())
            return false;

    LOADSYM(_avcodec, avcodec_open2, int(*)(AVCodecContext*, const AVCodec*, AVDictionary**))
    LOADSYM(_avcodec, avcodec_decode_audio4, int(*)(AVCodecContext*, AVFrame*, int*, const AVPacket*))
    LOADSYM(_avutil, av_frame_alloc, AVFrame*(*)())
    LOADSYM(_avutil, av_frame_free, void(*)(AVFrame**))
    LOADSYM(_avutil, av_sample_fmt_is_planar, int(*)(enum AVSampleFormat))
    LOADSYM(_avutil, av_samples_get_buffer_size, int(*)(int*, int, int, enum AVSampleFormat, int))
    LOADSYM(_avformat, avformat_open_input, int(*)(AVFormatContext **ps, const char *filename, AVInputFormat *fmt, AVDictionary **options))
    LOADSYM(_avformat, avformat_close_input, void(*)(AVFormatContext**))
    LOADSYM(_avformat, avformat_find_stream_info, int(*)(AVFormatContext*, AVDictionary**))
    LOADSYM(_avformat, av_read_frame, int(*)(AVFormatContext*, AVPacket*))
    LOADSYM(_avformat, av_seek_frame, int(*)(AVFormatContext*, int, int64_t, int))
    LOADSYM(_avutil, av_dict_get, AVDictionaryEntry*(*)(AVDictionary*, const char*, const AVDictionaryEntry*, int))
    LOADSYM(_avcodec, av_init_packet, void(*)(AVPacket*))
    LOADSYM(_avcodec, avcodec_find_decoder, AVCodec*(*)(enum AVCodecID))
    LOADSYM(_avcodec, avcodec_flush_buffers, void(*)(AVCodecContext*))
    LOADSYM(_avcodec, avcodec_close, int(*)(AVCodecContext*))
    LOADSYM(_avcodec, av_free_packet, void(*)(AVPacket*))
    //LOADSYM(_avcodec, av_rescale_q, int64_t (*)(int64_t, AVRational, AVRational)) // FIXME in libavutil now?

    AVInputFormat *(*dl_av_find_input_format)(const char*);
    LOADSYM(_avformat, av_find_input_format, AVInputFormat*(*)(const char*))

    void (*dl_av_register_all)(void);
    LOADSYM(_avformat, av_register_all, void(*)(void))

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
    qDebug() << "sndBackend::supports: " << ext;

    QRegExp rx(ext);
    return rx.exactMatch(fileName);
}

inline QStringList ffmpegBackend::ext() const { return _ext; }

ffmpegBackend::ffmpegBackend() :
    inputBackend(name, iconFfmpeg, 86),
    _audioStream(nullptr),
    _formatContext(nullptr) {}

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
        mTag = dl_av_dict_get(_formatContext->metadata, type, mTag, 0);
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
        return false;

    int audioStream = -1;
    if (dl_avformat_find_stream_info(fc, 0) < 0)
        goto error;

    for (unsigned int i=0; i<fc->nb_streams; i++)
    {
        if (fc->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStream = i;
            break;
        }
    }

    if (!openStream(fc, audioStream))
        goto error;

    _formatContext = fc;
    _audioStreamIndex = audioStream;
    _audioStream = fc->streams[audioStream];
    _decodeBufOffset = 0;
    _packet.data = 0;

    switch(_audioStream->codec->sample_fmt)
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
        goto error;
    }

    _planar = dl_av_sample_fmt_is_planar(_audioStream->codec->sample_fmt);

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
    dl_avformat_close_input(&fc);

    return false;
}

void ffmpegBackend::close()
{
    if (songLoaded().isEmpty())
        return;

    if (_packet.data)
        dl_av_free_packet(&_packet);

    dl_avcodec_close(_audioStream->codec);
    dl_avformat_close_input(&_formatContext);

    _audioStream = 0;
    _formatContext = 0;

    songLoaded(QString::null);
}

bool ffmpegBackend::seek(const int pos)
{
    if (dl_av_seek_frame(_formatContext,
        _audioStreamIndex,
        //dl_av_rescale_q(pos, AV_TIME_BASE_Q, _audioStream->time_base),
        0,
        AVSEEK_FLAG_ANY) < 0)
        return false;

    _decodeBufOffset = 0;
    if (_packet.data)
    {
        dl_av_free_packet(&_packet);
        _packet.data = 0;
    }

    dl_avcodec_flush_buffers(_formatContext->streams[_audioStreamIndex]->codec);

    return true;
}

bool ffmpegBackend::openStream(AVFormatContext* fc, const int streamIndex)
{
    if ((streamIndex < 0) || (streamIndex >= fc->nb_streams))
        return false;

    AVCodecContext* dec = fc->streams[streamIndex]->codec;
    AVCodec* codec = dl_avcodec_find_decoder(dec->codec_id);
    if (codec == nullptr)
        return false;

    if (dl_avcodec_open2(dec, codec, 0) < 0)
        return false;

    return true;
}

/*****************************************************************/

ffmpegConfig::ffmpegConfig(QWidget* win) :
    configFrame(win, ffmpegBackend::name, CREDITS, LINK)
{
    matrix()->addWidget(new QLabel(tr("No settings available"), this));
}
