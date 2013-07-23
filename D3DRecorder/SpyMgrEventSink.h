
#pragma once

class CHookEngine;

typedef enum {
  dispidNktSpyMgrEventsOnCustomDllLoad = 1,
  dispidNktSpyMgrEventsOnCustomDllUnload,
  dispidNktSpyMgrEventsOnCustomApiCall,
  dispidNktSpyMgrEventsOnHookStateChanged,
  dispidNktSpyMgrEventsOnFunctionCalled,
  dispidNktSpyMgrEventsOnCreateProcessCall,
  dispidNktSpyMgrEventsOnLoadLibraryCall,
  dispidNktSpyMgrEventsOnFreeLibraryCall,
  dispidNktSpyMgrEventsOnProcessStarted,
  dispidNktSpyMgrEventsOnProcessTerminated,
  dispidNktSpyMgrEventsOnAgentLoad,
  dispidNktSpyMgrEventsOnAgentUnload,
} eDispidNktSpyMgrEvents;


// CSpyMgrEventSink command target

class CSpyMgrEventSink : public CCmdTarget
{
	DECLARE_DYNAMIC(CSpyMgrEventSink)

public:
	CSpyMgrEventSink(CHookEngine*);
	virtual ~CSpyMgrEventSink();

	virtual void OnFinalRelease();

protected:
	DECLARE_MESSAGE_MAP()
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()

private:

	CHookEngine* m_pHookEng;

	void OnFunctionCalled (__in Deviare2::INktHook *Hook, __in Deviare2::INktProcess *proc,	 __in Deviare2::INktHookCallInfo *callInfo);
	void OnProcessTerminated(Deviare2::INktProcess*);
};


