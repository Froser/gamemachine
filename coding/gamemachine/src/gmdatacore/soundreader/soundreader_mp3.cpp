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
	SoundReader_MP3::Data* d = (SoundReader_MP3::Data*)data;

	if (d->bufferLoaded)
		return MAD_FLOW_STOP;

	mad_stream_buffer(stream, d->bufferIn->buffer, d->bufferIn->size);
	d->bufferLoaded = true;
	return MAD_FLOW_CONTINUE;
}

static mad_flow output(void *data, struct mad_header const *header, struct mad_pcm *pcm)
{
	SoundReader_MP3::Data* d = (SoundReader_MP3::Data*)data;
	if (!d->formatCreated)
	{
		d->format.cbSize = sizeof(WAVEFORMATEX);
		d->format.nChannels = pcm->channels;
		d->format.nBlockAlign = 4;
		d->format.wFormatTag = 1;
		d->format.nSamplesPerSec = pcm->samplerate;
		d->format.nAvgBytesPerSec = d->format.nSamplesPerSec * sizeof(unsigned short) * pcm->channels;
		d->format.wBitsPerSample = 16;
		d->formatCreated = true;
	}

	GMuint nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;

	/* pcm->samplerate contains the sampling frequency */

	nchannels = pcm->channels;
	nsamples = pcm->length;
	left_ch = pcm->samples[0];
	right_ch = pcm->samples[1];

	d->bufferOut.reserve(d->bufferOut.size() + (nchannels == 2 ? nsamples * 4 : nsamples * 2));

	while (nsamples--) {
		signed int sample;

		/* output sample(s) in 16-bit signed little-endian PCM */

		sample = scale(*left_ch++);
		d->bufferOut.push_back((sample >> 0) & 0xff);
		d->bufferOut.push_back((sample >> 8) & 0xff);

		if (nchannels == 2) {
			sample = scale(*right_ch++);
			d->bufferOut.push_back((sample >> 0) & 0xff);
			d->bufferOut.push_back((sample >> 8) & 0xff);
		}
	}

	return MAD_FLOW_CONTINUE;
}

bool SoundReader_MP3::load(GamePackageBuffer& buffer, OUT ISoundFile** sf)
{
	D(d);
	d.formatCreated = false;
	d.bufferLoaded = false;
	d.bufferIn = &buffer;

	bool b = decode();
	WaveData* wd;
	WaveData::newWaveData(&wd);
	wd->dwSize = d.bufferOut.size();
	wd->data = new GMbyte[wd->dwSize];
	memcpy(wd->data, d.bufferOut.data(), wd->dwSize);
	//SoundFile* f = new SoundFile(d.format, wd);
	//*sf = f;
	return b;
}

bool SoundReader_MP3::test(const GamePackageBuffer& buffer)
{
	return buffer.buffer[0] == 'I' && buffer.buffer[1] == 'D' && buffer.buffer[2] == '3';
}

bool SoundReader_MP3::decode()
{
	D(d);
	mad_decoder decoder;
	mad_decoder_init(&decoder, &d, input, nullptr, nullptr, output, nullptr, nullptr);
	GMint result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
	return mad_decoder_finish(&decoder) == 0;
}