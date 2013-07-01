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
//		File:	RSupport.h
//
//		Desc:	Definition of CRSupport class.
//=======================================================================

#ifndef __RSUPPORT_H_
#define __RSUPPORT_H_
#include "StdAfx.h"
#include <R.h>
#include <Rinternals.h>
#include <Rembedded.h>

// R type definitions
typedef int (*p_TYPEOF)(SEXP x);
typedef int (*pR_finite)(double);
typedef int (*pRf_nrows)(SEXP);
typedef int *(*p_INTEGER)(SEXP x);
typedef int *(*p_LOGICAL)(SEXP x);
typedef void (*pRf_unprotect)(int);
typedef void (*pRf_PrintValue)(SEXP);
typedef	void (*pRf_endEmbeddedR)(int fatal);
typedef void (*pRf_defineVar)(SEXP, SEXP, SEXP);
typedef void (*p_SET_STRING_ELT)(SEXP, int, SEXP);
typedef const char * (*pRf_translateChar)(SEXP);
typedef double *(*p_REAL)(SEXP x);
typedef double (*pRf_asReal)(SEXP x);
typedef R_len_t (*pRf_length)(SEXP);
typedef SEXP (*pRf_protect)(SEXP);
typedef SEXP (*pRf_eval)(SEXP, SEXP);
typedef SEXP (*pRf_lang2)(SEXP, SEXP);
typedef SEXP (*pRf_ScalarInteger)(int);
typedef SEXP (*pRf_findVar)(SEXP, SEXP);
typedef SEXP (*pRf_install)(const char *);
typedef SEXP (*pRf_mkChar)(const char *);
typedef SEXP (*pRf_mkString)(const char *);
typedef SEXP (*pR_tryEval)(SEXP, SEXP, int *);
typedef SEXP (*pRf_allocVector)(SEXPTYPE, R_len_t);
typedef SEXP (*pRf_allocMatrix)(SEXPTYPE, int, int);
typedef SEXP (*p_STRING_ELT)(SEXP x, int i);
typedef int (*pRf_initEmbeddedR)(int argc, char *argv[]);

enum enumDefState { dsNotFound, dsParsing, dsFound };

// PKG level class, shared by all executing function objects
class CRSupport
{
private:
	CComBSTR			m_sRInstallPath;				// root of R installation
	HINSTANCE			m_hRLib;						// handle for R.dll
	bool				m_bRInitialized;				// flag indicating state of R initialization
	wchar_t				m_sErrMsg[ERRMSG_BUFSIZE];		// error message buffer (R initialization error)

	pRf_protect			m_Rf_protect_ptr;
	pRf_lang2			m_lang2_ptr;
	pRf_install			m_install_ptr;
	pRf_mkChar			m_mkChar_ptr;
	pRf_mkString		m_mkString_ptr;
	pR_tryEval			m_R_tryEval_ptr;
	pRf_unprotect		m_Rf_unprotect_ptr;
	pRf_PrintValue		m_Rf_PrintValue_ptr;
	pRf_endEmbeddedR	m_Rf_endEmbeddedR_ptr; 
	pRf_allocVector		m_Rf_allocVector_ptr;
	pRf_allocMatrix		m_Rf_allocMatrix_ptr;
	p_REAL				m_REAL_ptr;
	p_INTEGER			m_INTEGER_ptr;
	p_LOGICAL			m_LOGICAL_ptr;
	pRf_defineVar		m_defineVar_ptr;
	pRf_asReal			m_asReal_ptr;
	pRf_ScalarInteger	m_ScalarInteger_ptr;
	pRf_length			m_length_ptr;
	p_TYPEOF			m_TYPEOF_ptr;
	pRf_findVar			m_findVar_ptr;
	pRf_eval			m_eval_ptr;
	SEXP				*m_R_GlobalEnv_ptr;
	SEXP				*m_R_UnboundValue_ptr;
	p_SET_STRING_ELT	m_SET_STRING_ELT_ptr;
	p_STRING_ELT		m_STRING_ELT_ptr;
	pRf_translateChar	m_translateChar_ptr;
	pR_finite			m_R_finite_ptr;
	pRf_nrows			m_pRf_nrows_ptr;
	double				*m_pNaReal_ptr;
	int					*m_pNaInteger_ptr;
	int					*m_pNaLogical_ptr;
	SEXP				*m_pNaString_ptr;
	pRf_initEmbeddedR	m_Rf_initEmbeddedR_ptr;

	STDMETHOD(InitR)();
	void EndR();
	STDMETHOD(GetRInstallPath)();
	void LoadRLibrary(wchar_t *sLibPath, wchar_t *sLibName, bool bLoadingR)
	{
		CComBSTR sDllPath = m_sRInstallPath;
		const wchar_t	FUNC_NAME[]		= L"CRSupport::LoadRLibrary";
		sDllPath.Append(sLibPath);
		if(!SetDllDirectory(sDllPath))
		{
			// error
			INIT_ERROR_MSG2(L"SetDllDirectory failed for path '%s'.", sDllPath)
		}
		else
		{
			if(bLoadingR)
				m_hRLib = LoadLibrary(sLibName);
			else
				LoadLibrary(sLibName);
		}
	}
public:
	CRSupport(void);
	virtual ~CRSupport(void);
	bool InitOK() { return m_bRInitialized; }
	wchar_t *GetErrMsg() { return m_sErrMsg; }
	void SetRVar(const char *pName, Int_32 nSize, double *pData, DSSData_Flags *pFlag);
	void SetRVar_rp(const char *pName, Int_32 nSize, Int_32 lPIndex, Int_32 lRepCnt, double *pData, DSSData_Flags *pFlag);
	STDMETHOD(SetRVar_mx)(const char *pName, Int_32 lPIndex, Int_32 nRows, Int_32 nCols, double *pData, DSSData_Flags *pFlag);
	STDMETHOD(SetRVarV)(const char *pName, VARIANT *pValue, DSSData_Flags *pFlag);
	STDMETHOD(SetRVarSA)(const char *pName, Int_32 nSize, EnumDSSDataType dataType, VARIANT *pData, DSSData_Flags *pFlag);
	STDMETHOD(SetRVarSA_rp)(const char *pName, Int_32 nSize, Int_32 lPIndex, Int_32 lInCnt, Int_32 lRepCnt,
		EnumDSSDataType dataType, VARIANT *pData, DSSData_Flags *pFlag);
	STDMETHOD(SetRVarSA_mx)(const char *pName, Int_32 lPIndex, Int_32 nRows, Int_32 nCols, EnumDSSDataType dataType,
		VARIANT *pData, DSSData_Flags *pFlag, bool bSafeArray, wchar_t *pErrMsg);
	void SetRVarStr(const char *pName, char *pData);
	void SetRVar_InputNames(std::vector<char *> &vInputNames);
	Int_32 SubmitR(const char *script);
	void GetRVar(const char *pName, Int_32 nSize, Int_32 *pRSize, double *pResult, DSSData_Flags *pFlag);
	STDMETHOD(GetRVarNN)(const char *pName, Int_32 nStrLen, Int_32 nSize, EnumDSSDataType eExpectedDT, Int_32 *pRSize,
		VARIANT *pResult, DSSData_Flags *pFlag, wchar_t *pErrMsg);
};

class CVarInfo
{
private:
	char					*m_sVarName;				// the variable name used in the R-Script
	char					*m_sFuncParamName;			// the name of one of the pre-defined function parameters
	bool					m_bInput;					// default: true (input)
	bool					m_bRepeatInput;				// default: false
	bool					m_bDisabledVar;				// default: false
	EnumDSSParameterType	m_eParamType;				// default: DssParameterVector
	EnumDSSDataType			m_eDataType;				// default: DssDataTypeUnknown (assume data type indicated by AE)
	Int_32					m_lParamIndex;				// pre-defined function parameter index
public:
	CVarInfo(void);
	virtual ~CVarInfo(void);
	void SetInputFlag(bool bInput) { m_bInput = bInput; }
	bool IsInput() { return m_bInput; }
	void SetRepeatFlag(bool bRepeat) { m_bRepeatInput = bRepeat; }
	bool IsRepeat() { return m_bRepeatInput; }
	void SetDisabledFlag(bool bDisabled) { m_bDisabledVar = bDisabled; }
	bool IsDisabled() { return m_bDisabledVar; }
	void PutParamType(EnumDSSParameterType eParamType) { m_eParamType = eParamType; }
	EnumDSSParameterType GetParamType() { return m_eParamType; }
	void PutDataType(EnumDSSDataType eDataType) { m_eDataType = eDataType; }
	EnumDSSDataType GetDataType() { return m_eDataType; }
	void PutParamIndex(Int_32 lIndex) { m_lParamIndex = lIndex; }
	Int_32 GetParamIndex() { return m_lParamIndex; }
	char *GetVarName() { return m_sVarName; }
	char *GetParamName() { return m_sFuncParamName; }
	STDMETHOD(CheckDataType)(EnumDSSDataType eDataType);
	STDMETHOD(PutVarName)(char *pName)
	{
		size_t lSize = strlen(pName) + 1;
		m_sVarName = new char [lSize];
		if(!m_sVarName)
			return E_OUTOFMEMORY;
		RFP_STRCPY(m_sVarName, lSize, pName)
		return S_OK;
	}
	STDMETHOD(PutParamName)(char *pName)
	{
		size_t lSize = strlen(pName) + 1;
		m_sFuncParamName = new char [lSize];
		if(!m_sFuncParamName)
			return E_OUTOFMEMORY;
		RFP_STRCPY(m_sFuncParamName, lSize, pName)
		return S_OK;
	}
};

typedef std::vector<CVarInfo *> VARINFO_VECTOR;

// function object level class (separate copy for each executing function object)
class CRSupport_FO
{
private:
	CRSupport					*m_pRSupp;				// pointer to package RSupport
	wchar_t						*m_sErrMsg;				// pointer to error message buffer
	EnumRFPType					m_eRFPType;				// R-FP type
	bool						m_bIsSorted;			// sorted flag
	VARINFO_VECTOR				m_vInputInfo;			// input variables
	VARINFO_VECTOR				m_vOutputInfo;			// output variables
	VARINFO_VECTOR				m_vParamInfo;			// function parameters
	Int_32						m_lRepeatCnt;			// input repeat count
	STR_VECTOR					m_vInputNames;			// list of input names

	bool IsRComment(char *buffer)
	{
		size_t lBufLen = strlen(buffer);
		for(Int32 i=0;i<lBufLen;i++)
		{
			if(buffer[i]==START_CMT)
			{
				if(i==(lBufLen-1))
					// the comment starter is the last character, so there is no reason to continue parsing
					return false;

				// more text following comment starter...replace comment stater with blank (so that it will be ignored when
				//	fetching first token) and return true so that this line will be parsed
				buffer[i] = SPACE_CHAR;
				return true;
			}
			else if((buffer[i] != SPACE_CHAR) && (buffer[i] != TAB_CHAR))
				return false;
			//else...must be a blank or tab...keep checking
		}

		// if here, # not found, so this is not a comment
		return false;
	}
	void ConvertToLower(char *pToken)
	{
		for(Int32 i=0;i<strlen(pToken);i++)
		{
			if((pToken[i] >= 'A') && (pToken[i] <= 'Z'))
				pToken[i] = pToken[i]-('Z'-'z');
			else if(pToken[i]=='–')
				pToken[i] = '-';
		}
	}
	bool IsDupVar(char *pVarName)
	{
		// check inputs
		for(Int_32 i=0;i<(Int_32)m_vInputInfo.size();i++)
		{
			if(strcmp(m_vInputInfo[i]->GetVarName(), pVarName)==0)
				return true;
		}
		// check output
		if((m_vOutputInfo.size() > 0) && (strcmp(pVarName, m_vOutputInfo[0]->GetVarName())==0))
			return true;
		// check function parameters
		for(Int_32 i=0;i<(Int_32)m_vParamInfo.size();i++)
		{
			if(strcmp(m_vParamInfo[i]->GetVarName(), pVarName)==0)
				return true;
		}
		return false;
	}
	bool IsDupParam(char *pParamName)
	{
		for(Int_32 i=0;i<(Int_32)m_vParamInfo.size();i++)
		{
			if(strcmp(m_vParamInfo[i]->GetParamName(), pParamName)==0)
				return true;
		}
		return false;
	}
	void AddInputNames()
	{
		for(Int_32 i=0;i<(Int_32)m_vInputInfo.size();i++)
		{
			m_vInputNames.push_back(m_vInputInfo[i]->GetVarName());
		}
	}
	bool IsReservedForR(char *pVarName);
	bool HasBadChar(char *pVarName);
public:
	CRSupport_FO(EnumRFPType eRFPType, bool bIsSorted, wchar_t *pErrMsg);
	virtual ~CRSupport_FO(void);
	void PutRSupp(CRSupport *pRSupp) { m_pRSupp = pRSupp; }
	bool InitOK();
	CRSupport *GetRSupp() { return m_pRSupp; }
	STDMETHOD(ParseRScript)(const char *sRScript, const char *sOutputVar);
	Int_32 GetInputCnt() { return m_vInputInfo.size(); }
	Int_32 GetRepeatCnt() { return m_lRepeatCnt; }
	Int_32 GetNonRepeatCnt() { return m_vInputInfo.size() - m_lRepeatCnt; }
	Int_32 GetParamCnt() { return m_vParamInfo.size(); }
	CVarInfo *GetInput(Int_32 lIndex) { return m_vInputInfo[lIndex]; }
	CVarInfo *GetParam(Int_32 lIndex);
	CVarInfo *GetOutput() { return m_vOutputInfo[0]; }
	STR_VECTOR & GetInputNames() { return m_vInputNames; }
	void AddRepeatNames(Int_32 lTotalInCnt)
	{
		Int_32 lInputCnt = m_vInputInfo.size();
		if(lTotalInCnt > lInputCnt)
		{
			Int_32 lRepeatNamesCnt = lTotalInCnt - lInputCnt;
			Int_32 lRepeatIndex = lInputCnt - m_lRepeatCnt;
			for(Int_32 i=0;i<lRepeatNamesCnt;i++)
			{
				m_vInputNames.push_back(m_vInputInfo[lRepeatIndex]->GetVarName());

				lRepeatIndex++;
				if(lRepeatIndex == lInputCnt)
					lRepeatIndex -= m_lRepeatCnt; 
			}
		}
		//else...no repeated inputs
	}
};
#endif // __RSUPPORT_H_
