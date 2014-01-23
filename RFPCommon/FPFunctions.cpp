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
//		File:	FPFunctions.cpp
//
//		Desc:	Implementation of CRFPCommonPKG & CRFPCommonNNGEN classes.
//=======================================================================

//=======================================================================
// includes
//=======================================================================
#include "StdAfx.h"
using namespace std;
#include "FPClasses.h"

//=======================================================================
// CRFPCommonPKG
//=======================================================================

/************************************************************************
FUNCTION:	CRFPCommonPKG::ReadRIPKeyValue()
INPUTS:		hKey			RIP key handle
			bstrKeyValue	key value name
			pKeyValue		pointer to returned key value
OUTPUTS:	none
RETURNS:	none			errors are ignored

This method fetches the specified RIP key value.  If an error occurs, an
error message will be saved, but execution will continue.  The error message
will be logged later, if possible.
*************************************************************************/
void CRFPCommonPKG::ReadRIPKeyValue(
	HKEY & hKey,
	BSTR bstrKeyValue,
	char **pKeyValue)
{
	DWORD			retCode;
	DWORD			dwValType;
	DWORD			dwSize							= KEY_BUFFER_SIZE;
	wchar_t			sKeyValue[KEY_BUFFER_SIZE];
	HRESULT			hr								= S_OK;
	const wchar_t	FUNC_NAME[]						= L"CRFPCommonPKG::ReadRIPKeyValue";

	try
	{
		// validate input
		if(!pKeyValue)
			throw E_INVALIDARG;
		*pKeyValue = NULL;

		// fetch the specified key value
		if(!(retCode=RegQueryValueEx(hKey, bstrKeyValue, 0, &dwValType, (BYTE *)&sKeyValue,
			&dwSize)))
		{
			// found it...validate its type
			if(dwValType==REG_SZ)
			{
				// expected type...check if folder specified
				size_t uiSrcLen = wcslen(sKeyValue);
				if(uiSrcLen > 0)
				{
					// folder specified...allocate destination buffer
					(*pKeyValue) = new char [RFP_BUFLEN(uiSrcLen)];
					if(!(*pKeyValue))
					{
						INIT_ERROR_MSG2(L"%s: Allocation error when processing registry key value '%s'.", bstrKeyValue)
						throw E_OUTOFMEMORY;
					}

					// convert wchar to char
					RFP_WCSTOMBS((*pKeyValue), RFP_BUFLEN(uiSrcLen), sKeyValue)
				}
				//else...no folder specified
			}
			else
			{
				// unexpected type
				INIT_ERROR_MSG2(L"%s: Registry key value '%s' not of type REG_SZ.", bstrKeyValue)
				throw E_FAIL;
			}
		}
		else
		{
			// key value not found
			INIT_ERROR_MSG2(L"%s: Registry key value '%s' not found.", bstrKeyValue)
			throw E_FAIL;
		}
	}
	catch(HRESULT catchHR)
	{
		hr = catchHR;
		INIT_HR_MSG(L"")
	}
	catch(...)
	{
		hr = E_FAIL;
		INIT_HR_MSG(L"Unhandled exception.")
	}
}

/************************************************************************
FUNCTION:	CRFPCommonPKG::ReadRIPRegEntries()
INPUTS:		none
OUTPUTS:	none
RETURNS:	none

This method reads the R Integration Pack registry entries.  If an error occurs,
an error message will be saved, but execution will continue.  The error message
will be logged later, if possible.
*************************************************************************/
void CRFPCommonPKG::ReadRIPRegEntries()
{
	CComBSTR		bstrKeyRIP						= L"SOFTWARE\\MicroStrategy\\R Integration Pack";
	CComBSTR		bstrValueInstallPath			= L"InstallPath";
	CComBSTR		bstrValueRScriptsFolder			= L"RScriptsFolder";
	HRESULT			hr								= S_OK;
	const wchar_t	FUNC_NAME[]						= L"CRFPCommonPKG::ReadRIPRegEntries";

	try
	{
		HKEY			hKey;

		// open MSTR installation key
		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, bstrKeyRIP, 0, KEY_READ, &hKey))
		{
			// key not found
			INIT_ERROR_MSG2(L"%s: Registry key '%s' not found.", bstrKeyRIP)
			throw E_FAIL;
		}

		// fetch the InstallPath value
		ReadRIPKeyValue(hKey, bstrValueInstallPath.m_str, &m_szInstallFolder);

		// fetch the RScriptsFolder value
		ReadRIPKeyValue(hKey, bstrValueRScriptsFolder.m_str, &m_szRScriptsFolder);

		if(m_szInstallFolder)
		{
			// init default RScripts folder (<InstallPath>/RScripts)
			Int_32 lLength = strlen(m_szInstallFolder) + strlen(RFP_PATH_DELIM) + strlen(RFP_RSCRIPTS) + 1;
			m_szDefRScriptsFolder = new char [lLength];
			if(m_szDefRScriptsFolder)
			{
				RFP_STRCPY(m_szDefRScriptsFolder, lLength, m_szInstallFolder)
				RFP_STRCAT(m_szDefRScriptsFolder, lLength, RFP_PATH_DELIM)
				RFP_STRCAT(m_szDefRScriptsFolder, lLength, RFP_RSCRIPTS)
			}
			else
			{
				INIT_ERROR_MSG(L"Allocation error when initializing default R script repository.")
			}
		}
		//else...InstallPath not valid...cannot access default repository
	}
	catch(HRESULT catchHR)
	{
		hr = catchHR;
		INIT_HR_MSG(L"")
	}
	catch(...)
	{
		hr = E_FAIL;
		INIT_HR_MSG(L"Unhandled exception.")
	}
}

/************************************************************************
FUNCTION:	CRFPCommonPKG::AllocRSupp()
INPUTS:		pErrMsg		pointer to error message buffer
OUTPUTS:	none
RETURNS:	pointer to new or existing CRSupport object

This method allocates and initializes CRSupport object, if it does not
already exist.

The RIP registry entries are also read here.  While not need to allocate the
RSupport object, this is a convenient place to make the call because it allows
registry errors to be corrected without requiring a restart of the I-Server.
*************************************************************************/
CRSupport *CRFPCommonPKG::AllocRSupp(
	wchar_t *pErrMsg)
{
	HRESULT			hr				= S_OK;
	const wchar_t	FUNC_NAME[]		= L"CRFPCommonPKG::AllocRSupp";

	try
	{
		if(!m_pRSupp)
		{
			//  first time, or error occurred last time...allocate object
			m_pRSupp = new CRSupport();
			if(m_pRSupp)
			{
				// check status
				if(!m_pRSupp->InitOK())
				{
					// error during CRSupport initialization...fetch error message
					RFP_WCSCPY(pErrMsg, ERRMSG_BUFSIZE, m_pRSupp->GetErrMsg())
				
					// release RSupport object
					delete m_pRSupp;
					m_pRSupp = NULL;
				}
			}
		}
		//else...already allocated...return existing pointer
	}
	catch(HRESULT catchHR)
	{
		hr = catchHR;
		INIT_HR_MSG_REF(L"")
	}
	catch(...)
	{
		hr = E_FAIL;
		INIT_HR_MSG_REF(L"Unhandled exception.")
	}

	return m_pRSupp;
}

//=======================================================================
// CRFPCommonNNGEN
//=======================================================================

/************************************************************************
FUNCTION:	CRFPCommonNNGEN::AllocRSuppFO()
INPUTS:		none
OUTPUTS:	none
RETURNS:	S_OK
			E_FAIL		error during construction of CRSupport object
						error during construction of CRSupport_FO object

This method sets up the CRSupport_FO object.
*************************************************************************/
STDMETHODIMP CRFPCommonNNGEN::AllocRSuppFO()
{
	HRESULT			hr				= S_OK;
	const wchar_t	FUNC_NAME[]		= L"CRFPCommonNNGEN::AllocRSuppFO";

	try
	{
		// initialize script and logging filenames
		hr = InitFileNames();
		if(FAILED(hr)) throw hr;

		// construct FO object
		m_pRSuppFO = new CRSupport_FO(
			(m_lFunctionIndex==1) ? rfpSimple : (m_lFunctionIndex==2 || m_lFunctionIndex==3) ? rfpRelative : rfpAgg,
			(m_lFunctionIndex==3 || m_lFunctionIndex==5) ? true : false,
			m_sErrMsg);
		if(!m_pRSuppFO)
		{
			INIT_ERROR_MSG(L"Error during CRSupport_FO construction.")
			throw E_FAIL;
		}

		// fetch package RSupport (will allocate if necessary)
		CRSupport *pRSupp = m_pPKG->AllocRSupp(m_sErrMsg);
		if(pRSupp)
			// save pointer to CRSupport in FO object
			m_pRSuppFO->PutRSupp(pRSupp);
		else
		{
			INIT_ERROR_MSG(L"Error during setup of CRSupport object.")
			throw E_FAIL;
		}

		// check for registry error
		if(wcslen(m_pPKG->GetErrMsg()) > 0)
		{
			// non-critical error occurred while processing registry...log and clear message
			LogErrMsg(m_pPKG->GetErrMsg());
			m_pPKG->ClearErrMsg();
		}
	}
	catch(HRESULT catchHR)
	{
		hr = catchHR;
		INIT_HR_MSG(L"Error during setup of CRSupport_FO object.")
	}
	catch(...)
	{
		hr = E_FAIL;
		INIT_HR_MSG(L"Unhandled exception.")
	}

	return hr;
}

/************************************************************************
FUNCTION:	CRFPCommonNNGEN::InitFileNames()
INPUTS:		none
OUTPUTS:	none
RETURNS:	S_OK
			E_OUTOFMEMORY		allocation error
			E_FAIL				no repository specified for relative script name

This method creates the error log file names, which are stored in member
variables for later use.  The following locations may be used for logging
(if available, chosen in the listed order):

	* working directory (only if specified by _WorkingDir parameter)
	* script location
		- as indicated by _RScriptFile (only if path provided)
		- as indicated by RScriptsFolder key value
		- default RScripts folder (<RIP install dir>\RScripts)
	* RIP installation folder

In addition, the _RScriptFile parameter is parsed to determine whether or
not a path precedes the filename.  If no path is found, an abosulte filename
is contructed, based on the specified script repository.

If the working directory was not set via the _WorkingDir parameter, it will
be set to the logging folder.
*************************************************************************/
STDMETHODIMP CRFPCommonNNGEN::InitFileNames()
{
	HRESULT			hr				= S_OK;
	const wchar_t	FUNC_NAME[]		= L"CRFPCommonNNGEN::InitFileNames";

	try
	{
		// initialize storage for script path
		char szRScriptPath [_MAX_PATH];
		szRScriptPath[0] = NULL_CHAR;

		// determine if _RScriptFile includes a path
		char szDrive[_MAX_DRIVE];
		char szDir[_MAX_DIR];
		_splitpath_s(m_sRScriptFile, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, NULL, 0, NULL, 0);

		// check if path found
		Int_32 lDriveLen = strlen(szDrive);
		Int_32 lDirLen = strlen(szDir);
		if(lDriveLen + lDirLen > 0)
		{
			// path found...construct path, which may be used for logging
			if(lDriveLen)
			{
				strcpy_s(szRScriptPath, _MAX_PATH, szDrive);
				if(lDirLen)
					strcat_s(szRScriptPath, _MAX_PATH, szDir);
			}
			else
				strcpy_s(szRScriptPath, _MAX_PATH, szDir);

			if(lDirLen > 1)
				// not root...delete trailing slash
				szRScriptPath[strlen(szRScriptPath) - 1] = NULL_CHAR;
		}

		// validate repository folder
		char *pRepository = NULL;
		if(FolderExists(m_pPKG->GetRScriptsFolder()))
			// use repository specified in registry
			pRepository = m_pPKG->GetRScriptsFolder();
		else if(FolderExists(m_pPKG->GetDefRScriptsFolder()))
			// use default repository
			pRepository = m_pPKG->GetDefRScriptsFolder();

		// determine where to log
		char *pLogFileFolder = NULL;
		if(FolderExists(m_sWorkingDir))
			// use working directory for logging
			pLogFileFolder = m_sWorkingDir;
		else if(FolderExists(szRScriptPath))
			// use script path for logging
			pLogFileFolder = szRScriptPath;
		else if(pRepository)
			// use script repository for logging
			pLogFileFolder = pRepository;
		else if(FolderExists(m_pPKG->GetInstallFolder()))
			// use install folder for logging
			pLogFileFolder = m_pPKG->GetInstallFolder();

		if(!m_sWorkingDir && pLogFileFolder)
		{
			// working directory not specified...default to logging folder
			Int32 lLength = strlen(pLogFileFolder) + 1;
			m_sWorkingDir = new char [lLength];
			if(!m_sWorkingDir) throw E_OUTOFMEMORY;
			RFP_STRCPY(m_sWorkingDir, lLength, pLogFileFolder)
		}

		if(pLogFileFolder)
		{
			// usable folder found for logging...allocate storage for primary and backup logfile names
			Int_32 lSrcLen = strlen(pLogFileFolder);
			Int_32 lDestLen1 = lSrcLen + strlen(RFP_PATH_DELIM) + strlen(RFP_ERRLOG) + 1;
			Int_32 lDestLen2 = lSrcLen + strlen(RFP_PATH_DELIM) + strlen(RFP_ERRLOG_BKUP) + 1;
			m_szErrLog = new char [lDestLen1];
			if(m_szErrLog)
			{
				m_szErrLogBkup = new char [lDestLen2];
				if(!m_szErrLogBkup)
				{
					// allocation failure...release primary buffer
					delete [] m_szErrLog;
					m_szErrLog = NULL;
				}
			}

			if(m_szErrLog)
			{
				// create logfile names
				RFP_STRCPY(m_szErrLog, lDestLen1, pLogFileFolder)
				RFP_STRCAT(m_szErrLog, lDestLen1, RFP_PATH_DELIM)
				RFP_STRCAT(m_szErrLog, lDestLen1, RFP_ERRLOG)
				RFP_STRCPY(m_szErrLogBkup, lDestLen2, pLogFileFolder)
				RFP_STRCAT(m_szErrLogBkup, lDestLen2, RFP_PATH_DELIM)
				RFP_STRCAT(m_szErrLogBkup, lDestLen2, RFP_ERRLOG_BKUP)
			}
		}

		if(strlen(szRScriptPath)==0)
		{
			// _RScriptFile did not include path...check for repository
			if(pRepository)
			{
				// valid repository found...allocate buffer and construct full name
				Int_32 lLength = strlen(pRepository) + strlen(m_sRScriptFile) + 2;
				m_szReposRScriptFile = new char [lLength];
				if(!m_szReposRScriptFile)
				{
					INIT_ERROR_MSG(L"Allocation error when processing R script repository location.")
					throw E_OUTOFMEMORY;
				}
				RFP_STRCPY(m_szReposRScriptFile, lLength, pRepository)
				RFP_STRCAT(m_szReposRScriptFile, lLength, RFP_PATH_DELIM)
				RFP_STRCAT(m_szReposRScriptFile, lLength, m_sRScriptFile)
			}
			else
			{
				// no repository, and _RScriptFile did not include path...cannot find script
				INIT_ERROR_MSG(L"Valid script repository must be specified if using relative R script filename.")
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

/************************************************************************
FUNCTION:	CRFPCommonNNGEN::LogErrMsg()
INPUTS:		pErrMsg		pointer to error message
OUTPUTS:	none
RETURNS:	none

This method logs the specified message to the error log file, if a usable
folder was found by InitFileNames().
*************************************************************************/
void CRFPCommonNNGEN::LogErrMsg(
	wchar_t *pErrMsg)
{
	HRESULT		hr	= S_OK;

	try
	{
		Int_32 lMsgLen = wcslen(pErrMsg);
		if(lMsgLen > 0)
		{
			// there is an error to be logged...check if logfile names were created
			if(m_szErrLog)
			{
				// file names created...allocate message buffer
				Int_32 lBufSize = lMsgLen + RFP_DATETIME_SIZE;
				char *szBuffer = new char [RFP_BUFLEN(lBufSize)];
				if(szBuffer)
				{
					// initialize message buffer with date/time
					Int_32 lLength = CurrentDateTime(szBuffer, lBufSize);

					// convert wchar error message to char
					char *pTemp = &szBuffer[lLength];
					RFP_WCSTOMBS(pTemp, RFP_BUFLEN(lBufSize)-lLength, pErrMsg)

					// add newline
					RFP_STRCAT(szBuffer, lBufSize, NEWLINE)

					// check for logfile
					bool bLoggingDisabled = false;
					struct stat fInfo;
					if(stat(m_szErrLog, &fInfo)==0)
					{
						// found file...check its size
						if(fInfo.st_size > RFP_ERRLOG_LIMIT)
						{
							// logfile exceeds limit...check for backup copy
							if(stat(m_szErrLogBkup, &fInfo)==0)
								// found backup logfile...delete it and set disable flag appropriately
								bLoggingDisabled = (remove(m_szErrLogBkup) != 0);

							if(!bLoggingDisabled)
								// rename logfile to backup
								bLoggingDisabled = (rename(m_szErrLog, m_szErrLogBkup) != 0);
						}
						//else...logfile limit not exceeded...new log will be appended to existing logfile
					}
					//else...file does not exist...logging will create new file

					if(!bLoggingDisabled)
					{
						// open stream and write message
						ofstream ofs(m_szErrLog, ios::app);
						if(ofs.is_open())
						{
							ofs.write(szBuffer, strlen(szBuffer));
							ofs.close();
						}
						//else...logfile failed to open
					}
					//else...could not delete old backup or rename current logfile...skip logging

					// release buffer
					delete [] szBuffer;
				}
				//else...buffer allocation failed
			}
			//else...logfile not available
		}
		//else...error message buffer is empty
	}
	catch(...)
	{
	}
}

/************************************************************************
FUNCTION:	CRFPCommonNNGEN::ProcessInputNames()
INPUTS:		none
OUTPUTS:	none
RETURNS:	S_OK
			E_FAIL		unhandled exception

This method processes the _InputNames function parameter.  A vector of
input names is constructed, either from the _InputNames parameter (if valid)
or from the variable names defined within the R script.  The vector is used
to initialize the mstr.InputNames variable in the R environment.
*************************************************************************/
STDMETHODIMP CRFPCommonNNGEN::ProcessInputNames()
{
	HRESULT			hr				= S_OK;
	const wchar_t	FUNC_NAME[]		= L"CRFPCommonNNGEN::ProcessInputNames";

	try
	{
		Int_32 lLength = m_sInputNames ? strlen(m_sInputNames) : 0;
		STR_VECTOR vInputNames;
		bool bValidList = true;
		if(lLength > 0)
		{
			// user has provided input names...create vector of names, eliminating any brackets
			bool bInBrackets = false;
			char *sName = m_sInputNames;
			char *sNext = m_sInputNames;
			for(Int_32 i=0;i<lLength;i++)
			{
				if(sNext[0]==OPENBRACKET_CHAR)
				{
					if(bInBrackets)
					{
						// invalid list
						LogErrMsg(L"Invalid _InputNames value:  Nested brackets not allowed.");
						bValidList = false;
						break;
					}
					else if(sName!=sNext)
					{
						// extra chars in from on open bracket...invalid list
						LogErrMsg(L"Invalid _InputNames value:  Missing comma separator before opening bracket.");
						bValidList = false;
						break;
					}

					// set to beginning of next name
					sName++;
					bInBrackets = true;
				}
				else if(sNext[0]==CLOSEBRACKET_CHAR)
				{
					if(!bInBrackets)
					{
						// invalid list
						LogErrMsg(L"Invalid _InputNames value:  Missing opening bracket.");
						bValidList = false;
						break;
					}

					// terminate current name (name will be added when next common encountered)
					sNext[0] = NULL_CHAR;

					// reset flag
					bInBrackets = false;
				}
				else if(sNext[0]==SPACE_CHAR)
				{
					if(!bInBrackets && (sName==sNext))
						// skip leading spaces
						sName++;
				}
				else if(sNext[0]==COMMA_CHAR)
				{
					if(!bInBrackets)
					{
						// terminate current name
						sNext[0] = NULL_CHAR;

						if(strlen(sName)==0)
						{
							// zero-length names not allowed
							LogErrMsg(L"Invalid _InputNames value:  Zero-length names not allowed.");
							bValidList = false;
							break;
						}

						// add name to list
						vInputNames.push_back(sName);
						sName = sNext + 1;
					}
				}
				sNext++;
			}

			if(bInBrackets)
			{
				// unmatched opening bracket...invalid list
				LogErrMsg(L"Invalid _InputNames value:  Missing closing bracket.");
				bValidList = false;
			}

			if(bValidList)
			{
				if(strlen(sName) > 0)
					// add last name to list
					vInputNames.push_back(sName);
			}
		}

		if(bValidList && (vInputNames.size()==m_lInputCnt))
		{
			// user provided valid list of names...use it
			m_pRSuppFO->GetRSupp()->SetRVar_InputNames(vInputNames);
		}
		else
		{
			// use R variable names instead
			if(bValidList && (lLength != 0))
				// valid list provided, but incorrect count
				LogErrMsg(L"Invalid _InputNames value:  Number of names does not match the actual count.");
			//else...either no list provided (don't log error) or list was invalid (error already logged)

			// added repeated inputs to input names list
			m_pRSuppFO->AddRepeatNames(m_lInputCnt);

			// send names to R
			m_pRSuppFO->GetRSupp()->SetRVar_InputNames(m_pRSuppFO->GetInputNames());
		}
	}
	catch(...)
	{
		hr = E_FAIL;
	}

	return hr;
}

/************************************************************************
FUNCTION:	CRFPCommonNNGEN::ExRScript()
INPUTS:		nSize		number of elements in each "m1_" member variable
			nDataSize	number of elements returned in pResult
			pResult		pointer to one-dimensional array of result
			pFlag		pointer to one-dimensional array of result flags
OUTPUTS:	none
RETURNS:	S_OK
			E_FAIL		script execution error
						failure when fetching results or script error message
						unsupported output data type

This method executes the specified R script.
*************************************************************************/
STDMETHODIMP CRFPCommonNNGEN::ExRScript(
	Int_32 nSize,
	Int_32 *nDataSize,
	VARIANT *pResult,
	DSSData_Flags *pFlag)
{
	Int_32			lRSize;
	CVarInfo		*pVI;
	HRESULT			hr			= S_OK;
	const wchar_t	FUNC_NAME[]	= L"CRFPCommonNNGEN::ExRScript";
	BLDSTR_DECL()

	try
	{
		// access package RSupport
		CRSupport *pRSupp = m_pRSuppFO->GetRSupp();

		// initialize the MSTR error message buffer and execution flag
		CComVariant vBuffer = L"";
		pRSupp->SetRVarV(RFP_ERRMSG, &vBuffer, NULL);
		double dblFlag = 1;
		pRSupp->SetRVar(RFP_EXFLAG, 1, &dblFlag, NULL);

		// submit R script
		Int32 lErrCode = pRSupp->SubmitR(m_szReposRScriptFile ? m_szReposRScriptFile : m_sRScriptFile);

		// check for script error message
		VARIANT vScriptErrMsg;
		vScriptErrMsg.vt = VT_BSTR;
		vScriptErrMsg.bstrVal = NULL;
		hr = pRSupp->GetRVarNN(RFP_ERRMSG, 1, DssDataTypeVarChar, &m_ioBuffer, &lRSize, &vScriptErrMsg, pFlag, m_sErrMsg);
		CHECK_HR(L"Failure while fetching script error message.")

		if((*pFlag==DssDataOk) && (SysStringLen(vScriptErrMsg.bstrVal) > 0))
		{
			// successfully retrieved script error message...return message to caller
			INIT_ERROR_MSG2(L"%s: R script execution failed:  %s", vScriptErrMsg.bstrVal)
			return E_FAIL;
		}
		else if(lErrCode > 0)
		{
			// script terminated abnormally...return default error message to caller
			INIT_ERROR_MSG(
				L"R script execution failed with no error message.  Possible causes: execution error outside tryCatch() or syntax error.")
			return E_FAIL;
		}
		//else...script terminated normally with no error message...assume script results available

		// access output var info
		pVI = m_pRSuppFO->GetOutput();

		// check output parameter type
		if(pVI->GetParamType()==DssParameterVector)
		{
			// result should be a one-dimensional array of Variants[nSize]
			pResult->vt = VT_ARRAY|VT_VARIANT;
			SAFEARRAYBOUND lSABound;
			lSABound.lLbound = 0;
			lSABound.cElements = nSize;
			pResult->parray = SafeArrayCreate(VT_VARIANT, 1, &lSABound);

			// fetch results from R
			hr = pRSupp->GetRVarNN(pVI->GetVarName(), nSize, pVI->GetDataType(), &m_ioBuffer, &lRSize, pResult, pFlag, m_sErrMsg);
			CHECK_HR(L"Failure while fetching results.")

			// set number of elements returned (must be <= nSize)
			*nDataSize = nSize;
		}
		else
		{
			// scalar output...check data type
			switch(pVI->GetDataType())
			{
			case DssDataTypeDouble:
				{
					pResult->vt = VT_R8;
					hr = pRSupp->GetRVarNN(pVI->GetVarName(), nSize, DssDataTypeDouble, NULL, &lRSize, pResult, pFlag, m_sErrMsg);
					CHECK_HR(L"Failure while fetching results.")
					break;
				}
			case DssDataTypeVarChar:
				{
					pResult->vt = VT_BSTR;
					pResult->bstrVal = NULL;
					hr = pRSupp->GetRVarNN(pVI->GetVarName(), nSize, DssDataTypeVarChar, &m_ioBuffer, &lRSize, pResult, pFlag, m_sErrMsg);
					CHECK_HR(L"Failure while fetching results.")
					break;
				}
			default:
				{
					INIT_ERROR_MSG2(L"%s: '%s' (%d) is an unsupported output data type.", RFP_GET_DT_STR(pVI->GetDataType()),
						pVI->GetDataType())
					throw E_FAIL;
				}
			}

			// set number of elements returned (output is scalar...must set to 1)
			*nDataSize = 1;
		}
	}
	catch(...)
	{
		return E_FAIL;
	}

	return S_OK;
}
