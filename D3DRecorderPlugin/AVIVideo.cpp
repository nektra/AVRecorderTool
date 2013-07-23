#include "stdafx.h"
#include "AVIVideo.h"
#include "../Common/utils.h"

#pragma comment(lib, "winmm.lib")
#pragma comment(lib,"vfw32.lib")

CAVIVideo::CAVIVideo(void) : 
	_pIAVIFile(0)	
{
	AVIFileInit();
	ZeroMemory(&_aviData, sizeof(AVIDATA));	
}

CAVIVideo::~CAVIVideo(void)
{
	Close();
	AVIFileExit();
}

HRESULT CAVIVideo::Create(wchar_t* fname)
{
	if (_pIAVIFile)
	{
		d_printf(L"%s: Still with OpenFile ref count >1, close file first.\n",__FUNCTIONW__);
		return AVIERR_ERROR;
	}
	else
	{
		_aviData.nFrames = 0;
		return AVIFileOpen(&_pIAVIFile, fname, OF_WRITE | OF_CREATE, NULL);
	}
}

void CAVIVideo::SetVideoOptions(const char codec[4], FRAMESIZE *fsz, int fps)
{
	ZeroMemory(&_aviData.vidStreamInfo,sizeof(AVISTREAMINFO));

	RECT rcFrame;
	SetRect(&rcFrame, 0, 0, fsz->width, fsz->height);

	_aviData.fourCC = mmioFOURCC(codec[0],codec[1],codec[2],codec[3]); // or 0??

	_aviData.vidStreamInfo.fccType			= streamtypeVIDEO;
	_aviData.vidStreamInfo.fccHandler		= 0; 
	_aviData.vidStreamInfo.dwScale			= 1;
	_aviData.vidStreamInfo.dwRate			= fps;
	_aviData.vidStreamInfo.dwSuggestedBufferSize = fsz->height * fsz->width * 4;
	_aviData.vidStreamInfo.dwQuality		= (DWORD)-1;
	_aviData.vidStreamInfo.rcFrame			= rcFrame;
	StringCchCopy(_aviData.vidStreamInfo.szName, 64, L"Nektra D3DRECORDER Video Stream");	
}
	
HRESULT CAVIVideo::GrabFrame(LPVOID pDIBFile)	
{
	if (!pDIBFile)
		return E_POINTER;

	if (!_pIAVIFile)
	{
		d_printf(L"_pIAVIFile==0\n");
		return E_UNEXPECTED;
	}

	// Skip past BITMAPFILEHEADER (but check at least signature...)

	BITMAPFILEHEADER* pHdr = (BITMAPFILEHEADER*)pDIBFile;
	
	if (pHdr->bfType != 0x4D42)
	{
		d_printf(L"%s  ERROR File header not 0x4D42\n", __FUNCTIONW__);
		return E_INVALIDARG;
	}

	BITMAPINFO* pBitmapInfo = (BITMAPINFO*) ((BYTE*)pDIBFile + sizeof(BITMAPFILEHEADER));
	BITMAPINFOHEADER* pDibHdr = &pBitmapInfo->bmiHeader;
	
	HRESULT hr;

	
	if (_aviData.nFrames == 0)
	{
		// Create a video stream

		if (SUCCEEDED(hr = AVIFileCreateStream(_pIAVIFile, 
			&_aviData.pVidStream, 
			&_aviData.vidStreamInfo)))
		{
			AVICOMPRESSOPTIONS copt;
			ZeroMemory(&copt,sizeof(AVICOMPRESSOPTIONS));

			AVICOMPRESSOPTIONS* pOpt[1];
			pOpt[0] = &copt;			

			//// Get default compressor options

			INT_PTR r = AVISaveOptions(GetDesktopWindow(), 0, 1, &_aviData.pVidStream, pOpt);

			if (r)
			{				
				if (SUCCEEDED(hr = AVIMakeCompressedStream(&_aviData.pCompVidStream, 
					_aviData.pVidStream, 
					pOpt[0], 
					NULL)))
				{
		
					if (FAILED(hr = AVIStreamSetFormat(_aviData.pCompVidStream, 
						0, 
						pDibHdr, 
						sizeof(BITMAPINFOHEADER))))
					{
						d_printf(L"AVISetStreamFormat failed. HR = 0x%08x\n", hr);

						DumpDIBHeader(pDibHdr);

						return hr;
					}

					AVISaveOptionsFree(1, pOpt);
				}
				else
				{
					d_printf(L"AVIMakeCompressedStream failed. HR = 0x%08x\n", hr);
					DumpDIBHeader(pDibHdr);
				}
			}
			else
			{
				d_printf(L"user cancelled video options, or error ocurred (0x%08x)\n", r);
			}
		}
		else
		{
			d_printf(L"AVIFileCreateStream failed. HR = 0x%08x\n", hr);
			DumpDIBHeader(pDibHdr);
		}
	}

	//
	// Write to stream
	//

	LONG lBytesWritten = 0;

	if (FAILED(hr = AVIStreamWrite(_aviData.pCompVidStream, 
		_aviData.nFrames,
		1,
		((BYTE*)pHdr + pHdr->bfOffBits),		
		pDibHdr->biWidth * pDibHdr->biHeight * (pDibHdr->biBitCount >> 3),
		AVIIF_KEYFRAME,
		0,
		&lBytesWritten)))
	{
		d_printf(L"AVIStreamWrite failed. HR = 0x%08x\n", hr);
	}
	else
	{
		d_printf(L"AVIStreamWrite commited %d bytes to AVI stream\n", lBytesWritten);
	}	

	_aviData.nFrames++;
	
	return hr;
}

void CAVIVideo::SetAudioOptions()
{
	_aviData.audStreamOpt.fccType = streamtypeAUDIO;
}

void CAVIVideo::Close()
{
	d_printf(__FUNCTIONW__);

	if (_pIAVIFile)
	{
		AVIFileRelease(_pIAVIFile);

		if (_aviData.pVidStream)
		{
			AVIStreamRelease(_aviData.pVidStream);
			_aviData.pVidStream = NULL;
		}
		if (_aviData.pCompVidStream)
		{
			AVIStreamRelease(_aviData.pCompVidStream);
			_aviData.pCompVidStream = NULL;
		}
	
		if (_aviData.pAudStream)
		{
			AVIStreamRelease(_aviData.pAudStream);
			_aviData.pAudStream = NULL;
		}
		if (_aviData.pCompAudStream)
		{
			AVIStreamRelease(_aviData.pCompAudStream);
			_aviData.pCompAudStream = NULL;
		}

		_pIAVIFile = NULL;
	}
	else
	{
		d_printf(L"%s: WARNING Called without file references.\n",__FUNCTIONW__);
	}
}
