
// stdafx.h : �� ������� �ʰ� ���� ����ϴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������ 
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
#endif

#define CURL_STATICLIB
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wldap32.lib")

#define NOMINMAX
#include <windows.h>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")


#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4651)
#endif

#include <algorithm>
#include <deque>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <map>
#include <queue>
//#include <strstream>
#include <string>
#include <vector>

// MS
#include <strsafe.h>
//#include <afxdlgs.h>
#include <assert.h>

/////////////////////////////////////

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

namespace Debug
{
    enum DebugType
    {
        DEBUG_LOG,
        DEBUG_NETWORK,
        DEBUG_WARNING,
        DEBUG_ERROR,

        MAX
    };;

    inline const char* const* EnumNamesDebugType()
    {
        static const char* const names[DebugType::MAX + 1] = {
          "DEBUG_LOG",
          "DEBUG_NETWORK",
          "DEBUG_WARNING",
          "DEBUG_ERROR",
          "MAX"
        };

        return names;
    }
}