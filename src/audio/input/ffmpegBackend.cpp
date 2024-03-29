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

int (*ffmpegBackend::dl_avformat_open_input)(AVFormatContext**, const char*,
    AVInputFormat*, AVDictionary**)=0;
void (*ffmpegBackend::dl_avformat_close_input)(AVFormatContext**)=0;
int (*ffmpegBackend::dl_avformat_find_stream_info)(AVFormatContext*, AVDictionary**)=0;
int (*ffmpegBackend::dl_avcodec_open2)(AVCodecContext*, const AVCodec*, AVDictionary**)=0;
int (*ffmpegBackend::dl_avcodec_send_packet)(AVCodecContext*, const AVPacket*)=0;
int (*ffmpegBackend::dl_avcodec_receive_frame)(AVCodecContext*, const AVFrame*)=0;
AVFrame* (*ffmpegBackend::dl_av_frame_alloc)();
void (*ffmpegBackend::dl_av_frame_free)(AVFrame**)=0;
int (*ffmpegBackend::dl_av_sample_fmt_is_planar)(enum AVSampleFormat)=0;
int (*ffmpegBackend::dl_av_read_frame)(AVFormatContext*, AVPacket*)=0;
int (*ffmpegBackend::dl_av_seek_frame)(AVFormatContext*, int, int64_t, int)=0;
AVDictionaryEntry* (*ffmpegBackend::dl_av_dict_get)(AVDictionary*, const char*,
    const AVDictionaryEntry*, int)=0;
AVCodec* (*ffmpegBackend::dl_avcodec_find_decoder)(enum AVCodecID)=0;
void (*ffmpegBackend::dl_av_packet_unref)(AVPacket*)=0;
void (*ffmpegBackend::dl_avcodec_flush_buffers)(AVCodecContext*)=0;
int64_t (*ffmpegBackend::dl_av_rescale_q)(int64_t, AVRational, AVRational)=0;
AVCodecContext* (*ffmpegBackend::dl_avcodec_alloc_context3)(const AVCodec *codec)=0;
void (*ffmpegBackend::dl_avcodec_free_context)(AVCodecContext **avctx)=0;
int (*ffmpegBackend::dl_av_find_best_stream)(AVFormatContext *ic, enum AVMediaType type,
    int wanted_stream_nb, int related_stream, AVCodec **decoder_ret, int flags)=0;
int (*ffmpegBackend::dl_avcodec_parameters_to_context)(AVCodecContext *codec,
    const AVCodecParameters *par)=0;
AVRational (*ffmpegBackend::dl_av_get_time_base_q)()=0;

QStringList ffmpegBackend::m_ext;

inputConfig* ffmpegBackend::cFactory() { return new ffmpegConfig(name, iconFfmpeg, 86); }

/*****************************************************************/

size_t ffmpegBackend::fillBuffer(void* buffer, const size_t bufferSize)
{
    size_t decodedSize = m_decodeBufOffset;

    while (decodedSize < bufferSize)
    {
        while (m_needData)
        {
            if (dl_av_read_frame(m_formatContext, &m_packet) < 0)
            {
                // AVERROR_EOF
                qDebug() << "Last frame:" << static_cast<int>(decodedSize);
                memcpy(buffer, m_decodeBuf, decodedSize);
                m_decodeBufOffset = 0;
                return decodedSize;
            }

            if (m_packet.stream_index == m_audioStreamIndex)
            {
                int res = dl_avcodec_send_packet(m_codecContext, &m_packet);
                if (res == 0)
                {
                    m_needData = false;
                }
                else if (res == AVERROR(EAGAIN))
                {
                    // FIXME receive frames and then send same packet again
                    qWarning() << "AVERROR(EAGAIN)";
                    break;
                }
                else if (res < 0)
                {
                    // TODO use av_err2str(res)
                    qWarning() << "Error sending packet";
                    return decodedSize;
                }
            }

            dl_av_packet_unref(&m_packet);
        }

        int res = dl_avcodec_receive_frame(m_codecContext, m_frame);
        if (res == 0)
        {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(59, 24, 100)
            const int channels = m_codecContext->channels;
#else
            const int channels = m_codecContext->ch_layout.nb_channels;
#endif
            int data_size = m_sampleSize * m_frame->nb_samples * channels;

            unsigned char *out = m_decodeBuf + decodedSize;
            if (m_planar && (channels > 1))
            {
                // Interleave channels
                for (int j=0, idx=0; j<m_frame->nb_samples; j++, idx+=m_sampleSize)
                {
                    for (int ch=0; ch<channels; ch++)
                    {
                        memcpy(out, m_frame->data[ch]+idx, m_sampleSize);
                        out += m_sampleSize;
                    }
                }
            }
            else
            {
                memcpy(out, m_frame->data[0], data_size);
            }

            decodedSize += data_size;
        }
        else if (res == AVERROR(EAGAIN))
        {
            m_needData = true;
        }
        else // TODO handle AVERROR_EOF
        {
            qWarning() << "Error receiving frame";
            return decodedSize;
        }
    }

    memcpy((char*)buffer, m_decodeBuf, bufferSize);
    m_decodeBufOffset = decodedSize - bufferSize;
    memmove(m_decodeBuf, m_decodeBuf+bufferSize, m_decodeBufOffset);

    return bufferSize;
}

/*****************************************************************/

bool ffmpegBackend::init()
{
    if (!avformatDll.loaded() || !avcodecDll.loaded() || !avutilDll.loaded())
    {
        qWarning() << "ffmpeg libraries not loaded";
        return false;
    }

    LOADSYM(avcodecDll, avcodec_open2, int(*)(AVCodecContext*, const AVCodec*, AVDictionary**))
    LOADSYM(avcodecDll, avcodec_send_packet, int(*)(AVCodecContext*, const AVPacket*))
    LOADSYM(avcodecDll, avcodec_receive_frame, int(*)(AVCodecContext*, const AVFrame*))
    LOADSYM(avcodecDll, avcodec_alloc_context3, AVCodecContext*(*)(const AVCodec *codec))
    LOADSYM(avcodecDll, avcodec_free_context, void(*)(AVCodecContext **avctx))
    LOADSYM(avcodecDll, avcodec_parameters_to_context, int(*)(AVCodecContext *codec,
        const AVCodecParameters *par))
    LOADSYM(avcodecDll, avcodec_flush_buffers, void(*)(AVCodecContext*))
    LOADSYM(avcodecDll, avcodec_find_decoder, AVCodec*(*)(enum AVCodecID))
    LOADSYM(avcodecDll, av_packet_unref, void(*)(AVPacket*))
    
    LOADSYM(avformatDll, avformat_open_input, int(*)(AVFormatContext **ps,
        const char *filename, AVInputFormat *fmt, AVDictionary **options))
    LOADSYM(avformatDll, avformat_close_input, void(*)(AVFormatContext**))
    LOADSYM(avformatDll, avformat_find_stream_info, int(*)(AVFormatContext*, AVDictionary**))
    LOADSYM(avformatDll, av_read_frame, int(*)(AVFormatContext*, AVPacket*))
    LOADSYM(avformatDll, av_seek_frame, int(*)(AVFormatContext*, int, int64_t, int))
    LOADSYM(avformatDll, av_find_best_stream, int(*)(AVFormatContext *ic,
        enum AVMediaType type, int wanted_stream_nb,
        int related_stream, AVCodec **decoder_ret, int flags))
    LOADSYM(avutilDll, av_frame_alloc, AVFrame*(*)())
    LOADSYM(avutilDll, av_frame_free, void(*)(AVFrame**))
    LOADSYM(avutilDll, av_sample_fmt_is_planar, int(*)(enum AVSampleFormat))
    LOADSYM(avutilDll, av_dict_get, AVDictionaryEntry*(*)(AVDictionary*, const char*,
        const AVDictionaryEntry*, int))
    LOADSYM(avutilDll, av_rescale_q, int64_t (*)(int64_t, AVRational, AVRational))
    LOADSYM(avutilDll, av_get_time_base_q, AVRational (*)())

    AVInputFormat *(*dl_av_find_input_format)(const char*);
    LOADSYM(avformatDll, av_find_input_format, AVInputFormat*(*)(const char*))

    // register supported extensions
    if (dl_av_find_input_format("aac"))
        m_ext << "aac";
    if (dl_av_find_input_format("ape"))
        m_ext << "ape" << "apl" << "mac";
    if (dl_av_find_input_format("asf"))
        m_ext << "wma";
    if (dl_av_find_input_format("au"))
        m_ext << "au";
    if (dl_av_find_input_format("flac"))
        m_ext << "flac";
    if (dl_av_find_input_format("mp3"))
        m_ext << "mp2" << "mp3" << "m2a";
    if (dl_av_find_input_format("mp4"))
        m_ext << "mp4" << "m4a";
    if (dl_av_find_input_format("mpc"))
        m_ext << "mpc";
    if (dl_av_find_input_format("ogg"))
        m_ext << "ogg" << "oga";
    if (dl_av_find_input_format("rm"))
        m_ext << "rm" << "ra" << "ram";
    if (dl_av_find_input_format("tta"))
        m_ext << "tta";
    if (dl_av_find_input_format("voc"))
        m_ext << "voc";
    if (dl_av_find_input_format("wav"))
        m_ext << "wav";
    if (dl_av_find_input_format("wv"))
        m_ext << "wv";

    qDebug() << "Ffmpeg extensions supported:" << m_ext;
    return true;
}

QStringList ffmpegBackend::ext() { return m_ext; }

ffmpegBackend::ffmpegBackend(const QString& fileName) :
    m_formatContext(nullptr),
    m_codecContext(nullptr),
    m_needData(true),
    m_decodeBufOffset(0),
    m_config(name, iconFfmpeg, 86)
{
    try
    {
        if (dl_avformat_open_input(&m_formatContext, fileName.toUtf8().constData(), nullptr, nullptr) < 0)
        {
            throw loadError("Cannot open input");
        }

        if (dl_avformat_find_stream_info(m_formatContext, nullptr) < 0)
        {
            throw loadError("Cannot find stream info");
        }

        openStream();

        m_audioStream = m_formatContext->streams[m_audioStreamIndex];

        switch(m_codecContext->sample_fmt)
        {
        case AV_SAMPLE_FMT_U8:
        case AV_SAMPLE_FMT_U8P:
            m_sampleSize = 1;
            m_precision = sample_t::U8;
            break;
        case AV_SAMPLE_FMT_S16:
        case AV_SAMPLE_FMT_S16P:
            m_sampleSize = 2;
            m_precision = sample_t::S16;
            break;
        case AV_SAMPLE_FMT_S32:
        case AV_SAMPLE_FMT_S32P:
            m_sampleSize = 4;
            m_precision = sample_t::S32;
            break;
        case AV_SAMPLE_FMT_FLT:
        case AV_SAMPLE_FMT_FLTP:
            m_sampleSize = 4;
            m_precision = sample_t::SAMPLE_FLOAT;
            break;
        default:
            throw loadError(QString("Unrecognized fomat %1").arg(m_codecContext->sample_fmt));
        }

        m_frame = dl_av_frame_alloc();

        m_planar = dl_av_sample_fmt_is_planar(m_codecContext->sample_fmt);

        m_metaData.addInfo(metaData::TITLE, getMetadata("title"));
        m_metaData.addInfo(metaData::ARTIST, getMetadata("artist"));
        m_metaData.addInfo(metaData::ALBUM, getMetadata("album"));
        m_metaData.addInfo(metaData::GENRE, getMetadata("genre"));
        m_metaData.addInfo(gettext("copyright"), getMetadata("copyright"));
        m_metaData.addInfo(metaData::CONTENT_CREATED, getMetadata("date"));
        m_metaData.addInfo(metaData::TRACK_NUMBER, getMetadata("track"));
        m_metaData.addInfo(metaData::COMMENT, getMetadata("comment"));

        setDuration(m_formatContext->duration/1000);

        songLoaded(fileName);
    }
    catch (const loadError& e)
    {
        dl_avcodec_free_context(&m_codecContext);
        dl_avformat_close_input(&m_formatContext);

        throw;
    }
}

ffmpegBackend::~ffmpegBackend()
{
    dl_av_frame_free(&m_frame);

    dl_avcodec_free_context(&m_codecContext);
    dl_avformat_close_input(&m_formatContext);
}

QString ffmpegBackend::getMetadata(const char* type)
{
    AVDictionaryEntry* mTag = nullptr;
    QString info;
    for (;;)
    {
        mTag = dl_av_dict_get(m_formatContext->metadata, type, mTag, 0);
        if (!mTag)
            break;
        if (!info.isEmpty())
            info.append(", ");
        info.append(QString::fromUtf8(mTag->value).trimmed());
    }

    return info;
}

bool ffmpegBackend::seek(double pos)
{
    int64_t timestamp = m_formatContext->duration * pos;
    if (dl_av_seek_frame(m_formatContext,
            m_audioStreamIndex,
            dl_av_rescale_q(timestamp, dl_av_get_time_base_q(), m_audioStream->time_base),
            AVSEEK_FLAG_ANY) < 0)
    {
        qWarning() << "Cannot seek";
        return false;
    }

    m_decodeBufOffset = 0;
    m_needData = true;

    dl_avcodec_flush_buffers(m_codecContext);

    return true;
}

void ffmpegBackend::openStream()
{
    // check for album art, it is treated as a single frame video stream
    int imageStreamIndex = dl_av_find_best_stream(m_formatContext, AVMEDIA_TYPE_VIDEO , -1, -1, 0, 0);
    if (imageStreamIndex != AVERROR_STREAM_NOT_FOUND)
    {
        if (m_formatContext->streams[imageStreamIndex]->disposition & AV_DISPOSITION_ATTACHED_PIC)
        {
            AVPacket pkt;
            dl_av_read_frame(m_formatContext, &pkt);
            m_metaData.addInfo(new QByteArray((char*)pkt.data, pkt.size));
            dl_av_packet_unref(&pkt);
        }
    }

    AVCodec* codec;
    m_audioStreamIndex = dl_av_find_best_stream(m_formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    if (m_audioStreamIndex == AVERROR_STREAM_NOT_FOUND)
    {
        throw loadError("No audio stream found");
    }
    if ((m_audioStreamIndex == AVERROR_DECODER_NOT_FOUND) || (codec == nullptr))
    {
        throw loadError("No decoder found");
    }

    m_codecContext = dl_avcodec_alloc_context3(codec);
    if (m_codecContext == nullptr)
    {
        throw loadError("Cannot alloc context");
    }

    if (dl_avcodec_parameters_to_context(m_codecContext,
        m_formatContext->streams[m_audioStreamIndex]->codecpar) < 0)
    {
        throw loadError("Error filling the codec context");
    }

    if (dl_avcodec_open2(m_codecContext, codec, nullptr) < 0)
    {
        throw loadError("Cannot open stream");
    }
}

/*****************************************************************/

ffmpegConfigFrame::ffmpegConfigFrame(QWidget* win) :
    configFrame(win, CREDITS, LINK)
{
    matrix()->addWidget(new QLabel(tr("No settings available"), this));
}
