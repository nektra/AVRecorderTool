#pragma once

//
//

struct FRAMESIZE
{
	unsigned int height;
	unsigned int width;
};

struct AVIDATA
{
	AVICOMPRESSOPTIONS	vidStreamOpt;
	AVICOMPRESSOPTIONS	audStreamOpt;
	AVISTREAMINFO		vidStreamInfo;	
	IAVIStream*			pVidStream;
	IAVIStream*			pCompVidStream;
	IAVIStream*			pAudStream;
	IAVIStream*			pCompAudStream;
	FRAMESIZE			size;
	int					fps;
	int					nFrames;
	DWORD				fourCC;
	bool				fActive;
};

class CAVIVideo
{
	IAVIFile* _pIAVIFile;
	AVIDATA _aviData;

public:
	CAVIVideo(void);
	~CAVIVideo(void);
	
	HRESULT Create(wchar_t* szAviFile);
	void Close();

	void SetVideoOptions(const char vCodec[4], FRAMESIZE *size, int fps);
	void SetAudioOptions();
	HRESULT StartRecord();
	HRESULT GrabFrame(LPVOID pDIBFile);
	int GetFrameCount() const { return _aviData.nFrames; }
	void StopRecord();
};

