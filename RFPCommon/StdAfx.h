//=======================================================================
//		Copyright 2013-2014 MicroStrategy, Inc.
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

#pragma once


#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>

#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <time.h>

// types
#define Int_32		int
enum EnumRFPType { rfpUnknown, rfpSimple, rfpRelative, rfpAgg };
typedef std::vector<char *> STR_VECTOR;
typedef std::pair<char *, size_t> IOBUFFER_PAIR;

// constants
const Int_32 ERRMSG_BUFSIZE = 256;
const Int_32 KEY_BUFFER_SIZE = 1024;
const Int_32 RFP_USER_FUNCPARAM_CNT = 27;
const Int_32 RFP_RESV_FOR_R_CNT = 20;
const Int_32 RFP_DATETIME_SIZE = 50;
const Int_32 RFP_ERRLOG_LIMIT = 0x100000;  // 1MB
const size_t RFP_MAX_TOKEN_LENGTH = 250;
const size_t RFP_INITIAL_STR_LENGTH = 0x1000;  // 4KB
const size_t RFP_MAX_STR_LENGTH = 0x10000;  // 64KB

// external decls (see StdAfx.cpp)
extern const char START_CMT;
extern const char SPACE_CHAR;
extern const char PERIOD_CHAR;
extern const char COMMA_CHAR;
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
extern const char *VAR_PARAMTYPE_SCALAR;
extern const char *VAR_PARAMTYPE_VECTOR;
extern const char *VAR_PARAMTYPE_PARAMETER;
extern const char *VAR_IOTYPE_INPUT;
extern const char *VAR_IOTYPE_OUTPUT;
extern const char *VAR_REPEAT_INPUT;
extern const char *VAR_DISABLED_VAR;
extern const char *RFP_FUNCPARAM_NAME_ARRAY[];
extern const char *RFP_RESERVED_FOR_R[];
extern const char *RFP_INVALID_R_IDENT_NAME_CHARS;
extern const char *RFP_INVALID_R_IDENT_FIRST_CHARS;
extern const char *RFP_ERRMSG;
extern const char *RFP_WORKDIR;
extern const char *RFP_EXFLAG;
extern const char *RFP_INPUTNAMES;
extern const char *RFP_PATH_DELIM;
extern const char *RFP_ERRLOG;
extern const char *RFP_ERRLOG_BKUP;
extern const char *RFP_RSCRIPTS;

//=======================================================================
// MicroStrategy libraries
//=======================================================================
#import "M5FunPack.tlb" raw_interfaces_only, raw_native_types, no_namespace, named_guids
#pragma warning( disable : 4146)
#import "M5TypLib.dll" raw_interfaces_only, raw_native_types, rename_namespace("M5TypLib"), named_guids
#pragma warning( default : 4146)
//=======================================================================

//=======================================================================
// non-numeric parameter/property support
#define MAXSIZE_DATE_STR 20
#define MAXSIZE_CHAR_STR 256
#define RFP_BUFLEN(cnt) 2*(cnt+1)
typedef char DATE_STR[MAXSIZE_DATE_STR+1];
typedef char CHSTR[MAXSIZE_CHAR_STR+1];
EXTERN_C const IID IID_IDSSTime;
//=======================================================================

// string manipulation macros
#define RFP_STRCPY(dest, destLen, src)												\
	strcpy_s(dest, destLen, src);
#define RFP_STRCAT(dest, destLen, src)												\
	strcat_s(dest, destLen, src);
#define RFP_WCSCPY(dest, destLen, src)												\
	wcscpy_s(dest, destLen, src);
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

// error message macros
#define CHECK_HR(msg)																\
if(FAILED(hr))																		\
{																					\
	INIT_HR_MSG(msg)																\
	throw hr;																		\
}

#define CHECK_HR_REF(msg)															\
if(FAILED(hr))																		\
{																					\
	INIT_HR_MSG_REF(msg)															\
	throw hr;																		\
}

// empty macro (not needed on Windows)
#define BLDSTR_DECL()

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

#define CHECK_SETRVAR_HR(varname)													\
if(FAILED(hr))																		\
{																					\
	if(wcslen(m_sErrMsg)==0)														\
		swprintf(m_sErrMsg, ERRMSG_BUFSIZE,											\
			L"[0x%X] %s: Setting of variable '%s' failed.", hr, FUNC_NAME,			\
			varname);																\
	return hr;																		\
}

// macro which converts EnumDSSDataType to appropriate string (used for logging)
#define RFP_GET_DT_STR(dt)													\
	(																		\
		(dt==DssDataTypeUnknown) ? L"unknown" :					/*  -1*/	\
		(dt==DssDataTypeReserved) ? L"reserved" :				/*   0*/	\
		(dt==DssDataTypeInteger) ? L"integer" :					/*   1*/	\
		(dt==DssDataTypeUnsigned) ? L"unsigned integer" :		/*   2*/	\
		(dt==DssDataTypeNumeric) ? L"numeric" :					/*   3*/	\
		(dt==DssDataTypeDecimal) ? L"decimal" :					/*   4*/	\
		(dt==DssDataTypeReal) ? L"real" :						/*   5*/	\
		(dt==DssDataTypeDouble) ? L"double" :					/*   6*/	\
		(dt==DssDataTypeFloat) ? L"float" :						/*   7*/	\
		(dt==DssDataTypeChar) ? L"char" :						/*   8*/	\
		(dt==DssDataTypeVarChar) ? L"varChar" :					/*   9*/	\
		(dt==DssDataTypeLongVarChar) ? L"longVarChar" :			/*  10*/	\
		(dt==DssDataTypeBinary) ? L"binary" :					/*  11*/	\
		(dt==DssDataTypeVarBin) ? L"varBinary" :				/*  12*/	\
		(dt==DssDataTypeLongVarBin) ? L"longVarBinary" :		/*  13*/	\
		(dt==DssDataTypeDate) ? L"date" :						/*  14*/	\
		(dt==DssDataTypeTime) ? L"time" :						/*  15*/	\
		(dt==DssDataTypeTimeStamp) ? L"timeStamp" :				/*  16*/	\
		(dt==DssDataTypeNChar) ? L"nChar" :						/*  17*/	\
		(dt==DssDataTypeNVarChar) ? L"nVarChar" :				/*  18*/	\
		(dt==DssDataTypeShort) ? L"short" :						/*  21*/	\
		(dt==DssDataTypeLong) ? L"long" :						/*  22*/	\
		(dt==DssDataTypeMBChar) ? L"mbChar" :					/*  23*/	\
		(dt==DssDataTypeBool) ? L"boolean" :					/*  24*/	\
		(dt==DssDataTypePattern) ? L"pattern" :					/*  25*/	\
		(dt==DssDataTypeNPattern) ? L"nPattern" :				/*  29*/	\
		(dt==DssDataTypeBigDecimal) ? L"bigDecimal" :			/*  30*/	\
		(dt==DssDataTypeCellFormatData) ? L"cellFormatData" :	/*  31*/	\
		(dt==DssDataTypeMissing) ? L"missing" :					/*  32*/	\
		(dt==DssDataTypeUTF8Char) ? L"utf8Char" :				/*  33*/	\
		(dt==DssDataTypeInt64) ? L"int64" :						/*  34*/	\
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

//{{AFX_INSERT_LOCATION}}
// 
