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
//		File:	RSupport.cpp
//
//		Desc:	Implementation of CRSupport class.
//=======================================================================

//=======================================================================
// includes
//=======================================================================
#include "StdAfx.h"
using namespace std;
#include "RSupport.h"

//=======================================================================
// CRSupport class
// constructor/destructor
//=======================================================================
CRSupport::CRSupport() :
	m_hRLib(NULL),
	m_bRInitialized(false)
{
	// initialize error message buffer to empty string
	m_sErrMsg[0] = NULL_WCHAR;

	// initialize R environment
	HRESULT hr=InitR();
	m_bRInitialized = SUCCEEDED(hr);
}

CRSupport::~CRSupport()
{
	try
	{
		EndR();
	}
	catch(...)
	{
	}
}

//=======================================================================
// CRSupport methods
//=======================================================================
STDMETHODIMP CRSupport::GetRInstallPath()
{
	CComBSTR		bstrKeyR						= L"SOFTWARE\\R-core\\R";
	CComBSTR		bstrValueInstallPath			= L"InstallPath";
	HKEY			hKey;
	DWORD			retCode;
	DWORD			dwValType;
	DWORD			dwSize							= KEY_BUFFER_SIZE;
	wchar_t			sKeyValue[KEY_BUFFER_SIZE];
	HRESULT			hr								= S_OK;
	const wchar_t	FUNC_NAME[]						= L"CRSupport::GetRInstallPath";

	try
	{
		// open R installation key
		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, bstrKeyR, 0, KEY_READ, &hKey))
		{
			// key not found
			INIT_ERROR_MSG2(L"%s: Registry key '%s' not found.  R environment cannot be initialized.", bstrKeyR)
			throw E_FAIL;
		}

		// fetch the InstallPath value
		if(!(retCode=RegQueryValueEx(hKey, bstrValueInstallPath, 0, &dwValType, (BYTE *)&sKeyValue,
			&dwSize)))
		{
			// found it...validate its type
			if(dwValType==REG_SZ)
			{
				// expected type...save R install path
				m_sRInstallPath = sKeyValue;
				if(!m_sRInstallPath)
				{
					INIT_ERROR_MSG2(L"%s: Allocation error when processing registry key value '%s'.  R environment cannot be initialized.",
						bstrValueInstallPath)
					throw E_OUTOFMEMORY;
				}
			}
			else
			{
				// unexpected type
				INIT_ERROR_MSG2(L"%s: Registry key value '%s' not of type REG_SZ.  R environment cannot be initialized.",
					bstrValueInstallPath)
				throw E_FAIL;
			}
		}
		else
		{
			// key value not found
			INIT_ERROR_MSG2(L"%s: Registry key value '%s' not found in key 'HKLM\\%s'.  R environment cannot be initialized.",
				bstrValueInstallPath, bstrKeyR)
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

STDMETHODIMP CRSupport::InitR()
{
	char			*localArgs[]	= {"R", "--silent"};
	CComBSTR		sDllPath;
	DWORD			dwError			= 0;
	HRESULT			hr				= S_OK;
	const wchar_t	FUNC_NAME[]		= L"CRSupport::InitR";

	if(m_hRLib==NULL)
	{
		// get a handle to the DLL module (search <R install root>\bin\i386 first)
		hr = GetRInstallPath();
		if FAILED(hr) return hr;
		LoadRLibrary(L"\\bin\\i386", L"R.dll", true);

		if(!m_hRLib)
		{
			// DLL not found...try <R install path>\bin
			LoadRLibrary(L"\\bin", L"R.dll", true);
			if(m_hRLib)
				// R.dll loaded successfully...load Rlapack.dll
				LoadRLibrary(L"\\bin", L"Rlapack.dll", false);
		}
		else
			// R.dll loaded successfully...load Rlapack.dll
			LoadRLibrary(L"\\bin\\i386", L"Rlapack.dll", false);
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
		m_Rf_protect_ptr = (pRf_protect)GetProcAddress(m_hRLib, "Rf_protect");
		m_lang2_ptr = (pRf_lang2)GetProcAddress(m_hRLib, "Rf_lang2");
		m_install_ptr = (pRf_install)GetProcAddress(m_hRLib, "Rf_install");
		m_mkChar_ptr = (pRf_mkChar)GetProcAddress(m_hRLib, "Rf_mkChar");
		m_mkString_ptr = (pRf_mkString)GetProcAddress(m_hRLib, "Rf_mkString");
		m_R_tryEval_ptr = (pR_tryEval)GetProcAddress(m_hRLib, "R_tryEval");
		m_Rf_unprotect_ptr = (pRf_unprotect)GetProcAddress(m_hRLib, "Rf_unprotect");
		m_Rf_PrintValue_ptr = (pRf_PrintValue)GetProcAddress(m_hRLib, "Rf_PrintValue");
		m_Rf_endEmbeddedR_ptr = (pRf_endEmbeddedR)GetProcAddress(m_hRLib, "Rf_endEmbeddedR");
		m_Rf_allocVector_ptr = (pRf_allocVector)GetProcAddress(m_hRLib, "Rf_allocVector");
		m_Rf_allocMatrix_ptr = (pRf_allocMatrix)GetProcAddress(m_hRLib, "Rf_allocMatrix");
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

		if(m_Rf_initEmbeddedR_ptr && m_Rf_protect_ptr && m_lang2_ptr && m_install_ptr && m_mkChar_ptr && m_mkString_ptr &&
			m_R_tryEval_ptr && m_Rf_unprotect_ptr && m_Rf_PrintValue_ptr && m_Rf_endEmbeddedR_ptr && m_Rf_allocVector_ptr &&
			m_Rf_allocMatrix_ptr && m_REAL_ptr && m_INTEGER_ptr && m_LOGICAL_ptr && m_defineVar_ptr && m_asReal_ptr &&
			m_ScalarInteger_ptr && m_length_ptr && m_TYPEOF_ptr && m_findVar_ptr && m_eval_ptr && m_R_GlobalEnv_ptr &&
			m_R_UnboundValue_ptr && m_SET_STRING_ELT_ptr && m_STRING_ELT_ptr && m_translateChar_ptr && m_R_finite_ptr &&
			m_Rf_nrows_ptr && m_R_NaReal_ptr && m_R_NaInt_ptr && m_R_NaLogical_ptr && m_R_NaString_ptr && m_R_Interactive_ptr)
		{
			Int32 lInitR = m_Rf_initEmbeddedR_ptr(sizeof(localArgs) / sizeof(localArgs[0]), localArgs);
			if(!lInitR)
			{
				// error
				INIT_ERROR_MSG(L"Initialization of R environment failed.")
				return E_FAIL;
			}

			// set session to 'not interactive'
			*m_R_Interactive_ptr = FALSE;

			// success
			return S_OK;
		}
	}

	// error
	dwError = GetLastError();
	INIT_ERROR_MSG2(L"%s: Load of R.DLL failed (error code=%d).", dwError)
	return E_FAIL;
}

void CRSupport::EndR()
{
	if(m_bRInitialized)
		m_Rf_endEmbeddedR_ptr(0);

	if(m_hRLib)
	{
		// free the DLL module
        FreeLibrary(m_hRLib);
		m_hRLib=NULL;
    }
}

// handles numeric, scalar/vector, non-repeated inputs for SIM, GEN & AGG FPs
void CRSupport::SetRVar(
	const char *pName,
	Int_32 nSize,
	double *pData,
	DSSData_Flags *pFlag)
{
	SEXP	pRVector;
	double	*pRValues;
	bool	bValid;

	// allocate vector of REAL in R
	m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(REALSXP, nSize));
	pRValues = m_REAL_ptr(pRVector);

	// initialize vector with input data
	for(Int_32 i=0;i<nSize;i++)
	{
		// check for valid input
		bValid = pFlag ? (pFlag[i]==DssDataOk) : true;
		if(bValid)
			// initialize value with input data
			*(pRValues+i) = pData[i];
		else // missing...set to NA
			*(pRValues+i) = *m_R_NaReal_ptr;
	}

	// define variable
	m_defineVar_ptr(m_install_ptr(pName), pRVector, *m_R_GlobalEnv_ptr);
	m_Rf_unprotect_ptr(1);
}

// handles numeric, scalar, repeated inputs for SIM FPs
void CRSupport::SetRVar_rp(
	const char *pName,
	Int_32 nSize,
	Int_32 lPIndex,
	Int_32 lRepCnt,
	double *pData,
	DSSData_Flags *pFlag)
{
	SEXP	pRVector;
	double	*pRValues;
	bool	bValid;
	Int_32	lSkipIndex;

	// allocate vector of REAL in R
	m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(REALSXP, nSize / lRepCnt));
	pRValues = m_REAL_ptr(pRVector);

	// initialize vector with input data
	lSkipIndex = lPIndex - 1;
	Int_32 j = 0;
	for(Int_32 i=0;i<nSize;i++)
	{
		lSkipIndex++;
		if(lSkipIndex % lRepCnt)
			continue;

		// check for valid input
		bValid = pFlag ? (pFlag[i]==DssDataOk) : true;
		if(bValid)
			// initialize value with input data
			*(pRValues+j) = pData[i];
		else // missing...set to NA
			*(pRValues+j) = *m_R_NaReal_ptr;
		j++;
	}

	// define variable
	m_defineVar_ptr(m_install_ptr(pName), pRVector, *m_R_GlobalEnv_ptr);
	m_Rf_unprotect_ptr(1);
}

// handles numeric, scalar/vector, repeated inputs for GEN FPs
STDMETHODIMP CRSupport::SetRVar_mx(
	const char *pName,
	Int_32 lPIndex,
	Int_32 nRows,
	Int_32 nCols,
	double *pData,
	DSSData_Flags *pFlag)
{
	SEXP		pRMatrix;
	double		*pRValues;
	bool		bValid;
	HRESULT		hr			= S_OK;

	try
	{
		if(lPIndex==0)
			// allocate matrix of REAL in R
			m_Rf_protect_ptr(pRMatrix = m_Rf_allocMatrix_ptr(REALSXP, nRows, nCols));
		else
		{
			// find previously allocated matix
			m_Rf_protect_ptr(pRMatrix=m_findVar_ptr(m_install_ptr(pName), *m_R_GlobalEnv_ptr));
			if(pRMatrix==*m_R_UnboundValue_ptr)
				// variable not found in R environment
				throw E_FAIL;
		}
		pRValues = m_REAL_ptr(pRMatrix);

		// initialize specified column of matrix with input data
		for(Int_32 i=0,j=lPIndex*nRows;i<nRows;i++,j++)
		{
			// check for valid input
			bValid = pFlag ? (pFlag[i]==DssDataOk) : true;
			if(bValid)
				// initialize value with input data
				*(pRValues+j) = pData[i];
			else // missing...set to NA
				*(pRValues+j) = *m_R_NaReal_ptr;
		}

		if(lPIndex==0)
			// define variable
			m_defineVar_ptr(m_install_ptr(pName), pRMatrix, *m_R_GlobalEnv_ptr);
		m_Rf_unprotect_ptr(1);
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

// handles variant properties for all FPs, and variant, scalar, non-repeated inputs for NNSIM and NNGEN FPs
STDMETHODIMP CRSupport::SetRVarV(
	const char *pName,
	IOBUFFER_PAIR *pioBuffer,
	VARIANT *pValue,
	DSSData_Flags *pFlag)
{
	SEXP		pRVector;
	bool		bValid;
	HRESULT		hr			= S_OK;

	try
	{
		switch(pValue->vt)
		{
		case VT_R8:
		case VT_R4:
		case VT_I4:
		case VT_I2:
		case VT_UI1:
		case VT_EMPTY:  // used for NULL inputs...flag should be set to DssDataNull (i.e. not DssDataOk)
			{
				// allocate vector of REAL in R
				m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(REALSXP, 1));
				double *pRValue = m_REAL_ptr(pRVector);

				// check for valid input
				bValid = pFlag ? (*pFlag==DssDataOk) : true;
				if(bValid)
				{
					// change to VT_R8
					hr = VariantChangeType(pValue, pValue, 0, VT_R8);
					if(FAILED(hr)) throw hr;

					// initialize value with input data
					*pRValue = pValue->dblVal;
				}
				else // DssDataNull...set to NA
					*pRValue = *m_R_NaReal_ptr;

				// define variable
				m_defineVar_ptr(m_install_ptr(pName), pRVector, *m_R_GlobalEnv_ptr);
				m_Rf_unprotect_ptr(1);

				break;
			}
		case VT_BOOL:
			{
				// allocate vector of logical in R
				m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(LGLSXP, 1));
				int *pRValue = m_LOGICAL_ptr(pRVector);

				// check for valid input
				bValid = pFlag ? (*pFlag==DssDataOk) : true;
				if(bValid)
					// convert boolean to 0 (false) or 1 (true)
					*pRValue = (pValue->boolVal==VARIANT_TRUE) ? 1 : 0;
				else // DssDataNull...set to NA
					*pRValue = *m_R_NaLogical_ptr;

				// define variable
				m_defineVar_ptr(m_install_ptr(pName), pRVector, *m_R_GlobalEnv_ptr);
				m_Rf_unprotect_ptr(1);

				break;
			}
		case VT_BSTR:
			{
				// allocate vector of string in R
				m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(STRSXP, 1));

				// check for valid input
				bValid = pFlag ? (*pFlag==DssDataOk) : true;
				if(bValid)
				{
					// convert variant string to char
					size_t uiSrcLen = SysStringLen(pValue->bstrVal);
					CheckBufLen(uiSrcLen, pioBuffer);
					RFP_WCSTOMBS(pioBuffer->first, RFP_BUFLEN(pioBuffer->second), pValue->bstrVal)

					// initialize value with input data
					m_SET_STRING_ELT_ptr(pRVector, 0, m_mkChar_ptr(pioBuffer->first));
				}
				else // missing...set to NA
					m_SET_STRING_ELT_ptr(pRVector, 0, *m_R_NaString_ptr);

				// define variable
				m_defineVar_ptr(m_install_ptr(pName), pRVector, *m_R_GlobalEnv_ptr);
				m_Rf_unprotect_ptr(1);

				break;
			}
		default:
			{
				// unexpected data type
				throw E_FAIL;
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

// handles variant, vector, non-repeated inputs for NNGEN FPs
STDMETHODIMP CRSupport::SetRVarSA(
	const char *pName,
	Int_32 nSize,
	EnumDSSDataType dataType,
	IOBUFFER_PAIR *pioBuffer,
	VARIANT *pData,
	DSSData_Flags *pFlag)
{
	SEXP		pRVector;
	VARIANT		varElem;
	bool		bValid;
	HRESULT		hr			= S_OK;

	try
	{
		// initialize
		VariantInit(&varElem);

		// check data type
		switch(dataType)
		{
		case DssDataTypeDouble:
			{
				double *pRValues;

				// allocate vector of REAL in R
				m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(REALSXP, nSize));
				pRValues = m_REAL_ptr(pRVector);

				// initialize vector with input data
				for(Int_32 i=0;i<nSize;i++)
				{
					// check for valid input
					bValid = pFlag ? (pFlag[i]==DssDataOk) : true;
					if(bValid)
					{
						// fetch element and convert to double
						SafeArrayGetElement(pData->parray, (LONG *)&i, &varElem);
						hr = VariantChangeType(&varElem, &varElem, 0, VT_R8);
						if(FAILED(hr)) throw hr;

						// initialize value with input data
						*(pRValues+i) = varElem.dblVal;
						VariantClear(&varElem);
					}
					else // missing...set to NA
						*(pRValues+i) = *m_R_NaReal_ptr;
				}

				// define variable
				m_defineVar_ptr(m_install_ptr(pName), pRVector, *m_R_GlobalEnv_ptr);
				m_Rf_unprotect_ptr(1);

				break;
			}
		case DssDataTypeVarChar:
			{
				// allocate vector of string in R
				m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(STRSXP, nSize));

				// initialize vector with input data
				for(Int_32 i=0;i<nSize;i++)
				{
					// check for valid input
					bValid = pFlag ? (pFlag[i]==DssDataOk) : true;
					if(bValid)
					{
						// fetch element and convert to CHSTR
						SafeArrayGetElement(pData->parray, (LONG *)&i, &varElem);
						hr = VariantChangeType(&varElem, &varElem, 0, VT_BSTR);
						if(FAILED(hr)) throw hr;
						size_t uiSrcLen = wcslen(varElem.bstrVal);
						CheckBufLen(uiSrcLen, pioBuffer);
						RFP_WCSTOMBS(pioBuffer->first, RFP_BUFLEN(pioBuffer->second), varElem.bstrVal)

						// initialize value with input data
						m_SET_STRING_ELT_ptr(pRVector, i, m_mkChar_ptr(pioBuffer->first));
						VariantClear(&varElem);
					}
					else // missing...set to NA
						m_SET_STRING_ELT_ptr(pRVector, i, *m_R_NaString_ptr);
				}

				// define variable
				m_defineVar_ptr(m_install_ptr(pName), pRVector, *m_R_GlobalEnv_ptr);
				m_Rf_unprotect_ptr(1);

				break;
			}
		default:
			{
				// unexpected data type
				throw E_FAIL;
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

// handles variant, scalar, repeated inputs for NNSIM FPs
STDMETHODIMP CRSupport::SetRVarSA_rp(
	const char *pName,
	Int_32 nSize,
	Int_32 lPIndex,
	Int_32 lInCnt,
	Int_32 lRepCnt,
	EnumDSSDataType dataType,
	IOBUFFER_PAIR *pioBuffer,
	VARIANT *pData,
	DSSData_Flags *pFlag)
{
	SEXP		pRVector;
	VARIANT		varElem;
	bool		bValid;
	Int_32		lSkipIndex;
	Int_32		lNonRepeatCnt;
	Int_32		lRepeatSize;
	HRESULT		hr				= S_OK;

	try
	{
		// initialize
		VariantInit(&varElem);
		lNonRepeatCnt = lInCnt - lRepCnt;
		lRepeatSize = (nSize - lNonRepeatCnt) / lRepCnt;

		// check data type
		switch(dataType)
		{
		case DssDataTypeDouble:
			{
				double *pRValues;

				// allocate vector of REAL in R
				m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(REALSXP, lRepeatSize));
				pRValues = m_REAL_ptr(pRVector);

				// initialize vector with input data
				lSkipIndex = lPIndex - 1;
				Int_32 j = 0;
				for(Int_32 i=lNonRepeatCnt;i<nSize;i++)
				{
					lSkipIndex++;
					if(lSkipIndex % lRepCnt)
						continue;

					// check for valid input
					bValid = pFlag ? (pFlag[i]==DssDataOk) : true;
					if(bValid)
					{
						// fetch element and convert to double
						SafeArrayGetElement(pData->parray, (LONG *)&i, &varElem);
						hr = VariantChangeType(&varElem, &varElem, 0, VT_R8);
						if(FAILED(hr)) throw hr;

						// initialize value with input data
						*(pRValues+j) = varElem.dblVal;
						VariantClear(&varElem);
					}
					else // missing...set to NA
						*(pRValues+j) = *m_R_NaReal_ptr;
					j++;
				}

				// define variable
				m_defineVar_ptr(m_install_ptr(pName), pRVector, *m_R_GlobalEnv_ptr);
				m_Rf_unprotect_ptr(1);

				break;
			}
		case DssDataTypeVarChar:
			{
				// allocate vector of string in R
				m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(STRSXP, lRepeatSize));

				// initialize vector with input data
				lSkipIndex = lPIndex - 1;
				Int_32 j = 0;
				for(Int_32 i=lNonRepeatCnt;i<nSize;i++)
				{
					lSkipIndex++;
					if(lSkipIndex % lRepCnt)
						continue;

					// check for valid input
					bValid = pFlag ? (pFlag[i]==DssDataOk) : true;
					if(bValid)
					{
						// fetch element and convert to CHSTR
						SafeArrayGetElement(pData->parray, (LONG *)&i, &varElem);
						hr = VariantChangeType(&varElem, &varElem, 0, VT_BSTR);
						if(FAILED(hr)) throw hr;
						size_t uiSrcLen = wcslen(varElem.bstrVal);
						CheckBufLen(uiSrcLen, pioBuffer);
						RFP_WCSTOMBS(pioBuffer->first, RFP_BUFLEN(pioBuffer->second), varElem.bstrVal)

						// initialize value with input data
						m_SET_STRING_ELT_ptr(pRVector, j, m_mkChar_ptr(pioBuffer->first));
						VariantClear(&varElem);
					}
					else // missing...set to NA
						m_SET_STRING_ELT_ptr(pRVector, j, *m_R_NaString_ptr);
					j++;
				}

				// define variable
				m_defineVar_ptr(m_install_ptr(pName), pRVector, *m_R_GlobalEnv_ptr);
				m_Rf_unprotect_ptr(1);

				break;
			}
		default:
			{
				// unexpected data type
				throw E_FAIL;
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

// handles variant, scalar/vector, repeated inputs for NNGEN FPs
STDMETHODIMP CRSupport::SetRVarSA_mx(
	const char *pName,
	Int_32 lPIndex,
	Int_32 nRows,
	Int_32 nCols,
	EnumDSSDataType dataType,
	IOBUFFER_PAIR *pioBuffer,
	VARIANT *pData,
	DSSData_Flags *pFlag,
	bool bSafeArray,
	wchar_t *pErrMsg)
{
	SEXP			pRMatrix;
	VARIANT			varElem;
	bool			bValid;
	HRESULT			hr				= S_OK;
	const wchar_t	FUNC_NAME[]		= L"CRSupport::SetRVarSA_mx";

	try
	{
		// initialize
		VariantInit(&varElem);

		// check data type
		switch(dataType)
		{
		case DssDataTypeDouble:
			{
				double *pRValues;

				if(lPIndex==0)
					// allocate matrix of REAL in R
					m_Rf_protect_ptr(pRMatrix = m_Rf_allocMatrix_ptr(REALSXP, nRows, nCols));
				else
				{
					// find previously allocated matix
					m_Rf_protect_ptr(pRMatrix=m_findVar_ptr(m_install_ptr(pName), *m_R_GlobalEnv_ptr));
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
						INIT_ERROR_MSG_REF(
							L"%s: When mixing scalar/vector inputs with repeated input, first input must be vector.")
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
					bValid = pFlag ? (pFlag[i]==DssDataOk) : true;
					if(bValid)
					{
						if(bSafeArray)
						{
							// fetch element and convert to double
							SafeArrayGetElement(pData->parray, (LONG *)&i, &varElem);
							hr = VariantChangeType(&varElem, &varElem, 0, VT_R8);
							if(FAILED(hr)) throw hr;
						}
						else
						{
							// scalar input...coerce variant
							hr = VariantChangeType(&varElem, pData, 0, VT_R8);
							if(FAILED(hr)) throw hr;
						}

						// initialize value with input data
						*(pRValues+j) = varElem.dblVal;
						VariantClear(&varElem);
					}
					else // missing...set to NA
						*(pRValues+j) = *m_R_NaReal_ptr;
				}

				break;
			}
		case DssDataTypeVarChar:
			{
				if(lPIndex==0)
					// allocate matrix of strings in R
					m_Rf_protect_ptr(pRMatrix = m_Rf_allocMatrix_ptr(STRSXP, nRows, nCols));
				else
				{
					// find previously allocated matrix
					m_Rf_protect_ptr(pRMatrix=m_findVar_ptr(m_install_ptr(pName), *m_R_GlobalEnv_ptr));
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
						INIT_ERROR_MSG_REF(
							L"%s: When mixing scalar/vector inputs with repeated vector input, first input must be vector.")
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
					bValid = pFlag ? (pFlag[i]==DssDataOk) : true;
					if(bValid)
					{
						if(bSafeArray)
						{
							// fetch element and convert to CHSTR
							SafeArrayGetElement(pData->parray, (LONG *)&i, &varElem);
							hr = VariantChangeType(&varElem, &varElem, 0, VT_BSTR);
							if(FAILED(hr)) throw hr;
							size_t uiSrcLen = wcslen(varElem.bstrVal);
							CheckBufLen(uiSrcLen, pioBuffer);
							RFP_WCSTOMBS(pioBuffer->first, RFP_BUFLEN(pioBuffer->second), varElem.bstrVal)
						}
						else
						{
							// scalar input...convert to CHSTR
							hr = VariantChangeType(&varElem, pData, 0, VT_BSTR);
							if(FAILED(hr)) throw hr;
							size_t uiSrcLen = wcslen(varElem.bstrVal);
							CheckBufLen(uiSrcLen, pioBuffer);
							RFP_WCSTOMBS(pioBuffer->first, RFP_BUFLEN(pioBuffer->second), varElem.bstrVal)
						}

						// initialize value with input data
						m_SET_STRING_ELT_ptr(pRMatrix, j, m_mkChar_ptr(pioBuffer->first));
						VariantClear(&varElem);
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
			m_defineVar_ptr(m_install_ptr(pName), pRMatrix, *m_R_GlobalEnv_ptr);
		m_Rf_unprotect_ptr(1);
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

// handles string, scalar, non-repeated input
void CRSupport::SetRVarStr(
	const char *pName,
	char *pData)
{
	SEXP	pRVector;

	// allocate vector in R
	m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(STRSXP, 1));

	// initialize vector with input data
	m_SET_STRING_ELT_ptr(pRVector, 0, m_mkChar_ptr(pData));

	// define variable
	m_defineVar_ptr(m_install_ptr(pName), pRVector, *m_R_GlobalEnv_ptr);
	m_Rf_unprotect_ptr(1);
}

// handles _InputNames parameter
void CRSupport::SetRVar_InputNames(
	std::vector<char *> &vInputNames)
{
	SEXP	pRVector;

	// allocate vector in R
	m_Rf_protect_ptr(pRVector = m_Rf_allocVector_ptr(STRSXP, vInputNames.size()));

	// initialize vector with input data
	for(Int_32 i=0;i<(Int_32)vInputNames.size();i++)
	{
		m_SET_STRING_ELT_ptr(pRVector, i, m_mkChar_ptr(vInputNames[i]));
	}

	// define variable
	m_defineVar_ptr(m_install_ptr(RFP_INPUTNAMES), pRVector, *m_R_GlobalEnv_ptr);
	m_Rf_unprotect_ptr(1);
}

Int_32 CRSupport::SubmitR(
	const char *script)
{
	SEXP	e;
	int		lErrorCode;

	m_Rf_protect_ptr(e=m_lang2_ptr(m_install_ptr("source"), m_mkString_ptr(script)));
	m_R_tryEval_ptr(e, *m_R_GlobalEnv_ptr, &lErrorCode);
	m_Rf_unprotect_ptr(1);

	return lErrorCode;
}

void CRSupport::GetRVar(
	const char *pName,
	Int_32 nSize,
	Int_32 *pRSize,
	double *pResult,
	DSSData_Flags *pFlag)
{
	SEXP	s;
	Int_32	lSize	= 0;

	m_Rf_protect_ptr(s=m_findVar_ptr(m_install_ptr(pName), *m_R_GlobalEnv_ptr));
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
		switch(m_TYPEOF_ptr(s))
		{
		case REALSXP:
			{
				double dblTemp;
				double *pREAL = m_REAL_ptr(s);
				for(Int_32 i=0;i<lSize;i++)
				{
					pFlag[i] = DssDataInvalid;
					dblTemp = pREAL[i];

					// TODO: add your post-processing code here

					if(m_R_finite_ptr(dblTemp))
					{
						pResult[i] = dblTemp;
						pFlag[i] = DssDataOk;
					}
					//else...result is invalid...flag already set to invalid
				}
				break;
			}
		case INTSXP:
			{
				int iTemp;
				int *pINTEGER = m_INTEGER_ptr(s);
				for(Int_32 i=0;i<lSize;i++)
				{
					pFlag[i] = DssDataInvalid;
					iTemp = pINTEGER[i];

					// TODO: add your post-processing code here

					if(iTemp!=*m_R_NaInt_ptr)
					{
						pResult[i] = (double)iTemp;
						pFlag[i] = DssDataOk;
					}
					//else...result is invalid...flag already set to invalid
				}
				break;
			}
		case LGLSXP:
			{
				int iTemp;
				int *pLOGICAL = m_LOGICAL_ptr(s);
				for(Int_32 i=0;i<lSize;i++)
				{
					pFlag[i] = DssDataInvalid;
					iTemp = pLOGICAL[i];

					// TODO: add your post-processing code here

					if(iTemp!=*m_R_NaLogical_ptr)
					{
						pResult[i] = (double)iTemp;
						pFlag[i] = DssDataOk;
					}
					//else...result is invalid...flag already set to invalid
				}
				break;
			}
		default:
			{
				// unsupported data type
				*pRSize = 0;

				// setting lSize to zero will result in all flags being set to invalid
				lSize = 0;
			}
		}
	}

	// if fewer results returned than expected (i.e. lSize < nSize), set remaining flags to invalid
	for(Int_32 i=lSize;i<nSize;i++)
		pFlag[i] = DssDataInvalid;

	m_Rf_unprotect_ptr(1);
}

STDMETHODIMP CRSupport::GetRVarNN(
	const char *pName,
	Int_32 nSize,
	EnumDSSDataType eExpectedDT,
	IOBUFFER_PAIR *pioBuffer,
	Int_32 *pRSize,
	VARIANT *pResult,
	DSSData_Flags *pFlag,
	wchar_t *pErrMsg)
{
	SEXP			s;
	Int_32			lSize			= 0;
	VARIANT			output;
	wchar_t			wcsVarName[MAXSIZE_CHAR_STR];
	wchar_t			*wcBuffer		= NULL;
	HRESULT			hr				= S_OK;
	const wchar_t	FUNC_NAME[]		= L"CRSupport::GetRVarNN";

	USES_CONVERSION;

	try
	{
		// initialize
		VariantInit(&output);
		RFP_MBSTOWCS(wcsVarName, MAXSIZE_CHAR_STR, pName)

		m_Rf_protect_ptr(s=m_findVar_ptr(m_install_ptr(pName), *m_R_GlobalEnv_ptr));
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
					if(eExpectedDT != DssDataTypeDouble)
					{
						INIT_ERROR_MSG_REF(L"%s: Actual data type REALSXP is not compatible with expected data type %s for output '%s'.",
							RFP_GET_DT_STR(eExpectedDT), wcsVarName)
						throw E_FAIL;
					}

					double dblTemp;
					double *pREAL = m_REAL_ptr(s);

					if(pResult->vt==VT_R8)
					{
						// scalar output...init flag
						*pFlag = DssDataInvalid;

						// verify size
						if(lSize > 0)
						{
							// output available...fetch first result
							dblTemp = pREAL[0];

							// TODO: add your post-processing code here

							// return single result and set flag
							if(m_R_finite_ptr(dblTemp))
							{
								pResult->dblVal = dblTemp;
								*pFlag = DssDataOk;
							}
							//else...result is invalid...flag already set to invalid
						}
						//else...unexpected size...NULL would return size=0...flag already set to invalid
					}
					else if(pResult->vt==VT_BSTR)
						// unexpected type...set flag to invalid
						*pFlag = DssDataInvalid;
					else
					{
						// vector output...copy results to safe array
						for(Int_32 i=0;i<lSize;i++)
						{
							// init flag and fetch result
							pFlag[i] = DssDataInvalid;
							dblTemp = pREAL[i];

							// TODO: add your post-processing code here

							if(m_R_finite_ptr(dblTemp))
							{
								output.vt = VT_R8;
								output.dblVal = dblTemp;

								// save to safe array
								hr=SafeArrayPutElement(pResult->parray, (LONG *)&i, &output);
								VariantClear(&output);
								pFlag[i] = DssDataOk;
							}
							else
								// result is invalid...flag already set to invalid...save empty value to safe array
								hr=SafeArrayPutElement(pResult->parray, (LONG *)&i, &output);
						}
					}

					break;
				}
			case INTSXP:
				{
					// verify output data type
					if(eExpectedDT != DssDataTypeDouble)
					{
						INIT_ERROR_MSG_REF(L"%s: Actual data type INTSXP is not compatible with expected data type %s for output '%s'.",
							RFP_GET_DT_STR(eExpectedDT), wcsVarName)
						throw E_FAIL;
					}

					int iTemp;
					int *pINTEGER = m_INTEGER_ptr(s);

					if(pResult->vt==VT_R8)
					{
						// scalar output...init flag
						*pFlag = DssDataInvalid;

						// verify size
						if(lSize > 0)
						{
							// output available...fetch first result
							iTemp = pINTEGER[0];

							// TODO: add your post-processing code here

							// return single result and set flag
							if(iTemp!=*m_R_NaInt_ptr)
							{
								pResult->dblVal = (double)iTemp;
								*pFlag = DssDataOk;
							}
							//else...result is invalid...flag already set to invalid
						}
						//else...unexpected size...NULL would return size=0...flag already set to invalid
					}
					else if(pResult->vt==VT_BSTR)
						// unexpected type...set flag to invalid
						*pFlag = DssDataInvalid;
					else
					{
						// vector output...copy results to safe array
						for(Int_32 i=0;i<lSize;i++)
						{
							// init flag and fetch result
							pFlag[i] = DssDataInvalid;
							iTemp = pINTEGER[i];

							// TODO: add your post-processing code here

							if(iTemp!=*m_R_NaInt_ptr)
							{
								output.vt = VT_R8;
								output.dblVal = (double)iTemp;

								// save to safe array and set flag
								hr=SafeArrayPutElement(pResult->parray, (LONG *)&i, &output);
								VariantClear(&output);
								pFlag[i] = DssDataOk;
							}
							else
								// result is invalid...flag already set to invalid...save empty value to safe array
								hr=SafeArrayPutElement(pResult->parray, (LONG *)&i, &output);
						}
					}

					break;
				}
			case LGLSXP:
				{
					// verify output data type
					if(eExpectedDT != DssDataTypeDouble)
					{
						INIT_ERROR_MSG_REF(L"%s: Actual data type LGLSXP is not compatible with expected data type %s for output '%s'.",
							RFP_GET_DT_STR(eExpectedDT), wcsVarName)
						throw E_FAIL;
					}

					int iTemp;
					int *pLOGICAL = m_LOGICAL_ptr(s);

					if(pResult->vt==VT_R8)
					{
						// scalar output...init flag
						*pFlag = DssDataInvalid;

						// verify size
						if(lSize > 0)
						{
							// output available...fetch first result
							iTemp = pLOGICAL[0];

							// TODO: add your post-processing code here

							// return single result and set flag
							if(iTemp!=*m_R_NaLogical_ptr)
							{
								pResult->dblVal = (double)iTemp;
								*pFlag = DssDataOk;
							}
							//else...result is invalid...flag already set to invalid
						}
						//else...unexpected size...NULL would return size=0...flag already set to invalid
					}
					else if(pResult->vt==VT_BSTR)
						// unexpected type...set flag to invalid
						*pFlag = DssDataInvalid;
					else
					{
						// vector output...copy results to safe array
						for(Int_32 i=0;i<lSize;i++)
						{
							// init flag and fetch result
							pFlag[i] = DssDataInvalid;
							iTemp = pLOGICAL[i];

							// TODO: add your post-processing code here

							if(iTemp!=*m_R_NaLogical_ptr)
							{
								output.vt = VT_R8;
								output.dblVal = (double)iTemp;

								// save to safe array and set flag
								hr=SafeArrayPutElement(pResult->parray, (LONG *)&i, &output);
								VariantClear(&output);
								pFlag[i] = DssDataOk;
							}
							else
								// result is invalid...flag already set to invalid...save empty value to safe array
								hr=SafeArrayPutElement(pResult->parray, (LONG *)&i, &output);
						}
					}

					break;
				}
			case STRSXP:
				{
					// verify output data type
					if(eExpectedDT != DssDataTypeVarChar)
					{
						INIT_ERROR_MSG_REF(L"%s: Actual data type STRSXP is not compatible with expected data type %s for output '%s'.",
							RFP_GET_DT_STR(eExpectedDT), wcsVarName)
						throw E_FAIL;
					}

					SEXP strElem;

					if(pResult->vt==VT_BSTR)
					{
						// scalar output...init flag
						*pFlag = DssDataInvalid;

						// verify size
						if(lSize > 0)
						{
							// output available...fetch first result
							strElem = m_STRING_ELT_ptr(s, 0);

							// check if NA
							if(strElem != *m_R_NaString_ptr)
							{
								// determine output size
								size_t uiOutLen = m_length_ptr(strElem);
								uiOutLen = (uiOutLen > RFP_MAX_STR_LENGTH) ? RFP_MAX_STR_LENGTH : uiOutLen + 1;

								// allocate destination buffer
								wcBuffer = new wchar_t[uiOutLen];
								if(!wcBuffer)
									throw E_OUTOFMEMORY;

								// copy output from R environment, converting to wide char
								RFP_MBSTOWCS(wcBuffer, uiOutLen, m_translateChar_ptr(strElem))

								// return single string and set flag
								pResult->bstrVal = SysAllocString(wcBuffer);
								if(!pResult->bstrVal)
									throw E_OUTOFMEMORY;
								*pFlag = DssDataOk;
							}
							//else...string is NA...flag already set to invalid
						}
						//else...unexpected size...NULL would return size=0...flag already set to invalid
					}
					else if(pResult->vt==VT_R8)
						// unexpected type...set flag to invalid
						*pFlag = DssDataInvalid;
					else
					{
						// vector output...copy results to safe array
						for(Int_32 i=0;i<lSize;i++)
						{
							// init flag and fetch result
							pFlag[i] = DssDataInvalid;
							strElem = m_STRING_ELT_ptr(s, i);

							// check if NA
							if(strElem != *m_R_NaString_ptr)
							{
								// determine output size
								size_t uiOutLen = m_length_ptr(strElem);
								uiOutLen = (uiOutLen > RFP_MAX_STR_LENGTH) ? RFP_MAX_STR_LENGTH : uiOutLen + 1;

								// allocate destination buffer
								wcBuffer = new wchar_t[uiOutLen];
								if(!wcBuffer)
									throw E_OUTOFMEMORY;

								// copy output from R environment, converting to wide char
								RFP_MBSTOWCS(wcBuffer, uiOutLen, m_translateChar_ptr(strElem))
								output.vt = VT_BSTR;
								output.bstrVal = SysAllocString(wcBuffer);
								if(!output.bstrVal)
									throw E_OUTOFMEMORY;

								// save to safe array and set flag
								hr=SafeArrayPutElement(pResult->parray, (LONG *)&i, &output);
								VariantClear(&output);
								pFlag[i] = DssDataOk;

								// release buffer
								delete [] wcBuffer;
								wcBuffer = NULL;
							}
							else
								// element is NA...flag already set to invalid...save empty value to safe array
								hr=SafeArrayPutElement(pResult->parray, (LONG *)&i, &output);
						}
					}

					break;
				}
			default:
				{
					// unsupported data type
					INIT_ERROR_MSG_REF(L"%s: Actual data type %s (%d) is not compatible with expected data type %s for output '%s'.",
						RFP_GET_RDT_STR(iRType), iRType, RFP_GET_DT_STR(eExpectedDT), wcsVarName)
					throw E_FAIL;
				}
			}
		}

		// if fewer results returned than expected (i.e. lSize < nSize), set remaining flags to invalid
		for(Int_32 i=lSize;i<nSize;i++)
			pFlag[i] = DssDataInvalid;

		m_Rf_unprotect_ptr(1);
	}
	catch(HRESULT catchHR)
	{
		hr = catchHR;
	}
	catch(...)
	{
		hr = E_FAIL;
	}

	// clean up
	delete [] wcBuffer;

	return hr;
}

//=======================================================================
// CVarInfo class
// constructor/destructor
//=======================================================================
CVarInfo::CVarInfo() :
	m_sVarName(NULL),
	m_sFuncParamName(NULL),
	m_bInput(true),
	m_bRepeatInput(false),
	m_bDisabledVar(false),
	m_eParamType(DssParameterVector),
	m_eDataType(DssDataTypeUnknown),
	m_lParamIndex(-1)
{
}

CVarInfo::~CVarInfo()
{
	if(m_sVarName) delete [] m_sVarName;
	if(m_sFuncParamName) delete [] m_sFuncParamName;
}

STDMETHODIMP CVarInfo::CheckDataType(
	EnumDSSDataType eDataType)
{
	switch(eDataType)
	{
	case DssDataTypeDouble:
	case DssDataTypeShort:
	case DssDataTypeFloat:
	case DssDataTypeInteger:
		{
			if(m_eDataType==DssDataTypeVarChar)
				// not double or unknown...incompatible data type
				return E_FAIL;
			else
				// set data type (just in case it is currently set to unknown)
				m_eDataType = DssDataTypeDouble;

			break;
		}
	case DssDataTypeVarChar:
	case DssDataTypeUTF8Char:
		{
			if(m_eDataType==DssDataTypeDouble)
				// not varchar or unknown...incompatible data type
				return E_FAIL;
			else
				// set data type (just in case it is currently set to unknown)
				m_eDataType = DssDataTypeVarChar;

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
	EnumRFPType eRFPType,
	bool bIsSorted,
	wchar_t *pErrMsg) :
		m_eRFPType(eRFPType),
		m_bIsSorted(bIsSorted),
		m_lRepeatCnt(0)
{
	// initialize error message buffer to empty string
	m_sErrMsg = pErrMsg;
}

CRSupport_FO::~CRSupport_FO()
{
	try
	{
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
		if(m_vParamInfo.size() > 0)
		{
			for(Int32 i=0;i<m_vParamInfo.size();i++)
				delete m_vParamInfo[i];
			m_vParamInfo.clear();
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
bool CRSupport_FO::InitOK()
{
	if(m_pRSupp)
		return m_pRSupp->InitOK();
	else
		return false;
}

CVarInfo *CRSupport_FO::GetParam(
	Int_32 lIndex)
{
	CVarInfo		*pVI			= NULL;
	const wchar_t	FUNC_NAME[]		= L"CRSupport_FO::GetParam";

	try
	{
		// search for specified index...if not found, return NULL
		for(Int32 i=0;i<m_vParamInfo.size();i++)
		{
			if(m_vParamInfo[i]->GetParamIndex()==lIndex)
			{
				// found match...set return var
				pVI = m_vParamInfo[i];
				break;
			}
		}
	}
	catch(...)
	{
		INIT_ERROR_MSG(L"Error searching for function parameter.")
	}

	return pVI;
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
INPUTS:		sRScript			R script absolute filename
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
STDMETHODIMP CRSupport_FO::ParseRScript(
	const char *sRScript,
	const char *sOutputVar)
{
	const Int_32	BUFLEN				= 1000;
	char			szBuffer[BUFLEN];
	enumDefState	eVarDefs			= dsNotFound;
	char			*pToken				= NULL;
	char			*pContext			= NULL;
	bool			bAllScalarInput		= true;
	ifstream		ifs;
	wchar_t			wcToken[MAXSIZE_CHAR_STR];
	wchar_t			*wcRScript			= NULL;
	wchar_t			*wcOutputVar		= NULL;
	HRESULT			hr					= S_OK;
	const wchar_t	FUNC_NAME[]			= L"CRSupport_FO::ParseRScript";

	try
	{
		// verify script name provided
		if(strlen(sRScript)==0)
		{
			INIT_ERROR_MSG(L"Missing R script file name.")
			throw E_FAIL;
		}

		// convert script and output var names to wchar (for logging)
		size_t uiBufLen = strlen(sRScript) + 1;
		wcRScript = new wchar_t[uiBufLen];
		if(!wcRScript)
			throw E_OUTOFMEMORY;
		RFP_MBSTOWCS(wcRScript, uiBufLen, sRScript)
		if(sOutputVar)
		{
			uiBufLen = strlen(sOutputVar) + 1;
			wcOutputVar = new wchar_t[uiBufLen];
			if(!wcOutputVar)
				throw E_OUTOFMEMORY;
			RFP_MBSTOWCS(wcOutputVar, uiBufLen, sOutputVar)
		}

		// check if script exists
		struct stat fInfo;
		if(stat(sRScript, &fInfo) != 0)
		{
			// script not found
			INIT_ERROR_MSG2(L"%s: R script file '%s' not found.", wcRScript)
			throw E_FAIL;
		}
		
		// script located...open it
		ifs.open(sRScript);
		if(!ifs.is_open())
		{
			INIT_ERROR_MSG2(L"%s: Error opening R script file '%s'.", wcRScript)
			throw E_FAIL;
		}

		// parse until end tag encountered or EOF
		bool bDone = false;
		bool bOutputsFound = false;
		while(!bDone)
		{
			ifs.getline(&szBuffer[0], BUFLEN);
			if(ifs.fail() && !ifs.eof())
			{
				// probably indicates buffer overflow...ignore remainder of line
				ifs.clear();
				ifs.ignore(INT_MAX, NL_CHAR);
			}
			else if(ifs.bad())
			{
				INIT_ERROR_MSG2(L"%s: Error reading R script file '%s'.", wcRScript)
				throw E_FAIL;
			}

			if(ifs.eof())
				// indicates last line of file...may or may not be empty...in either case, process current line, then exit loop
				bDone = true;

			if(!IsRComment(szBuffer))
				// not a comment line...no need to parse...continue with next line
				continue;

			// comment line...initialize token search and convert initial token to lowercase
			pToken = strtok_s(szBuffer, WHITESPACE_DELIMS, &pContext);
			if(pToken)
				ConvertToLower(pToken);
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
					pToken = strtok_s(NULL, WHITESPACE_DELIMS, &pContext);

					// process variable definition
					while(pToken)
					{
						// line is expected to define a variable...check if this is first token (variable name) on this line
						if(!pVarInfo)
						{
							// 'var info' structure doesn't exist yet...must be at first token...check length of variable name
							if(strlen(pToken) > RFP_MAX_TOKEN_LENGTH)
							{
								INIT_ERROR_MSG2(L"%s: One of the defined RVAR variable names exceeds the maximum allowable length (%d).",
									RFP_MAX_TOKEN_LENGTH)
								throw E_FAIL;
							}

							// convert token to wchar (for logging)
							RFP_MBSTOWCS(wcToken, MAXSIZE_CHAR_STR, pToken)

							// check for duplicates
							if(IsDupVar(pToken))
							{
								INIT_ERROR_MSG2(L"%s: The variable name '%s' is defined multiple times.", wcToken)
								throw E_FAIL;
							}

							// check for R reserved names
							if(IsReservedForR(pToken))
							{
								INIT_ERROR_MSG2(L"%s: The name '%s' is reserved and cannot be used as a variable name.", wcToken)
								throw E_FAIL;
							}

							// check for invalid characters
							if(HasBadChar(pToken))
							{
								INIT_ERROR_MSG2(L"%s: The name '%s' contains an invalid character.", wcToken)
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
								INIT_ERROR_MSG2(L"%s: One of the defined RVAR options exceeds the maximum allowable length (%d).",
									RFP_MAX_TOKEN_LENGTH)
								throw E_FAIL;
							}

							// convert token to wchar (for logging)
							RFP_MBSTOWCS(wcToken, MAXSIZE_CHAR_STR, pToken)

							// check for comment following variable definition
							if(pToken[0]==START_CMT)
								// found comment...no need to parse remainder of current line
								break;

							// convert token to lower case
							ConvertToLower(pToken);

							// check for valid variable options
							bool bIsParameter = false;
							if(strlen(pToken)==2)
							{
								// check for two-char tokens (-s, -o, etc.)
								if(strncmp(pToken, VAR_PARAMTYPE_SCALAR, 2)==0)
									pVarInfo->PutParamType(DssParameterScalar);
								else if(strncmp(pToken, VAR_PARAMTYPE_VECTOR, 2)==0)
									pVarInfo->PutParamType(DssParameterVector);
								else if(strncmp(pToken, VAR_IOTYPE_OUTPUT, 2)==0)
									pVarInfo->SetInputFlag(false);
								else if(strncmp(pToken, VAR_IOTYPE_INPUT, 2)==0)
									pVarInfo->SetInputFlag(true);
								else if(strncmp(pToken, VAR_PARAMTYPE_PARAMETER, 2)==0)
									bIsParameter = true;
								else if(strncmp(pToken, VAR_REPEAT_INPUT, 2)==0)
									pVarInfo->SetRepeatFlag(true);
								else if(strncmp(pToken, VAR_DISABLED_VAR, 2)==0)
								{
									pVarInfo->SetDisabledFlag(true);
									break;  // no need to parse remaining tokens
								}
								else
								{
									INIT_ERROR_MSG2(L"%s: The token '%s' is an unsupported variable option.", wcToken)
									throw E_FAIL;
								}
							}
							else if(strlen(pToken)==4)
							{
								// check for four-char tokens (-num & -str)
								if(strncmp(pToken, VAR_DATATYPE_NUM, 4)==0)
									pVarInfo->PutDataType(DssDataTypeDouble);
								else if(strncmp(pToken, VAR_DATATYPE_STR, 4)==0)
									pVarInfo->PutDataType(DssDataTypeVarChar);
								else
								{
									INIT_ERROR_MSG2(L"%s: The token '%s' is an unsupported variable option.", wcToken)
									throw E_FAIL;
								}
							}
							else
							{
								// check for long versions of tokens (scalar, numeric, etc.)
								if(strcmp(pToken, VAR_DATATYPE_NUM)==0)
									pVarInfo->PutDataType(DssDataTypeDouble);
								else if(strcmp(pToken, VAR_DATATYPE_STR)==0)
									pVarInfo->PutDataType(DssDataTypeVarChar);
								else if(strcmp(pToken, VAR_PARAMTYPE_SCALAR)==0)
									pVarInfo->PutParamType(DssParameterScalar);
								else if(strcmp(pToken, VAR_PARAMTYPE_VECTOR)==0)
									pVarInfo->PutParamType(DssParameterVector);
								else if(strcmp(pToken, VAR_IOTYPE_OUTPUT)==0)
									pVarInfo->SetInputFlag(false);
								else if(strcmp(pToken, VAR_IOTYPE_INPUT)==0)
									pVarInfo->SetInputFlag(true);
								else if(strcmp(pToken, VAR_PARAMTYPE_PARAMETER)==0)
									bIsParameter = true;
								else if(strcmp(pToken, VAR_REPEAT_INPUT)==0)
									pVarInfo->SetRepeatFlag(true);
								else if(strcmp(pToken, VAR_DISABLED_VAR)==0)
								{
									pVarInfo->SetDisabledFlag(true);
									break;  // no need to parse remaining tokens
								}
								else
								{
									INIT_ERROR_MSG2(L"%s: The token '%s' is an unsupported variable option.", wcToken)
									throw E_FAIL;
								}
							}

							if(bIsParameter)
							{
								// indicates a function parameter...fetch next token (should be one of the reserved
								//	function parameter names)
								pToken = strtok_s(NULL, WHITESPACE_DELIMS, &pContext);

								if(strlen(pToken) > RFP_MAX_TOKEN_LENGTH)
								{
									INIT_ERROR_MSG2(L"%s: One of the defined RVAR parameter names exceeds the maximum allowable length (%d).",
										RFP_MAX_TOKEN_LENGTH)
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
									INIT_ERROR_MSG2(L"%s: '%s' is not a valid function parameter name.", wcToken)
									throw E_FAIL;
								}

								// check for duplicates
								if(IsDupParam(pToken))
								{
									INIT_ERROR_MSG2(L"%s: The function parameter name '%s' is defined multiple times.", wcToken)
									throw E_FAIL;
								}

								// save copy of parameter name
								hr = pVarInfo->PutParamName(pToken);
								CHECK_HR(L"Failure while storing parameter name.")
							}
						}

						// fetch next token
						pToken = strtok_s(NULL, WHITESPACE_DELIMS, &pContext);
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
					else if(pVarInfo->IsInput())
					{
						// process input
						if(pVarInfo->GetParamName())
						{
							// function parameter
							m_vParamInfo.push_back(pVarInfo);
						}
						else
						{
							// input...add to input list
							m_vInputInfo.push_back(pVarInfo);

							// check input parameter type
							if(pVarInfo->GetParamType()==DssParameterVector)
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
					}
					else
					{
						// output...set flag
						bOutputsFound = true;
						
						// check data type
						if(pVarInfo->GetDataType()==DssDataTypeUnknown)
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
		}  // while(!bDone)

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
					INIT_ERROR_MSG2(L"%s: The output '%s' specified in the _OutputVar parameter is not defined.", wcOutputVar);
				}
				else
				{
					INIT_ERROR_MSG(L"At least one output is required.")
				}
				throw E_FAIL;
			}

			// sorted functions require vector for first input
			if(m_bIsSorted && (m_vInputInfo[0]->GetParamType() != DssParameterVector))
			{
				INIT_ERROR_MSG(L"RScript and RScriptAgg require a vector for the first input.")
				throw E_FAIL;
			}

			// verify specified function is consistent with specified input parameter types
			if(bAllScalarInput)
			{
				if(m_vOutputInfo[0]->GetParamType()==DssParameterVector)
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
				if(m_vOutputInfo[0]->GetParamType()==DssParameterVector)
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
	if(ifs.is_open()) ifs.close();
	delete [] wcRScript;
	delete [] wcOutputVar;

	return hr;
}
