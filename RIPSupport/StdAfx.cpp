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
//		File:	StdAfx.cpp
//
//		Desc:	Include file for standard system include files, or project
//				specific include files that are used frequently, but are
//				changed infrequently.
//=======================================================================

#include "StdAfx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

// R Script variable/parameter literals
const char START_CMT = '#';
const char SPACE_CHAR = ' ';
const char PERIOD_CHAR = '.';
const char COMMA_CHAR = ',';
const char HYPHEN_CHAR = '-';
const char EQUAL_CHAR = '=';
const char SQUOTE_CHAR = '\'';
const char ESCAPE_CHAR = '\\';
const char TAB_CHAR = '\t';
const char NULL_CHAR = '\0';
const wchar_t NULL_WCHAR = L'\0';
const char OPENBRACKET_CHAR = '[';
const char CLOSEBRACKET_CHAR = ']';
const char NL_CHAR = '\n';
const char *NEWLINE = "\n";
const char *WHITESPACE_DELIMS = " \t\r\n";
const char *MSTR_BEGIN = "microstrategy_begin";
const char *MSTR_END = "microstrategy_end";
const char *RVAR_TAG = "rvar";
const char *VAR_DATATYPE_NUM = "-numeric";
const char *VAR_DATATYPE_STR = "-string";
const char *VAR_DATATYPE_BOOL = "-boolean";
const char *VAR_PARAMTYPE_SCALAR = "-scalar";
const char *VAR_PARAMTYPE_VECTOR = "-vector";
const char *VAR_PARAMTYPE_PARAMETER = "-parameter";
const char *VAR_IOTYPE_INPUT = "-input";
const char *VAR_IOTYPE_OUTPUT = "-output";
const char *VAR_REPEAT_INPUT = "-repeat";
const char *VAR_DISABLED_VAR = "-disabled";
const char *RFP_FALSE = "false";
const char *RFP_TRUE = "true";
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

// R commands
const char *RCMD_ASNULL = "as.null";
const char *RCMD_EVAL = "eval";
const char *RCMD_GET = "get";
const char *RCMD_PARSE = "parse";
const char *RCMD_READLINES = "readLines";
const char *RCMD_RM = "rm";
const char *RCMD_SETINTERNET2 = "setInternet2";
const char *RCMD_URL = "url";

// R reserved words
const char *RFP_RESERVED_FOR_R[] = {
	"if", "else", "repeat", "while", "function", "for", "in", "next", "break", "TRUE", "FALSE",
	"NULL", "Inf", "NaN", "NA", "NA_integer_", "NA_real_", "NA_complex_", "NA_character_", "..."
};

// invalid R chars
const char *RFP_INVALID_R_IDENT_NAME_CHARS = "/;,:|\\{}[]+=-!@#$%^&*() ~?><'`\"";	// allows period
const char *RFP_INVALID_R_IDENT_FIRST_CHARS = "_0123456789";						// disallows underscore

// char set used to generate unique environment names
const char *RFP_RANDOM_CHARS_SET = "abcdefghijklmnopqrstuvwxyz";

// MSTR-reserved R variable names
const char *RFP_ERRMSG = "mstr.ErrMsg";
const char *RFP_INPUTNAMES = "mstr.InputNames";
const char *RFP_SCRIPT = "mstr.Script";
const char *RFP_ENVNAME = "mstr.Env.";
