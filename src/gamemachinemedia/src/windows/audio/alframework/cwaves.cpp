/*
 * Copyright (c) 2006, Creative Labs Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided
 * that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and
 * 	     the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 * 	     and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of Creative Labs Inc. nor the names of its contributors may be used to endorse or
 * 	     promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <gmmcommon.h>
#include <gmtools.h>
#include <gmstring.h>
#include "CWaves.h"
#include <errno.h>

#pragma pack(push, 4)

typedef struct
{
	char			szRIFF[4];
	unsigned long	ulRIFFSize;
	char			szWAVE[4];
} WAVEFILEHEADER;

typedef struct
{
	char			szChunkName[4];
	unsigned long	ulChunkSize;
} RIFFCHUNK;

typedef struct
{
	unsigned short	usFormatTag;
	unsigned short	usChannels;
	unsigned long	ulSamplesPerSec;
	unsigned long	ulAvgBytesPerSec;
	unsigned short	usBlockAlign;
	unsigned short	usBitsPerSample;
	unsigned short	usSize;
	unsigned short  usReserved;
	unsigned long	ulChannelMask;
    GUID            guidSubFormat;
} WAVEFMT;

#ifndef _KSMEDIA_
#define _KSMEDIA_
// Speaker Positions:
#define SPEAKER_FRONT_LEFT              0x1
#define SPEAKER_FRONT_RIGHT             0x2
#define SPEAKER_FRONT_CENTER            0x4
#define SPEAKER_LOW_FREQUENCY           0x8
#define SPEAKER_BACK_LEFT               0x10
#define SPEAKER_BACK_RIGHT              0x20
#define SPEAKER_FRONT_LEFT_OF_CENTER    0x40
#define SPEAKER_FRONT_RIGHT_OF_CENTER   0x80
#define SPEAKER_BACK_CENTER             0x100
#define SPEAKER_SIDE_LEFT               0x200
#define SPEAKER_SIDE_RIGHT              0x400
#define SPEAKER_TOP_CENTER              0x800
#define SPEAKER_TOP_FRONT_LEFT          0x1000
#define SPEAKER_TOP_FRONT_CENTER        0x2000
#define SPEAKER_TOP_FRONT_RIGHT         0x4000
#define SPEAKER_TOP_BACK_LEFT           0x8000
#define SPEAKER_TOP_BACK_CENTER         0x10000
#define SPEAKER_TOP_BACK_RIGHT          0x20000

// Bit mask locations reserved for future use
#define SPEAKER_RESERVED                0x7FFC0000

// Used to specify that any possible permutation of speaker configurations
#define SPEAKER_ALL                     0x80000000

// DirectSound Speaker Config
#if (NTDDI_VERSION >= NTDDI_WINXP)
#define KSAUDIO_SPEAKER_DIRECTOUT       0
#endif
#define KSAUDIO_SPEAKER_MONO            (SPEAKER_FRONT_CENTER)
#define KSAUDIO_SPEAKER_STEREO          (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT)
#define KSAUDIO_SPEAKER_QUAD            (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | \
                                         SPEAKER_BACK_LEFT  | SPEAKER_BACK_RIGHT)
#define KSAUDIO_SPEAKER_SURROUND        (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | \
                                         SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER)
#define KSAUDIO_SPEAKER_5POINT1         (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | \
                                         SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | \
                                         SPEAKER_BACK_LEFT  | SPEAKER_BACK_RIGHT)
#define KSAUDIO_SPEAKER_7POINT1         (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | \
                                         SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | \
                                         SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | \
                                         SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER)

#if ( (NTDDI_VERSION >= NTDDI_WINXPSP2) && (NTDDI_VERSION < NTDDI_WS03) ) || (NTDDI_VERSION >= NTDDI_WS03SP1)

#define KSAUDIO_SPEAKER_5POINT1_SURROUND (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | \
                                         SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | \
                                         SPEAKER_SIDE_LEFT  | SPEAKER_SIDE_RIGHT)
#define KSAUDIO_SPEAKER_7POINT1_SURROUND (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | \
                                         SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | \
                                         SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | \
                                         SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT)
// The following are obsolete 5.1 and 7.1 settings (they lack side speakers).  Note this means
// that the default 5.1 and 7.1 settings (KSAUDIO_SPEAKER_5POINT1 and KSAUDIO_SPEAKER_7POINT1 are
// similarly obsolete but are unchanged for compatibility reasons).
#define KSAUDIO_SPEAKER_5POINT1_BACK     KSAUDIO_SPEAKER_5POINT1
#define KSAUDIO_SPEAKER_7POINT1_WIDE     KSAUDIO_SPEAKER_7POINT1

#endif // XP SP2 and later (chronologically)

// DVD Speaker Positions
#define KSAUDIO_SPEAKER_GROUND_FRONT_LEFT   SPEAKER_FRONT_LEFT
#define KSAUDIO_SPEAKER_GROUND_FRONT_CENTER SPEAKER_FRONT_CENTER
#define KSAUDIO_SPEAKER_GROUND_FRONT_RIGHT  SPEAKER_FRONT_RIGHT
#define KSAUDIO_SPEAKER_GROUND_REAR_LEFT    SPEAKER_BACK_LEFT
#define KSAUDIO_SPEAKER_GROUND_REAR_RIGHT   SPEAKER_BACK_RIGHT
#define KSAUDIO_SPEAKER_TOP_MIDDLE          SPEAKER_TOP_CENTER
#define KSAUDIO_SPEAKER_SUPER_WOOFER        SPEAKER_LOW_FREQUENCY
#endif //__KSMEDIA__

#pragma pack(pop)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWaves::CWaves()
{
	memset(&m_WaveIDs, 0, sizeof(m_WaveIDs));
}

CWaves::~CWaves()
{
	long lLoop;

	for (lLoop = 0; lLoop < MAX_NUM_WAVEID; lLoop++)
	{
		if (m_WaveIDs[lLoop])
		{
			if (m_WaveIDs[lLoop]->pData)
				delete m_WaveIDs[lLoop]->pData;

			if (m_WaveIDs[lLoop]->pFile)
				fclose(m_WaveIDs[lLoop]->pFile);

			delete m_WaveIDs[lLoop];
			m_WaveIDs[lLoop] = 0;
		}
	}
}


WAVERESULT CWaves::LoadWaveFile(const char *szFilename, WAVEID *pWaveID)
{
	WAVERESULT wr = WR_OUTOFMEMORY;
	LPWAVEFILEINFO pWaveInfo;

	pWaveInfo = new WAVEFILEINFO;
	if (pWaveInfo)
	{
		if (WAVE_SUCCEEDED(wr = ParseFile(szFilename, pWaveInfo)))
		{
			// Allocate memory for sample data
			pWaveInfo->pData = new char[pWaveInfo->ulDataSize];
			if (pWaveInfo->pData)
			{
				// Seek to start of audio data
				fseek(pWaveInfo->pFile, pWaveInfo->ulDataOffset, SEEK_SET);

				// Read Sample Data
				if (fread(pWaveInfo->pData, 1, pWaveInfo->ulDataSize, pWaveInfo->pFile) == pWaveInfo->ulDataSize)
				{
					long lLoop = 0;
					for (lLoop = 0; lLoop < MAX_NUM_WAVEID; lLoop++)
					{
						if (!m_WaveIDs[lLoop])
						{
							m_WaveIDs[lLoop] = pWaveInfo;
							*pWaveID = lLoop;
							wr = WR_OK;
							break;
						}
					}

					if (lLoop == MAX_NUM_WAVEID)
					{
						delete pWaveInfo->pData;
						wr = WR_OUTOFMEMORY;
					}
				}
				else
				{
					delete pWaveInfo->pData;
					wr = WR_BADWAVEFILE;
				}
			}
			else
			{
				wr = WR_OUTOFMEMORY;
			}

			fclose(pWaveInfo->pFile);
			pWaveInfo->pFile = 0;
		}

		if (wr != WR_OK)
			delete pWaveInfo;
	}

	return wr;
}

WAVERESULT CWaves::OpenWaveFile(const char *szFilename, WAVEID *pWaveID)
{
	WAVERESULT wr = WR_OUTOFMEMORY;
	LPWAVEFILEINFO pWaveInfo;

	pWaveInfo = new WAVEFILEINFO;
	if (pWaveInfo)
	{
		if (WAVE_SUCCEEDED(wr = ParseFile(szFilename, pWaveInfo)))
		{
			long lLoop = 0;
			for (lLoop = 0; lLoop < MAX_NUM_WAVEID; lLoop++)
			{
				if (!m_WaveIDs[lLoop])
				{
					m_WaveIDs[lLoop] = pWaveInfo;
					*pWaveID = lLoop;
					wr = WR_OK;
					break;
				}
			}

			if (lLoop == MAX_NUM_WAVEID)
				wr = WR_OUTOFMEMORY;
		}

		if (wr != WR_OK)
			delete pWaveInfo;
	}

	return wr;
}

WAVERESULT CWaves::ReadWaveData(WAVEID WaveID, void *pData, unsigned long ulDataSize, unsigned long *pulBytesWritten)
{
	LPWAVEFILEINFO pWaveInfo;
	WAVERESULT wr = WR_BADWAVEFILE;

	if (!pData || !pulBytesWritten || ulDataSize == 0)
		return WR_INVALIDPARAM;

	if (IsWaveID(WaveID))
	{
		pWaveInfo = m_WaveIDs[WaveID];
		if (pWaveInfo->pFile)
		{
			unsigned long ulOffset = ftell(pWaveInfo->pFile);

			if ((ulOffset - pWaveInfo->ulDataOffset + ulDataSize) > pWaveInfo->ulDataSize)
				ulDataSize = pWaveInfo->ulDataSize - (ulOffset - pWaveInfo->ulDataOffset);

			*pulBytesWritten = (unsigned long)fread(pData, 1, ulDataSize, pWaveInfo->pFile);

			wr = WR_OK;
		}
	}
	else
	{
		wr = WR_INVALIDWAVEID;
	}

	return wr;
}

WAVERESULT CWaves::SetWaveDataOffset(WAVEID WaveID, unsigned long ulOffset)
{
	LPWAVEFILEINFO pWaveInfo;
	WAVERESULT wr = WR_INVALIDPARAM;

	if (IsWaveID(WaveID))
	{
		pWaveInfo = m_WaveIDs[WaveID];
		if (pWaveInfo->pFile)
		{
			// Seek into audio data
			fseek(pWaveInfo->pFile, pWaveInfo->ulDataOffset + ulOffset, SEEK_SET);
			wr = WR_OK;
		}
	}
	else
	{
		wr = WR_INVALIDWAVEID;
	}

	return wr;
}

WAVERESULT CWaves::GetWaveDataOffset(WAVEID WaveID, unsigned long *pulOffset)
{
	LPWAVEFILEINFO pWaveInfo;
	WAVERESULT wr = WR_INVALIDPARAM;

	if (IsWaveID(WaveID))
	{
		pWaveInfo = m_WaveIDs[WaveID];
		if ((pWaveInfo->pFile) && (pulOffset))
		{
			// Get current position
			*pulOffset = ftell(pWaveInfo->pFile);
			*pulOffset -= pWaveInfo->ulDataOffset;
			wr = WR_OK;
		}
	}
	else
	{
		wr = WR_INVALIDWAVEID;
	}

	return wr;
}

WAVERESULT CWaves::ParseFile(const char *szFilename, LPWAVEFILEINFO pWaveInfo)
{
	WAVEFILEHEADER	waveFileHeader;
	RIFFCHUNK		riffChunk;
	WAVEFMT			waveFmt;
	WAVERESULT		wr = WR_BADWAVEFILE;

	if (!szFilename || !pWaveInfo)
		return WR_INVALIDPARAM;

	memset(pWaveInfo, 0, sizeof(WAVEFILEINFO));

	// Open the wave file for reading
	fopen_s(&pWaveInfo->pFile, szFilename, "rb");
	if (pWaveInfo->pFile)
	{
		// Read Wave file header
		fread(&waveFileHeader, 1, sizeof(WAVEFILEHEADER), pWaveInfo->pFile);
		if (strnEqual(waveFileHeader.szRIFF, "RIFF", 4) && strnEqual(waveFileHeader.szWAVE, "WAVE", 4))
		{
			while (fread(&riffChunk, 1, sizeof(RIFFCHUNK), pWaveInfo->pFile) == sizeof(RIFFCHUNK))
			{
				if (strnEqual(riffChunk.szChunkName, "fmt ", 4))
				{
					if (riffChunk.ulChunkSize <= sizeof(WAVEFMT))
					{
						fread(&waveFmt, 1, riffChunk.ulChunkSize, pWaveInfo->pFile);

						// Determine if this is a WAVEFORMATEX or WAVEFORMATEXTENSIBLE wave file
						if (waveFmt.usFormatTag == WAVE_FORMAT_PCM)
						{
							pWaveInfo->wfType = WF_EX;
							memcpy(&pWaveInfo->wfEXT.Format, &waveFmt, sizeof(PCMWAVEFORMAT));
						}
						else if (waveFmt.usFormatTag == WAVE_FORMAT_EXTENSIBLE)
						{
							pWaveInfo->wfType = WF_EXT;
							memcpy(&pWaveInfo->wfEXT, &waveFmt, sizeof(WAVEFORMATEXTENSIBLE));
						}
					}
					else
					{
						fseek(pWaveInfo->pFile, riffChunk.ulChunkSize, SEEK_CUR);
					}
				}
				else if (strnEqual(riffChunk.szChunkName, "data", 4))
				{
					pWaveInfo->ulDataSize = riffChunk.ulChunkSize;
					pWaveInfo->ulDataOffset = ftell(pWaveInfo->pFile);
					fseek(pWaveInfo->pFile, riffChunk.ulChunkSize, SEEK_CUR);
				}
				else
				{
					fseek(pWaveInfo->pFile, riffChunk.ulChunkSize, SEEK_CUR);
				}

				// Ensure that we are correctly aligned for next chunk
				if (riffChunk.ulChunkSize & 1)
					fseek(pWaveInfo->pFile, 1, SEEK_CUR);
			}

			if (pWaveInfo->ulDataSize && pWaveInfo->ulDataOffset && ((pWaveInfo->wfType == WF_EX) || (pWaveInfo->wfType == WF_EXT)))
				wr = WR_OK;
			else
				fclose(pWaveInfo->pFile);
		}
	}
	else
	{
		wr = WR_INVALIDFILENAME;
	}

	return wr;
}

WAVERESULT CWaves::ParseBuffer(gm::GMMemoryStream& ms, LPWAVEFILEINFO pWaveInfo)
{
	WAVEFILEHEADER	waveFileHeader;
	RIFFCHUNK		riffChunk;
	WAVEFMT			waveFmt;
	WAVERESULT		wr = WR_BADWAVEFILE;

	if (!pWaveInfo)
		return WR_INVALIDPARAM;

	ms.read(reinterpret_cast<gm::GMbyte*>(&waveFileHeader), sizeof(WAVEFILEHEADER));

	if (strnEqual(waveFileHeader.szRIFF, "RIFF", 4) && strnEqual(waveFileHeader.szWAVE, "WAVE", 4))
	{
		while (ms.read(reinterpret_cast<gm::GMbyte*>(&riffChunk), sizeof(RIFFCHUNK)) == sizeof(RIFFCHUNK))
		{
			if (strnEqual(riffChunk.szChunkName, "fmt ", 4))
			{
				if (riffChunk.ulChunkSize <= sizeof(WAVEFMT))
				{
					ms.read(reinterpret_cast<gm::GMbyte*>(&waveFmt), riffChunk.ulChunkSize);

					// Determine if this is a WAVEFORMATEX or WAVEFORMATEXTENSIBLE wave file
					if (waveFmt.usFormatTag == WAVE_FORMAT_PCM)
					{
						pWaveInfo->wfType = WF_EX;
						memcpy(&pWaveInfo->wfEXT.Format, &waveFmt, sizeof(PCMWAVEFORMAT));
					}
					else if (waveFmt.usFormatTag == WAVE_FORMAT_EXTENSIBLE)
					{
						pWaveInfo->wfType = WF_EXT;
						memcpy(&pWaveInfo->wfEXT, &waveFmt, sizeof(WAVEFORMATEXTENSIBLE));
					}
				}
				else
				{
					ms.seek(riffChunk.ulChunkSize, gm::GMMemoryStream::FromNow);
				}
			}
			else if (strnEqual(riffChunk.szChunkName, "data", 4))
			{
				pWaveInfo->ulDataSize = riffChunk.ulChunkSize;
				pWaveInfo->ulDataOffset = ms.tell();
				ms.seek(riffChunk.ulChunkSize, gm::GMMemoryStream::FromNow);
			}
			else
			{
				ms.seek(riffChunk.ulChunkSize, gm::GMMemoryStream::FromNow);
			}

			// Ensure that we are correctly aligned for next chunk
			if (riffChunk.ulChunkSize & 1)
				ms.seek(1, gm::GMMemoryStream::FromNow);
		}

		if (pWaveInfo->ulDataSize && pWaveInfo->ulDataOffset && ((pWaveInfo->wfType == WF_EX) || (pWaveInfo->wfType == WF_EXT)))
			wr = WR_OK;
	}
	else
	{
		wr = WR_BADWAVEFILE;
	}
	return wr;
}

WAVERESULT CWaves::LoadWaveBuffer(const gm::GMBuffer& buffer, WAVEID *pWaveID)
{
	WAVERESULT wr = WR_OUTOFMEMORY;
	LPWAVEFILEINFO pWaveInfo;

	pWaveInfo = new WAVEFILEINFO;
	pWaveInfo->pFile = nullptr; //NOT A FILE
	if (pWaveInfo)
	{
		gm::GMMemoryStream ms(buffer.buffer, buffer.size);
		if (WAVE_SUCCEEDED(wr = ParseBuffer(ms, pWaveInfo)))
		{
			pWaveInfo->pData = new char[pWaveInfo->ulDataSize];
			if (pWaveInfo->pData)
			{
				// Seek to start of audio data
				ms.seek(pWaveInfo->ulDataOffset, gm::GMMemoryStream::FromStart);
				
				// Read Sample Data
				if (ms.read(reinterpret_cast<gm::GMbyte*>(pWaveInfo->pData), pWaveInfo->ulDataSize) == pWaveInfo->ulDataSize)
				{
					long lLoop = 0;
					for (lLoop = 0; lLoop < MAX_NUM_WAVEID; lLoop++)
					{
						if (!m_WaveIDs[lLoop])
						{
							m_WaveIDs[lLoop] = pWaveInfo;
							*pWaveID = lLoop;
							wr = WR_OK;
							break;
						}
					}

					if (lLoop == MAX_NUM_WAVEID)
					{
						delete pWaveInfo->pData;
						wr = WR_OUTOFMEMORY;
					}
				}
				else
				{
					delete pWaveInfo->pData;
					wr = WR_BADWAVEFILE;
				}
			}
			else
			{
				wr = WR_OUTOFMEMORY;
			}
		}

		if (wr != WR_OK)
			delete pWaveInfo;
	}

	return wr;
}

WAVERESULT CWaves::DeleteWaveFile(WAVEID WaveID)
{
	WAVERESULT wr = WR_OK;

	if (IsWaveID(WaveID))
	{
		if (m_WaveIDs[WaveID]->pData)
			delete m_WaveIDs[WaveID]->pData;

		if (m_WaveIDs[WaveID]->pFile)
			fclose(m_WaveIDs[WaveID]->pFile);

		delete m_WaveIDs[WaveID];
		m_WaveIDs[WaveID] = 0;
	}
	else
	{
		wr = WR_INVALIDWAVEID;
	}

	return wr;
}

WAVERESULT CWaves::GetWaveType(WAVEID WaveID, WAVEFILETYPE *wfType)
{
	if (!IsWaveID(WaveID))
		return WR_INVALIDWAVEID;

	if (!wfType)
		return WR_INVALIDPARAM;

	*wfType = m_WaveIDs[WaveID]->wfType;

	return WR_OK;
}

WAVERESULT CWaves::GetWaveFormatExHeader(WAVEID WaveID, WAVEFORMATEX *wfex)
{
	if (!IsWaveID(WaveID))
		return WR_INVALIDWAVEID;

	if (!wfex)
		return WR_INVALIDPARAM;

	memcpy(wfex, &(m_WaveIDs[WaveID]->wfEXT.Format), sizeof(WAVEFORMATEX));

	return WR_OK;
}

WAVERESULT CWaves::GetWaveFormatExtensibleHeader(WAVEID WaveID, WAVEFORMATEXTENSIBLE *wfext)
{
	if (!IsWaveID(WaveID))
		return WR_INVALIDWAVEID;

	if (m_WaveIDs[WaveID]->wfType != WF_EXT)
		return WR_NOTWAVEFORMATEXTENSIBLEFORMAT;

	if (!wfext)
		return WR_INVALIDPARAM;

	memcpy(wfext, &(m_WaveIDs[WaveID]->wfEXT), sizeof(WAVEFORMATEXTENSIBLE));

	return WR_OK;
}

WAVERESULT CWaves::GetWaveData(WAVEID WaveID, void **lplpAudioData)
{
	if (!IsWaveID(WaveID))
		return WR_INVALIDWAVEID;

	if (!lplpAudioData)
		return WR_INVALIDPARAM;

	*lplpAudioData = m_WaveIDs[WaveID]->pData;

	return WR_OK;
}

WAVERESULT CWaves::GetWaveSize(WAVEID WaveID, unsigned long *size)
{
	if (!IsWaveID(WaveID))
		return WR_INVALIDWAVEID;

	if (!size)
		return WR_INVALIDPARAM;

	*size = m_WaveIDs[WaveID]->ulDataSize;

	return WR_OK;
}

WAVERESULT CWaves::GetWaveFrequency(WAVEID WaveID, unsigned long *pulFrequency)
{
	WAVERESULT wr = WR_OK;

	if (IsWaveID(WaveID))
	{
		if (pulFrequency)
			*pulFrequency = m_WaveIDs[WaveID]->wfEXT.Format.nSamplesPerSec;
		else
			wr = WR_INVALIDPARAM;
	}
	else
	{
		wr = WR_INVALIDWAVEID;
	}

	return wr;
}

WAVERESULT CWaves::GetWaveALBufferFormat(WAVEID WaveID, PFNALGETENUMVALUE pfnGetEnumValue, unsigned long *pulFormat)
{
	WAVERESULT wr = WR_OK;

	if (IsWaveID(WaveID))
	{
		if (pfnGetEnumValue && pulFormat)
		{
			*pulFormat = 0;

			if (m_WaveIDs[WaveID]->wfType == WF_EX)
			{
				if (m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 1)
				{
					switch (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample)
					{
					case 4:
						*pulFormat = pfnGetEnumValue("AL_FORMAT_MONO_IMA4");
						break;
					case 8:
						*pulFormat = pfnGetEnumValue("AL_FORMAT_MONO8");
						break;
					case 16:
						*pulFormat = pfnGetEnumValue("AL_FORMAT_MONO16");
						break;
					}
				}
				else if (m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 2)
				{
					switch (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample)
					{
					case 4:
						*pulFormat = pfnGetEnumValue("AL_FORMAT_STEREO_IMA4");
						break;
					case 8:
						*pulFormat = pfnGetEnumValue("AL_FORMAT_STEREO8");
						break;
					case 16:
						*pulFormat = pfnGetEnumValue("AL_FORMAT_STEREO16");
						break;
					}
				}
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 4) && (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16))
					*pulFormat = pfnGetEnumValue("AL_FORMAT_QUAD16");
			}
			else if (m_WaveIDs[WaveID]->wfType == WF_EXT)
			{
				if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 1) &&
					((m_WaveIDs[WaveID]->wfEXT.dwChannelMask == SPEAKER_FRONT_CENTER) ||
					 (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT)) ||
					 (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == 0)))
				{
					switch (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample)
					{
					case 4:
						*pulFormat = pfnGetEnumValue("AL_FORMAT_MONO_IMA4");
						break;
					case 8:
						*pulFormat = pfnGetEnumValue("AL_FORMAT_MONO8");
						break;
					case 16:
						*pulFormat = pfnGetEnumValue("AL_FORMAT_MONO16");
						break;
					}
				}
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 2) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT)))
				{
					switch (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample)
					{
					case 4:
						*pulFormat = pfnGetEnumValue("AL_FORMAT_STEREO_IMA4");
						break;
					case 8:
						*pulFormat = pfnGetEnumValue("AL_FORMAT_STEREO8");
						break;
					case 16:
						*pulFormat = pfnGetEnumValue("AL_FORMAT_STEREO16");
						break;
					}
				}
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 2) && (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)))
					*pulFormat =  pfnGetEnumValue("AL_FORMAT_REAR16");
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 4) && (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)))
					*pulFormat = pfnGetEnumValue("AL_FORMAT_QUAD16");
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 6) && (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)))
					*pulFormat = pfnGetEnumValue("AL_FORMAT_51CHN16");
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 7) && (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_BACK_CENTER)))
					*pulFormat = pfnGetEnumValue("AL_FORMAT_61CHN16");
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 8) && (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT)))
					*pulFormat = pfnGetEnumValue("AL_FORMAT_71CHN16");
			}

			if (*pulFormat == 0)
				wr = WR_INVALIDWAVEFILETYPE;
		}
		else
		{
			wr = WR_INVALIDPARAM;
		}
	}
	else
	{
		wr = WR_INVALIDWAVEID;
	}

	return wr;
}

bool CWaves::IsWaveID(WAVEID WaveID)
{
	bool bReturn = false;

	if ((WaveID >= 0) && (WaveID < MAX_NUM_WAVEID))
	{
		if (m_WaveIDs[WaveID])
			bReturn = true;
	}

	return bReturn;
}

char *CWaves::GetErrorString(WAVERESULT wr, char *szErrorString, unsigned long nSizeOfErrorString)
{
	switch (wr)
	{
		case WR_OK:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "Success\n", nSizeOfErrorString-1);
			break;

		case WR_INVALIDFILENAME:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "Invalid file name or file does not exist\n", nSizeOfErrorString-1);
			break;

		case WR_BADWAVEFILE:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "Invalid Wave file\n", nSizeOfErrorString-1);
			break;

		case WR_INVALIDPARAM:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "Invalid parameter passed to function\n", nSizeOfErrorString-1);
			break;

		case WR_FILEERROR:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "File I/O error\n", nSizeOfErrorString-1);
			break;

		case WR_INVALIDWAVEID:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "Invalid WAVEID\n", nSizeOfErrorString-1);
			break;

		case WR_NOTSUPPORTEDYET:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "Function not supported yet\n", nSizeOfErrorString-1);
			break;

		case WR_WAVEMUSTBEMONO:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "Input wave files must be mono\n", nSizeOfErrorString-1);
			break;

		case WR_WAVEMUSTBEWAVEFORMATPCM:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "Input wave files must be in Wave Format PCM\n", nSizeOfErrorString-1);
			break;

		case WR_WAVESMUSTHAVESAMEBITRESOLUTION:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "Input wave files must have the same Bit Resolution\n", nSizeOfErrorString-1);
			break;

		case WR_WAVESMUSTHAVESAMEFREQUENCY:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "Input wave files must have the same Frequency\n", nSizeOfErrorString-1);
			break;

		case WR_WAVESMUSTHAVESAMEBITRATE:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "Input wave files must have the same Bit Rate\n", nSizeOfErrorString-1);
			break;

		case WR_WAVESMUSTHAVESAMEBLOCKALIGNMENT:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "Input wave files must have the same Block Alignment\n", nSizeOfErrorString-1);
			break;

		case WR_OFFSETOUTOFDATARANGE:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "Wave files Offset is not within audio data\n", nSizeOfErrorString-1);
			break;

		case WR_INVALIDSPEAKERPOS:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "Invalid Speaker Destinations\n", nSizeOfErrorString-1);
			break;

		case WR_OUTOFMEMORY:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "Out of memory\n", nSizeOfErrorString-1);
			break;

		case WR_INVALIDWAVEFILETYPE:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "Invalid Wave File Type\n", nSizeOfErrorString-1);
			break;

		case WR_NOTWAVEFORMATEXTENSIBLEFORMAT:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "Wave file is not in WAVEFORMATEXTENSIBLE format\n", nSizeOfErrorString-1);
			break;

		default:
			gm::GMString::stringCopyN(szErrorString, nSizeOfErrorString, "Undefined error\n", nSizeOfErrorString-1);
	}
	szErrorString[nSizeOfErrorString-1] = '\0';
	return szErrorString;
}
