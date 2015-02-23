//=======================================================================
//		Copyright 2014 MicroStrategy, Inc.
//
//		Licensed under the Apache License, Version 2.0 (the "License");
//		you may not use this file except in compliance with the License.
//		You may obtain a copy of the License at
//
//		http://www.apache.org/licenses/LICENSE-2.0
//
//		Unless required by applicable law or agreed to in writing, software
//		distributed under the License is distributed on an "AS IS" BASIS,
//		WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//		See the License for the specific language governing permissions and
//		limitations under the License.
//
//		File:	StdAfx.h
//
//		Desc:	Include file for standard system include files, or project
//				specific include files that are used frequently, but are
//				changed infrequently.
//=======================================================================
#if !defined(STDAFX_H_RIPSUPPORT_INCLUDED_)
#define STDAFX_H_RIPSUPPORT_INCLUDED_
 
#pragma once

#ifdef WIN32
#define _WIN32_WINNT_NT4                    0x0400 //
#define _WIN32_WINNT_WIN2K                  0x0500
#define _WIN32_WINNT_WINXP                  0x0501
#define _WIN32_WINNT_WS03                   0x0502
#define _WIN32_WINNT_WIN6                   0x0600
#define _WIN32_WINNT_VISTA                  0x0600
#define _WIN32_WINNT_WS08                   0x0600
#define _WIN32_WINNT_LONGHORN               0x0600
#define _WIN32_WINNT_WIN7                   0x0601 //to replace SDKDDKVer.h 
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
typedef __int64 INT64;
#define STRICT_R_HEADERS //to remove the warning that ERROR macro redefinition in r\include\r_ext\rs.h(43) with c:\program files (x86)\microsoft sdks\windows\v7.0a\include\wingdi.h(112)
#else  // Unix/Linux
#include <ctype.h>
#include <wchar.h>
typedef long long INT64;
typedef INT64 MIDL_uhyper;

typedef MIDL_uhyper ULONGLONG;

typedef long HRESULT;

#define S_OK ((HRESULT)0x00000000L)
#define E_FAIL ((HRESULT)0x80004005L)
#define E_OUTOFMEMORY ((HRESULT)0x8007000EL)
#define E_INVALIDARG ((HRESULT)0x80070057L)

	

#endif



#ifndef FAR
#define FAR
#endif
/*
 * VARENUM usage key,
 *
 * * [V] - may appear in a VARIANT
 * * [T] - may appear in a TYPEDESC
 * * [P] - may appear in an OLE property set
 * * [S] - may appear in a Safe Array
 *
 *
 *  VT_EMPTY            [V]   [P]     nothing
 *  VT_NULL             [V]   [P]     SQL style Null
 *  VT_I2               [V][T][P][S]  2 byte signed int
 *  VT_I4               [V][T][P][S]  4 byte signed int
 *  VT_R4               [V][T][P][S]  4 byte real
 *  VT_R8               [V][T][P][S]  8 byte real
 *  VT_CY               [V][T][P][S]  currency
 *  VT_DATE             [V][T][P][S]  date
 *  VT_BSTR             [V][T][P][S]  OLE Automation string
 *  VT_DISPATCH         [V][T][P][S]  IDispatch *
 *  VT_ERROR            [V][T][P][S]  SCODE
 *  VT_BOOL             [V][T][P][S]  True=-1, False=0
 *  VT_VARIANT          [V][T][P][S]  VARIANT *
 *  VT_UNKNOWN          [V][T]   [S]  IUnknown *
 *  VT_DECIMAL          [V][T]   [S]  16 byte fixed point
 *  VT_RECORD           [V]   [P][S]  user defined type
 *  VT_I1               [V][T][P][s]  signed char
 *  VT_UI1              [V][T][P][S]  unsigned char
 *  VT_UI2              [V][T][P][S]  unsigned short
 *  VT_UI4              [V][T][P][S]  unsigned short
 *  VT_I8                  [T][P]     signed 64-bit int
 *  VT_UI8                 [T][P]     unsigned 64-bit int
 *  VT_INT              [V][T][P][S]  signed machine int
 *  VT_UINT             [V][T]   [S]  unsigned machine int
 *  VT_VOID                [T]        C style void
 *  VT_HRESULT             [T]        Standard return type
 *  VT_PTR                 [T]        pointer type
 *  VT_SAFEARRAY           [T]        (use VT_ARRAY in VARIANT)
 *  VT_CARRAY              [T]        C style array
 *  VT_USERDEFINED         [T]        user defined type
 *  VT_LPSTR               [T][P]     null terminated string
 *  VT_LPWSTR              [T][P]     wide null terminated string
 *  VT_FILETIME               [P]     FILETIME
 *  VT_BLOB                   [P]     Length prefixed bytes
 *  VT_STREAM                 [P]     Name of the stream follows
 *  VT_STORAGE                [P]     Name of the storage follows
 *  VT_STREAMED_OBJECT        [P]     Stream contains an object
 *  VT_STORED_OBJECT          [P]     Storage contains an object
 *  VT_BLOB_OBJECT            [P]     Blob contains an object 
 *  VT_CF                     [P]     Clipboard format
 *  VT_CLSID                  [P]     A Class ID
 *  VT_VECTOR                 [P]     simple counted array
 *  VT_ARRAY            [V]           SAFEARRAY*
 *  VT_BYREF            [V]           void* for local use
 *  VT_BSTR_BLOB                      Reserved for system use
 */
enum RFP_VARENUM
{
	RFP_VT_EMPTY	= 0,
	RFP_VT_NULL	= 1,
	RFP_VT_I2	= 2,
	RFP_VT_I4	= 3,
	RFP_VT_R4	= 4,
	RFP_VT_R8	= 5,
	RFP_VT_CY	= 6,
	RFP_VT_DATE	= 7,
	RFP_VT_BSTR	= 8,
	RFP_VT_DISPATCH	= 9,
	RFP_VT_ERROR	= 10,
	RFP_VT_BOOL	= 11,
	RFP_VT_VARIANT	= 12,
	RFP_VT_UNKNOWN	= 13,
	RFP_VT_DECIMAL	= 14,
	RFP_VT_I1	= 16,
	RFP_VT_UI1	= 17,
	RFP_VT_UI2	= 18,
	RFP_VT_UI4	= 19,
	RFP_VT_I8	= 20,
	RFP_VT_UI8	= 21,
	RFP_VT_INT	= 22,
	RFP_VT_UINT	= 23,
	RFP_VT_VOID	= 24,
	RFP_VT_HRESULT	= 25,
	RFP_VT_PTR	= 26,
	RFP_VT_SAFEARRAY	= 27,
	RFP_VT_CARRAY	= 28,
	RFP_VT_USERDEFINED	= 29,
	RFP_VT_LPSTR	= 30,
	RFP_VT_LPWSTR	= 31,
	RFP_VT_RECORD	= 36,
	RFP_VT_FILETIME	= 64,
	RFP_VT_BLOB	= 65,
	RFP_VT_STREAM	= 66,
	RFP_VT_STORAGE	= 67,
	RFP_VT_STREAMED_OBJECT	= 68,
	RFP_VT_STORED_OBJECT	= 69,
	RFP_VT_BLOB_OBJECT	= 70,
	RFP_VT_CF	= 71,
	RFP_VT_CLSID	= 72,
	RFP_VT_BSTR_BLOB	= 0xfff,
	RFP_VT_VECTOR	= 0x1000,
	RFP_VT_ARRAY	= 0x2000,
	RFP_VT_BYREF	= 0x4000,
	RFP_VT_RESERVED	= 0x8000,
	RFP_VT_ILLEGAL	= 0xffff,
	RFP_VT_ILLEGALMASKED	= 0xfff,
	RFP_VT_TYPEMASK	= 0xfff
};

#define VARIANT_TRUE ((VARIANT_BOOL)0xffff)
#define VARIANT_FALSE ((VARIANT_BOOL)0)

typedef int INT;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int UINT;
typedef long LONG;
typedef unsigned long DWORD;
typedef DWORD ULONG;
typedef short SHORT;
typedef unsigned short USHORT;
typedef float FLOAT;
typedef double DOUBLE;
typedef short VARIANT_BOOL;

typedef LONG SCODE;
typedef double DATE;
typedef unsigned short USHORT;

	
typedef INT64 hyper;

typedef hyper LONGLONG;

#ifndef _tagCY_DEFINED
#define _tagCY_DEFINED
#define _CY_DEFINED

/* real definition that makes the C++ compiler happy */
typedef union tagCY {
	struct  {
		unsigned long Lo;
		long      Hi;
	} _Parts;
	LONGLONG int64;
} CY;

#endif /* _tagCY_DEFINED */
 
 
typedef char CHAR;
#define __RPC_FAR
typedef void __RPC_FAR *PVOID;

  

#include <vector>

// types
typedef int Int_32;
typedef unsigned short RFP_VARTYPE;
typedef struct tagRFP_VARIANT RFP_VARIANT;

typedef struct tagRFP_VARIANT
{//24bytes
	RFP_VARTYPE vt;//2bytes
	WORD wReserved1;//2
	WORD wReserved2;//2
	WORD wReserved3;//2
	union {

	  BYTE                    bVal;                 // VT_UI1.1byte
      SHORT                   iVal;                 // VT_I2.2
      LONG                    lVal;                 // VT_I4.4
	  INT64					  llVal;				// VT_I8.8
      FLOAT                   fltVal;               // VT_R4.4
      DOUBLE                  dblVal;               // VT_R8.8
      VARIANT_BOOL            boolVal;              // VT_BOOL.2
      SCODE                   scode;                // VT_ERROR.4
      CY                      cyVal;                // VT_CY.16
      DATE                    date;                 // VT_DATE.8
      wchar_t *               bstrVal;              // VT_BSTR.8
      PVOID                   byref;           // Generic ByRef.
      CHAR                    cVal;                 // VT_I1.1
      USHORT				  uiVal;                // VT_UI2.
      ULONG					  ulVal;                // VT_UI4.
      INT                     intVal;               // VT_INT.4
      UINT					  uintVal;              // VT_UINT.4		 
	};
} RFP_VARIANT_t;

// constants
const Int_32 ERRMSG_BUFSIZE = 256;
const Int_32 MAXSIZE_CHAR_STR = 256;
const Int_32 KEY_BUFFER_SIZE = 1024;
const Int_32 RFP_USER_FUNCPARAM_CNT = 27;
const Int_32 RFP_RESV_FOR_R_CNT = 20;
const Int_32 RFP_ENVNAME_MINSIZE = 6;
const Int_32 RFP_ENVNAME_RANGE = 10;
const Int_32 RFP_ENVNAME_BUFLEN = RFP_ENVNAME_MINSIZE + RFP_ENVNAME_RANGE + 10;
const size_t RFP_MAX_TOKEN_LENGTH = 250;
const size_t RFP_MAX_STR_LENGTH = 0x10000;  // 64KB
const size_t RFP_INITIAL_BUFSIZE = 1024;

// external decls (see StdAfx.cpp)
extern const char START_CMT;
extern const char SPACE_CHAR;
extern const char PERIOD_CHAR;
extern const char COMMA_CHAR;
extern const char HYPHEN_CHAR;
extern const char EQUAL_CHAR;
extern const char SQUOTE_CHAR;
extern const char ESCAPE_CHAR;
extern const char TAB_CHAR;
extern const char NULL_CHAR;
extern const wchar_t NULL_WCHAR;
extern const char OPENBRACKET_CHAR;
extern const char CLOSEBRACKET_CHAR;
extern const char NL_CHAR;
extern const char *NEWLINE;
extern const char *WHITESPACE_DELIMS;
extern const char *MSTR_BEGIN;
extern const char *MSTR_END;
extern const char *RVAR_TAG;
extern const char *VAR_DATATYPE_NUM;
extern const char *VAR_DATATYPE_STR;
extern const char *VAR_DATATYPE_BOOL;
extern const char *VAR_PARAMTYPE_SCALAR;
extern const char *VAR_PARAMTYPE_VECTOR;
extern const char *VAR_PARAMTYPE_PARAMETER;
extern const char *VAR_IOTYPE_INPUT;
extern const char *VAR_IOTYPE_OUTPUT;
extern const char *VAR_REPEAT_INPUT;
extern const char *VAR_DISABLED_VAR;
extern const char *RFP_FALSE;
extern const char *RFP_TRUE;
extern const char *RFP_FUNCPARAM_NAME_ARRAY[];
extern const char *RCMD_ASNULL;
extern const char *RCMD_EVAL;
extern const char *RCMD_GET;
extern const char *RCMD_PARSE;
extern const char *RCMD_READLINES;
extern const char *RCMD_RM;
extern const char *RCMD_SETINTERNET2;
extern const char *RCMD_URL;
extern const char *RFP_RESERVED_FOR_R[];
extern const char *RFP_INVALID_R_IDENT_NAME_CHARS;
extern const char *RFP_INVALID_R_IDENT_FIRST_CHARS;
extern const char *RFP_RANDOM_CHARS_SET;
extern const char *RFP_ERRMSG;
extern const char *RFP_INPUTNAMES;
extern const char *RFP_SCRIPT;
extern const char *RFP_ENVNAME;

// string manipulation macros
#define RFP_BUFLEN(cnt) 4*cnt+1
#ifdef WIN32
#define RFP_STRCPY(dest, destLen, src)												\
	strcpy_s(dest, destLen, src);
#define RFP_WCSCPY(dest, destLen, src)												\
	wcscpy_s(dest, destLen, src);
#define RFP_WCSCAT(dest, destLen, src)												\
	wcscat_s(dest, destLen, src);
#define RFP_WCSTOMBS(dest, destLen, src)											\
{																					\
	size_t returnLen = 0;															\
	wcstombs_s(&returnLen, dest, destLen, src, _TRUNCATE);							\
}
#define RFP_MBSTOWCS(dest, destLen, src)											\
{																					\
	size_t returnLen = 0;															\
	mbstowcs_s(&returnLen, dest, destLen, src, _TRUNCATE);							\
}
#define RFP_STRTOK(token, src, delims, context)										\
	token = strtok_s(src, delims, &context);
#else
#define RFP_STRCPY(dest, destLen, src)												\
	strcpy(dest, src);
#define RFP_WCSCPY(dest, destLen, src)												\
	wcscpy(dest, src);
#define RFP_WCSCAT(dest, destLen, src)												\
	wcscat(dest, src);
#define RFP_WCSTOMBS(dest, destLen, src)											\
{																					\
	size_t retLen = wcstombs(dest, src, destLen);									\
	if(retLen==destLen)																\
		dest[destLen-1] = NULL_CHAR;												\
}
#define RFP_MBSTOWCS(dest, destLen, src)											\
{																					\
	size_t retLen = mbstowcs(dest, src, destLen);									\
	if(retLen==destLen)																\
		dest[destLen-1] = NULL_WCHAR;												\
}
#define RFP_STRTOK(token, src, delims, context)										\
	token = strtok(src, delims);
#endif

// error message macros
#define CHECK_HR(msg)																\
if(hr!=S_OK)																		\
{																					\
	INIT_HR_MSG(msg)																\
	throw hr;																		\
}

#ifdef WIN32
// empty macro (not needed on Windows)
#define BLDSTR_DECL()

#define INIT_MSG(msg)															\
if(wcslen(m_sErrMsg)==0)															\
	swprintf(m_sErrMsg, ERRMSG_BUFSIZE, L"%s", msg);

#define INIT_MSG2(format, ...)												\
if(wcslen(m_sErrMsg)==0)															\
	swprintf(m_sErrMsg, ERRMSG_BUFSIZE, format, ##__VA_ARGS__);

#define INIT_ERROR_MSG(msg)															\
if(wcslen(m_sErrMsg)==0)															\
	swprintf(m_sErrMsg, ERRMSG_BUFSIZE, L"%s: %s", FUNC_NAME, msg);

#define INIT_ERROR_MSG2(format, ...)												\
if(wcslen(m_sErrMsg)==0)															\
	swprintf(m_sErrMsg, ERRMSG_BUFSIZE, format, FUNC_NAME, ##__VA_ARGS__);

#define INIT_ERROR_MSG_REF(format, ...)												\
if(wcslen(pErrMsg)==0)																\
	swprintf(pErrMsg, ERRMSG_BUFSIZE, format, FUNC_NAME, ##__VA_ARGS__);

#define INIT_HR_MSG(msg)															\
if(wcslen(m_sErrMsg)==0)															\
	swprintf(m_sErrMsg, ERRMSG_BUFSIZE, L"[0x%X]%s: %s", hr, FUNC_NAME, msg);

#define INIT_HR_MSG_REF(msg)														\
if(wcslen(pErrMsg)==0)																\
	swprintf(pErrMsg, ERRMSG_BUFSIZE, L"[0x%X]%s: %s", hr, FUNC_NAME, msg);
#else  // Unix
#define BLDSTR_DECL()																\
	const Int32 lWorklen = 25;														\
	wchar_t wcsWork [lWorklen];

#define INIT_MSG(msg)															\
if(wcslen(m_sErrMsg)==0)															\
{																					\
	wcscpy(m_sErrMsg, msg);															\
}

#define INIT_ERROR_MSG(msg)															\
if(wcslen(m_sErrMsg)==0)															\
{																					\
	wcscpy(m_sErrMsg, FUNC_NAME);													\
	wcscat(m_sErrMsg, L": ");														\
	wcscat(m_sErrMsg, msg);															\
}

#define INIT_ERROR_MSG_REF(msg)														\
if(wcslen(pErrMsg)==0)																\
{																					\
	wcscpy(pErrMsg, FUNC_NAME);														\
	wcscat(pErrMsg, L": ");															\
	wcscat(pErrMsg, msg);															\
}

#define INIT_HR_MSG(msg)															\
if(wcslen(m_sErrMsg)==0)															\
{																					\
	wcscpy(m_sErrMsg, L"[0x");														\
	swprintf(wcsWork, lWorklen, L"%X", hr);											\
	wcscat(m_sErrMsg, wcsWork);														\
	wcscat(m_sErrMsg, L"]");														\
	wcscat(m_sErrMsg, FUNC_NAME);													\
	wcscat(m_sErrMsg, L": ");														\
	wcscat(m_sErrMsg, msg);															\
}

#define INIT_HR_MSG_REF(msg)														\
if(wcslen(pErrMsg)==0)																\
{																					\
	wcscpy(pErrMsg, L"[0x");														\
	swprintf(wcsWork, lWorklen, L"%X", hr);											\
	wcscat(pErrMsg, wcsWork);														\
	wcscat(pErrMsg, L"]");															\
	wcscat(pErrMsg, FUNC_NAME);														\
	wcscat(pErrMsg, L": ");															\
	wcscat(pErrMsg, msg);															\
}
#endif

// macro which converts EnumRfpDataType to appropriate string (used for logging)
#define RFP_GET_DT_STR(dt)													\
	(																		\
		(dt==RfpDataTypeUnknown) ? L"unknown" :					/*  -1*/	\
		(dt==RfpDataTypeInteger) ? L"integer" :					/*   1*/	\
		(dt==RfpDataTypeDouble) ? L"double" :					/*   6*/	\
		(dt==RfpDataTypeFloat) ? L"float" :						/*   7*/	\
		(dt==RfpDataTypeVarChar) ? L"varChar" :					/*   9*/	\
		(dt==RfpDataTypeShort) ? L"short" :						/*  21*/	\
		(dt==RfpDataTypeBool) ? L"boolean" :					/*  24*/	\
		L"unsupported"														\
	)

// macro which converts R data types to appropriate string (used for logging)
#define RFP_GET_RDT_STR(dt)													\
	(																		\
		(dt==NILSXP) ? L"NILSXP" :								/*   0*/	\
		(dt==SYMSXP) ? L"SYMSXP" :								/*   1*/	\
		(dt==LISTSXP) ? L"LISTSXP" :							/*   2*/	\
		(dt==CLOSXP) ? L"CLOSXP" :								/*   3*/	\
		(dt==ENVSXP) ? L"ENVSXP" :								/*   4*/	\
		(dt==PROMSXP) ? L"PROMSXP" :							/*   5*/	\
		(dt==LANGSXP) ? L"LANGSXP" :							/*   6*/	\
		(dt==SPECIALSXP) ? L"SPECIALSXP" :						/*   7*/	\
		(dt==BUILTINSXP) ? L"BUILTINSXP" :						/*   8*/	\
		(dt==CHARSXP) ? L"CHARSXP" :							/*   9*/	\
		(dt==LGLSXP) ? L"LGLSXP" :								/*  10*/	\
		(dt==INTSXP) ? L"INTSXP" :								/*  13*/	\
		(dt==REALSXP) ? L"REALSXP" :							/*  14*/	\
		(dt==CPLXSXP) ? L"CPLXSXP" :							/*  15*/	\
		(dt==STRSXP) ? L"STRSXP" :								/*  16*/	\
		(dt==DOTSXP) ? L"DOTSXP" :								/*  17*/	\
		(dt==ANYSXP) ? L"ANYSXP" :								/*  18*/	\
		(dt==VECSXP) ? L"VECSXP" :								/*  19*/	\
		(dt==EXPRSXP) ? L"EXPRSXP" :							/*  20*/	\
		(dt==BCODESXP) ? L"BCODESXP" :							/*  21*/	\
		(dt==EXTPTRSXP) ? L"EXTPTRSXP" :						/*  22*/	\
		(dt==WEAKREFSXP) ? L"WEAKREFSXP" :						/*  23*/	\
		(dt==RAWSXP) ? L"RAWSXP" :								/*  24*/	\
		(dt==S4SXP) ? L"S4SXP" :								/*  25*/	\
		(dt==NEWSXP) ? L"NEWSXP" :								/*  30*/	\
		(dt==FREESXP) ? L"FREESXP" :							/*  31*/	\
		(dt==FUNSXP) ? L"FUNSXP" :								/*  99*/	\
		L"[undefined]"														\
	)

// macro which converts input string to lower case
#define CONVERT_TO_LOWER(s)													\
{																			\
	for(Int32 i=0;i<strlen(s);i++)											\
	{																		\
		if((s[i] >= 'A') && (s[i] <= 'Z'))									\
			s[i] = s[i]-('Z'-'z');											\
		else if(s[i]=='–')													\
			s[i] = '-';														\
	}																		\
}

#endif
