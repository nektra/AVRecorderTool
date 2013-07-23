#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <Vfw.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>
#include <cassert>

#include <strsafe.h>

// C++ headers
//
#include <queue>
#include <vector>
#include <sstream>
#include <algorithm>

// D3D

#include <d3d9.h>
#include <d3dx9tex.h>

#ifdef _M_IX86
#import "../dll/deviarecom.dll" raw_interfaces_only, named_guids, raw_dispinterfaces, auto_rename
#elif defined _M_X64
#import "../dll/deviareCom64.dll" raw_interfaces_only, named_guids, raw_dispinterfaces, auto_rename
#else
#error "Unsupported system architecture"
#endif
