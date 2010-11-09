#pragma once

#include "../../frame/Frame.h"
#include "../../audio/AudioManager.h"

#include <tbb/scalable_allocator.h>
#include <type_traits>

#include <boost/noncopyable.hpp>

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable : 4244)
#endif
extern "C" 
{
	#define __STDC_CONSTANT_MACROS
	#define __STDC_LIMIT_MACROS
	#include <libavformat/avformat.h>
}
#if defined(_MSC_VER)
#pragma warning (pop)
#endif

namespace caspar{ namespace ffmpeg{

typedef std::tr1::shared_ptr<AVFrame> AVFramePtr;	
typedef std::tr1::shared_ptr<AVPacket> AVPacketPtr;

struct video_packet : boost::noncopyable
{
	video_packet(const AVPacket& packet, FramePtr&& frame, AVCodecContext* codec_context, AVCodec* codec) 
		:  size(packet.size), codec_context(codec_context), codec(codec), frame(std::move(frame)), 
			data(packet.size), decoded_frame(avcodec_alloc_frame(), av_free)
	{
		memcpy(data.data(), packet.data, packet.size);
	}
			
	const size_t					size;
	std::vector<uint8_t>			data;
	AVCodecContext*	const			codec_context;
	const AVCodec* const			codec;
	FramePtr						frame;
	AVFramePtr						decoded_frame;
};	
typedef std::shared_ptr<video_packet> video_packet_ptr;

struct audio_packet : boost::noncopyable
{
	audio_packet(const AVPacket& packet, AVCodecContext* codec_context, AVCodec* codec, double frame_rate = 25.0) 
		: 
		size(packet.size), 
		codec_context(codec_context), 
		codec(codec),
		data(packet.size)
	{
		memcpy(data.data(), packet.data, packet.size);

		size_t bytesPerSec = (codec_context->sample_rate * codec_context->channels * 2);

		audio_frame_size = bytesPerSec / 25;
		src_audio_frame_size = static_cast<size_t>(static_cast<double>(bytesPerSec) / frame_rate);

		//make sure the framesize is a multiple of the samplesize
		int sourceSizeMod = src_audio_frame_size % (codec_context->channels * 2);
		if(sourceSizeMod != 0)
			src_audio_frame_size += (codec_context->channels * 2) - sourceSizeMod;
	}
				
	size_t					src_audio_frame_size;
	size_t					audio_frame_size;

	AVCodecContext*	const	codec_context;
	const AVCodec* const	codec;
	const size_t			size;
	std::vector<uint8_t>	data;

	std::vector<audio::AudioDataChunkPtr> audio_chunks;
};
typedef std::shared_ptr<audio_packet> audio_packet_ptr;

	}
}
