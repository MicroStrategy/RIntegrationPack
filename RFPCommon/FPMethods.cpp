//=======================================================================
//		Copyright 2013 MicroStrategy, Inc.
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
//		File:	FPMethods.cpp
//
//		Desc:	Implementation of CRFPCommonPKG & CRFPCommonNNGEN classes.
//=======================================================================

//=======================================================================
// includes
//=======================================================================
#include "StdAfx.h"
#include "FPClasses.h"

//=======================================================================
// CRFPCommonPKG
//=======================================================================
CRFPCommonPKG::CRFPCommonPKG() :
	m_pRSupp(NULL),
	m_szInstallFolder(NULL),
	m_szRScriptsFolder(NULL),
	m_szDefRScriptsFolder(NULL)
{
	// initialize error message buffer to empty string
	m_sErrMsg[0] = NULL_WCHAR;

	// read RIP registry entries
	ReadRIPRegEntries();
}

CRFPCommonPKG::~CRFPCommonPKG()
{
	delete m_pRSupp;
	delete [] m_szInstallFolder;
	delete [] m_szRScriptsFolder;
	delete [] m_szDefRScriptsFolder;
}

#ifdef _FP_SUPPORT_ERRORINFO
STDMETHODIMP CRFPCommonPKG::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IRFPCommonPKG
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
#endif

//-----------------------------------------------------------------------
//	CRFPCommonPKG::Count()
//
//	DESCRIPTION:
//		Returns the number of functions implemented by this package.
//
//	PARAMETERS:
//		pCount	[out]	pointer to function count
//
//	RETURNS:
//		S_OK
//		E_POINTER	NULL input pointer
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonPKG::Count(Int_32 *pCount)
{
	const Int_32 lFunctionCount = 5;

	if (pCount == NULL)
		return E_POINTER;

	*pCount = lFunctionCount;
			
	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonPKG::get_CLSID()
//
//	DESCRIPTION:
//		This method is not used.
//
//	PARAMETERS:
//		pCLSID	[out]	pointer to package class ID
//
//	RETURNS:
//		E_NOTIMPL		method not implemented
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonPKG::get_CLSID(GUID *pCLSID)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------
//	CRFPCommonPKG::get_Description()
//
//	DESCRIPTION:
//		Returns the description of this package.
//
//	PARAMETERS:
//		pDescription	[out]	pointer to package description
//
//	RETURNS:
//		S_OK
//		E_POINTER		NULL input pointer
//		E_OUTOFMEMORY	allocation failure
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonPKG::get_Description(BSTR *pDescription)
{
	CComBSTR bstrDesc = L"Common R Function Plug-ins";

	if (pDescription == NULL)
		return E_POINTER;
			
	*pDescription = bstrDesc.Copy();

	if (*pDescription == NULL)
		return E_OUTOFMEMORY;
			
	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonPKG::get_Name()
//
//	DESCRIPTION:
//		Returns the name of this package.
//
//	PARAMETERS:
//		pName	[out]	pointer to package name
//
//	RETURNS:
//		S_OK
//		E_POINTER		NULL input pointer
//		E_OUTOFMEMORY	allocation failure
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonPKG::get_Name(BSTR *pName)
{
	CComBSTR bstrName = L"RFPCommonPKG Package";

	if (pName == NULL)
		return E_POINTER;

	*pName = bstrName.Copy();

	if (*pName == NULL)
		return E_OUTOFMEMORY;
			
	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonPKG::get_Version()
//
//	DESCRIPTION:
//		Returns the version of this package.
//
//	PARAMETERS:
//		pVersion	[out, retval]	pointer to package version
//
//	RETURNS:
//		S_OK
//		E_POINTER		NULL input pointer
//		E_OUTOFMEMORY	allocation failure
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonPKG::get_Version(BSTR *pVersion)
{
	// TODO: set version to desired value
	CComBSTR bstrVersion = L"1.0";

	if (pVersion == NULL)
		return E_POINTER;
			
	*pVersion = bstrVersion.Copy();

	if (*pVersion == NULL)
		return E_OUTOFMEMORY;
			
	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonPKG::get_FunctionName()
//
//	DESCRIPTION:
//		Returns the name of the function corresponding to the specified index.
//
//	PARAMETERS:
//		Index	[in]	function index (within package)
//		pName	[out]	pointer to function name
//
//	RETURNS:
//		S_OK
//		E_POINTER		NULL input pointer
//		E_OUTOFMEMORY	allocation failure
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonPKG::get_FunctionName(Int_32 Index, BSTR *pName)
{
	CComBSTR bstrName;

	if (pName == NULL)
		return E_POINTER;

	switch(Index)
	{
	case 1:
		{
			bstrName = L"RScriptSimple";
			break;
		}
	case 2:
		{
			bstrName = L"RScriptU";
			break;
		}
	case 3:
		{
			bstrName = L"RScript";
			break;
		}
	case 4:
		{
			bstrName = L"RScriptAggU";
			break;
		}
	case 5:
		{
			bstrName = L"RScriptAgg";
			break;
		}
	default:
		{
			return E_FAIL;  // invalid function index
		}
	}
	*pName = bstrName.Copy();

	if (*pName == NULL)
		return E_OUTOFMEMORY;
			
	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonPKG::Item()
//
//	DESCRIPTION:
//		Returns a reference to the object representing the function corresponding
//		to the specified index.
//
//	PARAMETERS:
//		Index				[in]	function index (within package)
//		ppFunctionObject	[out]	pointer to pointer to function object
//
//	RETURNS:
//		S_OK
//		E_POINTER			NULL input pointer
//		E_FAIL				invalid function index
//							unhandled exception
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonPKG::Item(
	Int_32 Index,
	ICDSSFunctionObject **ppFunctionObject)
{
	if (ppFunctionObject == NULL)
		return E_POINTER;

	try
	{
		switch(Index)
		{
		case 1:  // RScriptSimple
		case 2:  // RScriptU
		case 3:  // RScript
		case 4:  // RScriptAggU
		case 5:  // RScriptAgg

			{
				// These functions are implemented by CRFPCommonNNGEN.
				CRFPCommonNNGEN* pFunction = (new CComObject<CRFPCommonNNGEN>());
				pFunction->Init(Index, this);
				*ppFunctionObject = (ICDSSFunctionObject*)(pFunction);
				(*ppFunctionObject)->AddRef();
				break;
			}
		default:
			{
				return E_FAIL;  // invalid function index
			}
		}
	}
	catch(...)
	{
		return E_FAIL;
	}
		
	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonPKG::get_CLSIDString()
//
//	DESCRIPTION:
//		Returns a string version of the function package's class id.
//
//	PARAMETERS:
//		pCLSIDString	[out]	pointer to string version of class id
//
//	RETURNS:
//		S_OK
//		E_POINTER		NULL input pointer
//		E_OUTOFMEMORY	allocation error
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonPKG::get_CLSIDString(
	BSTR *pCLSIDString)
{
	if (pCLSIDString == NULL)
		return E_POINTER;

	*pCLSIDString = SysAllocString(L"EBE6A5E95DB64DE3812064CF0D701E84");
	if(*pCLSIDString == NULL)
		return E_OUTOFMEMORY;

	return S_OK;
}

//=======================================================================
// CRFPCommonNNGEN
//=======================================================================

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::CRFPCommonNNGEN()
//
//	DESCRIPTION:
//		Constructor
//
//	PARAMETERS:
//		none
//
//	RETURNS:
//		none
//-----------------------------------------------------------------------
CRFPCommonNNGEN::CRFPCommonNNGEN() :
	m_lFunctionIndex(0),
	m_pPKG(NULL),
	m_pRSuppFO(NULL),
	m_lInputSize(0),
	m_lInputCnt(0),
	m_bParseOK(false),
	m_sRScriptFile(NULL),
	m_sWorkingDir(NULL),
	m_sOutputVar(NULL),
	m_sInputNames(NULL),
	m_bIsNullProcessable(VARIANT_TRUE),
	m_bCheckInputCnt(true),
	m_szReposRScriptFile(NULL),
	m_szErrLog(NULL),
	m_szErrLogBkup(NULL)
{
#ifdef _FPNNGEN_FTM
	m_pUnkMarshaler = NULL;
#endif

	// initialize error message buffer to empty string
	m_sErrMsg[0] = NULL_WCHAR;
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::~CRFPCommonNNGEN()
//
//	DESCRIPTION:
//		Destructor
//
//	PARAMETERS:
//		none
//
//	RETURNS:
//		none
//-----------------------------------------------------------------------
CRFPCommonNNGEN::~CRFPCommonNNGEN()
{
	Clear();

	try
	{
		// release memory
		delete m_pRSuppFO;
		delete [] m_sRScriptFile;
		delete [] m_sWorkingDir;
		delete [] m_sOutputVar;
		delete [] m_sInputNames;
		delete [] m_szReposRScriptFile;
		delete [] m_szErrLog;
		delete [] m_szErrLogBkup;
	}
	catch(...)
	{
	}
}

#ifdef _FP_SUPPORT_ERRORINFO
STDMETHODIMP CRFPCommonNNGEN::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IRFPCommonNNGEN
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
#endif
STDMETHODIMP CRFPCommonNNGEN::GetDescription(
	BSTR *pDesc)
{
	HRESULT		hr = S_OK;

	try
	{
		// attempt to log error
		LogErrMsg(m_sErrMsg);

		// return error message (may be empty)
		*pDesc = SysAllocString(m_sErrMsg);
	}
	catch(...)
	{
		hr = E_FAIL;
	}

	return hr;
}

STDMETHODIMP CRFPCommonNNGEN::GetGUID(
	struct _GUID *)
{
	return E_NOTIMPL;
}

STDMETHODIMP CRFPCommonNNGEN::GetSource(
	BSTR *)
{
	return E_NOTIMPL;
}

STDMETHODIMP CRFPCommonNNGEN::GetHelpFile(
	BSTR *)
{
	return E_NOTIMPL;
}

STDMETHODIMP CRFPCommonNNGEN::GetHelpContext(
	unsigned long *)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::Init()
//
//	DESCRIPTION:
//		Sets the internal function index, corresponding to the requested
//		function plug-in.
//
//	PARAMETERS:
//		lFunctionIndex	[in]	function index
//		pPKG			[in]	pointer to FP package object
//
//	RETURNS:
//		S_OK
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonNNGEN::Init(
	Int_32 lFunctionIndex,
	CRFPCommonPKG *pPKG)
{
	m_lFunctionIndex = lFunctionIndex;
	m_pPKG = pPKG;

	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::get_FunctionType()
//
//	DESCRIPTION:
//		Returns the function type.
//
//	PARAMETERS:
//		pType	[out]	pointer to function type
//
//	RETURNS:
//		S_OK
//		E_POINTER	NULL input pointer
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonNNGEN::get_FunctionType(
	EnumDSSFunctionType *pType)
{
	if (pType == NULL)
		return E_POINTER;
		
	*pType = DssFunctionTypeGeneric;

	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::SetProperty()
//
//	DESCRIPTION:
//		Sets member variable to requested function plug-in property.
//
//	PARAMETERS:
//		Index	[in]	property index
//		pValue	[in]	property value
//
//	RETURNS:
//		S_OK
//		E_POINTER	NULL input pointer
//		E_FAIL		invalid function index
//					invalid property index
//					value not compatible with defined type
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonNNGEN::SetProperty(
	Int_32 Index,
	VARIANT *pValue)
{
	wchar_t			wcsVarName[MAXSIZE_CHAR_STR+1];
	HRESULT			hr				= S_OK;
	const wchar_t	FUNC_NAME[]		= L"CRFPCommonNNGEN::SetProperty";

	if (pValue == NULL)
		return E_POINTER;

	if((Index > 32) && !m_bParseOK)
		// no need to continue processing properties
		return E_FAIL;

	switch (m_lFunctionIndex)
	{
	case 1:  // RScriptSimple
	case 2:  // RScriptU
	case 3:  // RScript
	case 4:  // RScriptAggU
	case 5:  // RScriptAgg
		{
			switch (Index)
			{
			case 1:  // BooleanParam1
			case 2:  // BooleanParam2
			case 3:  // BooleanParam3
			case 4:  // BooleanParam4
			case 5:  // BooleanParam5
			case 6:  // BooleanParam6
			case 7:  // BooleanParam7
			case 8:  // BooleanParam8
			case 9:  // BooleanParam9
				{
					hr = VariantChangeType(pValue, pValue, 0, VT_BOOL);
					if(FAILED(hr)) return hr;

					if(pValue->vt==VT_BOOL)
					{
						hr = VariantCopy(&m_vPropValues[Index-1], pValue);
						if(FAILED(hr)) return hr;
					}
					else
						return E_FAIL;

					break;
				}
			case 10:  // NumericParam1
			case 11:  // NumericParam2
			case 12:  // NumericParam3
			case 13:  // NumericParam4
			case 14:  // NumericParam5
			case 15:  // NumericParam6
			case 16:  // NumericParam7
			case 17:  // NumericParam8
			case 18:  // NumericParam9
				{
					if(pValue->vt==VT_R8)
					{
						hr = VariantCopy(&m_vPropValues[Index-1], pValue);
						if(FAILED(hr)) return hr;
					}
					else
						return E_FAIL;

					break;
				}
			case 19:  // StringParam1
			case 20:  // StringParam2
			case 21:  // StringParam3
			case 22:  // StringParam4
			case 23:  // StringParam5
			case 24:  // StringParam6
			case 25:  // StringParam7
			case 26:  // StringParam8
			case 27:  // StringParam9
				{
					if(pValue->vt==VT_BSTR)
					{
						hr = VariantCopy(&m_vPropValues[Index-1], pValue);
						if(FAILED(hr)) return hr;
					}
					else
						return E_FAIL;

					break;
				}
			case 28:  // _WorkingDir
				{
					hr = VariantChangeType(pValue, pValue, 0, VT_BSTR);
					if ( FAILED(hr) ) return hr;

					if(pValue->vt == VT_BSTR)
					{
						Int_32 lSrcLen = SysStringLen(pValue->bstrVal);
						if(lSrcLen > 0)
						{
							// convert BSTR to char
							m_sWorkingDir = new char [lSrcLen + 1];
							if(!m_sWorkingDir) return E_OUTOFMEMORY;
							size_t returnLen;
							wcstombs_s(&returnLen, m_sWorkingDir, lSrcLen+1, pValue->bstrVal, lSrcLen);

							// verify that specified folder exists
							struct stat fInfo;
							if(stat(m_sWorkingDir, &fInfo)!=0)
							{
								// folder does not exist...ignore it
								delete [] m_sWorkingDir;
								m_sWorkingDir = NULL;
							}
						}
						else
							// _WorkingDir not set...set to NULL
							m_sWorkingDir = NULL;
					}
					else
					{
						INIT_ERROR_MSG(L"Error processing _WorkingDir parameter.")
						return E_FAIL;
					}

					break;
				}
			case 29:  // _OutputVar
				{
					hr = VariantChangeType(pValue, pValue, 0, VT_BSTR);
					if ( FAILED(hr) ) return hr;

					if(pValue->vt == VT_BSTR)
					{
						Int_32 lSrcLen = SysStringLen(pValue->bstrVal);
						if(lSrcLen > 0)
						{
							// convert BSTR to char
							m_sOutputVar = new char [lSrcLen + 1];
							if(!m_sOutputVar) return E_OUTOFMEMORY;
							size_t returnLen;
							wcstombs_s(&returnLen, m_sOutputVar, lSrcLen+1, pValue->bstrVal, lSrcLen);
						}
						else
							// _OutputVar not set...set to NULL
							m_sOutputVar = NULL;
					}
					else
					{
						INIT_ERROR_MSG(L"Error processing _OutputVar parameter.")
						return E_FAIL;
					}

					break;
				}
			case 30:  // _NullsAllowed
				{
					hr = VariantChangeType(pValue, pValue, 0, VT_BOOL);
					if ( FAILED(hr) ) return hr;

					if(pValue->vt == VT_BOOL)
						m_bIsNullProcessable = (pValue->boolVal==VARIANT_TRUE) ? VARIANT_TRUE : VARIANT_FALSE;
					else
					{
						INIT_ERROR_MSG(L"Error processing _NullsAllowed parameter.")
						return E_FAIL;
					}

					break;
				}
			case 31:  // _CheckInputCount
				{
					hr = VariantChangeType(pValue, pValue, 0, VT_BOOL);
					if ( FAILED(hr) ) return hr;

					if(pValue->vt == VT_BOOL)
						m_bCheckInputCnt = (pValue->boolVal==VARIANT_TRUE);
					else
					{
						INIT_ERROR_MSG(L"Error processing _CheckInputCount parameter.")
						return E_FAIL;
					}

					break;
				}
			case 32:  // _RScriptFile
				{
					hr = VariantChangeType(pValue, pValue, 0, VT_BSTR);
					if ( FAILED(hr) ) return hr;

					if(pValue->vt == VT_BSTR)
					{
						Int_32 lSrcLen = SysStringLen(pValue->bstrVal);
						m_sRScriptFile = new char [lSrcLen + 1];
						if(!m_sRScriptFile) return E_OUTOFMEMORY;
						size_t returnLen;
						wcstombs_s(&returnLen, m_sRScriptFile, lSrcLen+1, pValue->bstrVal, lSrcLen);

						// set up CRSupport_FO object (this will initialize the R environment)
						hr = AllocRSuppFO();
						if(FAILED(hr)) return hr;

						// parse R script file
						hr = m_pRSuppFO->ParseRScript(m_szReposRScriptFile ? m_szReposRScriptFile : m_sRScriptFile, m_sOutputVar);
						if(SUCCEEDED(hr))
						{
							// set parse OK flag
							m_bParseOK = true;

							// process user function parameters
							for(Int_32 i=0;i<RFP_USER_FUNCPARAM_CNT;i++)
							{
								CVarInfo *pVI = m_pRSuppFO->GetParam(i+1);
								if(pVI)
								{
									RFP_MBSTOWCS(wcsVarName, pVI->GetVarName())
									hr = m_pRSuppFO->GetRSupp()->SetRVarV(pVI->GetVarName(), &m_vPropValues[i], NULL);
									CHECK_SETRVAR_HR(wcsVarName)
								}
								//else...parameter not defined by R script...skip it
							}

							if(m_sWorkingDir)
								// working directory exists...pass to R
								m_pRSuppFO->GetRSupp()->SetRVarStr(RFP_WORKDIR, m_sWorkingDir);
						}
						else
							return hr;
					}
					else
					{
						INIT_ERROR_MSG(L"Error processing _RScriptFile parameter.")
						return E_FAIL;
					}

					break;
				}
			case 33:  // _InputNames
				{
					hr = VariantChangeType(pValue, pValue, 0, VT_BSTR);
					if ( FAILED(hr) ) return hr;

					if(pValue->vt == VT_BSTR)
					{
						Int_32 lSrcLen = SysStringLen(pValue->bstrVal);
						if(lSrcLen > 0)
						{
							// convert BSTR to char
							m_sInputNames = new char [lSrcLen + 1];
							if(!m_sInputNames) return E_OUTOFMEMORY;
							size_t returnLen;
							wcstombs_s(&returnLen, m_sInputNames, lSrcLen+1, pValue->bstrVal, lSrcLen);
						}
						else
							// _InputNames not set...set to NULL
							m_sInputNames = NULL;
					}
					else
					{
						INIT_ERROR_MSG(L"Error processing _InputNames parameter.")
						return E_FAIL;
					}

					break;
				}
			default:
				{
					return E_FAIL;  // invalid property index
				}
			}

			break;
		}
	default:
		{
			return E_FAIL;  // invalid function index
		}
	}

	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::get_Description()
//
//	DESCRIPTION:
//		Returns the description of the current function plug-in.
//
//	PARAMETERS:
//		pDescription	[out]	pointer to function description
//
//	RETURNS:
//		S_OK
//		E_POINTER		NULL input pointer
//		E_FAIL			invalid function index
//		E_OUTOFMEMORY	allocation failure
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonNNGEN::get_Description(
	BSTR *pDescription)
{
	CComBSTR bstrDesc;

	if (pDescription == NULL)
		return E_POINTER;

	switch(m_lFunctionIndex)
	{
	case 1:  // RScriptSimple
		{
			bstrDesc = L"Execute R script using scalar inputs/output.";
			break;
		}
	case 2:  // RScriptU
		{
			bstrDesc = L"Execute R script using vector inputs/output, unsorted.";
			break;
		}
	case 3:  // RScript
		{
			bstrDesc = L"Execute R script using vector inputs/output, with sorting.";
			break;
		}
	case 4:  // RScriptAggU
		{
			bstrDesc = L"Execute R script using vector inputs and scalar output, unsorted.";
			break;
		}
	case 5:  // RScriptAgg
		{
			bstrDesc = L"Execute R script using vector inputs and scalar output, with sorting.";
			break;
		}
	default:
		{
			return E_FAIL;  // invalid function index
		}
	}
			
	*pDescription = bstrDesc.Copy();

	if (*pDescription == NULL)
		return E_OUTOFMEMORY;
			
	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::get_ParamCount()
//
//	DESCRIPTION:
//		Returns the parameter counts for the current function (used during
//		import).
//
//	PARAMETERS:
//		pInCount		[out]	pointer to input parameter count
//		pOutCount		[out]	pointer to output parameter count
//		pRepeatCount	[out]	pointer to repeat parameter count
//
//	RETURNS:
//		S_OK
//		E_POINTER		NULL input pointer
//		E_FAIL			invalid function index
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonNNGEN::get_ParamCount(
	Int_32 *pInCount,
	Int_32 *pOutCount,
	Int_32 *pRepeatCount)
{
	if ((pInCount == NULL) || (pOutCount == NULL) || (pRepeatCount == NULL))
		return E_POINTER;

	switch(m_lFunctionIndex)
	{
	case 1:  // RScriptSimple
	case 2:  // RScriptU
	case 3:  // RScript
	case 4:  // RScriptAggU
	case 5:  // RScriptAgg
		{
			*pInCount=1;
			*pOutCount=1;
			*pRepeatCount=1;
			break;
		}
	default:
		{
			return E_FAIL;  // invalid function index
		}
	}
			
	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::get_ParamInfo()
//
//	DESCRIPTION:
//		Returns the name and type of the specified parameter for the current
//		function (used during import).
//
//	PARAMETERS:
//		nParamIndex	[in]	parameter index
//		InParameter	[in]	input parameter flag (1 if input)
//		pName		[out]	pointer to parameter name
//		pType		[out]	pointer to parameter type
//
//	RETURNS:
//		S_OK
//		E_POINTER		NULL input pointer
//		E_FAIL			invalid function index
//						invalid parameter index
//		E_OUTOFMEMORY	allocation failure
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonNNGEN::get_ParamInfo(
	Int_32 nParamIndex,
	Int_32 InParameter,
	BSTR *pName,
	enum EnumDSSParameterType *pType)
{
	CComBSTR bstrName;

	if (pType == NULL)
		return E_POINTER;

	switch(m_lFunctionIndex)
	{
	case 1:  // RScriptSimple
	case 2:  // RScriptU
	case 3:  // RScript
	case 4:  // RScriptAggU
	case 5:  // RScriptAgg
		{
			if(InParameter)
			{
				switch (nParamIndex)
				{
				case 1:  // m1_inParam
					{
						bstrName=L"inParam";
						if(m_lFunctionIndex==1)
							*pType=DssParameterScalar;
						else
							*pType=DssParameterVector;
						break;
					}
				default:
					{
						return E_FAIL;  // invalid parameter index
					}
				}
			}
			else  // out parameter
			{
				switch (nParamIndex)
				{
				case 1:  // m1_result
					{
						bstrName=L"result";
						if(m_lFunctionIndex==2 || m_lFunctionIndex==3)
							*pType=DssParameterVector;
						else
							*pType=DssParameterScalar;
						break;
					}
				default:
					{
						return E_FAIL;  // invalid parameter index
					}
				}
			}
			break;
		}
	default:
		{
			return E_FAIL;  // invalid function index
		}
	}
			
	*pName = bstrName.Copy();

	if (*pName == NULL)
		return E_OUTOFMEMORY;
			
	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::get_SqlType()
//
//	DESCRIPTION:
//		Returns the SQL type of the current function plug-in.
//
//	PARAMETERS:
//		pSqlType	[out]	pointer to the SQL type
//
//	RETURNS:
//		S_OK
//		E_POINTER	NULL input pointer
//		E_FAIL		invalid function index
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonNNGEN::get_SqlType(
	enum EnumDSSFunctionSQLType *pSqlType)
{
	if (pSqlType == NULL)
		return E_POINTER;

	switch(m_lFunctionIndex)
	{
	case 1:  // RScriptSimple
		{
			*pSqlType = DssFunctionSQLTypeArithmetic;

			break;
		}
	case 2:  // RScriptU
	case 3:  // RScript
		{
			*pSqlType = DssFunctionSQLTypeRelative;

			break;
		}
	case 4:  // RScriptAggU
	case 5:  // RScriptAgg
		{
			*pSqlType = DssFunctionSQLTypeAggregation;

			break;
		}
	default:
		{
			return E_FAIL;  // invalid function index
		}
	}

	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::get_PropertyCount()
//
//	DESCRIPTION:
//		Returns the property count for the current function (used during
//		import).
//
//	PARAMETERS:
//		pCount	[out]	pointer to property count
//
//	RETURNS:
//		S_OK
//		E_POINTER	NULL input pointer
//		E_FAIL		invalid function index
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonNNGEN::get_PropertyCount(
	Int_32 *pCount)
{
	if (pCount == NULL)
		return E_POINTER;

	switch(m_lFunctionIndex)
	{
	case 1:  // RScriptSimple
	case 2:  // RScriptU
	case 3:  // RScript
	case 4:  // RScriptAggU
	case 5:  // RScriptAgg
		{
			*pCount=33;
			break;
		}
	default:
		{
			return E_FAIL;  // invalid function index
		}
	}

	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::get_PropertyInfo()
//
//	DESCRIPTION:
//		Returns the name, description and default vaule of the specified
//		property for the current function (used during import).
//
//	PARAMETERS:
//		PropertyIndex	[in]	property index
//		pName			[out]	pointer to property name
//		pDescription	[out]	pointer to property description
//		pValue			[out]	pointer to property default value
//
//	RETURNS:
//		S_OK
//		E_POINTER		NULL input pointer
//		E_FAIL			invalid function index
//						invalid property index
//		E_OUTOFMEMORY	allocation failure
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonNNGEN::get_PropertyInfo(
	Int_32 PropertyIndex,
	BSTR *pName,
	BSTR *pDescription,
	VARIANT *pValue)
{
	CComBSTR bstrName;
	CComBSTR bstrDesc;

	if (pName == NULL)
		return E_POINTER;
	if (pDescription == NULL)
		return E_POINTER;
	if (pValue == NULL)
		return E_POINTER;

	switch(m_lFunctionIndex)
	{
	case 1:  // RScriptSimple
	case 2:  // RScriptU
	case 3:  // RScript
	case 4:  // RScriptAggU
	case 5:  // RScriptAgg
		{
			switch(PropertyIndex)
			{
			case 1:  // BooleanParam1
				{
					bstrName = L"BooleanParam1";
					bstrDesc = L"user-defined boolean parameter 1";
					(*pValue).vt = VT_BOOL;
					(*pValue).boolVal = VARIANT_FALSE;
					break;
				}
			case 2:  // BooleanParam2
				{
					bstrName = L"BooleanParam2";
					bstrDesc = L"user-defined boolean parameter 2";
					(*pValue).vt = VT_BOOL;
					(*pValue).boolVal = VARIANT_FALSE;
					break;
				}
			case 3:  // BooleanParam3
				{
					bstrName = L"BooleanParam3";
					bstrDesc = L"user-defined boolean parameter 3";
					(*pValue).vt = VT_BOOL;
					(*pValue).boolVal = VARIANT_FALSE;
					break;
				}
			case 4:  // BooleanParam4
				{
					bstrName = L"BooleanParam4";
					bstrDesc = L"user-defined boolean parameter 4";
					(*pValue).vt = VT_BOOL;
					(*pValue).boolVal = VARIANT_FALSE;
					break;
				}
			case 5:  // BooleanParam5
				{
					bstrName = L"BooleanParam5";
					bstrDesc = L"user-defined boolean parameter 5";
					(*pValue).vt = VT_BOOL;
					(*pValue).boolVal = VARIANT_FALSE;
					break;
				}
			case 6:  // BooleanParam6
				{
					bstrName = L"BooleanParam6";
					bstrDesc = L"user-defined boolean parameter 6";
					(*pValue).vt = VT_BOOL;
					(*pValue).boolVal = VARIANT_FALSE;
					break;
				}
			case 7:  // BooleanParam7
				{
					bstrName = L"BooleanParam7";
					bstrDesc = L"user-defined boolean parameter 7";
					(*pValue).vt = VT_BOOL;
					(*pValue).boolVal = VARIANT_FALSE;
					break;
				}
			case 8:  // BooleanParam8
				{
					bstrName = L"BooleanParam8";
					bstrDesc = L"user-defined boolean parameter 8";
					(*pValue).vt = VT_BOOL;
					(*pValue).boolVal = VARIANT_FALSE;
					break;
				}
			case 9:  // BooleanParam9
				{
					bstrName = L"BooleanParam9";
					bstrDesc = L"user-defined boolean parameter 9";
					(*pValue).vt = VT_BOOL;
					(*pValue).boolVal = VARIANT_FALSE;
					break;
				}
			case 10:  // NumericParam1
				{
					bstrName = L"NumericParam1";
					bstrDesc = L"user-defined numeric parameter #1";
					(*pValue).vt = VT_R8;
					(*pValue).dblVal = 0;
					break;
				}
			case 11:  // NumericParam2
				{
					bstrName = L"NumericParam2";
					bstrDesc = L"user-defined numeric parameter #2";
					(*pValue).vt = VT_R8;
					(*pValue).dblVal = 0;
					break;
				}
			case 12:  // NumericParam3
				{
					bstrName = L"NumericParam3";
					bstrDesc = L"user-defined numeric parameter #3";
					(*pValue).vt = VT_R8;
					(*pValue).dblVal = 0;
					break;
				}
			case 13:  // NumericParam4
				{
					bstrName = L"NumericParam4";
					bstrDesc = L"user-defined numeric parameter #4";
					(*pValue).vt = VT_R8;
					(*pValue).dblVal = 0;
					break;
				}
			case 14:  // NumericParam5
				{
					bstrName = L"NumericParam5";
					bstrDesc = L"user-defined numeric parameter #5";
					(*pValue).vt = VT_R8;
					(*pValue).dblVal = 0;
					break;
				}
			case 15:  // NumericParam6
				{
					bstrName = L"NumericParam6";
					bstrDesc = L"user-defined numeric parameter #6";
					(*pValue).vt = VT_R8;
					(*pValue).dblVal = 0;
					break;
				}
			case 16:  // NumericParam7
				{
					bstrName = L"NumericParam7";
					bstrDesc = L"user-defined numeric parameter #7";
					(*pValue).vt = VT_R8;
					(*pValue).dblVal = 0;
					break;
				}
			case 17:  // NumericParam8
				{
					bstrName = L"NumericParam8";
					bstrDesc = L"user-defined numeric parameter #8";
					(*pValue).vt = VT_R8;
					(*pValue).dblVal = 0;
					break;
				}
			case 18:  // NumericParam9
				{
					bstrName = L"NumericParam9";
					bstrDesc = L"user-defined numeric parameter #9";
					(*pValue).vt = VT_R8;
					(*pValue).dblVal = 0;
					break;
				}
			case 19:  // StringParam1
				{
					bstrName = L"StringParam1";
					bstrDesc = L"user-defined non-numeric parameter #1";
					(*pValue).vt = VT_BSTR;
					(*pValue).bstrVal = SysAllocString(OLESTR(""));
					break;
				}
			case 20:  // StringParam2
				{
					bstrName = L"StringParam2";
					bstrDesc = L"user-defined non-numeric parameter #2";
					(*pValue).vt = VT_BSTR;
					(*pValue).bstrVal = SysAllocString(OLESTR(""));
					break;
				}
			case 21:  // StringParam3
				{
					bstrName = L"StringParam3";
					bstrDesc = L"user-defined non-numeric parameter #3";
					(*pValue).vt = VT_BSTR;
					(*pValue).bstrVal = SysAllocString(OLESTR(""));
					break;
				}
			case 22:  // StringParam4
				{
					bstrName = L"StringParam4";
					bstrDesc = L"user-defined non-numeric parameter #4";
					(*pValue).vt = VT_BSTR;
					(*pValue).bstrVal = SysAllocString(OLESTR(""));
					break;
				}
			case 23:  // StringParam5
				{
					bstrName = L"StringParam5";
					bstrDesc = L"user-defined non-numeric parameter #5";
					(*pValue).vt = VT_BSTR;
					(*pValue).bstrVal = SysAllocString(OLESTR(""));
					break;
				}
			case 24:  // StringParam6
				{
					bstrName = L"StringParam6";
					bstrDesc = L"user-defined non-numeric parameter #6";
					(*pValue).vt = VT_BSTR;
					(*pValue).bstrVal = SysAllocString(OLESTR(""));
					break;
				}
			case 25:  // StringParam7
				{
					bstrName = L"StringParam7";
					bstrDesc = L"user-defined non-numeric parameter #7";
					(*pValue).vt = VT_BSTR;
					(*pValue).bstrVal = SysAllocString(OLESTR(""));
					break;
				}
			case 26:  // StringParam8
				{
					bstrName = L"StringParam8";
					bstrDesc = L"user-defined non-numeric parameter #8";
					(*pValue).vt = VT_BSTR;
					(*pValue).bstrVal = SysAllocString(OLESTR(""));
					break;
				}
			case 27:  // StringParam9
				{
					bstrName = L"StringParam9";
					bstrDesc = L"user-defined non-numeric parameter #9";
					(*pValue).vt = VT_BSTR;
					(*pValue).bstrVal = SysAllocString(OLESTR(""));
					break;
				}
			case 28:  // _WorkingDir
				{
					bstrName = L"_WorkingDir";
					bstrDesc = L"Working directory";
					(*pValue).vt = VT_BSTR;
					(*pValue).bstrVal = SysAllocString(OLESTR(""));
					break;
				}
			case 29:  // _OutputVar
				{
					bstrName = L"_OutputVar";
					bstrDesc = L"Output variable";
					(*pValue).vt = VT_BSTR;
					(*pValue).bstrVal = SysAllocString(OLESTR(""));
					break;
				}
			case 30:  // _NullsAllowed
				{
					bstrName = L"_NullsAllowed";
					bstrDesc = L"Null-inputs-allowed flag";
					(*pValue).vt = VT_BOOL;
					(*pValue).boolVal = VARIANT_TRUE;
					break;
				}
			case 31:  // _CheckInputCount
				{
					bstrName = L"_CheckInputCount";
					bstrDesc = L"Check-input-count flag";
					(*pValue).vt = VT_BOOL;
					(*pValue).boolVal = VARIANT_TRUE;
					break;
				}
			case 32:  // _RScriptFile
				{
					bstrName = L"_RScriptFile";
					bstrDesc = L"R script";
					(*pValue).vt = VT_BSTR;
					(*pValue).bstrVal = SysAllocString(OLESTR(""));
					break;
				}
			case 33:  // _InputNames
				{
					bstrName = L"_InputNames";
					bstrDesc = L"Input names";
					(*pValue).vt = VT_BSTR;
					(*pValue).bstrVal = SysAllocString(OLESTR(""));
					break;
				}
			default:
				{
					return E_FAIL;  // invalid property index
				}
			}
			break;
		}
	default:
		{
			return E_FAIL;  // invalid function index
		}
	}
			
	*pName = bstrName.Copy();
	if (*pName == NULL)
		return E_OUTOFMEMORY;
			
	*pDescription = bstrDesc.Copy();
	if (*pDescription == NULL)
		return E_OUTOFMEMORY;

	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::get_IsOrderSignificant()
//
//	DESCRIPTION:
//		Returns the 'is order significant' flag for the current function
//		(used during import).
//
//	PARAMETERS:
//		pIsOrderSignificant	[out]	pointer to 'is order significant' flag
//
//	RETURNS:
//		S_OK
//		E_POINTER	NULL input pointer
//		E_FAIL		invalid function index
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonNNGEN::get_IsOrderSignificant(
	VARIANT_BOOL *pIsOrderSignificant)
{
	if (pIsOrderSignificant == NULL)
		return E_POINTER;

	switch(m_lFunctionIndex)
	{
	case 1:  // RScriptSimple
	case 2:  // RScriptU
	case 4:  // RScriptAggU
		{
			*pIsOrderSignificant=VARIANT_FALSE;
			break;
		}
	case 3:  // RScript
	case 5:  // RScriptAgg
		{
			*pIsOrderSignificant=VARIANT_TRUE;
			break;
		}
	default:
		{
			return E_FAIL;  // invalid function index
		}
	}

	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::ConvertFromVariant()
//
//	DESCRIPTION:
//		Extract real data from variant pVar according to iType,
//		and put it into pData.
//
//	PARAMETERS:
//		pVar	[in]	pointer to variant input
//		iType	[in]	destination type
//		pData	[out]	pointer to destination
//
//	RETURNS:
//		S_OK
//		E_FAIL		invalid iType
//					variant type is not compatible with type specified by iType
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonNNGEN::ConvertFromVariant(
	VARIANT *pVar,
	enum EnumDSSDataType iType,
	BYTE *pData)
{
	HRESULT hr = S_OK;

	switch (iType)
	{
	case DssDataTypeDouble:
		if (pVar->vt == VT_I2)  // short
			*reinterpret_cast<double *>(pData) = pVar->iVal;
		else if (pVar->vt == VT_I4)  // long
			*reinterpret_cast<double *>(pData) = pVar->lVal;
		else if (pVar->vt == VT_R4)  // float
			*reinterpret_cast<double *>(pData) = pVar->fltVal;
		else if (pVar->vt == VT_R8)  // double
			*reinterpret_cast<double *>(pData) = pVar->dblVal;
		else
			return E_FAIL;
		break;
	case DssDataTypeDate:
		if (pVar->vt == VT_DISPATCH)
		{
			M5TypLib::IDSSTime *ptime;
			hr = (pVar->pdispVal)->QueryInterface(IID_IDSSTime, (void **)&ptime);
			if (hr != S_OK)	return hr;
			Int_32 lYear, lDay;
			M5TypLib::EnumDSSMonth eMonth;
			try
			{
				hr = ptime->get_Year(&lYear);
				if FAILED(hr) throw(hr);
				hr = ptime->get_Month(&eMonth);
				if FAILED(hr) throw(hr);
				hr = ptime->get_Day(&lDay);
				if FAILED(hr) throw(hr);
			}
			catch(HRESULT hr)
			{
				ptime->Release();
				return hr;
			}
			sprintf_s(reinterpret_cast<char *>(pData), MAXSIZE_DATE_STR, "%02d.%02d.%04d", lDay, eMonth, lYear);
			ptime->Release();
		}
		else if (pVar->vt == VT_BSTR)
		{
			int srcLen = SysStringLen(pVar->bstrVal);
			size_t returnLen;
			wcstombs_s(&returnLen, reinterpret_cast<char *>(pData), MAXSIZE_CHAR_STR+1,
				pVar->bstrVal, srcLen);
		}
		else
			return E_FAIL;
		break;
	case DssDataTypeVarChar:
		{
			if (pVar->vt != VT_BSTR)
				return E_FAIL;
			int srcLen = SysStringLen(pVar->bstrVal);
			size_t returnLen;
			wcstombs_s(&returnLen, reinterpret_cast<char *>(pData), MAXSIZE_CHAR_STR+1,
				pVar->bstrVal, srcLen);
			break;
		}
	default:
		return E_FAIL;
	}

	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::getParamFromVariant()
//
//	DESCRIPTION:
//		Extract real data from variant pParam[index] according to type,
//		and put it into outResult.
//
//	PARAMETERS:
//		pParam		[in]	pointer to array of variant inputs
//		index		[in]	index into pParam of desired parameter
//		type		[in]	the destination data type
//		outResult	[out]	pointer to the result of the conversion of pParam[index]
//
//	RETURNS:
//		hr			HRESULT returned by ConvertFromVariant()
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonNNGEN::getParamFromVariant(
	VARIANT *pParam,
	Int_32 index,
	enum EnumDSSDataType type,
	BYTE *outResult)
{
	VARIANT element;
	VariantInit(&element);
	SafeArrayGetElement(pParam->parray, (LONG *)&index, &element);
	
	HRESULT hr = ConvertFromVariant(&element, type, outResult);
	VariantClear(&element);

	return hr;
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::Calculate()
//
//	DESCRIPTION:
//		Does nothing.  The GetOutput() method does the actual processing.
//
//	PARAMETERS:
//		None
//
//	RETURNS:
//		S_OK
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonNNGEN::Calculate()
{
	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::GetOutput()
//
//	DESCRIPTION:
//		Invokes the current function plug-in, and returns results to the engine.
//
//	PARAMETERS:
//		nSize		[in]	number of elements in "m<function index>_" member variable
//		nDataSize	[out]	number of elements returned in pResult (must be 1, if scalar output)
//		pData		[out]	if vector output, pointer to one-dimensional array of results; else,
//								(scalar output) pointer to result
//		pFlag		[out]	if vector output, pointer to one-dimensional array of result flags;
//								else, (scalar output) pointer to result flag
//
//	RETURNS:
//		S_OK
//		E_POINTER	NULL input pointer
//		E_FAIL		invalid function index
//					unhandled exception
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonNNGEN::GetOutput(
	Int_32 nSize,
	Int_32 *nDataSize,
	VARIANT *pData,
	DSSData_Flags *pFlag)
{
	HRESULT hr = S_OK;
	const wchar_t FUNC_NAME[] = L"CRFPCommonNNGEN::GetOutput";

	if (nDataSize == NULL)
		return E_POINTER;
			
	if (pData == NULL)
		return E_POINTER;
			
	if (pFlag == NULL)
		return E_POINTER;

	if (nSize < 1)
		return E_FAIL;  // Should be at least size = 1

	try
	{
		// check R script parse
		if(!m_bParseOK)
		{
			if(!m_pRSuppFO)
			{
				INIT_ERROR_MSG(L"Error during CRSupport_FO construction.")
			}
			throw E_FAIL;
		}

		switch (m_lFunctionIndex)
		{
		case 1:  // RScriptSimple
		case 2:  // RScriptU
		case 3:  // RScript
		case 4:  // RScriptAggU
		case 5:  // RScriptAgg
			{
				hr = ExRScript(nSize, nDataSize, pData, pFlag);
				break;
			}
		default:
			{
				return E_FAIL;  // invalid function index
			}
		}
	}
	catch(...)
	{
		return E_FAIL;
	}

	return hr;
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::GetOutputType()
//
//	DESCRIPTION:
//		Returns the output type of the current function plug-in.
//
//	PARAMETERS:
//		nSize				[in]	number of inputs to the current function plug-in
//		pInputType			[in]	one-dimensional array[nSize] of input data types
//		pOutputDataType		[out]	pointer to the data type of the current function
//										plug-in's output
//		pOutputParamType	[out]	pointer to the parameter type of the current function
//										plug-in's output
//
//	RETURNS:
//		S_OK
//		E_POINTER	NULL input pointer
//		E_FAIL		invalid function index
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonNNGEN::GetOutputType(
	Int_32 nSize,
	EnumDSSDataType *pInputType,
	EnumDSSDataType *pOutputDataType,
	EnumDSSParameterType *pOutputParamType)
{
	if (pOutputDataType == NULL)
		return E_POINTER;
			
	if (pOutputParamType == NULL)
		return E_POINTER;

	switch(m_lFunctionIndex)
	{
		case 1:  // RScriptSimple
		case 2:  // RScriptU
		case 3:  // RScript
		case 4:  // RScriptAggU
		case 5:  // RScriptAgg
		{
			if(m_bParseOK)
			{
				*pOutputDataType = m_pRSuppFO->GetOutput()->GetDataType();
				*pOutputParamType = m_pRSuppFO->GetOutput()->GetParamType();

				// number of inputs known at this point...process input names
				ProcessInputNames();
			}
			else
			{
				// parse failed...return default types
				*pOutputDataType = DssDataTypeVarChar;
				if(m_lFunctionIndex==2 || m_lFunctionIndex==3)
					*pOutputParamType = DssParameterVector;
				else
					*pOutputParamType = DssParameterScalar;
			}
			break;
		}
	default:
		{
			return E_FAIL;  // invalid function index
		}
	}
			
	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::SetInputParam()
//
//	DESCRIPTION:
//		Copies pData[] into the appropriate member variable.
//
//	PARAMETERS:
//		nParamIndex	[in]	parameter index (1-based)
//		nSize		[in]	number of elements in pData
//		dataType	[in]	type of data in pData
//		pData		[in]	one-dimensional (0-based) array[nSize] containing input
//		pFlag		[in]	one-dimensional (0-based) array[nSize] of flags, indicating state
//								of each element in pData (DssDataOK, DssDataInvalid)
//
//	RETURNS:
//		S_OK
//		E_POINTER		NULL input pointer
//		E_OUTOFMEMORY	allocation failure
//		E_FAIL			invalid function index
//						invalid parameter index
//						invalid input (pData)
//						element count < 1
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonNNGEN::SetInputParam(
	Int_32 nParamIndex,
	Int_32 nSize,
	EnumDSSDataType dataType,
	VARIANT *pData,
	DSSData_Flags *pFlag)
{
	Int_32 lPIndex = 0;
	Int_32 bound1;
	HRESULT hr = S_OK;
	wchar_t wcsVarName[MAXSIZE_CHAR_STR+1];
	const wchar_t FUNC_NAME[] = L"CRFPCommonNNGEN::SetInputParam";

	if (pData == NULL)
		return E_POINTER;

	if (pFlag == NULL)
		return E_POINTER;

	if (nSize < 1)
		return E_FAIL;

	if ((nParamIndex < 1) || (nParamIndex > m_lInputCnt))
		return E_FAIL;

	if(!m_bParseOK)
		// parse failed...return good status so that GetOutput() will be called, which will return error
		return S_OK;

	try
	{
		// fetch counts
		const Int_32 INCNT = m_pRSuppFO->GetInputCnt();
		const Int_32 REPCNT = m_pRSuppFO->GetRepeatCnt();
		const Int_32 NONREPCNT = m_pRSuppFO->GetNonRepeatCnt();

		if((nParamIndex==1) && m_bCheckInputCnt)
		{
			// check input count
			if(REPCNT==0)
			{
				// no repeat inputs...actual input count must equal input count specfied by R script
				if(m_lInputCnt != INCNT)
				{
					INIT_ERROR_MSG2(
						L"%s: The actual number of inputs (%d) does not equal to the number of inputs specified within the R script (%d).",
						m_lInputCnt, INCNT)

					throw E_FAIL;
				}
			}
			else
			{
				// repeat inputs...actual input count must be greater or equal to the input count specified by the R script
				if(m_lInputCnt < INCNT)
				{
					INIT_ERROR_MSG2(
						L"%s: The actual number of inputs (%d) is less than the number of inputs specified within the R script (%d).",
						m_lInputCnt, INCNT)

					throw E_FAIL;
				}

				// make sure actual input count is consistent with repeat count (e.g. if script defines two inputs,
				//	both repeated, then the actual input count must be even)
				if((m_lInputCnt - NONREPCNT) % REPCNT != 0)
				{
					INIT_ERROR_MSG2(
						L"%s: The actual number of inputs (%d) is not consistent with the number of repeated inputs specified within the R script (%d).",
						m_lInputCnt, REPCNT)

					throw E_FAIL;
				}
			}
		}
		//else...let R script handle 'too few inputs' ('too many inputs' is detected by GetInputParamType(), and will always fail)

		// pData must be either a one-dimensional array of Variants[nSize], or a scalar value
		// check if safe array
		bool bSafeArray = false;
		if ((pData->vt & (VT_ARRAY|VT_VARIANT)) == (VT_ARRAY|VT_VARIANT))
		{
			// safe array...check number of dimensions
			if (SafeArrayGetDim(pData->parray) != 1)
				// not one-dimensional
				return E_FAIL;

			// check size
			hr = SafeArrayGetUBound(pData->parray, 1, (LONG *)&bound1);
			if((hr != S_OK) || (bound1 < nSize - 1))
				// unexpected size
				return E_FAIL;

			// set flag
			bSafeArray = true;
		}
		//else...not a safe array...must be a scalar input...size will be checked below

		switch (m_lFunctionIndex)
		{
		case 1:  // RScriptSimple
		case 2:  // RScriptU
		case 3:  // RScript
		case 4:  // RScriptAggU
		case 5:  // RScriptAgg
			{
				// access specified varInfo, based on paramIndex
				lPIndex = (nParamIndex > INCNT) ? (((nParamIndex-(INCNT+1)) % REPCNT) + (NONREPCNT+1)) : nParamIndex;
				CVarInfo *pVI = m_pRSuppFO->GetInput(lPIndex-1);
				CRSupport *pRSupp = m_pRSuppFO->GetRSupp();
				RFP_MBSTOWCS(wcsVarName, pVI->GetVarName())

				// check for potential of pairing-up problem
				if(bSafeArray && (bound1 != nSize - 1) && (m_pRSuppFO->GetInputCnt() > 1))
				{
					// indicates there were null inputs that were skipped...this could lead to pairing-up problem
					INIT_ERROR_MSG2(
						L"%s: The input variable '%s' contained nulls which were skipped due to null-processing settings.  Failing report to avoid potential pairing-up problems.",
						wcsVarName)

					throw E_FAIL;
				}

				// check data type
				hr = pVI->CheckDataType(dataType);
				if(FAILED(hr))
				{
					if(hr==E_FAIL)
					{
						INIT_ERROR_MSG2(
							L"%s: The actual data type (%s) for input variable '%s' is not compatible with the expected data type (%s).",
							RFP_GET_DT_STR(dataType), wcsVarName, RFP_GET_DT_STR(pVI->GetDataType()))
					}
					else if(hr==E_INVALIDARG)
					{
						INIT_ERROR_MSG2(L"%s: The actual data type (%s) for input variable '%s' is not supported.",
							RFP_GET_DT_STR(dataType), wcsVarName)
					}
					else
						CHECK_HR(L"Unexpected error occurred when checking input data types.")

					throw hr;
				}

				if(!pVI->IsRepeat())
				{
					// current input not repeated
					if(pVI->GetParamType()==DssParameterVector)
					{
						hr = pRSupp->SetRVarSA(pVI->GetVarName(), nSize, pVI->GetDataType(), pData, pFlag);
						CHECK_SETRVAR_HR(wcsVarName);
					}
					else // scalar input
					{
						if (nSize != 1)
							return E_FAIL;
						hr = pRSupp->SetRVarV(pVI->GetVarName(), pData, pFlag);
						CHECK_SETRVAR_HR(wcsVarName);
					}
				}
				else
				{
					// repeated inputs
					if ((pVI->GetParamType()==DssParameterScalar) && (nSize != 1))
						return E_FAIL;
					Int_32 lRepIndex = (nParamIndex - lPIndex) / REPCNT;  // provides access to the proper repeated parameter
					Int_32 lColCnt = ((m_lInputCnt - nParamIndex) / REPCNT) + 1;
					hr = pRSupp->SetRVarSA_mx(pVI->GetVarName(), lRepIndex, nSize, lColCnt, pVI->GetDataType(), pData, pFlag,
						bSafeArray, m_sErrMsg);
					CHECK_SETRVAR_HR(wcsVarName);
				}

				// validate vector input size
				if (pVI->GetParamType()==DssParameterVector)
				{
					// vector input...all vector inputs must have same size
					if (m_lInputSize == 0)
						// first vector input parameter
						m_lInputSize = nSize;
					else
					{
						// all vector input parameters expected to have same size
						if (m_lInputSize != nSize)
							return E_FAIL;
					}
				}
				//else...scalar input...no need to compare size to other inputs

				break;
			}
		default:
			{
				return E_FAIL;  // invalid function index
			}
		}

		return S_OK;
	}
	catch(...)
	{
		return E_FAIL;
	}
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::GetInputParamType()
//
//	DESCRIPTION:
//		Returns the parameter type of the specified input parameter for the
//		current function plug-in.
//
//	PARAMETERS:
//		nParamIndex	[in]	parameter index (1-based)
//		pType		[out]	pointer to the input parameter type
//
//	RETURNS:
//		S_OK
//		E_POINTER	NULL input pointer
//		E_FAIL		invalid function index
//					invalid parameter index
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonNNGEN::GetInputParamType(
	Int_32 nParamIndex,
	EnumDSSParameterType *pType)
{
	Int_32			lPIndex			= 0;
	const wchar_t	FUNC_NAME[]		= L"CRFPCommonNNGEN::GetInputParamType";

	if (pType == NULL)
		return E_POINTER;

	if (nParamIndex < 1)
		return E_FAIL;

	if(!m_bParseOK)
		return E_FAIL;

	try
	{
		switch (m_lFunctionIndex)
		{
		case 1:  // RScriptSimple
		case 2:  // RScriptU
		case 3:  // RScript
		case 4:  // RScriptAggU
		case 5:  // RScriptAgg
			{
				// increment input count
				m_lInputCnt ++;

				// check parse status
				if(m_bParseOK)
				{
					const Int_32 INCNT = m_pRSuppFO->GetInputCnt();
					const Int_32 REPCNT = m_pRSuppFO->GetRepeatCnt();

					// make sure number of defined non-repeated inputs is not exceeded (cannot exceed number of defined inputs
					//	if repeated inputs has been specified)
					if((REPCNT==0) && (nParamIndex > INCNT))
					{
						INIT_ERROR_MSG2(
							L"%s: The number of inputs exceeds the expected input count of %d, as defined within the R script.",
							INCNT)

						throw E_FAIL;
					}

					lPIndex = (nParamIndex > INCNT) ? (((nParamIndex-(INCNT+1)) % REPCNT) + (INCNT+1-REPCNT)) : nParamIndex;

					// set parameter type
					*pType = m_pRSuppFO->GetInput(lPIndex-1)->GetParamType();
				}
				else
					// parse failed...return default type
					*pType = DssParameterVector;

				break;
			}
		default:
			{
				return E_FAIL;  // invalid function index
			}
		}
	}
	catch(...)
	{
		return E_FAIL;
	}

	return S_OK;
}

//-----------------------------------------------------------------------
//	CRFPCommonNNGEN::Clear()
//
//	DESCRIPTION:
//		This method is called by the analytical engine prior to processing
//		each group of data.  User-defined code can be added to perform any
//		necessary cleanup.  The method is also called by the destructor.
//
//	PARAMETERS:
//		none
//
//	RETURNS:
//		S_OK
//		E_FAIL		invalid function index
//-----------------------------------------------------------------------
STDMETHODIMP CRFPCommonNNGEN::Clear()
{
	try
	{
		// TODO: add any necessary code for your function plug-in(s)

		// reset input size
		m_lInputSize = 0;

		switch (m_lFunctionIndex)
		{
		case 1:  // RScriptSimple
		case 2:  // RScriptU
		case 3:  // RScript
		case 4:  // RScriptAggU
		case 5:  // RScriptAgg
			{
				break;
			}
		default:
			{
				return E_FAIL;  // invalid function index
			}
		}
	}
	catch(...)
	{
		return E_FAIL;
	}

	return S_OK;
}
