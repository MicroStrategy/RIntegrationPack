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
//		File:	RIPSupport.cpp
//
//		Desc:	Implementation of CRSupport class and exported RIP functions.
//=======================================================================

//=======================================================================
// includes
//=======================================================================
#include "StdAfx.h"
#ifndef WIN32
#include <dlfcn.h>
#endif
using namespace std;
#include "RIPSupport.h"

//=======================================================================
// CRSupport class
// constructor/destructor
//=======================================================================
CRSupport::CRSupport() :
	m_sRInstallPath(NULL),
	m_hRLib(NULL),
	m_bRInitialized(false)
{
	// initialize error message buffer to empty string
	m_sErrMsg[0] = NULL_WCHAR;
	
	// initialize R environment
	HRESULT hr=InitR();
	m_bRInitialized = (hr==S_OK);
}

CRSupport::~CRSupport()
{
	try
	{
		EndR();

		// release memory
		delete [] m_sRInstallPath;
	}
	catch(...)
	{
	}
}

//=======================================================================
// CRSupport methods
//=======================================================================
#ifdef WIN32
HRESULT CRSupport::GetRInstallPath()
{
	wchar_t			*wcsKeyR						= L"SOFTWARE\\R-core\\R";
	wchar_t			*wcsValueInstallPath			= L"InstallPath";
	DWORD			dwSize							= KEY_BUFFER_SIZE;
	HRESULT			hr								= S_OK;
	const wchar_t	FUNC_NAME[]						= L"CRSupport::GetRInstallPath";
	BLDSTR_DECL()

	try
	{
		HKEY			hKey;
		DWORD			retCode;
		DWORD			dwValType;
		wchar_t			sKeyValue[KEY_BUFFER_SIZE];

		// open R installation key
		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, wcsKeyR, 0, KEY_READ, &hKey))
		{
			// key not found
			INIT_MSG(L"R install is not found")
			throw E_FAIL;
		}

		// fetch the InstallPath value
		if(!(retCode=RegQueryValueEx(hKey, wcsValueInstallPath, 0, &dwValType, (BYTE *)&sKeyValue,
			&dwSize)))
		{
			// found it...validate its type
			if(dwValType==REG_SZ)
			{
				// expected type...save R install path
				size_t lDestLen = wcslen(sKeyValue) + 1;
				m_sRInstallPath = new wchar_t [lDestLen];
				if(!m_sRInstallPath)
				{
					INIT_MSG(L"R install is not found")
					throw E_OUTOFMEMORY;
				}

				RFP_WCSCPY(m_sRInstallPath, lDestLen, sKeyValue)
			}
			else
			{
				// unexpected type
				INIT_MSG(L"R install is not found")
				throw E_FAIL;
			}
		}
		else
		{
			// key value not found
			INIT_MSG(L"R install is not found")
			throw E_FAIL;
		}
	}
	catch(HRESULT catchHR)
	{
		hr = catchHR;
	}
	catch(...)
	{
		hr = E_FAIL;
	}

	return hr;
}
#endif

HRESULT CRSupport::InitR()
{
	const wchar_t	FUNC_NAME[]		= L"CRSupport::InitR";

#ifdef WIN32
	char			*localArgs[]	= {"R", "--silent"};

	HRESULT			hr				= S_OK;

	if(m_hRLib==NULL)
	{
		// locate R installation
		hr = GetRInstallPath();
		if (hr!=S_OK) return hr;

		// get a handle to the DLL module
#ifdef WIN64
		// x64 version of RIP is executing...load x64 version of R
		hr = LoadRLibrary(L"\\bin\\x64", L"R.dll", true);
		if (hr!=S_OK) return hr;

		if(m_hRLib)
			// R.dll loaded successfully...load Rlapack.dll
			LoadRLibrary(L"\\bin\\x64", L"Rlapack.dll", false);
#else

		// 32 ripsupport loads 32 R dll...search <R install root>\bin\i386 first for x86 version of R
		hr = LoadRLibrary(L"\\bin\\i386", L"R.dll", true);
		if (hr!=S_OK) return hr;

		if(!m_hRLib)
		{
			// DLL not found...try <R install path>\bin
			hr = LoadRLibrary(L"\\bin", L"R.dll", true);
			if (hr!=S_OK) return hr;

			if(m_hRLib)
				// R.dll loaded successfully...load Rlapack.dll
				LoadRLibrary(L"\\bin", L"Rlapack.dll", false);
		}
		else
			// R.dll loaded successfully...load Rlapack.dll
			LoadRLibrary(L"\\bin\\i386", L"Rlapack.dll", false);
#endif
	}
	else
		// handle already exists
		return S_OK;
 
	// check if handle is valid
    if(m_hRLib != NULL) 
    {
		// valid handle...fetch function addresses
		m_R_GlobalEnv_ptr = (SEXP *)GetProcAddress(m_hRLib, "R_GlobalEnv");
		m_R_UnboundValue_ptr = (SEXP *)GetProcAddress(m_hRLib, "R_UnboundValue");
		m_R_NilValue_ptr = (SEXP *)GetProcAddress(m_hRLib, "R_NilValue");
		m_Rf_protect_ptr = (pRf_protect)GetProcAddress(m_hRLib, "Rf_protect");
		m_lang1_ptr = (pRf_lang1)GetProcAddress(m_hRLib, "Rf_lang1");
		m_lang2_ptr = (pRf_lang2)GetProcAddress(m_hRLib, "Rf_lang2");
		m_lang3_ptr = (pRf_lang3)GetProcAddress(m_hRLib, "Rf_lang3");
		m_lang4_ptr = (pRf_lang4)GetProcAddress(m_hRLib, "Rf_lang4");
		m_install_ptr = (pRf_install)GetProcAddress(m_hRLib, "Rf_install");
		m_mkChar_ptr = (pRf_mkChar)GetProcAddress(m_hRLib, "Rf_mkChar");
		m_mkString_ptr = (pRf_mkString)GetProcAddress(m_hRLib, "Rf_mkString");
		m_R_tryEval_ptr = (pR_tryEval)GetProcAddress(m_hRLib, "R_tryEval");
		m_Rf_unprotect_ptr = (pRf_unprotect)GetProcAddress(m_hRLib, "Rf_unprotect");
		m_Rf_PrintValue_ptr = (pRf_PrintValue)GetProcAddress(m_hRLib, "Rf_PrintValue");
		m_Rf_endEmbeddedR_ptr = (pRf_endEmbeddedR)GetProcAddress(m_hRLib, "Rf_endEmbeddedR");
		m_Rf_allocVector_ptr = (pRf_allocVector)GetProcAddress(m_hRLib, "Rf_allocVector");
		m_Rf_allocMatrix_ptr = (pRf_allocMatrix)GetProcAddress(m_hRLib, "Rf_allocMatrix");
		m_Rf_allocSExp_ptr = (pRf_allocSExp)GetProcAddress(m_hRLib, "Rf_allocSExp");
		m_REAL_ptr = (p_REAL)GetProcAddress(m_hRLib, "REAL");
		m_INTEGER_ptr = (p_INTEGER)GetProcAddress(m_hRLib, "INTEGER");
		m_LOGICAL_ptr = (p_INTEGER)GetProcAddress(m_hRLib, "LOGICAL");
		m_defineVar_ptr = (pRf_defineVar)GetProcAddress(m_hRLib, "Rf_defineVar");
		m_asReal_ptr = (pRf_asReal)GetProcAddress(m_hRLib, "Rf_asReal");
		m_ScalarInteger_ptr = (pRf_ScalarInteger)GetProcAddress(m_hRLib, "Rf_ScalarInteger");
		m_length_ptr = (pRf_length)GetProcAddress(m_hRLib, "Rf_length");
		m_TYPEOF_ptr = (p_TYPEOF)GetProcAddress(m_hRLib, "TYPEOF");
		m_findVar_ptr = (pRf_findVar)GetProcAddress(m_hRLib, "Rf_findVar");
		m_eval_ptr = (pRf_eval)GetProcAddress(m_hRLib, "Rf_eval");
		m_SET_STRING_ELT_ptr = (p_SET_STRING_ELT)GetProcAddress(m_hRLib, "SET_STRING_ELT");
		m_STRING_ELT_ptr = (p_STRING_ELT)GetProcAddress(m_hRLib, "STRING_ELT");
		m_translateChar_ptr = (pRf_translateChar)GetProcAddress(m_hRLib, "Rf_translateChar");
		m_R_finite_ptr = (pR_finite)GetProcAddress(m_hRLib, "R_finite");
		m_Rf_nrows_ptr = (pRf_nrows)GetProcAddress(m_hRLib, "Rf_nrows");
		m_R_NaReal_ptr = (double *)GetProcAddress(m_hRLib, "R_NaReal");
		m_R_NaInt_ptr = (int *)GetProcAddress(m_hRLib, "R_NaInt");
		m_R_NaLogical_ptr = (int *)GetProcAddress(m_hRLib, "R_NaInt");
		m_R_NaString_ptr = (SEXP *)GetProcAddress(m_hRLib, "R_NaString");
		m_Rf_initEmbeddedR_ptr = (pRf_initEmbeddedR)GetProcAddress(m_hRLib, "Rf_initEmbeddedR");
		m_R_Interactive_ptr = (Rboolean *)GetProcAddress(m_hRLib, "R_Interactive");
		m_SET_FRAME_ptr = (p_SET_FRAME)GetProcAddress(m_hRLib, "SET_FRAME");
		m_SET_ENCLOS_ptr = (p_SET_ENCLOS)GetProcAddress(m_hRLib, "SET_ENCLOS");
		m_SET_HASHTAB_ptr = (p_SET_HASHTAB)GetProcAddress(m_hRLib, "SET_HASHTAB");
		m_SET_ATTRIB_ptr = (p_SET_ATTRIB)GetProcAddress(m_hRLib, "SET_ATTRIB");

		if(m_Rf_initEmbeddedR_ptr && m_Rf_protect_ptr && m_lang1_ptr && m_lang2_ptr && m_lang3_ptr && m_lang4_ptr && m_install_ptr &&
			m_mkChar_ptr && m_mkString_ptr && m_R_tryEval_ptr && m_Rf_unprotect_ptr && m_Rf_PrintValue_ptr && m_Rf_endEmbeddedR_ptr &&
			m_Rf_allocVector_ptr && m_Rf_allocMatrix_ptr && m_Rf_allocSExp_ptr && m_REAL_ptr && m_INTEGER_ptr && m_LOGICAL_ptr &&
			m_defineVar_ptr && m_asReal_ptr && m_ScalarInteger_ptr && m_length_ptr && m_TYPEOF_ptr && m_findVar_ptr && m_eval_ptr &&
			m_R_GlobalEnv_ptr && m_R_UnboundValue_ptr && m_R_NilValue_ptr && m_SET_STRING_ELT_ptr && m_STRING_ELT_ptr && m_translateChar_ptr &&
			m_R_finite_ptr && m_Rf_nrows_ptr && m_R_NaReal_ptr && m_R_NaInt_ptr && m_R_NaLogical_ptr && m_R_NaString_ptr && m_R_Interactive_ptr &&
			m_SET_FRAME_ptr && m_SET_ENCLOS_ptr && m_SET_HASHTAB_ptr && m_SET_ATTRIB_ptr)
		{
			Int32 lInitR = m_Rf_initEmbeddedR_ptr(sizeof(localArgs) / sizeof(localArgs[0]), localArgs);
			if(!lInitR)
			{
				// error
				INIT_MSG(L"Initialization of R environment failed")
				return E_FAIL;
			}

			// set session to 'not interactive'
			*m_R_Interactive_ptr = FALSE;

			// yzhang, US53653, 2016-06-14, remove the call to setInternet2(TRUE) command
/*			// execute setInternet2(TRUE) command (required for URLs to work on Windows)
			SEXP eSetInt2;
			int lErrorCode;
			m_Rf_protect_ptr(eSetInt2=m_lang2_ptr(m_install_ptr(RCMD_SETINTERNET2), m_mkString_ptr(RFP_TRUE)));
			m_R_tryEval_ptr(eSetInt2, *m_R_GlobalEnv_ptr, &lErrorCode);
			m_Rf_unprotect_ptr(1);
			if(lErrorCode > 0)
			{
				INIT_MSG(L"Initialization of R environment failed")
				return E_FAIL;
			}*/

			// success
			return S_OK;
		}
	}

	// error
	DWORD dwError = GetLastError();
	if(dwError==126)
	{
#ifdef WIN64					
					INIT_MSG(L"64bit R is required")
#else					
					INIT_MSG(L"32bit R is required")
#endif
	}else
	{
	INIT_MSG2(L"Load of R.DLL failed (system error code=0x%x).", dwError)
	}
	return E_FAIL;
#else // Unix/Linux
	if(m_hRLib==NULL)
	{
#if defined(__APPLE__)
					// get handles to R libraries
			m_hRLib = dlopen("libR.dylib", RTLD_LAZY);
			if(m_hRLib)
				dlopen("libRlapack.dylib", RTLD_LAZY);
#else
			// get handles to R libraries
			m_hRLib = dlopen("libR.so", RTLD_LAZY);
			if(m_hRLib)
				dlopen("libRlapack.so", RTLD_LAZY);
#endif
	}
	else
		// handle already exists
		return S_OK;

	// check if handle is valid
	if(m_hRLib)
	{
		// valid handle...reset errors
		dlerror();
				
		// fetch function addresses
		m_R_GlobalEnv_ptr = (SEXP *)dlsym(m_hRLib, "R_GlobalEnv");
		m_R_UnboundValue_ptr = (SEXP *)dlsym(m_hRLib, "R_UnboundValue");
		m_R_NilValue_ptr = (SEXP *)dlsym(m_hRLib, "R_NilValue");
		m_Rf_protect_ptr = (pRf_protect)dlsym(m_hRLib, "Rf_protect");
		m_lang1_ptr = (pRf_lang1)dlsym(m_hRLib, "Rf_lang1");
		m_lang2_ptr = (pRf_lang2)dlsym(m_hRLib, "Rf_lang2");
		m_lang3_ptr = (pRf_lang3)dlsym(m_hRLib, "Rf_lang3");
		m_lang4_ptr = (pRf_lang4)dlsym(m_hRLib, "Rf_lang4");
		m_install_ptr = (pRf_install)dlsym(m_hRLib, "Rf_install");
		m_mkChar_ptr = (pRf_mkChar)dlsym(m_hRLib, "Rf_mkChar");
		m_mkString_ptr = (pRf_mkString)dlsym(m_hRLib, "Rf_mkString");
		m_R_tryEval_ptr = (pR_tryEval)dlsym(m_hRLib, "R_tryEval");
		m_Rf_unprotect_ptr = (pRf_unprotect)dlsym(m_hRLib, "Rf_unprotect");
		m_Rf_PrintValue_ptr = (pRf_PrintValue)dlsym(m_hRLib, "Rf_PrintValue");
		m_Rf_endEmbeddedR_ptr = (pRf_endEmbeddedR)dlsym(m_hRLib, "Rf_endEmbeddedR");
		m_Rf_allocVector_ptr = (pRf_allocVector)dlsym(m_hRLib, "Rf_allocVector");
		m_Rf_allocMatrix_ptr = (pRf_allocMatrix)dlsym(m_hRLib, "Rf_allocMatrix");
		m_Rf_allocSExp_ptr = (pRf_allocSExp)dlsym(m_hRLib, "Rf_allocSExp");
		m_REAL_ptr = (p_REAL)dlsym(m_hRLib, "REAL");
		m_INTEGER_ptr = (p_INTEGER)dlsym(m_hRLib, "INTEGER");
		m_LOGICAL_ptr = (p_INTEGER)dlsym(m_hRLib, "LOGICAL");
		m_defineVar_ptr = (pRf_defineVar)dlsym(m_hRLib, "Rf_defineVar");
		m_asReal_ptr = (pRf_asReal)dlsym(m_hRLib, "Rf_asReal");
		m_ScalarInteger_ptr = (pRf_ScalarInteger)dlsym(m_hRLib, "Rf_ScalarInteger");
		m_length_ptr = (pRf_length)dlsym(m_hRLib, "Rf_length");
		m_TYPEOF_ptr = (p_TYPEOF)dlsym(m_hRLib, "TYPEOF");
		m_findVar_ptr = (pRf_findVar)dlsym(m_hRLib, "Rf_findVar");
		m_eval_ptr = (pRf_eval)dlsym(m_hRLib, "Rf_eval");
		m_SET_STRING_ELT_ptr = (p_SET_STRING_ELT)dlsym(m_hRLib, "SET_STRING_ELT");
		m_STRING_ELT_ptr = (p_STRING_ELT)dlsym(m_hRLib, "STRING_ELT");
		m_translateChar_ptr = (pRf_translateChar)dlsym(m_hRLib, "Rf_translateChar");
		m_R_finite_ptr = (pR_finite)dlsym(m_hRLib, "R_finite");
		m_Rf_nrows_ptr = (pRf_nrows)dlsym(m_hRLib, "Rf_nrows");
		m_R_NaReal_ptr = (double *)dlsym(m_hRLib, "R_NaReal");
		m_R_NaInt_ptr = (int *)dlsym(m_hRLib, "R_NaInt");
		m_R_NaLogical_ptr = (int *)dlsym(m_hRLib, "R_NaInt");
		m_R_NaString_ptr = (SEXP *)dlsym(m_hRLib, "R_NaString");
		m_Rf_initialize_R_ptr = (pRf_initialize_R)dlsym(m_hRLib, "Rf_initialize_R");
		m_R_CStackLimit_ptr = (uintptr_t *)dlsym(m_hRLib, "R_CStackLimit");
		m_R_Interactive_ptr = (Rboolean *)dlsym(m_hRLib, "R_Interactive");
		m_SET_FRAME_ptr = (p_SET_FRAME)dlsym(m_hRLib, "SET_FRAME");
		m_SET_ENCLOS_ptr = (p_SET_ENCLOS)dlsym(m_hRLib, "SET_ENCLOS");
		m_SET_HASHTAB_ptr = (p_SET_HASHTAB)dlsym(m_hRLib, "SET_HASHTAB");
		m_SET_ATTRIB_ptr = (p_SET_ATTRIB)dlsym(m_hRLib, "SET_ATTRIB");
		m_setup_Rmainloop_ptr = (p_setup_Rmainloop)dlsym(m_hRLib, "setup_Rmainloop");

		// check for errors
		const char *dlsym_error = dlerror();
		if(dlsym_error)
		{
			const size_t destSize = 100;
			wchar_t wcsErrMsg[destSize];
			mbstowcs(wcsErrMsg, dlsym_error, destSize);
			INIT_MSG(wcsErrMsg)
			return E_FAIL;
		}

		char * ev=getenv("R_HOME");
		if(ev==NULL)
		{
			INIT_MSG(L"R_HOME env variable has not been set yet")
			return E_FAIL;
		}
		// initialize R environment
		char *localArgs[] = {"R", "--slave","--silent"};
		Int32 lInitR = m_Rf_initialize_R_ptr(sizeof(localArgs)/sizeof(localArgs[0]), localArgs);
		if(lInitR < 0)
		{
			// error
			INIT_MSG(L"Initialization of R environment failed.")
			return E_FAIL;
		}

		// disable stack checking, because threads will throw it off
#ifdef CSTACK_DEFNS
		*m_R_CStackLimit_ptr = (uintptr_t) -1;
#endif
		// set session to 'not interactive'
		*m_R_Interactive_ptr = FALSE;
		m_setup_Rmainloop_ptr();
	}
	else
	{
		// dlopen() returned NULL handle
		const char *dlopen_error = dlerror();
		if(dlopen_error)
		{
			const size_t destSize = ERRMSG_BUFSIZE;
			wchar_t wcsErrMsg[destSize];
			mbstowcs(wcsErrMsg, dlopen_error, destSize);
			wcsErrMsg[ERRMSG_BUFSIZE-1] = NULL_WCHAR;  // just in case dlopen_error exceeds error message buffer
			INIT_MSG(wcsErrMsg)
		}
		else
		{
			INIT_MSG(L"Load of R library failed")
		}

		return E_FAIL;
	}

	// success
	return S_OK;
#endif
}

void CRSupport::EndR()
{
	if(m_bRInitialized)
		m_Rf_endEmbeddedR_ptr(0);

	if(m_hRLib)
	{
		// free the DLL module
#ifdef WIN32
        FreeLibrary(m_hRLib);
#else
		dlclose(m_hRLib);
#endif
		m_hRLib=NULL;
    }
}

HRESULT CRSupport::SetupREnv(
	SEXP& REnv,
	const char *pName,
	wchar_t *pErrMsg)
{
	HRESULT			hr				= S_OK;
	const wchar_t	FUNC_NAME[]		= L"CRSupport::SetupREnv";

	try
	{
		// allocate a new environment
		m_Rf_protect_ptr(REnv = m_Rf_allocSExp_ptr(ENVSXP));
		if(REnv)
		{
			m_SET_FRAME_ptr(REnv, *m_R_NilValue_ptr);
			m_SET_ENCLOS_ptr(REnv, *m_R_GlobalEnv_ptr);
			m_SET_HASHTAB_ptr(REnv, *m_R_NilValue_ptr);
			m_SET_ATTRIB_ptr(REnv, *m_R_NilValue_ptr);
		}
		else
		{
#ifdef WIN32
			INIT_ERROR_MSG_REF(L"%s: Allocation of local environment failed.")
#else
			INIT_ERROR_MSG_REF(L"Allocation of local environment failed.")
#endif
			hr = E_OUTOFMEMORY;
		}

		// define a variable for the new environment
		m_defineVar_ptr(m_install_ptr(pName), REnv, *m_R_GlobalEnv_ptr);

		m_Rf_unprotect_ptr(1);
	}
	catch(...)
	{
		hr = E_FAIL;
	}

	return hr;
}

/************************************************************************
FUNCTION:	CRSupport::ReadScript()
INPUTS:		sRScript			R script absolute filename or URL
			bIsURL				indicates that sRScript is a URL
			REnv				current local environment
			sREnvName			unique name of current local environment
OUTPUTS:	RScript				R variable into which script is read
			pLineCnt			pointer to script line count
			pErrMsg				pointer to error message buffer
RETURNS:	S_OK
			E_FAIL				unhandled exception

This method uses the R readLines() function to read the specified R script,
from either an absolute filename or a URL.  The results will be stored within
the specified local enviroment, in the variable mstr.script.  The SEXP associated
with this variable is returned to the caller, to be used to access the lines
of the script.
*************************************************************************/
HRESULT CRSupport::ReadScript(
	const char *sRScript,
	bool bIsURL,
	SEXP REnv,
	const char *sREnvName,
	SEXP& RScript,
	Int_32 *pLineCnt,
	wchar_t *pErrMsg)
{
	SEXP			/*eUrl,*/ eRdLns, eErrMsg, RErrMsg;
	int				lErrorCode;
	Int_32			lProtectCnt		= 0;
	wchar_t			wcScript[MAXSIZE_CHAR_STR];
	wchar_t			wcRErrMsg[MAXSIZE_CHAR_STR];
	HRESULT			hr				= S_OK;
	const wchar_t	FUNC_NAME[]		= L"CRSupport::ReadScript";
	BLDSTR_DECL()

	try
	{
		// verify script name provided
		if(strlen(sRScript)==0)
		{
#ifdef WIN32
			INIT_ERROR_MSG_REF(L"%s: Missing R script filename/URL.")
#else
			INIT_ERROR_MSG_REF(L"Missing R script filename/URL.")
#endif
			throw E_FAIL;
		}

		if(!bIsURL)
		{
			// script is a path...generate the following expression - readLines("<script path>")
			m_Rf_protect_ptr(eRdLns=m_lang2_ptr(m_install_ptr(RCMD_READLINES), m_mkString_ptr(sRScript)));
			lProtectCnt++;
		}
		else
		{
			// yzhang, US53653, 2016-06-14, disable URL retrieval of R Script
			
			#ifdef WIN32
				INIT_ERROR_MSG_REF(L"%s: Retrieval of R scripts from the metric expression cannot be a URL; please consider adding such script to an administrator approved location.")
			#else
				INIT_ERROR_MSG_REF(L"Retrieval of R scripts from the metric expression cannot be a URL; please consider adding such script to an administrator approved location.")
			#endif
			
			throw E_FAIL;

			// script is a URL...generate the following expression - readLines(url("<script URL>"))
			/*m_Rf_protect_ptr(eUrl=m_lang2_ptr(m_install_ptr(RCMD_URL), m_mkString_ptr(sRScript)));
			m_Rf_protect_ptr(eRdLns=m_lang2_ptr(m_install_ptr(RCMD_READLINES), eUrl));
			lProtectCnt+=2;*/
		}

		// evaluate the readLines() expression
		RScript = m_R_tryEval_ptr(eRdLns, *m_R_GlobalEnv_ptr, &lErrorCode);
		if(lErrorCode > 0)
		{
			m_Rf_protect_ptr(eErrMsg=m_lang1_ptr(m_install_ptr("geterrmessage")));
			lProtectCnt++;
			
			RErrMsg=m_R_tryEval_ptr(eErrMsg, *m_R_GlobalEnv_ptr,&lErrorCode);

			if(lErrorCode>0)
			{
				RFP_WCSCPY(wcRErrMsg, MAXSIZE_CHAR_STR, L"error out when eval geterrmessag");

			}else
			{
				Int_32 lErrSize=m_length_ptr(RErrMsg);
				Int_32 iRType=m_TYPEOF_ptr(RErrMsg);
				if(lErrSize<=0)
				{
					RFP_WCSCPY(wcRErrMsg, MAXSIZE_CHAR_STR, L"no error");
				}else if (iRType!=STRSXP)
				{
					RFP_WCSCPY(wcRErrMsg, MAXSIZE_CHAR_STR, L"no error of string data type");
				}else
				{
					SEXP strElem= m_STRING_ELT_ptr(RErrMsg,0);
					if(strElem!=*m_R_NaString_ptr)
					{
						const char * sRErrMsg=m_translateChar_ptr(strElem);
						RFP_MBSTOWCS(wcRErrMsg,MAXSIZE_CHAR_STR,sRErrMsg)
					}
					else
						RFP_WCSCPY(wcRErrMsg, MAXSIZE_CHAR_STR, L"error is null");
				}
				
			}
			// convert script name to wchar (for logging)
			RFP_MBSTOWCS(wcScript, MAXSIZE_CHAR_STR, sRScript)
			
			
#ifdef WIN32
			if(bIsURL)
			{
				INIT_ERROR_MSG_REF(L"%s: The expression readLines(url(%s)) failed evaluation.  Please check _RScriptFile parameter setting.", wcScript)
			}
			else
			{
				INIT_ERROR_MSG_REF(L"%s: The expression readLines(%s) failed evaluation.  Please check _RScriptFile parameter setting.", wcScript)
			}
#else
			if(wcslen(pErrMsg)==0)
			{	
				if(bIsURL)
				{
					wcscpy(pErrMsg, FUNC_NAME);
					wcscat(pErrMsg, L": The expression readLines(url(");
					wcsncat(pErrMsg, wcScript,100);
					wcscat(pErrMsg,L")) failed evaluation.  Please check _RScriptFile parameter setting.");

				}
				else
				{
					wcscpy(pErrMsg, FUNC_NAME);
					wcscat(pErrMsg, L": The expression readLines(");
					wcsncat(pErrMsg, wcScript,100);
					wcscat(pErrMsg,L") failed evaluation.  Please check _RScriptFile parameter setting.");

				}
			}
#endif
			return E_FAIL;
		}

		// define a variable for the script in the local environment
		m_defineVar_ptr(m_install_ptr(RFP_SCRIPT), RScript, REnv);

		// get the script line count
		*pLineCnt = m_length_ptr(RScript);
		if(*pLineCnt==0)
		{
			// no script found
#ifdef WIN32
			INIT_ERROR_MSG_REF(L"%s: The specified R script is empty.")
#else
			INIT_ERROR_MSG_REF(L"The specified R script is empty.")
#endif
			throw E_FAIL;
		}

		m_Rf_unprotect_ptr(lProtectCnt);
	}
	catch(HRESULT catchHR)
	{
		hr = catchHR;
		INIT_HR_MSG_REF(L"Failure during read of R script.")
		if(lProtectCnt > 0)
			m_Rf_unprotect_ptr(lProtectCnt);
	}
	catch(...)
	{
		hr = E_FAIL;
		INIT_HR_MSG_REF(L"Unhandled exception.")
		if(lProtectCnt > 0)
			m_Rf_unprotect_ptr(lProtectCnt);
	}

	return hr;
}

/************************************************************************
FUNCTION:	CRSupport::ReadScriptLine()
INPUTS:		pRScript			R script variable
			lLineNumber			script line to be accessed
			ppScriptBuffer		pointer to buffer
			pScriptBufLen		pointer to current buffer length
OUTPUTS:	none
RETURNS:	S_OK
			E_OUTOFMEMORY		allocation error
			E_FAIL				unhandled exception

This method copies the specified script line from the specified R variable
to the indicated buffer.  The buffer will be expanded, if necessary.
*************************************************************************/
HRESULT CRSupport::ReadScriptLine(
	SEXP pRScript,
	Int_32 lLineNumber,
	char **ppScriptBuffer,
	size_t *pScriptBufLen)
{
	HRESULT		hr	= S_OK;

	try
	{
		SEXP strElem;
		strElem = m_STRING_ELT_ptr(pRScript, lLineNumber);
		if(strElem != *m_R_NaString_ptr)
		{
			size_t uiLineLen = m_length_ptr(strElem);
			if(uiLineLen > 0)
			{
				// make sure buffer is large enough
				if(uiLineLen > *pScriptBufLen)
				{
					// need to re-allocate buffer...release existing buffer
					delete [] *ppScriptBuffer;

					// allocate new buffer
					*pScriptBufLen = uiLineLen + 256;
					(*ppScriptBuffer) = new char [*pScriptBufLen];
					if(!(*ppScriptBuffer))
						throw E_OUTOFMEMORY;
				}

				// copy script line from R to buffer
				RFP_STRCPY((*ppScriptBuffer), *pScriptBufLen, m_translateChar_ptr(strElem))
			}
			else
				// empty line...clear buffer
				(*ppScriptBuffer)[0] = NULL_CHAR;
		}
	}
	catch(HRESULT catchHR)
	{
		hr = catchHR;
	}
	catch(...)
	{
		hr = E_FAIL;
	}

	return hr;
}

void CRSupport::RemRVar(
	const char *pName)
{
	SEXP	eRm, eGc;
	int		lErrorCode;
	Int_32	lProtectCnt		= 0;

	try
	{
		// generate the following expression - rm("<variable name>")
		m_Rf_protect_ptr(eRm=m_lang2_ptr(m_install_ptr(RCMD_RM), m_mkString_ptr(pName)));
		lProtectCnt++;

		// evaluate the rm() expression
		m_R_tryEval_ptr(eRm, *m_R_GlobalEnv_ptr, &lErrorCode);

		// generate the following expression - gc()
		m_Rf_protect_ptr(eGc=m_lang1_ptr(m_install_ptr("gc")));
		lProtectCnt++;

		// evaluate the gc() expression
		m_R_tryEval_ptr(eGc, *m_R_GlobalEnv_ptr, &lErrorCode);

		m_Rf_unprotect_ptr(lProtectCnt);
	}
	catch(...)
	{
		if(lProtectCnt > 0)
			m_Rf_unprotect_ptr(lProtectCnt);
	}
}

// handles numeric, scalar/vector, non-repeated inputs for SIM, GEN & AGG FPs
void CRSupport::SetRVar(
	IRIPSupport_FO *pIRSuppFO,
	const char *pName,
	Int_32 nSize,
	double *pData,
	RFPData_Flags *pFlag)
{
	SEXP	pRVector;
	double	*pRValues;
	bool	bValid;

	// access specified R environment
	CRSupport_FO *pRSuppFO = (CRSupport_FO *)pIRSuppFO;
	SEXP REnv = pRSuppFO->GetREnv();

	// allocate vector of REAL in R
	m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(REALSXP, nSize));
	pRValues = m_REAL_ptr(pRVector);

	// initialize vector with input data
	for(Int_32 i=0;i<nSize;i++)
	{
		// check for valid input
		bValid = pFlag ? (pFlag[i]==RfpDataOk) : true;
		if(bValid)
			// initialize value with input data
			*(pRValues+i) = pData[i];
		else // missing...set to NA
			*(pRValues+i) = *m_R_NaReal_ptr;
	}

	// define variable
	m_defineVar_ptr(m_install_ptr(pName), pRVector, REnv);
	m_Rf_unprotect_ptr(1);
}

// handles variant properties for all FPs, and variant, scalar, non-repeated inputs for NNSIM and NNGEN FPs
HRESULT CRSupport::SetRVarV(
	IRIPSupport_FO *pIRSuppFO,
	const char *pName,
	void *pValue,
	RFPData_Flags *pFlag)
{
	SEXP		pRVector;
	bool		bValid;
	Int_32		lProtectCnt		= 0;
	HRESULT		hr				= S_OK;
	char * pDestBuffer	= NULL;
	try
	{
		// access specified R environment
		CRSupport_FO *pRSuppFO = (CRSupport_FO *)pIRSuppFO;
		SEXP REnv = pRSuppFO->GetREnv();

		// access variant input data
		RFP_VARIANT *pVarValue = reinterpret_cast<RFP_VARIANT *>(pValue);

		// check data type
		switch(pVarValue->vt)
		{
		case RFP_VT_R8:
		case RFP_VT_R4:
		case RFP_VT_I4:
		case RFP_VT_I2:
		case RFP_VT_UI1:
		case RFP_VT_EMPTY:  // used for NULL inputs...flag should be set to DssDataNull (i.e. not DssDataOk)
			{
				// allocate vector of REAL in R
				m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(REALSXP, 1));
				lProtectCnt++;
				double *pRValue = m_REAL_ptr(pRVector);

				// check for valid input
				bValid = pFlag ? (*pFlag==RfpDataOk) : true;
				if(bValid)
				{
					// initialize value with input data
					switch(pVarValue->vt)
					{
					case RFP_VT_UI1: *pRValue = pVarValue->bVal; break;
					case RFP_VT_I2: *pRValue = pVarValue->iVal; break;
					case RFP_VT_I4: *pRValue = pVarValue->lVal; break;
					case RFP_VT_R4: *pRValue = pVarValue->fltVal; break;
					case RFP_VT_R8: *pRValue = pVarValue->dblVal; break;
					case RFP_VT_EMPTY: *pRValue = *m_R_NaReal_ptr; break;	// used for NULL inputs...flag should be set to RfpDataNull
					default: throw E_FAIL;
					}
				}
				else // DssDataNull...set to NA
					*pRValue = *m_R_NaReal_ptr;

				// define variable
				m_defineVar_ptr(m_install_ptr(pName), pRVector, REnv);

				break;
			}
		case RFP_VT_BOOL:
			{
				// allocate vector of logical in R
				m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(LGLSXP, 1));
				lProtectCnt++;
				int *pRValue = m_LOGICAL_ptr(pRVector);

				// check for valid input
				bValid = pFlag ? (*pFlag==RfpDataOk) : true;
				if(bValid)
					// convert boolean to 0 (false) or 1 (true)
					*pRValue = (pVarValue->boolVal!=VARIANT_FALSE) ? 1 : 0;
				else // DssDataNull...set to NA
					*pRValue = *m_R_NaLogical_ptr;

				// define variable
				m_defineVar_ptr(m_install_ptr(pName), pRVector, REnv);

				break;
			}
		case RFP_VT_BSTR:
			{
				// allocate vector of string in R
				m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(STRSXP, 1));
				lProtectCnt++;

				// check for valid input
				bValid = pFlag ? (*pFlag==RfpDataOk) : true;
				if(bValid)
				{
					// convert variant to CHSTR
					wchar_t *pSource = pVarValue->bstrVal;
					//DE43238  widechar to utf8 conversion
					pDestBuffer = RBase::WideCharToUTF8(pSource, 0, m_sErrMsg);
					// initialize value with input data
					m_SET_STRING_ELT_ptr(pRVector, 0, m_mkChar_ptr(pDestBuffer));
				}
				else // missing...set to NA
					m_SET_STRING_ELT_ptr(pRVector, 0, *m_R_NaString_ptr);

				// define variable
				m_defineVar_ptr(m_install_ptr(pName), pRVector, REnv);

				break;
			}
		default:
			{
				// unexpected data type
				throw E_FAIL;
			}
		}

		if(lProtectCnt > 0)
			m_Rf_unprotect_ptr(lProtectCnt);
	}
	catch(HRESULT catchHR)
	{
		hr = catchHR;
		if(lProtectCnt > 0)
			m_Rf_unprotect_ptr(lProtectCnt);
	}
	catch(...)
	{
		hr = E_FAIL;
		if(lProtectCnt > 0)
			m_Rf_unprotect_ptr(lProtectCnt);
	}
	if(pDestBuffer)
		delete [] pDestBuffer;
	return hr;
}

// handles variant, vector, non-repeated inputs for NNGEN FPs
HRESULT CRSupport::SetRVarSA(
	IRIPSupport_FO *pIRSuppFO,
	const char *pName,
	Int_32 nSize,
	EnumRFPDataType eDataType,
	void *pData,
	RFPData_Flags *pFlag)
{
	SEXP		pRVector;
	bool		bValid;
	Int_32		lProtectCnt		= 0;
	HRESULT		hr				= S_OK;
	char*		pDestBuffer		= NULL;
	try
	{
		// access specified R environment
		CRSupport_FO *pRSuppFO = (CRSupport_FO *)pIRSuppFO;
		SEXP REnv = pRSuppFO->GetREnv();

		// access variant input data
		RFP_VARIANT *pVarValues = reinterpret_cast<RFP_VARIANT *>(pData);

		// check data type
		switch(eDataType)
		{
		case RfpDataTypeDouble:
			{
				double *pRValues;

				// allocate vector of REAL in R
				m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(REALSXP, nSize));
				lProtectCnt++;
				pRValues = m_REAL_ptr(pRVector);

				// initialize vector with input data
				for(Int_32 i=0;i<nSize;i++)
				{
					Int_32 tmps=sizeof(pVarValues[0]);
					// check for valid input
					bValid = pFlag ? (pFlag[i]==RfpDataOk) : true;
					if(bValid)
					{
						// initialize value with input data
						switch(pVarValues[i].vt)
						{
						case RFP_VT_UI1: *(pRValues+i) = pVarValues[i].bVal; break;
						case RFP_VT_I2: *(pRValues+i) = pVarValues[i].iVal; break;
						case RFP_VT_I4: *(pRValues+i) = pVarValues[i].lVal; break;
						case RFP_VT_R4: *(pRValues+i) = pVarValues[i].fltVal; break;
						case RFP_VT_R8: *(pRValues+i) = pVarValues[i].dblVal; break;
						case RFP_VT_EMPTY: *(pRValues+i) = *m_R_NaReal_ptr; break;	// used for NULL inputs...flag should be set to RfpDataNull
						default: throw E_FAIL;
						}
					}
					else // missing...set to NA
						*(pRValues+i) = *m_R_NaReal_ptr;
				}

				// define variable
				m_defineVar_ptr(m_install_ptr(pName), pRVector, REnv);

				break;
			}
		case RfpDataTypeVarChar:
			{
				// validate variant data type
				if(pVarValues[0].vt != RFP_VT_BSTR)
					throw E_FAIL;

				// allocate vector of string in R
				m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(STRSXP, nSize));
				lProtectCnt++;

				// initialize vector with input data
				for(Int_32 i=0;i<nSize;i++)
				{
					// check for valid input
					bValid = pFlag ? (pFlag[i]==RfpDataOk) : true;
					if(bValid)
					{
						// access element and convert to CHSTR
						wchar_t *pSource = pVarValues[i].bstrVal;
						//DE43238  widechar to utf8 conversion
						pDestBuffer = RBase::WideCharToUTF8(pSource, 0, m_sErrMsg);
						// initialize value with input data
						m_SET_STRING_ELT_ptr(pRVector, i, m_mkChar_ptr(pDestBuffer));
					}
					else // missing...set to NA
						m_SET_STRING_ELT_ptr(pRVector, i, *m_R_NaString_ptr);
				}

				// define variable
				m_defineVar_ptr(m_install_ptr(pName), pRVector, REnv);

				break;
			}
		default:
			{
				// unexpected data type
				throw E_FAIL;
			}
		}

		if(lProtectCnt > 0)
			m_Rf_unprotect_ptr(lProtectCnt);
	}
	catch(HRESULT catchHR)
	{
		hr = catchHR;
		if(lProtectCnt > 0)
			m_Rf_unprotect_ptr(lProtectCnt);
	}
	catch(...)
	{
		hr = E_FAIL;
		if(lProtectCnt > 0)
			m_Rf_unprotect_ptr(lProtectCnt);
	}
	if(pDestBuffer)
		delete [] pDestBuffer;
	return hr;
}

// handles variant, scalar/vector, repeated inputs for NNGEN FPs
HRESULT CRSupport::SetRVarSA_mx(
	IRIPSupport_FO *pIRSuppFO,
	const char *pName,
	Int_32 lPIndex,
	Int_32 nRows,
	Int_32 nCols,
	EnumRFPDataType eDataType,
	void *pData,
	RFPData_Flags *pFlag,
	bool bSafeArray,
	wchar_t *pErrMsg)
{
	SEXP			pRMatrix;
	bool			bValid;
	Int_32			lProtectCnt		= 0;
	HRESULT			hr				= S_OK;
	const wchar_t	FUNC_NAME[]		= L"CRSupport::SetRVarSA_mx";
	char *			pDestBuffer		= NULL;
	try
	{
		// access specified R environment
		CRSupport_FO *pRSuppFO = (CRSupport_FO *)pIRSuppFO;
		SEXP REnv = pRSuppFO->GetREnv();

		// access variant input data
		RFP_VARIANT *pVarValues = reinterpret_cast<RFP_VARIANT *>(pData);

		// check data type
		switch(eDataType)
		{
		case RfpDataTypeDouble:
			{
				double *pRValues;

				if(lPIndex==0)
				{
					// allocate matrix of REAL in R
					m_Rf_protect_ptr(pRMatrix = m_Rf_allocMatrix_ptr(REALSXP, nRows, nCols));
					lProtectCnt++;
				}
				else
				{
					// find previously allocated matix
					m_Rf_protect_ptr(pRMatrix=m_findVar_ptr(m_install_ptr(pName), REnv));
					lProtectCnt++;
					if(pRMatrix==*m_R_UnboundValue_ptr)
						// variable not found in R environment
						throw E_FAIL;

					// check number of rows
					Int_32 lRowCount = m_Rf_nrows_ptr(pRMatrix);
					if(nRows > lRowCount)
					{
						// first input consisted of fewer rows (probably was a scalar)...matrix cannot handle
						//	increased number of rows (NOTE: The AE appears to pass in a safe array with the scalar
						//	value repeated to the length of the expected vector.  So, this check main not be necessary.)
#ifdef WIN32
						INIT_ERROR_MSG_REF(
							L"%s: When mixing scalar/vector inputs with repeated input, first input must be vector.")
#else
						INIT_ERROR_MSG_REF(
							L"When mixing scalar/vector inputs with repeated input, first input must be vector.")
#endif
						throw E_FAIL;
					}
					else
						// set nRows to actual matrix row count...handles case where scalar passed to repeated input
						//	defined to be vector...this will result in scalar value being copied to all rows
						nRows = lRowCount;
				}

				// initialize specified column of matrix with input data
				pRValues = m_REAL_ptr(pRMatrix);
				for(Int_32 i=0,j=lPIndex*nRows;i<nRows;i++,j++)
				{
					// check for valid input
					bValid = pFlag ? (pFlag[i]==RfpDataOk) : true;
					if(bValid)
					{
						// initialize value with input data
						switch(pVarValues[i].vt)
						{
						case RFP_VT_UI1: *(pRValues+j) = pVarValues[i].bVal; break;
						case RFP_VT_I2: *(pRValues+j) = pVarValues[i].iVal; break;
						case RFP_VT_I4: *(pRValues+j) = pVarValues[i].lVal; break;
						case RFP_VT_R4: *(pRValues+j) = pVarValues[i].fltVal; break;
						case RFP_VT_R8: *(pRValues+j) = pVarValues[i].dblVal; break;
						case RFP_VT_EMPTY: *(pRValues+j) = *m_R_NaReal_ptr; break;	// used for NULL inputs...flag should be set to RfpDataNull
						default: throw E_FAIL;
						}
					}
					else // missing...set to NA
						*(pRValues+j) = *m_R_NaReal_ptr;
				}

				break;
			}
		case RfpDataTypeVarChar:
			{
				// validate variant data type
				if(pVarValues[0].vt != RFP_VT_BSTR)
					throw E_FAIL;

				if(lPIndex==0)
				{
					// allocate matrix of strings in R
					m_Rf_protect_ptr(pRMatrix = m_Rf_allocMatrix_ptr(STRSXP, nRows, nCols));
					lProtectCnt++;
				}
				else
				{
					// find previously allocated matrix
					m_Rf_protect_ptr(pRMatrix=m_findVar_ptr(m_install_ptr(pName), REnv));
					lProtectCnt++;
					if(pRMatrix==*m_R_UnboundValue_ptr)
						// variable not found in R environment
						throw E_FAIL;

					// check number of rows
					Int_32 lRowCount = m_Rf_nrows_ptr(pRMatrix);
					if(nRows > lRowCount)
					{
						// first input consisted of fewer rows (probably was a scalar)...matrix cannot handle
						//	increased number of rows (NOTE: The AE appears to pass in a safe array with the scalar
						//	value repeated to the length of the expected vector.  So, this check main not be necessary.)
#ifdef WIN32
						INIT_ERROR_MSG_REF(
							L"%s: When mixing scalar/vector inputs with repeated vector input, first input must be vector.")
#else
						INIT_ERROR_MSG_REF(
							L"When mixing scalar/vector inputs with repeated vector input, first input must be vector.")
#endif
						throw E_FAIL;
					}
					else
						// set nRows to actual matrix row count...handles case where scalar passed to repeated input
						//	defined to be vector...this will result in scalar value being copied to all rows
						nRows = lRowCount;
				}

				// initialize specified column of matrix with input data
				for(Int_32 i=0,j=lPIndex*nRows;i<nRows;i++,j++)
				{
					// check for valid input
					bValid = pFlag ? (pFlag[i]==RfpDataOk) : true;
					if(bValid)
					{
						// access element and convert to CHSTR
						wchar_t *pSource = pVarValues[i].bstrVal;
						//DE43238  widechar to utf8 conversion
						pDestBuffer = RBase::WideCharToUTF8(pSource, 0, m_sErrMsg);
						// initialize value with input data
						m_SET_STRING_ELT_ptr(pRMatrix, j, m_mkChar_ptr(pDestBuffer));
					}
					else // missing...set to NA
						m_SET_STRING_ELT_ptr(pRMatrix, j, *m_R_NaString_ptr);
				}

				break;
			}
		default:
			{
				// unexpected data type
				throw E_FAIL;
			}
		}

		if(lPIndex==0)
			// define variable
			m_defineVar_ptr(m_install_ptr(pName), pRMatrix, REnv);

		if(lProtectCnt > 0)
			m_Rf_unprotect_ptr(lProtectCnt);
	}
	catch(HRESULT catchHR)
	{
		hr = catchHR;
		if(lProtectCnt > 0)
			m_Rf_unprotect_ptr(lProtectCnt);
	}
	catch(...)
	{
		hr = E_FAIL;
		if(lProtectCnt > 0)
			m_Rf_unprotect_ptr(lProtectCnt);
	}

	if(pDestBuffer)
		delete [] pDestBuffer;
	return hr;
}

// handles string, scalar, non-repeated input
void CRSupport::SetRVarStr(
	IRIPSupport_FO *pIRSuppFO,
	const char *pName,
	char *pData)
{
	SEXP	pRVector;

	// access specified R environment
	CRSupport_FO *pRSuppFO = (CRSupport_FO *)pIRSuppFO;
	SEXP REnv = pRSuppFO->GetREnv();

	// allocate vector in R
	m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(STRSXP, 1));

	// initialize vector with input data
	m_SET_STRING_ELT_ptr(pRVector, 0, m_mkChar_ptr(pData));

	// define variable
	m_defineVar_ptr(m_install_ptr(pName), pRVector, REnv);
	m_Rf_unprotect_ptr(1);
}

// handles _InputNames parameter
void CRSupport::SetRVar_InputNames(
	IRIPSupport_FO *pIRSuppFO,
	std::vector<char *> &vInputNames)
{
	SEXP	pRVector;

	// access specified R environment
	CRSupport_FO *pRSuppFO = (CRSupport_FO *)pIRSuppFO;
	SEXP REnv = pRSuppFO->GetREnv();

	// allocate vector in R
	m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(STRSXP, (R_len_t)vInputNames.size()));

	// initialize vector with input data
	for(Int_32 i=0;i<(Int_32)vInputNames.size();i++)
	{
		m_SET_STRING_ELT_ptr(pRVector, i, m_mkChar_ptr(vInputNames[i]));
	}

	// define variable
	m_defineVar_ptr(m_install_ptr(RFP_INPUTNAMES), pRVector, REnv);
	m_Rf_unprotect_ptr(1);
}

Int_32 CRSupport::SubmitR(
	IRIPSupport_FO *pIRSuppFO,
	wchar_t *pErrMsg)
{
	SEXP			eNull, eGetEnv, eGetScr, eParse, eEval;
	int				lErrorCode;
	const wchar_t	FUNC_NAME[]		= L"CRSupport::SubmitR";

	try
	{
		// access RSuppFO class
		CRSupport_FO *pRSuppFO = (CRSupport_FO *)pIRSuppFO;

		// generate the following expression - eval(parse("", as.null(), get("mstr.script", get("<REnvName>"))), get("<REnvName>"))
		m_Rf_protect_ptr(eNull=m_lang1_ptr(m_install_ptr(RCMD_ASNULL)));
		m_Rf_protect_ptr(eGetEnv=m_lang2_ptr(m_install_ptr(RCMD_GET), m_mkString_ptr(pRSuppFO->GetREnvName())));
		m_Rf_protect_ptr(eGetScr=m_lang3_ptr(m_install_ptr(RCMD_GET), m_mkString_ptr(RFP_SCRIPT), eGetEnv));
		m_Rf_protect_ptr(eParse=m_lang4_ptr(m_install_ptr(RCMD_PARSE), m_mkString_ptr(""), eNull, eGetScr));
		m_Rf_protect_ptr(eEval=m_lang3_ptr(m_install_ptr(RCMD_EVAL), eParse, eGetEnv));

		// evaluate the expression
		m_R_tryEval_ptr(eEval, *m_R_GlobalEnv_ptr, &lErrorCode);
		m_Rf_unprotect_ptr(5);
	}
	catch(...)
	{
#ifdef WIN32
		INIT_ERROR_MSG_REF(L"%s: R script execution failed abnormally.")
#else
		INIT_ERROR_MSG_REF(L"R script execution failed abnormally.")
#endif
		lErrorCode = 1;
	}

	return lErrorCode;
}

HRESULT CRSupport::GetRVarNN(
	IRIPSupport_FO *pIRSuppFO,
	const char *pName,
	Int_32 nSize,
	EnumRFPDataType eExpectedDT,
	EnumRFPParameterType eExpectedPT,
	Int_32 *pRSize,
	void *pResults,
	RFPData_Flags *pFlag,
	wchar_t *pErrMsg)
{
	SEXP			s;
	Int_32			lSize			= 0;
	Int_32			lProtectCnt		= 0;
	wchar_t			wcsVarName[MAXSIZE_CHAR_STR];
	HRESULT			hr				= S_OK;
	const wchar_t	FUNC_NAME[]		= L"CRSupport::GetRVarNN";
	BLDSTR_DECL()

	try
	{
		// access specified R environment
		CRSupport_FO *pRSuppFO = (CRSupport_FO *)pIRSuppFO;
		SEXP REnv = pRSuppFO->GetREnv();

		// initialize
		*(void **)pResults = NULL;
		RFP_MBSTOWCS(wcsVarName, MAXSIZE_CHAR_STR, pName)
		bool bScalar = (eExpectedPT==RfpParameterScalar);

		m_Rf_protect_ptr(s=m_findVar_ptr(m_install_ptr(pName), REnv));
		lProtectCnt++;
		if(s==*m_R_UnboundValue_ptr)
			// variable not found in R environment
			*pRSize = 0;	
		else
		{
			// fetch length of results
			*pRSize = m_length_ptr(s);

			// determine size of results to be copied (do not exceed expected size)
			lSize = (*pRSize > nSize) ? nSize : *pRSize;

			// access results based on data type
			int iRType = m_TYPEOF_ptr(s);
			switch(iRType)
			{
			case REALSXP:
				{
					// verify output data type
					if(eExpectedDT != RfpDataTypeDouble)
					{
#ifdef WIN32
						INIT_ERROR_MSG_REF(L"%s: Actual data type REALSXP is not compatible with expected data type %s for output '%s'.",
							RFP_GET_DT_STR(eExpectedDT), wcsVarName)
#else
						if(wcslen(pErrMsg)==0)
						{
						wcscpy(pErrMsg, FUNC_NAME);
						wcscat(pErrMsg, L": Actual data type REALSXP is not compatible with expected data type ");
						wcscat(pErrMsg, RFP_GET_DT_STR(eExpectedDT));
						wcscat(pErrMsg, L" for output '");
						wcsncat(pErrMsg, wcsVarName,100);
						wcscat(pErrMsg, L"'.");
						}

#endif
						throw E_FAIL;
					}

					double dblTemp;
					double *pREAL = m_REAL_ptr(s);

					if(bScalar)
					{
						// scalar output...init flag
						*pFlag = RfpDataInvalid;

						// verify size
						if(lSize > 0)
						{
							// output available...fetch first result
							dblTemp = pREAL[0];

							// TODO: add your post-processing code here

							// return single result and set flag
							if(m_R_finite_ptr(dblTemp))
							{
								*(double *)pResults = dblTemp;
								*pFlag = RfpDataOk;
							}
							//else...result is invalid...flag already set to invalid
						}
						//else...unexpected size...NULL would return size=0...flag already set to invalid
					}
					else
					{
						// vector output...access output array of variants
						RFP_VARIANT *pOutValues = reinterpret_cast<RFP_VARIANT *>(pResults);

						// copy outputs from R environment into results
						for(Int_32 i=0;i<lSize;i++)
						{
							// fetch result
							dblTemp = pREAL[i];

							// TODO: add your post-processing code here

							if(m_R_finite_ptr(dblTemp))
							{
								pOutValues[i].vt = RFP_VT_R8;
								pOutValues[i].dblVal = dblTemp;
								pFlag[i] = RfpDataOk;
							}
							else
							{
								// result is invalid...set array entry to empty value and flag to invalid
								pOutValues[i].vt = RFP_VT_EMPTY;
								pFlag[i] = RfpDataInvalid;
							}
						}
					}

					break;
				}
			case INTSXP:
				{
					// verify output data type
					if(eExpectedDT != RfpDataTypeDouble)
					{
#ifdef WIN32
						INIT_ERROR_MSG_REF(L"%s: Actual data type INTSXP is not compatible with expected data type %s for output '%s'.",
							RFP_GET_DT_STR(eExpectedDT), wcsVarName)
#else
						if(wcslen(pErrMsg)==0)
						{
						wcscpy(pErrMsg, FUNC_NAME);
						wcscat(pErrMsg, L": Actual data type INTSXP is not compatible with expected data type ");
						wcscat(pErrMsg, RFP_GET_DT_STR(eExpectedDT));
						wcscat(pErrMsg, L" for output '");
						wcsncat(pErrMsg, wcsVarName,100);
						wcscat(pErrMsg, L"'.");
						}
#endif
						throw E_FAIL;
					}

					int iTemp;
					int *pINTEGER = m_INTEGER_ptr(s);

					if(bScalar)
					{
						// scalar output...init flag
						*pFlag = RfpDataInvalid;

						// verify size
						if(lSize > 0)
						{
							// output available...fetch first result
							iTemp = pINTEGER[0];

							// TODO: add your post-processing code here

							// return single result and set flag
							if(iTemp!=*m_R_NaInt_ptr)
							{
								*(double *)pResults = (double)iTemp;
								*pFlag = RfpDataOk;
							}
							//else...result is invalid...flag already set to invalid
						}
						//else...unexpected size...NULL would return size=0...flag already set to invalid
					}
					else
					{
						// vector output...access output array of variants
						RFP_VARIANT *pOutValues = reinterpret_cast<RFP_VARIANT *>(pResults);

						// copy outputs from R environment into results
						for(Int_32 i=0;i<lSize;i++)
						{
							// fetch result
							iTemp = pINTEGER[i];

							// TODO: add your post-processing code here

							if(iTemp!=*m_R_NaInt_ptr)
							{
								pOutValues[i].vt = RFP_VT_R8;
								pOutValues[i].dblVal = (double)iTemp;
								pFlag[i] = RfpDataOk;
							}
							else
							{
								// result is invalid...set array entry to empty value and flag to invalid
								pOutValues[i].vt = RFP_VT_EMPTY;
								pFlag[i] = RfpDataInvalid;
							}
						}
					}

					break;
				}
			case LGLSXP:
				{
					// verify output data type
					if(eExpectedDT != RfpDataTypeDouble)
					{
#ifdef WIN32
						INIT_ERROR_MSG_REF(L"%s: Actual data type LGLSXP is not compatible with expected data type %s for output '%s'.",
							RFP_GET_DT_STR(eExpectedDT), wcsVarName)
#else
						if(wcslen(pErrMsg)==0)
						{
						wcscpy(pErrMsg, FUNC_NAME);
						wcscat(pErrMsg, L": Actual data type LGLSXP is not compatible with expected data type ");
						wcscat(pErrMsg, RFP_GET_DT_STR(eExpectedDT));
						wcscat(pErrMsg, L" for output '");
						wcsncat(pErrMsg, wcsVarName,100);
						wcscat(pErrMsg, L"'.");
						}
#endif
						throw E_FAIL;
					}

					int iTemp;
					int *pLOGICAL = m_LOGICAL_ptr(s);

					if(bScalar)
					{
						// scalar output...init flag
						*pFlag = RfpDataInvalid;

						// verify size
						if(lSize > 0)
						{
							// output available...fetch first result
							iTemp = pLOGICAL[0];

							// TODO: add your post-processing code here

							// return single result and set flag
							if(iTemp!=*m_R_NaLogical_ptr)
							{
								*(double *)pResults = (double)iTemp;
								*pFlag = RfpDataOk;
							}
							//else...result is invalid...flag already set to invalid
						}
						//else...unexpected size...NULL would return size=0...flag already set to invalid
					}
					else
					{
						// vector output...access output array of variants
						RFP_VARIANT *pOutValues = reinterpret_cast<RFP_VARIANT *>(pResults);

						// copy outputs from R environment into results
						for(Int_32 i=0;i<lSize;i++)
						{
							// fetch result
							iTemp = pLOGICAL[i];

							// TODO: add your post-processing code here

							if(iTemp!=*m_R_NaLogical_ptr)
							{
								pOutValues[i].vt = RFP_VT_R8;
								pOutValues[i].dblVal = (double)iTemp;
								pFlag[i] = RfpDataOk;
							}
							else
							{
								// result is invalid...set array entry to empty value and flag to invalid
								pOutValues[i].vt = RFP_VT_EMPTY;
								pFlag[i] = RfpDataInvalid;
							}
						}
					}

					break;
				}
			case STRSXP:
				{
					// verify output data type
					if(eExpectedDT != RfpDataTypeVarChar)
					{
#ifdef WIN32
						INIT_ERROR_MSG_REF(L"%s: Actual data type STRSXP is not compatible with expected data type %s for output '%s'.",
							RFP_GET_DT_STR(eExpectedDT), wcsVarName)
#else
						if(wcslen(pErrMsg)==0)
						{
						wcscpy(pErrMsg, FUNC_NAME);
						wcscat(pErrMsg, L": Actual data type STRSXP is not compatible with expected data type ");
						wcscat(pErrMsg, RFP_GET_DT_STR(eExpectedDT));
						wcscat(pErrMsg, L" for output '");
						wcsncat(pErrMsg, wcsVarName,100);
						wcscat(pErrMsg, L"'.");
						}
#endif
						throw E_FAIL;
					}

					SEXP strElem;

					if(bScalar)
					{
						// scalar output...init flag
						*pFlag = RfpDataInvalid;

						// verify size
						if(lSize > 0)
						{
							// output available...fetch first result
							strElem = m_STRING_ELT_ptr(s, 0);

							// check if NA
							if(strElem != *m_R_NaString_ptr)
							{
								// return single string and set flag to OK
								*(const char **)pResults = m_translateChar_ptr(strElem);
								*pFlag = RfpDataOk;
							}
							else
								// string is NA...return NULL...flag already set to invalid
								*(const char **)pResults = NULL;
						}
						//else...unexpected size...NULL would return size=0...flag already set to invalid
					}
					else
					{
						// vector output...copy results to array of wchars...allocate array
						const char **pOutputs = new const char * [lSize];
						if(!pOutputs)
							throw E_OUTOFMEMORY;

						// process outputs
						for(Int_32 i=0;i<lSize;i++)
						{
							// fetch result
							strElem = m_STRING_ELT_ptr(s, i);

							// check if NA
							if(strElem != *m_R_NaString_ptr)
							{
								// copy char * to output array and set flag to OK
								pOutputs[i] = m_translateChar_ptr(strElem);
								pFlag[i] = RfpDataOk;
							}
							else
							{
								// element is NA...set array element to NULL and flag to invalid
								pOutputs[i] = NULL;
								pFlag[i] = RfpDataInvalid;
							}
						}

						// return array of char *
						*(const char ***)pResults = pOutputs;
					}

					break;
				}
			default:
				{
					// unsupported data type
#ifdef WIN32
					INIT_ERROR_MSG_REF(L"%s: Actual data type %s (%d) is not compatible with expected data type %s for output '%s'.",
						RFP_GET_RDT_STR(iRType), iRType, RFP_GET_DT_STR(eExpectedDT), wcsVarName)
#else
					if(wcslen(pErrMsg)==0)
					{
					swprintf(wcsWork, lWorklen, L" (%d) ", iRType);
					wcscpy(pErrMsg, FUNC_NAME);
					wcscat(pErrMsg, L": Actual data type ");
					wcscat(pErrMsg, RFP_GET_DT_STR(iRType));
					wcscat(pErrMsg, wcsWork);
					wcscat(pErrMsg, L"is not compatible with expected data type ");
					wcscat(pErrMsg, RFP_GET_DT_STR(eExpectedDT));
					wcscat(pErrMsg, L" for output '");
					wcsncat(pErrMsg, wcsVarName,100);
					wcscat(pErrMsg, L"'.");
					}
#endif
					throw E_FAIL;
				}
			}
		}

		// if fewer results returned than expected (i.e. lSize < nSize), set remaining flags to invalid
		for(Int_32 i=lSize;i<nSize;i++)
			pFlag[i] = RfpDataInvalid;

		if(lProtectCnt > 0)
			m_Rf_unprotect_ptr(lProtectCnt);
	}
	catch(HRESULT catchHR)
	{
		hr = catchHR;
		if(lProtectCnt > 0)
			m_Rf_unprotect_ptr(lProtectCnt);
	}
	catch(...)
	{
		hr = E_FAIL;
		if(lProtectCnt > 0)
			m_Rf_unprotect_ptr(lProtectCnt);
	}

	return hr;
}

//=======================================================================
// CVarInfo class
// constructor/destructor
//=======================================================================
CVarInfo::CVarInfo() :
	m_sVarName(NULL),
	m_eRVarType(rvtInput),
	m_eParamType(RfpParameterVector),
	m_eDataType(RfpDataTypeUnknown),
	m_bDisabledVar(false),
	m_bRepeatInput(false),
	m_lParamIndex(-1),
	m_bInitialized(false)
{
}

CVarInfo::~CVarInfo()
{
	if(m_sVarName) delete [] m_sVarName;
}

HRESULT CVarInfo::CheckDataType(
	EnumRFPDataType eDataType)
{
	switch(eDataType)
	{
	case RfpDataTypeDouble:
	case RfpDataTypeShort:
	case RfpDataTypeFloat:
	case RfpDataTypeInteger:
		{
			if(m_eDataType==RfpDataTypeVarChar)
				// not double or unknown...incompatible data type
				return E_FAIL;
			else
				// set data type (just in case it is currently set to unknown)
				m_eDataType = RfpDataTypeDouble;

			break;
		}
	case RfpDataTypeVarChar:
	case RfpDataTypeUTF8Char:
		{
			if(m_eDataType==RfpDataTypeDouble)
				// not varchar or unknown...incompatible data type
				return E_FAIL;
			else
				// set data type (just in case it is currently set to unknown)
				m_eDataType = RfpDataTypeVarChar;

			break;
		}
	default:
		{
			return E_INVALIDARG;
		}
	}

	return S_OK;
}

//=======================================================================
// CRSupport_FO class
// constructor/destructor
//=======================================================================
CRSupport_FO::CRSupport_FO(
	IRIPSupport *pRSupp,
	EnumRFPType eRFPType,
	bool bIsSorted,
	wchar_t *pErrMsg) :
		m_pRSupp((CRSupport *)pRSupp),
		m_pREnvironment(NULL),
		m_pRScript(NULL),
		m_eRFPType(eRFPType),
		m_bIsSorted(bIsSorted),
		m_lRepeatCnt(0)
{
	try
	{
		// save pointer to parent's error message buffer
		m_sErrMsg = pErrMsg;

		if(pRSupp)
		{
			// initialize with name prefix
			RFP_STRCPY(m_szEnvName, RFP_ENVNAME_BUFLEN, RFP_ENVNAME)

			// generate unique name for environment
			char *pEnvName = &m_szEnvName[strlen(RFP_ENVNAME)];
			Int_32 lNameLen = rand() % RFP_ENVNAME_RANGE + RFP_ENVNAME_MINSIZE;
			for(Int_32 i=0;i<lNameLen;i++)
			{
				pEnvName[i] = RFP_RANDOM_CHARS_SET[rand() % (strlen(RFP_RANDOM_CHARS_SET))];
			}
			pEnvName[lNameLen] = NULL_CHAR;

			// set up R environment for this FO
			HRESULT hr = m_pRSupp->SetupREnv(m_pREnvironment, m_szEnvName, m_sErrMsg);
		}
		else
			m_szEnvName[0] = NULL_CHAR;
	}
	catch(...)
	{
	}
}

CRSupport_FO::~CRSupport_FO()
{
	try
	{
		if(m_pREnvironment)
			// release local environment
			m_pRSupp->RemRVar(m_szEnvName);

		// release memory
		if(m_vInputInfo.size() > 0)
		{
			for(Int32 i=0;i<m_vInputInfo.size();i++)
				delete m_vInputInfo[i];
			m_vInputInfo.clear();
		}
		if(m_vOutputInfo.size() > 0)
		{
			for(Int32 i=0;i<m_vOutputInfo.size();i++)
				delete m_vOutputInfo[i];
			m_vOutputInfo.clear();
		}
		if(m_vParamInfo_pd.size() > 0)
		{
			for(Int32 i=0;i<m_vParamInfo_pd.size();i++)
				delete m_vParamInfo_pd[i];
			m_vParamInfo_pd.clear();
		}
		if(m_vParamInfo_ud.size() > 0)
		{
			for(Int32 i=0;i<m_vParamInfo_ud.size();i++)
				delete m_vParamInfo_ud[i];
			m_vParamInfo_ud.clear();
		}
		m_vInputNames.clear();
	}
	catch(...)
	{
	}
}

//=======================================================================
// CRSupport_FO methods
//=======================================================================
IVarInfo *CRSupport_FO::GetParam_pd(
	Int_32 lIndex)
{
	CVarInfo		*pVI			= NULL;
	const wchar_t	FUNC_NAME[]		= L"CRSupport_FO::GetParam_pd";

	try
	{
		// search pre-defined parameters for specified index...if not found, return NULL
		for(Int32 i=0;i<m_vParamInfo_pd.size();i++)
		{
			if(m_vParamInfo_pd[i]->GetParamIndex()==lIndex)
			{
				// found match...set return var
				pVI = m_vParamInfo_pd[i];
				break;
			}
		}
	}
	catch(...)
	{
		INIT_ERROR_MSG(L"Error searching for rvar parameter.")
	}

	return pVI;
}

IVarInfo *CRSupport_FO::GetParam_ud(
	char *sVarName)
{
	CVarInfo		*pVI			= NULL;
	const wchar_t	FUNC_NAME[]		= L"CRSupport_FO::GetParam_ud";

	try
	{
		// search user-defined parameters for specified name
		for(Int32 i=0;i<m_vParamInfo_ud.size();i++)
		{
			if(strcmp(sVarName, m_vParamInfo_ud[i]->GetVarName())==0)
			{
				// found match...set return var
				pVI = m_vParamInfo_ud[i];
				break;
			}
		}

		if(!pVI)
		{
			// not found on list of user-defined parameters...check pre-defined list
			for(Int32 i=0;i<m_vParamInfo_pd.size();i++)
			{
				if(strcmp(sVarName, m_vParamInfo_pd[i]->GetVarName())==0)
				{
					// found match...set return var
					pVI = m_vParamInfo_pd[i];
					break;
				}
			}
		}
	}
	catch(...)
	{
		INIT_ERROR_MSG(L"Error searching for rvar parameter.")
	}

	return pVI;
}

HRESULT CRSupport_FO::InitParams_ud()
{
	HRESULT			hr				= S_OK;
	const wchar_t	FUNC_NAME[]		= L"CRSupport_FO::InitParams_ud";

	try
	{
		// search for uninitialized parameters
		for(Int_32 i=0;i<(Int_32)m_vParamInfo_ud.size();i++)
		{
			CVarInfo *pVI = m_vParamInfo_ud[i];

			if(!pVI->IsInitialized())
			{
				// initialize to default value
				switch(pVI->GetDataType())
				{
				case RfpDataTypeDouble:
				case RfpDataTypeUnknown:  // default data type is numeric
					{
						// intialize to 0
						double dblValue = 0;
						m_pRSupp->SetRVar(this, pVI->GetVarName(), 1, &dblValue, NULL);
						break;
					}
				case RfpDataTypeVarChar:
					{
						// initialize to empty string
						m_pRSupp->SetRVarStr(this, pVI->GetVarName(), "");
						break;
					}
				case RfpDataTypeBool:
					{
						// initialize to false
						RFP_VARIANT vVarBool;
						vVarBool.vt = RFP_VT_BOOL;
						vVarBool.boolVal = VARIANT_FALSE;
						m_pRSupp->SetRVarV(this, pVI->GetVarName(), (void *)&vVarBool, NULL);
						break;
					}
				default:
					{
						INIT_ERROR_MSG(L"Unexpected data type.")
						throw E_FAIL;
					}
				}
			}
		}
	}
	catch(HRESULT catchHR)
	{
		hr = catchHR;
	}
	catch(...)
	{
		hr = E_FAIL;
	}

	return hr;
}

/************************************************************************
FUNCTION:	CRSupport_FO::IsReservedForR()
INPUTS:		pVarName		variable name
OUTPUTS:	none
RETURNS:	true			variable name is reserved for R
			false			variable name is available for R script

This method checks for variable names that conflict with R reserve words.
The following is a list of R reserve words:
   * if,else,repeat,while,function,for,in,next,break
   * TRUE,FALSE,NULL,Inf,NaN,NA,NA_integer_,NA_real_,NA_complex_,NA_character_
   * ...
   * . or .. followed by number
*************************************************************************/
bool CRSupport_FO::IsReservedForR(
	char *pVarName)
{
	bool			bReserved		= false;
	const wchar_t	FUNC_NAME[]		= L"CRSupport_FO::IsReservedForR";

	try
	{
		// check list of reserved names
		for(Int_32 i=0;i<RFP_RESV_FOR_R_CNT;i++)
		{
			if(strcmp(pVarName, RFP_RESERVED_FOR_R[i])==0)
			{
				bReserved = true;
				break;
			}
		}

		// check for '.' or '..' followed by a number
		if(!bReserved && (strlen(pVarName) > 1) && (pVarName[0]==PERIOD_CHAR))
		{
			if(isdigit(pVarName[1]))
				bReserved = true;
			else if((strlen(pVarName) > 2) && (pVarName[1]==PERIOD_CHAR))
			{
				if(isdigit(pVarName[2]))
					bReserved = true;
			}
		}
	}
	catch(...)
	{
		INIT_ERROR_MSG(L"Error occurred while checking for reserved variable names.")
		bReserved = true;
	}

	return bReserved;
}

/************************************************************************
FUNCTION:	CRSupport_FO::HasBadChar()
INPUTS:		pVarName		variable name
OUTPUTS:	none
RETURNS:	true			variable name contains a bad char
			false			variable name is valid for R script

This method checks for invalid characters in the variable name.
*************************************************************************/
bool CRSupport_FO::HasBadChar(
	char *pVarName)
{
	bool			bHasBadChar		= false;
	const wchar_t	FUNC_NAME[]		= L"CRSupport_FO::HasBadChar";

	try
	{
		// the first character cannot be a number or underscore
		for(Int_32 i=0;i<(Int_32)strlen(RFP_INVALID_R_IDENT_FIRST_CHARS);i++)
		{
			if(pVarName[0]==RFP_INVALID_R_IDENT_FIRST_CHARS[i])
			{
				bHasBadChar = true;
				break;
			}
		}

		if(!bHasBadChar)
		{
			// check for additional invalid chars
			for(Int_32 i=0;i<(Int_32)strlen(RFP_INVALID_R_IDENT_NAME_CHARS);i++)
			{
				if(strchr(pVarName, RFP_INVALID_R_IDENT_NAME_CHARS[i]))
				{
					bHasBadChar = true;
					break;
				}
			}
		}
	}
	catch(...)
	{
		INIT_ERROR_MSG(L"Error occurred while checking for invalid characters in variable names.")
		bHasBadChar = true;
	}

	return bHasBadChar;
}

/************************************************************************
FUNCTION:	CRSupport_FO::ParseRScript()
INPUTS:		sRScript			R script absolute filename or URL
			bIsURL				indicates that sRScript is a URL
			sOutputVar			the output variable specified by the R-FP metric
OUTPUTS:	none
RETURNS:	S_OK
			E_OUTOFMEMORY		allocation error
			E_FAIL				script file name not specified
								script file not found
								open of script file failed
								parse error

This method parses the MSTR variables block, in order to determine the inputs,
outputs and function parameters defined in the R script.  This information will
be used to ensure that input data passed to the R-FP from the AE is passed
properly to the R environment, prior to the execution of the R script.
*************************************************************************/
HRESULT CRSupport_FO::ParseRScript(
	const char *sRScript,
	bool bIsURL,
	const char *sOutputVar)
{
	EnumDefState	eVarDefs			= dsNotFound;
	char			*pToken				= NULL;
	char			*pContext			= NULL;
	bool			bAllScalarInput		= true;
	wchar_t			wcToken[MAXSIZE_CHAR_STR];
	wchar_t			*wcOutputVar		= NULL;
	char			*pScriptBuffer		= NULL;
	HRESULT			hr					= S_OK;
	const wchar_t	FUNC_NAME[]			= L"CRSupport_FO::ParseRScript";
	BLDSTR_DECL()

	try
	{
		// read script from specfied location
		Int_32 lLineCnt=0;
		hr = m_pRSupp->ReadScript(sRScript, bIsURL, m_pREnvironment, m_szEnvName, m_pRScript, &lLineCnt, m_sErrMsg);
		if(hr!=S_OK) throw hr;

		// allocate buffer to process script lines
		size_t lScriptBufLen = RFP_INITIAL_BUFSIZE;
		pScriptBuffer = new char [lScriptBufLen];
		if(!pScriptBuffer)
			throw E_OUTOFMEMORY;

		// parse until end tag encountered or EOF
		bool bOutputsFound = false;
		for(Int_32 i=0;i<lLineCnt;i++)
		{
			// access next script line
			hr = m_pRSupp->ReadScriptLine(m_pRScript, i, &pScriptBuffer, &lScriptBufLen);
			if(hr!=S_OK) throw hr;

			if(strlen(pScriptBuffer)==0)
				// empty line...continue to next line
				continue;

			if(!IsRComment(pScriptBuffer))
				// not a comment line...no need to parse...continue with next line
				continue;

			// comment line...initialize token search and convert initial token to lowercase
			RFP_STRTOK(pToken, pScriptBuffer, WHITESPACE_DELIMS, pContext)
			if(pToken)
			{
				CONVERT_TO_LOWER(pToken)
			}
			else
				// no token following comment starter...no need to parse...continue with next line
				continue;

			// check variables block state
			CVarInfo *pVarInfo = NULL;
			if(eVarDefs==dsParsing)
			{
				// parsing the variables block...identify initial token
				if(strcmp(pToken, MSTR_END)==0)
				{
					// end of variables block
					eVarDefs = dsFound;
					break;
				}
				else if(strcmp(pToken, RVAR_TAG)==0)
				{
					// variable definition...fetch next token, which is expected to be the variable name
					RFP_STRTOK(pToken, NULL, WHITESPACE_DELIMS, pContext)

					// process variable definition
					while(pToken)
					{
						// line is expected to define a variable...check if this is first token (variable name) on this line
						if(!pVarInfo)
						{
							// 'var info' structure doesn't exist yet...must be at first token...check length of variable name
							if(strlen(pToken) > RFP_MAX_TOKEN_LENGTH)
							{
#ifdef WIN32
								INIT_ERROR_MSG2(L"%s: One of the defined RVAR variable names exceeds the maximum allowable length (%d).",
									RFP_MAX_TOKEN_LENGTH)
#else
								INIT_ERROR_MSG(L"One of the defined RVAR variable names exceeds the maximum allowable length (250).")
								//bstrBuffer = L"One of the defined RVAR variable names exceeds the maximum allowable length (250).";
								//INIT_ERROR_MSG(bstrBuffer.m_str)
#endif
								throw E_FAIL;
							}

							// convert token to wchar (for logging)
							RFP_MBSTOWCS(wcToken, MAXSIZE_CHAR_STR, pToken)

							// check for duplicates
							if(IsDupVar(pToken))
							{
#ifdef WIN32
								INIT_ERROR_MSG2(L"%s: The variable name '%s' is defined multiple times.", wcToken)
#else
								wcscpy(m_sErrMsg, FUNC_NAME);
								wcscat(m_sErrMsg, L": The variable name '");
								wcsncat(m_sErrMsg, wcToken,100);
								wcscat(m_sErrMsg, L"' is defined multiple times.");
								//bstrBuffer = L"The variable name '";
								//bstrBuffer.Append(wcToken);
								//bstrBuffer.Append(L"' is defined multiple times.");
								//INIT_ERROR_MSG(bstrBuffer.m_str)
#endif
								throw E_FAIL;
							}

							// check for R reserved names
							if(IsReservedForR(pToken))
							{
#ifdef WIN32
								INIT_ERROR_MSG2(L"%s: The name '%s' is reserved and cannot be used as a variable name.", wcToken)
#else
								wcscpy(m_sErrMsg, FUNC_NAME);
								wcscat(m_sErrMsg, L": The name '");
								wcsncat(m_sErrMsg, wcToken,100);
								wcscat(m_sErrMsg, L"' is reserved and cannot be used as a variable name.");
								//bstrBuffer = L"The name '";
								//bstrBuffer.Append(wcToken);
								//bstrBuffer.Append(L"' is reserved and cannot be used as a variable name.");
								//INIT_ERROR_MSG(bstrBuffer.m_str)
#endif
								throw E_FAIL;
							}

							// check for invalid characters
							if(HasBadChar(pToken))
							{
#ifdef WIN32
								INIT_ERROR_MSG2(L"%s: The variable name '%s' contains an invalid character.", wcToken)
#else
								wcscpy(m_sErrMsg, FUNC_NAME);
								wcscat(m_sErrMsg, L": The variable name '");
								wcsncat(m_sErrMsg, wcToken,100);
								wcscat(m_sErrMsg, L"' contains an invalid character.");
								//bstrBuffer = L"The variable name '";
								//bstrBuffer.Append(wcToken);
								//bstrBuffer.Append(L"' contains an invalid character.");
								//INIT_ERROR_MSG(bstrBuffer.m_str)
#endif
								throw E_FAIL;
							}

							// check for MSTR-reserved variable names (those beginning with "mstr.")
							if(strncmp(pToken, RFP_ERRMSG, 5)==0)
							{
#ifdef WIN32
								INIT_ERROR_MSG2(L"%s: The variable name '%s' begins with 'mstr.', which is reserved for internal use only.", wcToken)
#else
								wcscpy(m_sErrMsg, FUNC_NAME);
								wcscat(m_sErrMsg, L": The variable name '");
								wcsncat(m_sErrMsg, wcToken,100);
								wcscat(m_sErrMsg, L"' begins with 'mstr.', which is reserved for internal use only.");
								//bstrBuffer = L"The variable name '";
								//bstrBuffer.Append(wcToken);
								//bstrBuffer.Append(L"' begins with 'mstr.', which is reserved for internal use only.");
								//INIT_ERROR_MSG(bstrBuffer.m_str)
#endif
								throw E_FAIL;
							}

							// allocate structure and save copy of variable name
							pVarInfo = new CVarInfo;
							if(!pVarInfo)
								throw E_OUTOFMEMORY;
							hr = pVarInfo->PutVarName(pToken);
							CHECK_HR(L"Failure while storing variable name.")
						}
						else
						{
							// name has been parsed...check token length
							if(strlen(pToken) > RFP_MAX_TOKEN_LENGTH)
							{
#ifdef WIN32
								INIT_ERROR_MSG2(L"%s: One of the defined RVAR options exceeds the maximum allowable length (%d).",
									RFP_MAX_TOKEN_LENGTH)
#else
								INIT_ERROR_MSG(L"One of the defined RVAR options exceeds the maximum allowable length (250).")
								//bstrBuffer = L"One of the defined RVAR options exceeds the maximum allowable length (250).";
								//INIT_ERROR_MSG(bstrBuffer.m_str)
#endif
								throw E_FAIL;
							}

							// convert token to wchar (for logging)
							RFP_MBSTOWCS(wcToken, MAXSIZE_CHAR_STR, pToken)

							// check for comment following variable definition
							if(pToken[0]==START_CMT)
								// found comment...no need to parse remainder of current line
								break;

							// convert token to lower case
							CONVERT_TO_LOWER(pToken)

							// check for valid variable options
							if(strlen(pToken)==2)
							{
								// check for two-char tokens (-s, -o, etc.)
								if(strncmp(pToken, VAR_PARAMTYPE_SCALAR, 2)==0)
									pVarInfo->PutParamType(RfpParameterScalar);
								else if(strncmp(pToken, VAR_PARAMTYPE_VECTOR, 2)==0)
									pVarInfo->PutParamType(RfpParameterVector);
								else if(strncmp(pToken, VAR_IOTYPE_OUTPUT, 2)==0)
									pVarInfo->SetRVarType(rvtOutput);
								else if(strncmp(pToken, VAR_IOTYPE_INPUT, 2)==0)
									pVarInfo->SetRVarType(rvtInput);
								else if(strncmp(pToken, VAR_PARAMTYPE_PARAMETER, 2)==0)
									pVarInfo->SetRVarType(rvtParameter);
								else if(strncmp(pToken, VAR_REPEAT_INPUT, 2)==0)
									pVarInfo->SetRepeatFlag(true);
								else if(strncmp(pToken, VAR_DISABLED_VAR, 2)==0)
								{
									pVarInfo->SetDisabledFlag(true);
									break;  // no need to parse remaining tokens
								}
								else
								{
#ifdef WIN32
									INIT_ERROR_MSG2(L"%s: '%s' is an unsupported variable option.", wcToken)
#else
									wcscpy(m_sErrMsg, FUNC_NAME);
									wcscat(m_sErrMsg, L": '");
									wcsncat(m_sErrMsg, wcToken,100);
									wcscat(m_sErrMsg, L"' is an unsupported variable option.");
									//bstrBuffer = L"'";
									//bstrBuffer.Append(wcToken);
									//bstrBuffer.Append(L"' is an unsupported variable option.");
									//INIT_ERROR_MSG(bstrBuffer.m_str)
#endif
									throw E_FAIL;
								}
							}
							else if(strlen(pToken)==4)
							{
								// check for four-char tokens (-num & -str)
								if(strncmp(pToken, VAR_DATATYPE_NUM, 4)==0)
									pVarInfo->PutDataType(RfpDataTypeDouble);
								else if(strncmp(pToken, VAR_DATATYPE_STR, 4)==0)
									pVarInfo->PutDataType(RfpDataTypeVarChar);
								else
								{
#ifdef WIN32
									INIT_ERROR_MSG2(L"%s: '%s' is an unsupported variable option.", wcToken)
#else
									wcscpy(m_sErrMsg, FUNC_NAME);
									wcscat(m_sErrMsg, L": '");
									wcsncat(m_sErrMsg, wcToken,100);
									wcscat(m_sErrMsg, L"' is an unsupported variable option.");
									//bstrBuffer = L"'";
									//bstrBuffer.Append(wcToken);
									//bstrBuffer.Append(L"' is an unsupported variable option.");
									//INIT_ERROR_MSG(bstrBuffer.m_str)
#endif
									throw E_FAIL;
								}
							}
							else
							{
								// check for long versions of tokens (scalar, numeric, etc.)
								if(strcmp(pToken, VAR_DATATYPE_NUM)==0)
									pVarInfo->PutDataType(RfpDataTypeDouble);
								else if(strcmp(pToken, VAR_DATATYPE_STR)==0)
									pVarInfo->PutDataType(RfpDataTypeVarChar);
								else if(strcmp(pToken, VAR_PARAMTYPE_SCALAR)==0)
									pVarInfo->PutParamType(RfpParameterScalar);
								else if(strcmp(pToken, VAR_PARAMTYPE_VECTOR)==0)
									pVarInfo->PutParamType(RfpParameterVector);
								else if(strcmp(pToken, VAR_IOTYPE_OUTPUT)==0)
									pVarInfo->SetRVarType(rvtOutput);
								else if(strcmp(pToken, VAR_IOTYPE_INPUT)==0)
									pVarInfo->SetRVarType(rvtInput);
								else if(strcmp(pToken, VAR_PARAMTYPE_PARAMETER)==0)
									pVarInfo->SetRVarType(rvtParameter);
								else if(strcmp(pToken, VAR_REPEAT_INPUT)==0)
									pVarInfo->SetRepeatFlag(true);
								else if(strcmp(pToken, VAR_DISABLED_VAR)==0)
								{
									pVarInfo->SetDisabledFlag(true);
									break;  // no need to parse remaining tokens
								}
								else
								{
#ifdef WIN32
									INIT_ERROR_MSG2(L"%s: '%s' is an unsupported variable option.", wcToken)
#else
									wcscpy(m_sErrMsg, FUNC_NAME);
									wcscat(m_sErrMsg, L": '");
									wcsncat(m_sErrMsg, wcToken,100);
									wcscat(m_sErrMsg, L"' is an unsupported variable option.");
									//bstrBuffer = L"'";
									//bstrBuffer.Append(wcToken);
									//bstrBuffer.Append(L"' is an unsupported variable option.");
									//INIT_ERROR_MSG(bstrBuffer.m_str)
#endif
									throw E_FAIL;
								}
							}

							if(pVarInfo->IsParam())
							{
								// indicates a function parameter...fetch next token
								RFP_STRTOK(pToken, NULL, WHITESPACE_DELIMS, pContext)

								// check for token
								if(pToken)
								{
									// found token...should be either one of the reserved function parameter names, or a data type
									if(pToken[0]==HYPHEN_CHAR)
									{
										// possible data type...check token length
										if(strlen(pToken) > RFP_MAX_TOKEN_LENGTH)
										{
#ifdef WIN32
											INIT_ERROR_MSG2(L"%s: One of the defined RVAR options exceeds the maximum allowable length (%d).",
												RFP_MAX_TOKEN_LENGTH)
#else
											INIT_ERROR_MSG(L"One of the defined RVAR parameters exceeds the maximum allowable length (250).")
											//bstrBuffer = L"One of the defined RVAR parameters exceeds the maximum allowable length (250).";
											//INIT_ERROR_MSG(bstrBuffer.m_str)
#endif
											throw E_FAIL;
										}

										// convert token to wchar (for logging)
										RFP_MBSTOWCS(wcToken, MAXSIZE_CHAR_STR, pToken)

										// convert token to lower case
										CONVERT_TO_LOWER(pToken)

										// identify data type
										if(strlen(pToken)==4)
										{
											// check for four-char tokens (-num & -str)
											if(strncmp(pToken, VAR_DATATYPE_NUM, 4)==0)
												pVarInfo->PutDataType(RfpDataTypeDouble);
											else if(strncmp(pToken, VAR_DATATYPE_STR, 4)==0)
												pVarInfo->PutDataType(RfpDataTypeVarChar);
											else
											{
#ifdef WIN32
												INIT_ERROR_MSG2(L"%s: '%s' is an unsupported parameter data type.", wcToken)
#else
												wcscpy(m_sErrMsg, FUNC_NAME);
												wcscat(m_sErrMsg, L": '");
												wcsncat(m_sErrMsg, wcToken,100);
												wcscat(m_sErrMsg, L"' is an unsupported parameter data type.");
												//bstrBuffer = L"'";
												//bstrBuffer.Append(wcToken);
												//bstrBuffer.Append(L"' is an unsupported parameter data type.");
												//INIT_ERROR_MSG(bstrBuffer.m_str)
#endif
												throw E_FAIL;
											}
										}
										else if(strlen(pToken)==5)
										{
											// check for five-char tokens (-bool)
											if(strncmp(pToken, VAR_DATATYPE_BOOL, 5)==0)
												pVarInfo->PutDataType(RfpDataTypeBool);
											else
											{
#ifdef WIN32
												INIT_ERROR_MSG2(L"%s: '%s' is an unsupported parameter data type.", wcToken)
#else
												wcscpy(m_sErrMsg, FUNC_NAME);
												wcscat(m_sErrMsg, L": '");
												wcsncat(m_sErrMsg, wcToken,100);
												wcscat(m_sErrMsg, L"' is an unsupported parameter data type.");
												//bstrBuffer = L"'";
												//bstrBuffer.Append(wcToken);
												//bstrBuffer.Append(L"' is an unsupported parameter data type.");
												//INIT_ERROR_MSG(bstrBuffer.m_str)
#endif
												throw E_FAIL;
											}
										}
										else
										{
											// check for long versions of tokens (-numeric, -string, -boolean)
											if(strcmp(pToken, VAR_DATATYPE_NUM)==0)
												pVarInfo->PutDataType(RfpDataTypeDouble);
											else if(strcmp(pToken, VAR_DATATYPE_STR)==0)
												pVarInfo->PutDataType(RfpDataTypeVarChar);
											else if(strcmp(pToken, VAR_DATATYPE_BOOL)==0)
												pVarInfo->PutDataType(RfpDataTypeBool);
											else
											{
#ifdef WIN32
												INIT_ERROR_MSG2(L"%s: '%s' is an unsupported parameter data type.", wcToken)
#else
												wcscpy(m_sErrMsg, FUNC_NAME);
												wcscat(m_sErrMsg, L": '");
												wcsncat(m_sErrMsg, wcToken,100);
												wcscat(m_sErrMsg, L"' is an unsupported parameter data type.");
												//bstrBuffer = L"'";
												//bstrBuffer.Append(wcToken);
												//bstrBuffer.Append(L"' is an unsupported parameter data type.");
												//INIT_ERROR_MSG(bstrBuffer.m_str)
#endif
												throw E_FAIL;
											}
										}
									}
									else
									{
										// must be a reserved function parameter name...identify it
										if(strlen(pToken) > RFP_MAX_TOKEN_LENGTH)
										{
#ifdef WIN32
											INIT_ERROR_MSG2(L"%s: One of the defined RVAR parameter names exceeds the maximum allowable length (%d).",
												RFP_MAX_TOKEN_LENGTH)
#else
											INIT_ERROR_MSG(L"One of the defined RVAR parameters exceeds the maximum allowable length (250).")
											//bstrBuffer = L"One of the defined RVAR parameters exceeds the maximum allowable length (250).";
											//INIT_ERROR_MSG(bstrBuffer.m_str)
#endif
											throw E_FAIL;
										}

										// convert token to wchar (for logging)
										RFP_MBSTOWCS(wcToken, MAXSIZE_CHAR_STR, pToken)

										// compare to list of R-FP parameter names
										for(Int_32 i=0;i<RFP_USER_FUNCPARAM_CNT;i++)
										{
											if(strcmp(pToken, RFP_FUNCPARAM_NAME_ARRAY[i])==0)
											{
												pVarInfo->PutParamIndex(i + 1);
												break;
											}
										}

										if(pVarInfo->GetParamIndex() < 0)
										{
#ifdef WIN32
											INIT_ERROR_MSG2(L"%s: '%s' is not a valid function parameter name.", wcToken)
#else
											wcscpy(m_sErrMsg, FUNC_NAME);
											wcscat(m_sErrMsg, L": '");
											wcsncat(m_sErrMsg, wcToken,100);
											wcscat(m_sErrMsg, L"' is not a valid function parameter name.");
											//bstrBuffer = L"'";
											//bstrBuffer.Append(wcToken);
											//bstrBuffer.Append(L"' is not a valid function parameter name.");
											//INIT_ERROR_MSG(bstrBuffer.m_str)
#endif
											throw E_FAIL;
										}

										// check for duplicates
										if(IsDupParam_pd(pVarInfo->GetParamIndex()))
										{
#ifdef WIN32
											INIT_ERROR_MSG2(L"%s: The function parameter name '%s' is defined multiple times.", wcToken)
#else
											wcscpy(m_sErrMsg, FUNC_NAME);
											wcscat(m_sErrMsg, L": The function parameter name '");
											wcsncat(m_sErrMsg, wcToken,100);
											wcscat(m_sErrMsg, L"' is defined multiple times.");
											//bstrBuffer = L"The function parameter name '";
											//bstrBuffer.Append(wcToken);
											//bstrBuffer.Append(L"' is defined multiple times.");
											//INIT_ERROR_MSG(bstrBuffer.m_str)
#endif
											throw E_FAIL;
										}
									}
								}
								//else...no token after -parameter option...rvar can be set to any data type (will default to numeric 0)
							}
						}

						// fetch next token
						RFP_STRTOK(pToken, NULL, WHITESPACE_DELIMS, pContext)
					} // while(pToken)
				}
				//else...not the end tag or an rvar...skip line

				// finished parsing line...check if new variable was parsed
				if(pVarInfo)
				{
					// process variable
					if(pVarInfo->IsDisabled())
						// variable is disabled...release it
						delete pVarInfo;
					else if(pVarInfo->IsParam())
					{
						if(pVarInfo->IsPreDefined())
							m_vParamInfo_pd.push_back(pVarInfo);
						else
							m_vParamInfo_ud.push_back(pVarInfo);
					}
					else if(pVarInfo->IsInput())
					{
						// input...add to input list
						m_vInputInfo.push_back(pVarInfo);

						// check input parameter type
						if(pVarInfo->GetParamType()==RfpParameterVector)
							// not all scalar
							bAllScalarInput = false;

						// check for repeat input...count should be incremented if this is the first input var specifying
						//	the -r option, or if the count is greater than 0 (indicates that a previous repeat input was
						//	parsed, which means that all remaining inputs will be repeated)
						if((pVarInfo->IsRepeat()) || (m_lRepeatCnt > 0))
						{
							m_lRepeatCnt++;

							// set repeat flag for all inputs after first repeat encountered (even if -r not specified)
							pVarInfo->SetRepeatFlag(true);
						}
					}
					else
					{
						// output...set flag
						bOutputsFound = true;
						
						// check data type
						if(pVarInfo->GetDataType()==RfpDataTypeUnknown)
						{
							// data type must be specified
							INIT_ERROR_MSG(L"Must specify data type (-num or -str) for all output (-o) variables.")
							throw E_FAIL;
						}

						if(!sOutputVar)
						{
							// output variable not specified...use first output variable defined in R script
							if(m_vOutputInfo.size()==0)
								// no outputs yet...add current output to list
								m_vOutputInfo.push_back(pVarInfo);
							else
								// current output will not be used...release it
								delete pVarInfo;
						}
						else
						{
							// check if current output is the one specified by the user
							if(strcmp(sOutputVar, pVarInfo->GetVarName())==0)
								// current output is the specified output...add it to the list
								m_vOutputInfo.push_back(pVarInfo);
							else
								// current output will not be used...release it
								delete pVarInfo;
						}
					}
				}

				// check if finished
				if(eVarDefs==dsFound)
					break;
			}
			else if(eVarDefs==dsNotFound)
			{
				// haven't found the variables block start tag yet...check for start tag
				if(pToken && strcmp(pToken, MSTR_BEGIN)==0)
					eVarDefs = dsParsing;
				//else...not the start tag...skip line
			}
			//else...not parsing variable block...skip line

			// check if done
			if(eVarDefs==dsFound)
				// variables block parsed...exit loop
				break;
		}  // for(lineCnt)

		if(eVarDefs==dsFound)
		{
			// validate variable counts
			if(m_vInputInfo.size()==0)
			{
				INIT_ERROR_MSG(L"At least one input is required.")
				throw E_FAIL;
			}
			if(m_vOutputInfo.size()==0)
			{
				if(bOutputsFound)
				{
					// convert output var name to wchar (for logging)
					size_t uiBufLen = strlen(sOutputVar) + 1;
					wcOutputVar = new wchar_t[uiBufLen];
					if(!wcOutputVar)
						throw E_OUTOFMEMORY;
					RFP_MBSTOWCS(wcOutputVar, uiBufLen, sOutputVar)

#ifdef WIN32
					INIT_ERROR_MSG2(L"%s: The output '%s' specified in the _OutputVar parameter is not defined.", wcOutputVar);
#else
					wcscpy(m_sErrMsg, FUNC_NAME);
					wcscat(m_sErrMsg, L": The output '");
					wcsncat(m_sErrMsg, wcOutputVar,100);
					wcscat(m_sErrMsg, L"' specified in the _OutputVar parameter is not defined.");
					//bstrBuffer = L"The output '";
					//bstrBuffer.Append(wcOutputVar);
					//bstrBuffer.Append(L"' specified in the _OutputVar parameter is not defined.");
					//INIT_ERROR_MSG(bstrBuffer.m_str)
#endif
				}
				else
				{
					INIT_ERROR_MSG(L"At least one output is required.")
				}
				throw E_FAIL;
			}

			// sorted functions require vector for first input
			if(m_bIsSorted && (m_vInputInfo[0]->GetParamType() != RfpParameterVector))
			{
				INIT_ERROR_MSG(L"RScript and RScriptAgg require a vector for the first input.")
				throw E_FAIL;
			}

			// verify specified function is consistent with specified input parameter types
			if(bAllScalarInput)
			{
				if(m_vOutputInfo[0]->GetParamType()==RfpParameterVector)
				{
					INIT_ERROR_MSG(L"Vector output requires at least one vector input.")
					throw E_FAIL;
				}
				else
				{
					// all scalar inputs/output requires Arithmetic function
					if(m_eRFPType != rfpSimple)
					{
						INIT_ERROR_MSG(L"All scalar inputs and output.  Must use RScriptSimple function.")
						throw E_FAIL;
					}
				}
			}
			else
			{
				// at least one vector input
				if(m_vOutputInfo[0]->GetParamType()==RfpParameterVector)
				{
					// vector input/output requires Relative function
					if(m_eRFPType != rfpRelative)
					{
						INIT_ERROR_MSG(L"Vector inputs and output.  Must use RScriptU or RScript function.")
						throw E_FAIL;
					}
				}
				else
				{
					// vector input and scalar output requires Aggregation function
					if(m_eRFPType != rfpAgg)
					{
						INIT_ERROR_MSG(L"Vector inputs and scalar output.  Must use RScriptAggU or RScriptAgg function.")
						throw E_FAIL;
					}
				}
			}

			// construct list of input names
			AddInputNames();
		}
		else if(eVarDefs==dsParsing)
		{
			INIT_ERROR_MSG(L"Missing 'MICROSTRATEGY_END' marker.")
			throw E_FAIL;
		}
		else // variables block not found
		{
			INIT_ERROR_MSG(L"Missing 'MICROSTRATEGY_BEGIN' marker.")
			throw E_FAIL;
		}
	}
	catch(HRESULT catchHR)
	{
		hr = catchHR;
		INIT_HR_MSG(L"Failure during parse of R script.")
	}
	catch(...)
	{
		hr = E_FAIL;
		INIT_HR_MSG(L"Unhandled exception.")
	}

	// cleanup
	delete [] wcOutputVar;
	delete [] pScriptBuffer;

	return hr;
}

extern "C"
{
	IMPEXP void *CreateRIPSupportInstance()
	{
		CRSupport *pRSupp = new CRSupport();
		return (void *)pRSupp;
	}

	IMPEXP void DestroyRIPSupportInstance(
		IRIPSupport *pRSupp)
	{
		delete (CRSupport *)pRSupp;
	}

	IMPEXP void *CreateRIPSupportFOInstance(
		IRIPSupport *pRSupp,
		EnumRFPType eRFPType,
		bool bIsSorted,
		wchar_t *pErrMsg)
	{
		CRSupport_FO *pRSuppFO = new CRSupport_FO(pRSupp, eRFPType, bIsSorted, pErrMsg);
		return (void *)pRSuppFO;
	}

	IMPEXP void DestroyRIPSupportFOInstance(
		IRIPSupport_FO *pRSuppFO)
	{
		delete (CRSupport_FO *)pRSuppFO;
	}
}
//DE43238 provide support for widechar to utf8 conversion

bool RBase::IsIllegalUnicodeValue(unsigned int iUnicodeValue)
{
	return (iUnicodeValue & 0xF800) == 0xD800 // UTF-16 surrogate characters range U+D800..U+DFFF
		|| iUnicodeValue == 0xFFFE // invalid value
		|| iUnicodeValue == 0xFFFF; // invalid value
}

/**
* This function converts a single wide character to a Unicode scalar value.
*/
unsigned int RBase::WideCharToUnicodeValue(wchar_t iWideChar, wchar_t *pErrMsg, bool ibUseSubstitutionCharacter)
{
	const wchar_t	FUNC_NAME[] = L"RBase::WideCharToUnicodeValue";
	// here we assume that wide character encoding is UCS, not UTF
	unsigned int lUnicodeValue = static_cast<unsigned int>(iWideChar);

	//
	// The ( lUnicodeValue > 0xFFFF ) condition is necessary while we support at least one platform
	// where sizeof( wchar_t ) == 2 bytes, and we would like to prohibit Unicode values above 64K
	// so that UTF conversion behavior on all platforms is identical.
	//
	if (IsIllegalUnicodeValue(lUnicodeValue) || (lUnicodeValue > 0xFFFF))
	{
		//
		// ERROR: converted value represents an invalid Unicode value
		//
		if (ibUseSubstitutionCharacter)
		{
			return UNICODE_REPLACEMENT_CHARACTER;
		}
		else
		{
			INIT_ERROR_MSG_REF(L"Invalid UNICODE value");
			throw E_FAIL;
		}
	}

	return lUnicodeValue;
}

char* RBase::WideCharToUTF8(const wchar_t* ipWideChar, unsigned int iLength, wchar_t *pErrMsg, bool ibUseSubstitutionCharacter)
{
	if (ipWideChar == NULL)
		return NULL;
	const wchar_t	FUNC_NAME[] = L"RBase::WideCharToUTF8";
	// calculate the buffer size in bytes
	const size_t lResultLength = ::wcslen(ipWideChar) * MAX_UTF8_SEQUENCE_LENGTH;
	const size_t lMemSize = (lResultLength + 1) * sizeof(wchar_t);



	char* lResult = new char[lResultLength + 1];
	if (!lResult)
		throw E_OUTOFMEMORY;


	// this pointer "walks" through the input buffer
	const wchar_t* lpWideChar = ipWideChar;


	// Note: we use 'unsigned char' for calculations to prevent automatic
	// sign expansion during 'unsigned Int32' <--> 'char' conversions.

	// this pointer "walks" through the output buffer
	unsigned char* lpUTF8 = reinterpret_cast<unsigned char*>(lResult);

	for (unsigned int i = 0; *lpWideChar != L'\0' && (iLength == 0 || i < iLength); i++)
	{
		unsigned int lnUnicodeValue = WideCharToUnicodeValue(*lpWideChar, pErrMsg, ibUseSubstitutionCharacter);

		if (lnUnicodeValue <= 0x0000007F)
		{
			// an ASCII character
			(*lpUTF8++) = lnUnicodeValue;
		}
		else if (lnUnicodeValue <= 0x000007FF)
		{
			// a two-byte sequence
			(*lpUTF8++) = BYTE_11000000 | (lnUnicodeValue >> 6);
			(*lpUTF8++) = BYTE_10000000 | (lnUnicodeValue & BYTE_00111111);
		}
		else if (lnUnicodeValue <= 0x0000FFFF)
		{
			// a three-byte sequence
			(*lpUTF8++) = BYTE_11100000 | (lnUnicodeValue >> 12);
			(*lpUTF8++) = BYTE_10000000 | ((lnUnicodeValue >> 6) & BYTE_00111111);
			(*lpUTF8++) = BYTE_10000000 | (lnUnicodeValue & BYTE_00111111);
		}
		else
		{
			// we check above for anything greater than 0xFFFF
			_ASSERT(false);
		}

		++lpWideChar;
	}

	// null-terminate
	*lpUTF8 = '\0';

	// make sure we didn't overwrite the buffer
	_ASSERT(lpUTF8 <= reinterpret_cast<const unsigned char*>(lResult) + lResultLength);

	return lResult;
}