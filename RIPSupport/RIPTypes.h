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
//		File:	RIPTypes.h
//
//		Desc:	MicroStategy-to-RIntegrationPack interfaces.
//=======================================================================

#ifndef __RIPTYPES_H_
#define __RIPTYPES_H_

// types
enum EnumRFPType { rfpUnknown, rfpSimple, rfpRelative, rfpAgg };
typedef std::vector<char *> STR_VECTOR;


// MicroStrategy type definitions
typedef enum EnumRFPDataFlags
{
    RfpDataOk = 0x00,
    RfpDataInvalid = 0x40
};

typedef enum EnumRFPDataType
{
    RfpDataTypeUnknown = -1,
    RfpDataTypeInteger = 1,
    RfpDataTypeDouble = 6,
    RfpDataTypeFloat = 7,
    RfpDataTypeVarChar = 9,
    RfpDataTypeShort = 21,
    RfpDataTypeBool = 24,
    RfpDataTypeUTF8Char = 33,
    RfpDataTypeLastOne
};

typedef enum EnumRFPParameterType
{
    RfpParameterReserved = 0x00,
    RfpParameterScalar = 0x01,
    RfpParameterVector = 0x02   
};

typedef BYTE RFPData_Flags;

class IRIPSupport_FO;

class IRIPSupport
{
public:
	virtual bool InitOK() = 0;
	virtual wchar_t *GetErrMsg() = 0;
	virtual void SetRVar(IRIPSupport_FO *pIRSuppFO, const char *pName, Int_32 nSize, double *pData, RFPData_Flags *pFlag) = 0;
	virtual HRESULT SetRVarV(IRIPSupport_FO *pIRSuppFO, const char *pName, void *pValue, RFPData_Flags *pFlag) = 0;
	virtual HRESULT SetRVarSA(IRIPSupport_FO *pIRSuppFO, const char *pName, Int_32 nSize, EnumRFPDataType eDataType, void *pData, RFPData_Flags *pFlag) = 0;
	virtual HRESULT SetRVarSA_mx(IRIPSupport_FO *pIRSuppFO, const char *pName, Int_32 lPIndex, Int_32 nRows, Int_32 nCols, EnumRFPDataType eDataType, void *pData, RFPData_Flags *pFlag, bool bSafeArray, wchar_t *pErrMsg) = 0;
	virtual void SetRVarStr(IRIPSupport_FO *pIRSuppFO, const char *pName, char *pData) = 0;
	virtual void SetRVar_InputNames(IRIPSupport_FO *pIRSuppFO, std::vector<char *> &vInputNames) = 0;
	virtual Int_32 SubmitR(IRIPSupport_FO *pIRSuppFO, wchar_t *pErrMsg) = 0;
	virtual HRESULT GetRVarNN(IRIPSupport_FO *pIRSuppFO, const char *pName, Int_32 nSize, EnumRFPDataType eExpectedDT, EnumRFPParameterType eExpectedPT, Int_32 *pRSize,
		void *pResults, RFPData_Flags *pFlag, wchar_t *pErrMsg) = 0;
};

class IVarInfo
{
public:
	virtual bool IsRepeat() = 0;
	virtual void SetInitFlag() = 0;
	virtual bool IsInitialized() = 0;
	virtual EnumRFPParameterType GetParamType() = 0;
	virtual void PutDataType(EnumRFPDataType eDataType) = 0;
	virtual EnumRFPDataType GetDataType() = 0;
	virtual char *GetVarName() = 0;
	virtual HRESULT CheckDataType(EnumRFPDataType eDataType) = 0;
};

class IRIPSupport_FO
{
public:
	virtual bool InitOK() = 0;
	virtual HRESULT ParseRScript(const char *sRScript, bool bIsURL, const char *sOutputVar) = 0;
	virtual Int_32 GetInputCnt() = 0;
	virtual Int_32 GetRepeatCnt() = 0;
	virtual Int_32 GetNonRepeatCnt() = 0;
	virtual IVarInfo *GetInput(Int_32 lIndex) = 0;
	virtual IVarInfo *GetParam_pd(Int_32 lIndex) = 0;
	virtual IVarInfo *GetParam_ud(char *sVarName) = 0;
	virtual IVarInfo *GetOutput() = 0;
	virtual STR_VECTOR & GetInputNames() = 0;
	virtual HRESULT InitParams_ud() = 0;
	virtual void AddRepeatNames(Int_32 lTotalInCnt) = 0;
};

#ifdef WIN32
#ifdef RIPEXP
#define IMPEXP __declspec(dllexport)
#else
#define IMPEXP __declspec(dllimport)
#endif  // RIPEXP
#else  // Unix/Linux
 #if __GNUC__ >=4
        #define IMPEXP __attribute__ ((__visibility__("default")))
 #else
        #define IMPEXP
 #endif

#endif  // WIN32

extern "C"
{
IMPEXP void *CreateRIPSupportInstance();
IMPEXP void DestroyRIPSupportInstance(IRIPSupport *pRSupp);
IMPEXP void *CreateRIPSupportFOInstance(IRIPSupport *pRSupp, EnumRFPType eRFPType, bool bIsSorted, wchar_t *pErrMsg);
IMPEXP void DestroyRIPSupportFOInstance(IRIPSupport_FO *pRSuppFO);
}

#endif // __RIPTYPES_H_
