#include "stdafx.h"
#include "soundreader_mp3.h"
#include "gmdatacore/gamepackage.h"
#include "mad.h"

static inline GMint scale(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static mad_flow input(void *data, mad_stream *stream)
{
	GamePackageBuffer* buffer = (GamePackageBuffer*)data;

	if (!buffer->size)
		return MAD_FLOW_STOP;

	mad_stream_buffer(stream, buffer->buffer, buffer->size);
	return MAD_FLOW_CONTINUE;
}

static mad_flow output(void *data, struct mad_header const *header, struct mad_pcm *pcm)
{
	GMuint nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;

	/* pcm->samplerate contains the sampling frequency */

	nchannels = pcm->channels;
	nsamples = pcm->length;
	left_ch = pcm->samples[0];
	right_ch = pcm->samples[1];

	while (nsamples--) {
		signed int sample;

		/* output sample(s) in 16-bit signed little-endian PCM */

		sample = scale(*left_ch++);
		putchar((sample >> 0) & 0xff);
		putchar((sample >> 8) & 0xff);

		if (nchannels == 2) {
			sample = scale(*right_ch++);
			putchar((sample >> 0) & 0xff);
			putchar((sample >> 8) & 0xff);
		}
	}

	return MAD_FLOW_CONTINUE;
}

static mad_flow __cdecl error(void *data, struct mad_stream *stream, struct mad_frame *frame)
{
	GamePackageBuffer *buffer = (GamePackageBuffer*)data;

	gm_error("decoding error 0x%04x (%s) at byte offset %u\n",
		stream->error, mad_stream_errorstr(stream),
		stream->this_frame - buffer->buffer);

	/* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

	return MAD_FLOW_CONTINUE;
}

bool SoundReader_MP3::load(GamePackageBuffer& buffer, OUT ISoundFile** sf)
{
	decode(buffer);
	return true;
}

bool SoundReader_MP3::test(const GamePackageBuffer& buffer)
{
	return buffer.buffer[0] == 'I' && buffer.buffer[1] == 'D' && buffer.buffer[2] == '3';
}

bool SoundReader_MP3::decode(GamePackageBuffer& buffer)
{
	mad_decoder decoder;
	mad_decoder_init(&decoder, &buffer, input, nullptr, nullptr, output, error, nullptr);
	GMint result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
	mad_decoder_finish(&decoder);
	return 0;// result == 0;
}