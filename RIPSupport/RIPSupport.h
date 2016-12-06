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
//		File:	RIPSupport.h
//
//		Desc:	Definition of CRSupport class.
//=======================================================================

#ifndef __RIPSUPPORT_H_
#define __RIPSUPPORT_H_
#include "StdAfx.h"
#undef Int32
#include <R.h>
#include <Rinternals.h>
#include <Rembedded.h>

#ifdef WIN32
#include <R_ext/Boolean.h>
extern Rboolean R_Interactive;
#else
#define CSTACK_DEFNS 1
#include <Rinterface.h>
#endif

#define RIPEXP
#include "RIPTypes.h"

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
typedef void (*p_SET_FRAME)(SEXP, SEXP);
typedef void (*p_SET_ENCLOS)(SEXP, SEXP);
typedef void (*p_SET_HASHTAB)(SEXP, SEXP);
typedef void (*p_SET_ATTRIB)(SEXP, SEXP);
typedef const char * (*pRf_translateChar)(SEXP);
typedef double *(*p_REAL)(SEXP x);
typedef double (*pRf_asReal)(SEXP x);
typedef R_len_t (*pRf_length)(SEXP);
typedef SEXP (*pRf_protect)(SEXP);
typedef SEXP (*pRf_eval)(SEXP, SEXP);
typedef SEXP (*pRf_lang1)(SEXP);
typedef SEXP (*pRf_lang2)(SEXP, SEXP);
typedef SEXP (*pRf_lang3)(SEXP, SEXP, SEXP);
typedef SEXP (*pRf_lang4)(SEXP, SEXP, SEXP, SEXP);
typedef SEXP (*pRf_ScalarInteger)(int);
typedef SEXP (*pRf_findVar)(SEXP, SEXP);
typedef SEXP (*pRf_install)(const char *);
typedef SEXP (*pRf_mkChar)(const char *);
typedef SEXP (*pRf_mkString)(const char *);
typedef SEXP (*pR_tryEval)(SEXP, SEXP, int *);
typedef SEXP (*pRf_allocVector)(SEXPTYPE, R_len_t);
typedef SEXP (*pRf_allocMatrix)(SEXPTYPE, int, int);
typedef SEXP (*pRf_allocSExp)(SEXPTYPE);
typedef SEXP (*p_STRING_ELT)(SEXP x, int i);
#ifdef WIN32
typedef int (*pRf_initEmbeddedR)(int argc, char *argv[]);
#else
typedef int (*pRf_initialize_R)(int ac, char **av);
typedef void (*p_setup_Rmainloop)(void);
#endif

enum EnumDefState { dsNotFound, dsParsing, dsFound };
enum EnumRVarType { rvtInput, rvtOutput, rvtParameter };
//DE43238 provide support for widechar to utf8 conversion
namespace RBase
{
	// Size (in chars) of a longest UTF-8 sequence.
	// since we allow only first 64K Unicode values in this implementation,
	// max UTF-8 sequence length is 4, (Unicode v.3 allows up to 4 octets)
	const size_t MAX_UTF8_SEQUENCE_LENGTH = 4;
	//
	// symbolic names for octet bit-masks
	//
	const unsigned char BYTE_10000000 = 0x80;
	const unsigned char BYTE_11000000 = 0xC0;
	const unsigned char BYTE_11100000 = 0xE0;
	const unsigned char BYTE_11110000 = 0xF0;
	const unsigned char BYTE_11111000 = 0xF8;
	const unsigned char BYTE_11111100 = 0xFC;
	const unsigned char BYTE_11111110 = 0xFE;

	const unsigned char BYTE_00111111 = 0x3F;
	const unsigned char BYTE_00011111 = 0x1F;
	const unsigned char BYTE_00001111 = 0x0F;
	const unsigned char BYTE_00000111 = 0x07;
	const unsigned char BYTE_00000011 = 0x03;
	const unsigned char BYTE_00000001 = 0x01;
	// Unicode defines this character to be used whenever conversion
	// to/from other encodings fails
	const unsigned short UNICODE_REPLACEMENT_CHARACTER = 0xFFFD;
	char* WideCharToUTF8(const wchar_t* ipWide, unsigned int iLength, wchar_t *pErrMsg, bool ibUseSubstitutionCharacter = false);
	bool IsIllegalUnicodeValue(unsigned int iUnicodeValue);
	unsigned int WideCharToUnicodeValue(wchar_t iWideChar, wchar_t *pErrMsg, bool ibUseSubstitutionCharacter);
	#ifdef WIN32
	#include <crtdbg.h>
	#else
	#include <assert.h>

	#define _ASSERT(x)	assert(x)
	#endif
}

// PKG level class, shared by all executing function objects
class CRSupport : public IRIPSupport
{
private:
	wchar_t				*m_sRInstallPath;				// root of R installation
#ifdef WIN32
	HINSTANCE			m_hRLib;						// handle for R library (Windows)
#else
	void				*m_hRLib;						// handle for R library (Unix/Linux)
#endif
	bool				m_bRInitialized;				// flag indicating state of R initialization
	wchar_t				m_sErrMsg[ERRMSG_BUFSIZE];		// error message buffer (R initialization error)

	pRf_protect			m_Rf_protect_ptr;
	pRf_lang1			m_lang1_ptr;
	pRf_lang2			m_lang2_ptr;
	pRf_lang3			m_lang3_ptr;
	pRf_lang4			m_lang4_ptr;
	pRf_install			m_install_ptr;
	pRf_mkChar			m_mkChar_ptr;
	pRf_mkString		m_mkString_ptr;
	pR_tryEval			m_R_tryEval_ptr;
	pRf_unprotect		m_Rf_unprotect_ptr;
	pRf_PrintValue		m_Rf_PrintValue_ptr;
	pRf_endEmbeddedR	m_Rf_endEmbeddedR_ptr; 
	pRf_allocVector		m_Rf_allocVector_ptr;
	pRf_allocMatrix		m_Rf_allocMatrix_ptr;
	pRf_allocSExp		m_Rf_allocSExp_ptr;
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
	SEXP				*m_R_NilValue_ptr;
	p_SET_STRING_ELT	m_SET_STRING_ELT_ptr;
	p_STRING_ELT		m_STRING_ELT_ptr;
	pRf_translateChar	m_translateChar_ptr;
	pR_finite			m_R_finite_ptr;
	pRf_nrows			m_Rf_nrows_ptr;
	double				*m_R_NaReal_ptr;
	int					*m_R_NaInt_ptr;
	int					*m_R_NaLogical_ptr;
	SEXP				*m_R_NaString_ptr;
	Rboolean			*m_R_Interactive_ptr;
#ifdef WIN32
	pRf_initEmbeddedR	m_Rf_initEmbeddedR_ptr;
#else
	pRf_initialize_R	m_Rf_initialize_R_ptr;
	uintptr_t			*m_R_CStackLimit_ptr;
	p_setup_Rmainloop	m_setup_Rmainloop_ptr;
#endif
	p_SET_FRAME			m_SET_FRAME_ptr;
	p_SET_ENCLOS		m_SET_ENCLOS_ptr;
	p_SET_HASHTAB		m_SET_HASHTAB_ptr;
	p_SET_ATTRIB		m_SET_ATTRIB_ptr;

	HRESULT InitR();
	void EndR();
#ifdef WIN32
	HRESULT GetRInstallPath();
	HRESULT LoadRLibrary(wchar_t *sLibPath, wchar_t *sLibName, bool bLoadingR)
	{
		const wchar_t	FUNC_NAME[]		= L"CRSupport::LoadRLibrary";
		size_t lPathLen = wcslen(m_sRInstallPath) + wcslen(sLibPath) + 5;
		wchar_t *sDllPath = new wchar_t [lPathLen];
		if(!sDllPath)
			return E_OUTOFMEMORY;
		RFP_WCSCPY(sDllPath, lPathLen, m_sRInstallPath);
		RFP_WCSCAT(sDllPath, lPathLen, sLibPath);
		if(!SetDllDirectory(sDllPath))
		{
			// error
			INIT_MSG2(L"SetDllDirectory failed for path '%s'.", sDllPath)
			delete[] sDllPath;
			sDllPath = NULL;
			return E_FAIL;
		}
		else
		{
			if(bLoadingR)
				m_hRLib = LoadLibrary(sLibName);
			else
				LoadLibrary(sLibName);
			delete[] sDllPath;
			sDllPath = NULL;

		}

		return S_OK;
	}
#endif
	
public:
	CRSupport();
	virtual ~CRSupport(void);
	bool InitOK() { return m_bRInitialized; }
	wchar_t *GetErrMsg() { return m_sErrMsg; }
	HRESULT SetupREnv(SEXP& REnv, const char *pName, wchar_t *pErrMsg);
	HRESULT ReadScript(const char *sScript, bool bIsURL, SEXP REnv, const char *sREnvName, SEXP& RScript, Int_32 *pLineCnt, wchar_t *pErrMsg);
	HRESULT ReadScriptLine(SEXP pRScript, Int_32 lLineNumber, char **ppScriptBuffer, size_t *pScriptBufLen);
	void RemRVar(const char *pName);
	void SetRVar(IRIPSupport_FO *pIRSuppFO, const char *pName, Int_32 nSize, double *pData, RFPData_Flags *pFlag);
	HRESULT SetRVarV(IRIPSupport_FO *pIRSuppFO, const char *pName, void *pValue, RFPData_Flags *pFlag);
	HRESULT SetRVarSA(IRIPSupport_FO *pIRSuppFO, const char *pName, Int_32 nSize, EnumRFPDataType eDataType, void *pData,
		RFPData_Flags *pFlag);
	HRESULT SetRVarSA_mx(IRIPSupport_FO *pIRSuppFO, const char *pName, Int_32 lPIndex, Int_32 nRows, Int_32 nCols, EnumRFPDataType eDataType,
		void *pData, RFPData_Flags *pFlag, bool bSafeArray, wchar_t *pErrMsg);
	void SetRVarStr(IRIPSupport_FO *pIRSuppFO, const char *pName, char *pData);
	void SetRVar_InputNames(IRIPSupport_FO *pIRSuppFO, std::vector<char *> &vInputNames);
	Int_32 SubmitR(IRIPSupport_FO *pIRSuppFO, wchar_t *pErrMsg);
	HRESULT GetRVarNN(IRIPSupport_FO *pIRSuppFO, const char *pName, Int_32 nSize, EnumRFPDataType eExpectedDT, EnumRFPParameterType eExpectedPT, Int_32 *pRSize,
		void *pResults, RFPData_Flags *pFlag, wchar_t *pErrMsg);
};

class CVarInfo : public IVarInfo
{
private:
	// commom member variables
	char					*m_sVarName;				// the variable name used in the R-Script
	EnumRVarType			m_eRVarType;				// default: input
	EnumRFPParameterType	m_eParamType;				// default: RfpParameterVector
	EnumRFPDataType			m_eDataType;				// default: RfpDataTypeUnknown (assume data type indicated by AE)
	bool					m_bDisabledVar;				// default: false
	// inputs
	bool					m_bRepeatInput;				// default: false
	// pre-defined parameters
	Int_32					m_lParamIndex;				// function parameter index (1-27 for pre-defined; -1 for user-defined)
	// user-defined parameters
	bool					m_bInitialized;				// non-initialized parameters will be initialized to default values
public:
	CVarInfo(void);
	virtual ~CVarInfo(void);
	void SetRVarType(EnumRVarType bRVarType) { m_eRVarType = bRVarType; }
	bool IsInput() { return (m_eRVarType==rvtInput); }
	bool IsParam() { return (m_eRVarType==rvtParameter); }
	bool IsPreDefined() { return (m_lParamIndex > 0); }
	void SetRepeatFlag(bool bRepeat) { m_bRepeatInput = bRepeat; }
	bool IsRepeat() { return m_bRepeatInput; }
	void SetDisabledFlag(bool bDisabled) { m_bDisabledVar = bDisabled; }
	bool IsDisabled() { return m_bDisabledVar; }
	void SetInitFlag() { m_bInitialized = true; }
	bool IsInitialized() { return m_bInitialized; }
	void PutParamType(EnumRFPParameterType eParamType) { m_eParamType = eParamType; }
	EnumRFPParameterType GetParamType() { return m_eParamType; }
	void PutDataType(EnumRFPDataType eDataType) { m_eDataType = eDataType; }
	EnumRFPDataType GetDataType() { return m_eDataType; }
	void PutParamIndex(Int_32 lIndex)
	{
		m_lParamIndex = lIndex;

		// set data type based on index
		if(lIndex < 10)
			m_eDataType = RfpDataTypeBool;
		else if(lIndex < 19)
			m_eDataType = RfpDataTypeDouble;
		else
			m_eDataType = RfpDataTypeVarChar;
	}
	Int_32 GetParamIndex() { return m_lParamIndex; }
	char *GetVarName() { return m_sVarName; }
	HRESULT CheckDataType(EnumRFPDataType eDataType);
	HRESULT PutVarName(char *pName)
	{
		size_t lSize = strlen(pName) + 1;
		m_sVarName = new char [lSize];
		if(!m_sVarName)
			return E_OUTOFMEMORY;
		RFP_STRCPY(m_sVarName, lSize, pName)
		return S_OK;
	}
};

typedef std::vector<CVarInfo *> VARINFO_VECTOR;

// function object level class (separate copy for each executing function object)
class CRSupport_FO : public IRIPSupport_FO
{
private:
	CRSupport				*m_pRSupp;							// pointer to package RSupport
	SEXP					m_pREnvironment;					// pointer to R environment for this function object
	char					m_szEnvName[RFP_ENVNAME_BUFLEN];	// unique environment name
	SEXP					m_pRScript;							// pointer to R script
	wchar_t					*m_sErrMsg;							// pointer to error message buffer
	EnumRFPType				m_eRFPType;							// R-FP type
	bool					m_bIsSorted;						// sorted flag
	VARINFO_VECTOR			m_vInputInfo;						// input variables
	VARINFO_VECTOR			m_vOutputInfo;						// output variables
	VARINFO_VECTOR			m_vParamInfo_pd;					// pre-defined parameters (Bool/Numeric/StringParam1-9)
	VARINFO_VECTOR			m_vParamInfo_ud;					// user-defined parameters (_Params)
	Int_32					m_lRepeatCnt;						// input repeat count
	STR_VECTOR				m_vInputNames;						// list of input names

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
		for(Int_32 i=0;i<(Int_32)m_vParamInfo_pd.size();i++)
		{
			if(strcmp(m_vParamInfo_pd[i]->GetVarName(), pVarName)==0)
				return true;
		}
		for(Int_32 i=0;i<(Int_32)m_vParamInfo_ud.size();i++)
		{
			if(strcmp(m_vParamInfo_ud[i]->GetVarName(), pVarName)==0)
				return true;
		}
		return false;
	}
	bool IsDupParam_pd(Int_32 lParamIndex)
	{
		for(Int_32 i=0;i<(Int_32)m_vParamInfo_pd.size();i++)
		{
			if(m_vParamInfo_pd[i]->GetParamIndex()==lParamIndex)
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
	CRSupport_FO(IRIPSupport *pRSupp, EnumRFPType eRFPType, bool bIsSorted, wchar_t *pErrMsg);
	virtual ~CRSupport_FO(void);
	bool InitOK() { return (m_pREnvironment != NULL); }
	//CRSupport *GetRSupp() { return m_pRSupp; }
	SEXP GetREnv() { return m_pREnvironment; }
	char *GetREnvName() { return m_szEnvName; }
	HRESULT ParseRScript(const char *sRScript, bool bIsURL, const char *sOutputVar);
	Int_32 GetInputCnt() { return (Int_32)m_vInputInfo.size(); }
	Int_32 GetRepeatCnt() { return m_lRepeatCnt; }
	Int_32 GetNonRepeatCnt() { return (Int_32)m_vInputInfo.size() - m_lRepeatCnt; }
	IVarInfo *GetInput(Int_32 lIndex) { return m_vInputInfo[lIndex]; }
	IVarInfo *GetParam_pd(Int_32 lIndex);
	IVarInfo *GetParam_ud(char *sVarName);
	IVarInfo *GetOutput() { return (IVarInfo *)m_vOutputInfo[0]; }
	STR_VECTOR & GetInputNames() { return m_vInputNames; }
	HRESULT InitParams_ud();
	void AddRepeatNames(Int_32 lTotalInCnt)
	{
		Int_32 lInputCnt = (Int_32)m_vInputInfo.size();
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
#endif // __RIPSUPPORT_H_
