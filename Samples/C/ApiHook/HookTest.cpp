/*
 * Copyright (C) 2010-2014 Nektra S.A., Buenos Aires, Argentina.
 * All rights reserved.
 *
 **/

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "..\..\..\Include\NktHookLib.h"

//-----------------------------------------------------------

#if _MSC_VER >= 1700
  #define X_LIBPATH "2012"
#elif  _MSC_VER >= 1600
  #define X_LIBPATH "2010"
#else
  #define X_LIBPATH "2008"
#endif

#if defined _M_IX86
  #ifdef _DEBUG
    #pragma comment (lib, "..\\..\\..\\..\\Libs\\" X_LIBPATH "\\NktHookLib_Debug.lib")
  #else //_DEBUG
    #pragma comment (lib, "..\\..\\..\\..\\Libs\\" X_LIBPATH "\\NktHookLib.lib")
  #endif //_DEBUG
#elif defined _M_X64
  #ifdef _DEBUG
    #pragma comment (lib, "..\\..\\..\\..\\Libs\\" X_LIBPATH "\\NktHookLib64_Debug.lib")
  #else //_DEBUG
    #pragma comment (lib, "..\\..\\..\\..\\Libs\\" X_LIBPATH "\\NktHookLib64.lib")
  #endif //_DEBUG
#else
  #error Unsupported platform
#endif

//-----------------------------------------------------------

typedef int (WINAPI *lpfnMessageBoxW)(__in_opt HWND hWnd, __in_opt LPCWSTR lpText, __in_opt LPCWSTR lpCaption, __in UINT uType);
static int WINAPI Hooked_MessageBoxW(__in_opt HWND hWnd, __in_opt LPCWSTR lpText, __in_opt LPCWSTR lpCaption, __in UINT uType);

static struct {
  SIZE_T nHookId;
  lpfnMessageBoxW fnMessageBoxW;
} sMessageBoxW_Hook = { 0, NULL };

//-----------------------------------------------------------

int WinMainCRTStartup()
//int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPTSTR lpCmdLine, __in int nShowCmd)
{
  CNktHookLib cHookMgr;
  HINSTANCE hUser32Dll;
  LPVOID fnOrigMessageBoxW;
  DWORD dwOsErr;

  cHookMgr.SetEnableDebugOutput(TRUE);

  hUser32Dll = NktHookLibHelpers::GetModuleBaseAddress(L"user32.dll");
  if (hUser32Dll == NULL) {
    ::MessageBoxW(0, L"Error: Cannot get handle of user32.dll", L"HookTest", MB_OK|MB_ICONERROR);
    return 0;
  }
  fnOrigMessageBoxW = NktHookLibHelpers::GetProcedureAddress(hUser32Dll, "MessageBoxW");
  if (fnOrigMessageBoxW == NULL) {
    ::MessageBoxW(0, L"Error: Cannot get address of MessageBoxW", L"HookTest", MB_OK|MB_ICONERROR);
    return 0;
  }

  dwOsErr = cHookMgr.Hook(&(sMessageBoxW_Hook.nHookId), (LPVOID*)&(sMessageBoxW_Hook.fnMessageBoxW),
                          fnOrigMessageBoxW, Hooked_MessageBoxW);

  ::MessageBoxW(0, L"This should be hooked", L"HookTest", MB_OK);
  //dwOsErr = cHookMgr.EnableHook(dwHookId_MessageBoxW, FALSE);
  dwOsErr = cHookMgr.Unhook(sMessageBoxW_Hook.nHookId);

  ::MessageBoxW(0, L"This should NOT be hooked", L"HookTest", MB_OK);

  return 0;
}

static int WINAPI Hooked_MessageBoxW(__in_opt HWND hWnd, __in_opt LPCWSTR lpText, __in_opt LPCWSTR lpCaption, __in UINT uType)
{
  return sMessageBoxW_Hook.fnMessageBoxW(hWnd, lpText, L"HOOKED!!!", uType);
}

//NOTE: The code below was added because we are linking without default VC runtime libraries in order to show
//      that NktHookLib does not depend on the VC runtime nor Kernel dlls.
//      Visual C's default setting is to create a SAFESEH compatible image executable. NktHookLib is compiled using
//      safeseh too.
//      Normally you application will be linked against the VC runtime libraries. If so, do not add the code below.
#if defined(_M_IX86)
  #ifdef __cplusplus
  extern "C" {
  #endif //__cplusplus
  extern PVOID __safe_se_handler_table[];
  extern BYTE  __safe_se_handler_count;

  IMAGE_LOAD_CONFIG_DIRECTORY32 _load_config_used = {
      sizeof(IMAGE_LOAD_CONFIG_DIRECTORY32),
      0,    0,    0,    0,    0,    0,    0,    0,
      0,    0,    0,    0,    0,    0,    0,    0,
      0,
      (DWORD)__safe_se_handler_table,
      (DWORD) &__safe_se_handler_count
  };

  #ifdef __cplusplus
  }
  #endif //__cplusplus
#endif //_M_IX86
