#pragma once
#include "stdafx.h"

enum COMMANDID
{
	CMD_NULL,
	CMD_VIDEORECORD_START,
	CMD_VIDEORECORD_STOP,
	CMD_TAKE_SCREENSHOT
};

class CCommand
{
public:
	COMMANDID id;
};

class CCommandQueue
{
	CRITICAL_SECTION		_cs;
	std::queue<CCommand>	_q;

	CCommand _peek(bool fRemove = false);

public:
	CCommandQueue(void);
	~CCommandQueue(void);

	void Enqueue(const CCommand& cc); 
	CCommand Dequeue();
	CCommand Peek();
	bool IsEmpty();
};

