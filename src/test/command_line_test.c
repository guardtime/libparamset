/*
 * Copyright 2013-2015 Guardtime, Inc.
 *
 * This file is part of the Guardtime client SDK.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *     http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES, CONDITIONS, OR OTHER LICENSES OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 * "Guardtime" and "KSI" are trademarks or registered trademarks of
 * Guardtime, Inc., and no license to trademarks is granted; Guardtime
 * reserves and retains all trademark rights.
 */

#include "cutest/CuTest.h"
#include "all_tests.h"
#include "../param_set/param_value.h"
#include "../param_set/parameter.h"
#include "../param_set/param_set.h"
#include "../param_set/param_set_obj_impl.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

int PARAM_SET_parseCMD(PARAM_SET *set, int argc, char **argv, const char *source, int priority);

static void assert_param_set_value_count(CuTest* tc,
		PARAM_SET* set, const char* names, const char* source, int priority,
		const char *file, int line, int C) {
	int res;
	int count = 0;
	char buf[2048];

	res = PARAM_SET_getValueCount(set, names, source, priority, &count);
	if (res != PST_OK || count != C) {
		snprintf(buf, sizeof(buf), "Invalid count %i, expected %i in file '%s' at line %i.", count, C, file, line);
		CuAssert(tc, buf, 0);
	}
}

static void assert_value(CuTest* tc,
		PARAM_SET *set, const char *name, int at,
		const char *file, int line,
		const char *expected) {
	int res;
	char *V = NULL;
	char buf[2048];
	int count = 0;

	count += snprintf(buf + count, sizeof(buf) - count, "Invalid value '%s' at line %i. ", file, line);

	res = PARAM_SET_getStr(set, name, NULL, PST_PRIORITY_NONE, at, &V);
	if (res != PST_OK) {
		count += snprintf(buf + count, sizeof(buf) - count, " Unable to get value from '%s' at %i.", name, at);
		CuAssert(tc, buf, 0);
	}

	if (((expected != NULL && V != NULL) && strcmp(V, expected) != 0)
			|| ((expected == NULL && V != NULL) || (expected != NULL && V == NULL))) {
		count += snprintf(buf + count, sizeof(buf) - count, " Value expected '%s' but is '%s'.", expected, V);
		CuAssert(tc, buf, 0);
	}
}

static void Test_param_set_from_cmd_flags_bacward_compatibility(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *argv[] = {"<path>", "-abc", "-x", "-e", "e_value", NULL};
	int argc = 0;
	int count = 0;
	char *value = NULL;

	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{a}{b}{c}{d}{x}{e}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 0);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	res = PARAM_SET_getValueCount(set, "{a}{b}{c}{d}{x}{e}", NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 5);

	res = PARAM_SET_getObj(set, "e", NULL, PST_PRIORITY_NONE, 0, &value);
	CuAssert(tc, "Invalid value extracted.", res == PST_OK && strcmp(value, argv[4]) == 0);

	res = PARAM_SET_getObj(set, "a", NULL, PST_PRIORITY_NONE, 0, &value);
	CuAssert(tc, "Invalid value extracted.", res == PST_OK && value == NULL);

	res = PARAM_SET_getObj(set, "d", NULL, PST_PRIORITY_NONE, 0, &value);
	CuAssert(tc, "Invalid value extracted.", res == PST_PARAMETER_EMPTY);


	CuAssert(tc, "There should be no typos.", !PARAM_SET_isTypoFailure(set));
	CuAssert(tc, "There should be no unknowns.", !PARAM_SET_isUnknown(set));

	PARAM_SET_free(set);
}

static void Test_param_set_cmd_special(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *argv[] = {
		"<path>",					/* A default path at the first place. */
		"-ywv", "unk_1", "unk_2",	/* Fits bunch of flags and some unknown parameters. (Default parsing). */
		"-x", "x1", "--xtra", "x2",	/* A short and a long representation. (Default parsing). */
		"-a", "unk_3",				/* A single parameter. (PST_PRSCMD_HAS_NO_VALUE). */
		"-bc","unk_4",				/* A bunch of flags. (PST_PRSCMD_HAS_NO_VALUE). */
		"-d", "d1", "d2", "d3", "-d4", /* An Array. (Break at first known parameter). */
		"-e", "e_value",			/* A short parameter with a value. (Default parsing). */
		"-f", "-f", NULL};			/* A short parameter with a value. (PST_PRSCMD_HAS_VALUE). */
	int argc = 0;
	int count = 0;
	char *value = NULL;
	char buf[1024];

	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{a}{b}{c}{d}{e}{x|xtra}{y}{w}{v}{f}{z}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res += PARAM_SET_setParseOptions(set, "{e}{x}{y}{w}{v}{z}", PST_PRSCMD_DEFAULT);
	res += PARAM_SET_setParseOptions(set, "{a}{b}{c}", PST_PRSCMD_HAS_NO_VALUE);
	res += PARAM_SET_setParseOptions(set, "{d}", PST_PRSCMD_HAS_MULTIPLE_INSTANCES | PST_PRSCMD_BREAK_VALUE_WITH_EXISTING_PARAMETER_MATCH);
	res += PARAM_SET_setParseOptions(set, "{f}", PST_PRSCMD_HAS_VALUE);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);


	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 0);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	res = PARAM_SET_getValueCount(set, "{a}{b}{c}{d}{e}{x}{y}{w}{v}{f}", NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 14);

	assert_value(tc, set, "d", 0, __FILE__, __LINE__, "d1");
	assert_value(tc, set, "d", 1, __FILE__, __LINE__, "d2");
	assert_value(tc, set, "d", 2, __FILE__, __LINE__, "d3");
	assert_value(tc, set, "d", 3, __FILE__, __LINE__, "-d4");
	CuAssert(tc, "Value should not be found.", PARAM_SET_getStr(set, "d", NULL, PST_PRIORITY_NONE, 4, &value) == PST_PARAMETER_VALUE_NOT_FOUND);

	assert_value(tc, set, "x", 0, __FILE__, __LINE__, "x1");
	assert_value(tc, set, "x", 1, __FILE__, __LINE__, "x2");
	CuAssert(tc, "Value should not be found.", PARAM_SET_getStr(set, "x", NULL, PST_PRIORITY_NONE, 2, &value) == PST_PARAMETER_VALUE_NOT_FOUND);

	assert_value(tc, set, "f", 0, __FILE__, __LINE__, "-f");
	CuAssert(tc, "Value should not be found.", PARAM_SET_getStr(set, "f", NULL, PST_PRIORITY_NONE, 1, &value) == PST_PARAMETER_VALUE_NOT_FOUND);

	assert_value(tc, set, "e", 0, __FILE__, __LINE__, "e_value");
	CuAssert(tc, "Value should not be found.", PARAM_SET_getStr(set, "e", NULL, PST_PRIORITY_NONE, 1, &value) == PST_PARAMETER_VALUE_NOT_FOUND);

	CuAssert(tc, "z should not be set.", !PARAM_SET_isSetByName(set, "z"));

	/**
	 * Check for unknown and typos.
	 */
	CuAssert(tc, "There should be no typos.", !PARAM_SET_isTypoFailure(set));

	buf[0] = '\0';
	PARAM_SET_unknownsToString(set, NULL, buf, sizeof(buf));
	CuAssert(tc, "Four unknown should be detected.", strcmp(buf, "Unknown parameter 'unk_1'.\n"
																"Unknown parameter 'unk_2'.\n"
																"Unknown parameter 'unk_3'.\n"
																"Unknown parameter 'unk_4'.\n") == 0);

	PARAM_SET_free(set);
}

static void Test_param_set_cmd_special_array_break(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *argv[] = {
		"<path>",					/* A default path at the first place. */
		"--mb", "v0", "v1", "-x",	/* Array ended by an existing parameter. */
		"-e", "e_value_1",
		"notdef_1",
		"--db", "v-2", "v3-",			/* Array ended by a beginning dash (-). */
		"-notdef_2",
		"notdef_3",
		"-e", "e_value_2",
		"--mdb", "v4", "v5",		/* Array ended by an existing parameter or beginning dash (-). */
		"-e", "e_value_3",
		"--unknown",
		"--mdb", "v6", "v7","-","--dbm",
		"--all", "-e", "-x", "--mdb", "--", /* Array that never ends. */
		"-f", "-f", NULL};
	int argc = 0;
	int count = 0;
	char *value = NULL;
	char buf[0xfff];

	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{mb}{db}{mdb}{all}{e}{f}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res += PARAM_SET_setParseOptions(set, "{e}{f}", PST_PRSCMD_DEFAULT);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);
	res += PARAM_SET_setParseOptions(set, "{mb}", PST_PRSCMD_HAS_MULTIPLE_INSTANCES | PST_PRSCMD_BREAK_VALUE_WITH_EXISTING_PARAMETER_MATCH);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);
	res += PARAM_SET_setParseOptions(set, "{db}", PST_PRSCMD_HAS_MULTIPLE_INSTANCES | PST_PRSCMD_BREAK_VALUE_WITH_DASH_PREFIX);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);
	res += PARAM_SET_setParseOptions(set, "{mdb}", PST_PRSCMD_HAS_MULTIPLE_INSTANCES | PST_PRSCMD_BREAK_VALUE_WITH_EXISTING_PARAMETER_MATCH | PST_PRSCMD_BREAK_VALUE_WITH_DASH_PREFIX);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);
	res += PARAM_SET_setParseOptions(set, "{all}", PST_PRSCMD_HAS_MULTIPLE_INSTANCES);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);


	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 0);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	res = PARAM_SET_getValueCount(set, "{mb}{db}{mdb}{all}{e}{f}", NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 18);

	assert_value(tc, set, "mb", 0, __FILE__, __LINE__, "v0");
	assert_value(tc, set, "mb", 1, __FILE__, __LINE__, "v1");
	assert_value(tc, set, "mb", 2, __FILE__, __LINE__, "-x");
	CuAssert(tc, "Value should not be found.", PARAM_SET_getStr(set, "mb", NULL, PST_PRIORITY_NONE, 3, &value) == PST_PARAMETER_VALUE_NOT_FOUND);

	assert_value(tc, set, "db", 0, __FILE__, __LINE__, "v-2");
	assert_value(tc, set, "db", 1, __FILE__, __LINE__, "v3-");
	CuAssert(tc, "Value should not be found.", PARAM_SET_getStr(set, "db", NULL, PST_PRIORITY_NONE, 2, &value) == PST_PARAMETER_VALUE_NOT_FOUND);

	assert_value(tc, set, "mdb", 0, __FILE__, __LINE__, "v4");
	assert_value(tc, set, "mdb", 1, __FILE__, __LINE__, "v5");
	assert_value(tc, set, "mdb", 2, __FILE__, __LINE__, "v6");
	assert_value(tc, set, "mdb", 3, __FILE__, __LINE__, "v7");
	CuAssert(tc, "Value should not be found.", PARAM_SET_getStr(set, "mdb", NULL, PST_PRIORITY_NONE, 4, &value) == PST_PARAMETER_VALUE_NOT_FOUND);

	assert_value(tc, set, "e", 0, __FILE__, __LINE__, "e_value_1");
	assert_value(tc, set, "e", 1, __FILE__, __LINE__, "e_value_2");
	assert_value(tc, set, "e", 2, __FILE__, __LINE__, "e_value_3");
	CuAssert(tc, "Value should not be found.", PARAM_SET_getStr(set, "e", NULL, PST_PRIORITY_NONE, 3, &value) == PST_PARAMETER_VALUE_NOT_FOUND);

	assert_value(tc, set, "all", 0, __FILE__, __LINE__, "-e");
	assert_value(tc, set, "all", 1, __FILE__, __LINE__, "-x");
	assert_value(tc, set, "all", 2, __FILE__, __LINE__, "--mdb");
	assert_value(tc, set, "all", 3, __FILE__, __LINE__, "--");
	assert_value(tc, set, "all", 4, __FILE__, __LINE__, "-f");
	assert_value(tc, set, "all", 5, __FILE__, __LINE__, "-f");
	CuAssert(tc, "Value should not be found.", PARAM_SET_getStr(set, "all", NULL, PST_PRIORITY_NONE, 6, &value) == PST_PARAMETER_VALUE_NOT_FOUND);

	CuAssert(tc, "f should not be set.", !PARAM_SET_isSetByName(set, "f"));

	/**
	 * Check for unknown and typos.
	 */
	buf[0] = '\0';
	PARAM_SET_typosToString(set, 0, NULL, buf, sizeof(buf));
	CuAssert(tc, "One typo should be detected.", strcmp(buf, "Did You mean 'db' instead of 'dbm'.\n") == 0);

	buf[0] = '\0';
	PARAM_SET_unknownsToString(set, NULL, buf, sizeof(buf));
	CuAssert(tc, "Five unknown should be detected.", strcmp(buf, "Unknown parameter 'notdef_1'.\n"
																"Unknown parameter 'notdef_2'.\n"
																"Unknown parameter 'notdef_3'.\n"
																"Unknown parameter 'unknown'.\n"
																"Unknown parameter '-'.\n") == 0);

	PARAM_SET_free(set);
}




CuSuite* Command_LineTest_getSuite(void) {
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, Test_param_set_from_cmd_flags_bacward_compatibility);
	SUITE_ADD_TEST(suite, Test_param_set_cmd_special);
	SUITE_ADD_TEST(suite, Test_param_set_cmd_special_array_break);
	return suite;
}


