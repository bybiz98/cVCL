// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�:
#include <windows.h>

// C ����ʱͷ�ļ�
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <Objidl.h>

// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
#ifdef CVCL_STATIC_LIB
#define cVCL_API
#else
#ifdef CVCL_DLL_EXPORTS
#define cVCL_API __declspec(dllexport)
#else
#define cVCL_API __declspec(dllimport)
#endif
#endif