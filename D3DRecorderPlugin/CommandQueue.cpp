#include "stdafx.h"
#include "CommandQueue.h"
#include "../Common/utils.h"

CCommandQueue::CCommandQueue(void)
{	
	try
	{
	InitializeCriticalSection(&_cs);
	}
	catch (...)
	{
		d_printf(L"%s: InitializeCriticalSection exception.",__FUNCTIONW__);
	}
}

bool CCommandQueue::IsEmpty()
{
	EnterCriticalSection(&_cs);
	bool r =  _q.empty();
	LeaveCriticalSection(&_cs);

	return r;
}

CCommand CCommandQueue::_peek(bool fRemove)
{
	CCommand c;
	memset(&c,0,sizeof(CCommand));

	if (!IsEmpty())
	{		
		EnterCriticalSection(&_cs);
		c = _q.front();
		
		if (fRemove)
		{
			_q.pop();
		}

		LeaveCriticalSection(&_cs);
	}	

	return c;
}

CCommand CCommandQueue::Peek()
{
	return _peek();
}

void CCommandQueue::Enqueue(const CCommand& c)
{
	EnterCriticalSection(&_cs);
	_q.push(c);	
	LeaveCriticalSection(&_cs);
}

CCommand CCommandQueue::Dequeue() 
{	
	return _peek(true);
}

CCommandQueue::~CCommandQueue(void)
{
	DeleteCriticalSection(&_cs);
}
