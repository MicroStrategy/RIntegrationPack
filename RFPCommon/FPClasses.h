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
//		File:	FPClasses.h
//
//		Desc:	Declaration of function plug-in classes.
//=======================================================================

#ifndef __FPCLASSES_H_
#define __FPCLASSES_H_

#include "resource.h"       // main symbols
#include "RFPCommon.h"
#include "RSupport.h"

//=======================================================================
// CRFPCommonPKG
//=======================================================================
class ATL_NO_VTABLE CRFPCommonPKG : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRFPCommonPKG, &CLSID_RFPCommonPKG>,
	public IDispatchImpl<IRFPCommonPKG, &IID_IRFPCommonPKG, &LIBID_RFPCommonLib>,
	public ICDSSFunctionPackage2
{
public:
	CRFPCommonPKG();
	~CRFPCommonPKG();

DECLARE_REGISTRY_RESOURCEID(IDR_RFPCOMMON)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRFPCommonPKG)
	COM_INTERFACE_ENTRY(IRFPCommonPKG)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ICDSSFunctionPackage)
	COM_INTERFACE_ENTRY(ICDSSFunctionPackage2)
END_COM_MAP()

// IRFPCommonPKG
private:
	wchar_t			m_sErrMsg[ERRMSG_BUFSIZE];		// package error message buffer
	CRSupport		*m_pRSupp;						// pointer to RSupport object
	char			*m_szInstallFolder;				// RIP install folder (may be used for logging)
	char			*m_szRScriptsFolder;			// RScript repository (may also be used for logging)
	char			*m_szDefRScriptsFolder;			// default RScripts repository (may also be used for logging)

	void ReadRIPKeyValue(HKEY & hKey, BSTR bstrKeyValue, char **pKeyValue);
	void ReadRIPRegEntries();
public:
// ICDSSFunctionPackage2
	STDMETHOD(Count)(Int_32 *pCount);
	STDMETHOD(Item)(Int_32 Index, ICDSSFunctionObject **ppFunctionObject);
	STDMETHOD(get_Name)(BSTR *pName);
	STDMETHOD(get_Version)(BSTR *pVersion);
	STDMETHOD(get_Description)(BSTR *pDescription);
	STDMETHOD(get_CLSID)(GUID *pCLSID);
	STDMETHOD(get_CLSIDString)(BSTR *pCLSIDString);
	STDMETHOD(get_FunctionName)(Int_32 Index, BSTR *pName);

	CRSupport *AllocRSupp(wchar_t *pErrMsg);
	wchar_t *GetErrMsg() { return m_sErrMsg; }
	void ClearErrMsg() { m_sErrMsg[0] = NULL_WCHAR; }
	char *GetInstallFolder() { return m_szInstallFolder; }
	char *GetRScriptsFolder() { return m_szRScriptsFolder; }
	char *GetDefRScriptsFolder() { return m_szDefRScriptsFolder; }
};

//=======================================================================
// CRFPCommonNNGEN
//=======================================================================
class ATL_NO_VTABLE CRFPCommonNNGEN : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRFPCommonNNGEN, &CLSID_RFPCommonNNGEN>,
	public IDispatchImpl<IRFPCommonNNGEN, &IID_IRFPCommonNNGEN, &LIBID_RFPCommonLib>,
	public IErrorInfo,
	public ICDSSFunctionObject4,
	public ICDSSNNGenericFunction
{
public:
	CRFPCommonNNGEN();
	~CRFPCommonNNGEN();

DECLARE_REGISTRY_RESOURCEID(IDR_RFPCOMMON)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRFPCommonNNGEN)
	COM_INTERFACE_ENTRY(IRFPCommonNNGEN)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IErrorInfo)
	COM_INTERFACE_ENTRY(ICDSSFunctionObject)
	COM_INTERFACE_ENTRY(ICDSSFunctionObject2)
	COM_INTERFACE_ENTRY(ICDSSFunctionObject4)
	COM_INTERFACE_ENTRY(ICDSSNNGenericFunction)
END_COM_MAP()

// IRFPCommonNNGEN
public:
// IErrorInfo
	STDMETHOD(GetDescription)(BSTR *pDesc);
	STDMETHOD(GetGUID)(struct _GUID *);
	STDMETHOD(GetSource)(BSTR *);
	STDMETHOD(GetHelpFile)(BSTR *);
	STDMETHOD(GetHelpContext)(unsigned long *);
// ICDSSFunctionObject2
	STDMETHOD(get_FunctionType)(EnumDSSFunctionType *pType);
	STDMETHOD(SetProperty)(Int_32 Index, VARIANT *pValue);
	STDMETHOD(get_Description)(BSTR *pDescription);
	STDMETHOD(get_ParamCount)(Int_32 *pInCount, Int_32 *pOutCount, Int_32 *pRepeatCount);
	STDMETHOD(get_ParamInfo)(Int_32 ParamIndex, Int_32 InParameter, BSTR *pName, enum EnumDSSParameterType *pType);
	STDMETHOD(get_SqlType)(enum EnumDSSFunctionSQLType *pSqlType);
	STDMETHOD(get_PropertyCount)(Int_32 *pCount);
	STDMETHOD(get_PropertyInfo)(Int_32 PropertyIndex, BSTR *pName, BSTR *pDescription, VARIANT *pValue);
// ICDSSFunctionObject4
	STDMETHOD(get_IsOrderSignificant)(VARIANT_BOOL *pIsOrderSignificant);
// ICDSSNNGenericFunction
	STDMETHOD(Calculate)();
	STDMETHOD(GetOutput)(Int_32 nSize, Int_32 *nDataSize, VARIANT *pData, DSSData_Flags *pFlag);
	STDMETHOD(GetOutputType)(Int_32 nSize, EnumDSSDataType *pInputType, EnumDSSDataType *pOutputDataType, EnumDSSParameterType *pOutputParamType);
	STDMETHOD(SetInputParam)(Int_32 nParamIndex, Int_32 nSize, EnumDSSDataType dataType, VARIANT *pData, DSSData_Flags *pFlag);
	STDMETHOD(GetInputParamType)(Int_32 nParamIndex, EnumDSSParameterType *pType);
	STDMETHOD(Clear)();
	STDMETHOD(IsNullProcessable)(VARIANT_BOOL * pCanProcessNULL)
	{
		if (pCanProcessNULL == NULL)
			return E_POINTER;
			
		*pCanProcessNULL = m_bIsNullProcessable;

		return S_OK;
	}
	STDMETHOD(Init)(Int_32 lFunctionIndex, CRFPCommonPKG *pPKG);
// support functions
	STDMETHOD(ConvertFromVariant)(VARIANT *pVar, enum EnumDSSDataType iType, BYTE *pData);
	STDMETHOD(getParamFromVariant)(VARIANT *pParam, Int_32 index, enum EnumDSSDataType type, BYTE *outResult);
private:
	Int_32			m_lFunctionIndex;						// current function index
	wchar_t			m_sErrMsg[ERRMSG_BUFSIZE];				// function plug-in error message buffer
	CRFPCommonPKG	*m_pPKG;								// pointer to FP package object
	CRSupport_FO	*m_pRSuppFO;							// pointer to FO RSupport object
	Int_32			m_lInputSize;							// number of input rows
	Int_32			m_lInputCnt;							// number of input columns
	bool			m_bParseOK;								// R script parse flag
	char			*m_sRScriptFile;						// value of _RScriptFile property (may be absolute or relative)
	char			*m_sWorkingDir;							// value of _WorkingDir property
	char			*m_sOutputVar;							// value of _OutputVar property
	char			*m_sInputNames;							// value of _InputNames property
	VARIANT_BOOL	m_bIsNullProcessable;					// value of _AllowNulls property
	bool			m_bCheckInputCnt;						// value of _CheckInputCount property
	CComVariant		m_vPropValues[RFP_USER_FUNCPARAM_CNT];	// user property values
	char			*m_szReposRScriptFile;					// used if accessing script from repository
	char			*m_szErrLog;							// logfile (absolute path, including filename)
	char			*m_szErrLogBkup;						// backup logfile (absolute path, including filename)
	IOBUFFER_PAIR	m_ioBuffer;								// buffer used to pass strings to R

	Int_32 CurrentDateTime(char *pBuffer, Int_32 lBufSize)
	{
		time_t now = time(0);
		struct tm tstruct;
		if(!localtime_s(&tstruct, &now))
			return strftime(pBuffer, lBufSize, "%Y-%m-%d %X ", &tstruct);
		else
			return 0;
	}
	bool FolderExists(char *sFolder)
	{
		struct stat fInfo;
		if(sFolder && strlen(sFolder) > 0)
			return (stat(sFolder, &fInfo) == 0);
		else
			return false;
	}
	STDMETHOD(AllocRSuppFO)();
	STDMETHOD(InitFileNames)();
	void LogErrMsg(wchar_t *pErrMsg);
	STDMETHOD(ProcessInputNames)();
	STDMETHOD(ExRScript)(Int_32 nSize, Int_32 *nDataSize, VARIANT *pData, DSSData_Flags *pFlag);
};

#endif //__FPCLASSES_H_
