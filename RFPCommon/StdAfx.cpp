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
//		File:	StdAfx.cpp
//
//		Desc:	source file that includes just the standard includes
//				RFPCommon.pch will be the pre-compiled header
//				StdAfx.obj will contain the pre-compiled type information
//=======================================================================

#include "StdAfx.h"

// R Script variable/parameter literals
const char START_CMT = '#';
const char SPACE_CHAR = ' ';
const char PERIOD_CHAR = '.';
const char COMMA_CHAR = ',';
const char TAB_CHAR = '\t';
const char NULL_CHAR = '\0';
const wchar_t NULL_WCHAR = L'\0';
const char OPENBRACKET_CHAR = '[';
const char CLOSEBRACKET_CHAR = ']';
const char *NEWLINE = "\n";
const char *WHITESPACE_DELIMS = " \t\r\n";
const char *MSTR_BEGIN = "microstrategy_begin";
const char *MSTR_END = "microstrategy_end";
const char *RVAR_TAG = "rvar";
const char *VAR_DATATYPE_NUM = "-numeric";
const char *VAR_DATATYPE_STR = "-string";
const char *VAR_PARAMTYPE_SCALAR = "-scalar";
const char *VAR_PARAMTYPE_VECTOR = "-vector";
const char *VAR_PARAMTYPE_PARAMETER = "-parameter";
const char *VAR_IOTYPE_INPUT = "-input";
const char *VAR_IOTYPE_OUTPUT = "-output";
const char *VAR_REPEAT_INPUT = "-repeat";
const char *VAR_DISABLED_VAR = "-disabled";
const char *RFP_FUNCPARAM_NAME_ARRAY[] = {
	"BooleanParam1",
	"BooleanParam2",
	"BooleanParam3",
	"BooleanParam4",
	"BooleanParam5",
	"BooleanParam6",
	"BooleanParam7",
	"BooleanParam8",
	"BooleanParam9",
	"NumericParam1",
	"NumericParam2",
	"NumericParam3",
	"NumericParam4",
	"NumericParam5",
	"NumericParam6",
	"NumericParam7",
	"NumericParam8",
	"NumericParam9",
	"StringParam1",
	"StringParam2",
	"StringParam3",
	"StringParam4",
	"StringParam5",
	"StringParam6",
	"StringParam7",
	"StringParam8",
	"StringParam9",
	"_WorkingDir",
	"_OutputVar",
	"_NullsAllowed",
	"_CheckInputCount",
	"_RScriptFile",
	"_InputNames"
};

// R reserved words
const char *RFP_RESERVED_FOR_R[] = {
	"if", "else", "repeat", "while", "function", "for", "in", "next", "break", "TRUE", "FALSE",
	"NULL", "Inf", "NaN", "NA", "NA_integer_", "NA_real_", "NA_complex_", "NA_character_", "..."
};

// invalid R chars
const char *RFP_INVALID_R_IDENT_NAME_CHARS = "/;,:|\\{}[]+=-!@#$%^&*() ~?><'`\"";	// allows period
const char *RFP_INVALID_R_IDENT_FIRST_CHARS = "_0123456789";						// disallows underscore

// MSTR-reserved R variable names
const char *RFP_ERRMSG = "mstr.ErrMsg";
const char *RFP_WORKDIR = "mstr.WorkingDir";
const char *RFP_EXFLAG = "mstr.ExFlag";
const char *RFP_INPUTNAMES = "mstr.InputNames";

// error log file names
const char *RFP_PATH_DELIM = "\\";
const char *RFP_ERRLOG = "RScriptErrors.log";
const char *RFP_ERRLOG_BKUP = "RScriptErrorsBkup.log";

// default script repository
const char *RFP_RSCRIPTS = "RScripts";
