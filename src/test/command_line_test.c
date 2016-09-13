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
		const char *expected, int must_fail) {
	int res;
	char *V = NULL;
	char buf[2048];
	int count = 0;

	count += snprintf(buf + count, sizeof(buf) - count, "Invalid value '%s' at line %i. ", file, line);

	res = PARAM_SET_getStr(set, name, NULL, PST_PRIORITY_NONE, at, &V);
	if (must_fail) {
		if (res == PST_PARAMETER_VALUE_NOT_FOUND) {
			return;
		} else {
			count += snprintf(buf + count, sizeof(buf) - count, "The value should not be extracted (err %X).", res);
			CuAssert(tc, buf, 0);
		}
	} else {
		if (res != PST_OK) {
			count += snprintf(buf + count, sizeof(buf) - count, " Unable to get value from '%s' at %i.", name, at);
			CuAssert(tc, buf, 0);
		}
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

	assert_value(tc, set, "d", 0, __FILE__, __LINE__, "d1", 0);
	assert_value(tc, set, "d", 1, __FILE__, __LINE__, "d2", 0);
	assert_value(tc, set, "d", 2, __FILE__, __LINE__, "d3", 0);
	assert_value(tc, set, "d", 3, __FILE__, __LINE__, "-d4", 0);
	assert_value(tc, set, "d", 4, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "x", 0, __FILE__, __LINE__, "x1", 0);
	assert_value(tc, set, "x", 1, __FILE__, __LINE__, "x2", 0);
	assert_value(tc, set, "x", 2, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "f", 0, __FILE__, __LINE__, "-f", 0);
	assert_value(tc, set, "f", 1, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "e", 0, __FILE__, __LINE__, "e_value", 0);
	assert_value(tc, set, "e", 1, __FILE__, __LINE__, NULL, 1);

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
		"--mdb", "v6", "v7","-", "--", "---","--dbm",
		"--all", "-e", "-x", "--mdb", "--",
		"-f", "-f", NULL};
	int argc = 0;
	int count = 0;
	char buf[0xfff];

	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{mb}{db}{mdb}{all}{e}{f}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res += PARAM_SET_setParseOptions(set, "{e}{f}", PST_PRSCMD_DEFAULT);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);
	res += PARAM_SET_setParseOptions(set, "{mb}", PST_PRSCMD_HAS_MULTIPLE_INSTANCES | PST_PRSCMD_BREAK_VALUE_WITH_EXISTING_PARAMETER_MATCH);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);
	res += PARAM_SET_setParseOptions(set, "{db}", PST_PRSCMD_HAS_MULTIPLE_INSTANCES | PST_PRSCMD_BREAK_WITH_POTENTIAL_PARAMETER);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);
	res += PARAM_SET_setParseOptions(set, "{mdb}", PST_PRSCMD_HAS_MULTIPLE_INSTANCES | PST_PRSCMD_BREAK_VALUE_WITH_EXISTING_PARAMETER_MATCH | PST_PRSCMD_BREAK_WITH_POTENTIAL_PARAMETER);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);
	res += PARAM_SET_setParseOptions(set, "{all}", PST_PRSCMD_HAS_MULTIPLE_INSTANCES);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);


	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 0);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	res = PARAM_SET_getValueCount(set, "{mb}{db}{mdb}{all}{e}{f}", NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 20);

	assert_value(tc, set, "mb", 0, __FILE__, __LINE__, "v0", 0);
	assert_value(tc, set, "mb", 1, __FILE__, __LINE__, "v1", 0);
	assert_value(tc, set, "mb", 2, __FILE__, __LINE__, "-x", 0);
	assert_value(tc, set, "mb", 3, __FILE__, __LINE__, NULL, 1);


	assert_value(tc, set, "db", 0, __FILE__, __LINE__, "v-2", 0);
	assert_value(tc, set, "db", 1, __FILE__, __LINE__, "v3-", 0);
	assert_value(tc, set, "db", 2, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "mdb", 0, __FILE__, __LINE__, "v4", 0);
	assert_value(tc, set, "mdb", 1, __FILE__, __LINE__, "v5", 0);
	assert_value(tc, set, "mdb", 2, __FILE__, __LINE__, "v6", 0);
	assert_value(tc, set, "mdb", 3, __FILE__, __LINE__, "v7", 0);
	assert_value(tc, set, "mdb", 4, __FILE__, __LINE__, "-", 0);
	assert_value(tc, set, "mdb", 5, __FILE__, __LINE__, "--", 0);
	assert_value(tc, set, "mdb", 6, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "e", 0, __FILE__, __LINE__, "e_value_1", 0);
	assert_value(tc, set, "e", 1, __FILE__, __LINE__, "e_value_2", 0);
	assert_value(tc, set, "e", 2, __FILE__, __LINE__, "e_value_3", 0);
	assert_value(tc, set, "e", 3, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "all", 0, __FILE__, __LINE__, "-e", 0);
	assert_value(tc, set, "all", 1, __FILE__, __LINE__, "-x", 0);
	assert_value(tc, set, "all", 2, __FILE__, __LINE__, "--mdb", 0);
	assert_value(tc, set, "all", 3, __FILE__, __LINE__, "--", 0);
	assert_value(tc, set, "all", 4, __FILE__, __LINE__, "-f", 0);
	assert_value(tc, set, "all", 5, __FILE__, __LINE__, "-f", 0);
	assert_value(tc, set, "all", 6, __FILE__, __LINE__, NULL, 1);

	CuAssert(tc, "f should not be set.", !PARAM_SET_isSetByName(set, "f"));

	/**
	 * Check for unknown and typos.
	 */
	buf[0] = '\0';
	PARAM_SET_typosToString(set, 0, NULL, buf, sizeof(buf));
	CuAssert(tc, "One typo should be detected.", strcmp(buf, "Did You mean 'db' instead of 'dbm'.\n") == 0);

	buf[0] = '\0';
	PARAM_SET_unknownsToString(set, NULL, buf, sizeof(buf));
	CuAssert(tc, "Four unknown should be detected.", strcmp(buf, "Unknown parameter 'notdef_1'.\n"
																"Unknown parameter 'notdef_2'.\n"
																"Unknown parameter 'notdef_3'.\n"
																"Unknown parameter 'unknown'.\n"
																"Unknown parameter '-'.\n") == 0);

	PARAM_SET_free(set);
}


static void Test_param_set_loose_parameters_end_of_commands(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *argv[] = {
		"<path>", "plah",					/* A default path at the first place. */
		"-a", "-b", "v1", "--unk_1",	/* Array ended by an existing parameter. */
		"-c", "-x",
		"-d", "-fname",
		"-e", "--unk_2","--",
		"-c", "e_value_1", "-",
		"-f", "f", NULL};
	int argc = 0;
	int count = 0;
	char buf[0xfff];

	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{a}{b}{c}{d}{e}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res += PARAM_SET_setParseOptions(set, "{a}{b}", PST_PRSCMD_DEFAULT);
	res += PARAM_SET_setParseOptions(set, "{c}", PST_PRSCMD_HAS_VALUE | PST_PRSCMD_COLLECT_LOOSE_VALUES | PST_PRSCMD_COLLECT_LOOSE_PERMIT_END_OF_COMMANDS);
	res += PARAM_SET_setParseOptions(set, "{d}", PST_PRSCMD_HAS_VALUE);
	res += PARAM_SET_setParseOptions(set, "{e}", PST_PRSCMD_HAS_VALUE | PST_PRSCMD_BREAK_WITH_POTENTIAL_PARAMETER);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);


	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 0);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	res = PARAM_SET_getValueCount(set, "{a}{b}{c}{d}{e}", NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 11);

	assert_value(tc, set, "a", 0, __FILE__, __LINE__, NULL, 0);
	assert_value(tc, set, "a", 1, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "b", 0, __FILE__, __LINE__, "v1", 0);
	assert_value(tc, set, "b", 1, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "c", 0, __FILE__, __LINE__, "plah", 0);
	assert_value(tc, set, "c", 1, __FILE__, __LINE__, "-x", 0);
	assert_value(tc, set, "c", 2, __FILE__, __LINE__, "-c", 0);
	assert_value(tc, set, "c", 3, __FILE__, __LINE__, "e_value_1", 0);
	assert_value(tc, set, "c", 4, __FILE__, __LINE__, "-", 0);
	assert_value(tc, set, "c", 5, __FILE__, __LINE__, "-f", 0);
	assert_value(tc, set, "c", 6, __FILE__, __LINE__, "f", 0);
	assert_value(tc, set, "c", 7, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "d", 0, __FILE__, __LINE__, "-fname", 0);
	assert_value(tc, set, "d", 1, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "e", 0, __FILE__, __LINE__, NULL, 0);
	assert_value(tc, set, "e", 1, __FILE__, __LINE__, NULL, 1);

		/**
	 * Check for unknown and typos.
	 */
	CuAssert(tc, "There should be no typos.", !PARAM_SET_isTypoFailure(set));

	buf[0] = '\0';
	PARAM_SET_unknownsToString(set, NULL, buf, sizeof(buf));
	CuAssert(tc, "Four unknown should be detected.", strcmp(buf, "Unknown parameter 'unk_1'.\n"
																"Unknown parameter 'unk_2'.\n") == 0);

	PARAM_SET_free(set);
}

static void Test_param_set_loose_parameters_collect_no_dashes(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *argv[] = {
		"<path>", "plah",					/* A default path at the first place. */
		"-ab", "-", "-",	/* Array ended by an existing parameter. */
		"-c", "x", "--",
		"-f", "f", NULL};
	int argc = 0;
	int count = 0;
	char buf[0xfff];

	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{a}{b}{c}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res += PARAM_SET_setParseOptions(set, "{a}{b}", PST_PRSCMD_DEFAULT);
	res += PARAM_SET_setParseOptions(set, "{c}", PST_PRSCMD_DEFAULT | PST_PRSCMD_COLLECT_LOOSE_VALUES);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);


	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 0);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	res = PARAM_SET_getValueCount(set, "{a}{b}{c}", NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 5);

	assert_value(tc, set, "a", 0, __FILE__, __LINE__, NULL, 0);
	assert_value(tc, set, "a", 1, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "b", 0, __FILE__, __LINE__, NULL, 0);
	assert_value(tc, set, "b", 1, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "c", 0, __FILE__, __LINE__, "plah", 0);
	assert_value(tc, set, "c", 1, __FILE__, __LINE__, "x", 0);
	assert_value(tc, set, "c", 2, __FILE__, __LINE__, "f", 0);
	assert_value(tc, set, "c", 3, __FILE__, __LINE__, NULL, 1);

	/**
	 * Check for unknown and typos.
	 */
	CuAssert(tc, "There should be no typos.", !PARAM_SET_isTypoFailure(set));

	buf[0] = '\0';
	PARAM_SET_unknownsToString(set, NULL, buf, sizeof(buf));
	CuAssert(tc, "Four unknown should be detected.", strcmp(buf, "Unknown parameter '-'.\n"
																"Unknown parameter '-'.\n"
																"Unknown parameter '--'.\n"
																"Unknown parameter 'f'.\n") == 0);

	PARAM_SET_free(set);
}

static void Test_param_set_loose_parameters_collect_dashes(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *argv[] = {
		"<path>", "plah",					/* A default path at the first place. */
		"-ab", "-", "-",	/* Array ended by an existing parameter. */
		"-c", "x", "--",
		"-f", "f", NULL};
	int argc = 0;
	int count = 0;
	char buf[0xfff];

	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{a}{b}{c}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res += PARAM_SET_setParseOptions(set, "{a}{b}", PST_PRSCMD_DEFAULT);
	res += PARAM_SET_setParseOptions(set, "{c}", PST_PRSCMD_HAS_VALUE | PST_PRSCMD_COLLECT_LOOSE_VALUES | PST_PRSCMD_COLLECT_LOOSE_DASHES);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);


	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 0);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	res = PARAM_SET_getValueCount(set, "{a}{b}{c}", NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 8);

	assert_value(tc, set, "a", 0, __FILE__, __LINE__, NULL, 0);
	assert_value(tc, set, "a", 1, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "b", 0, __FILE__, __LINE__, NULL, 0);
	assert_value(tc, set, "b", 1, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "c", 0, __FILE__, __LINE__, "plah", 0);
	assert_value(tc, set, "c", 1, __FILE__, __LINE__, "-", 0);
	assert_value(tc, set, "c", 2, __FILE__, __LINE__, "-", 0);
	assert_value(tc, set, "c", 3, __FILE__, __LINE__, "x", 0);
	assert_value(tc, set, "c", 4, __FILE__, __LINE__, "--", 0);
	assert_value(tc, set, "c", 5, __FILE__, __LINE__, "f", 0);
	assert_value(tc, set, "c", 6, __FILE__, __LINE__, NULL, 1);

	/**
	 * Check for unknown and typos.
	 */
	CuAssert(tc, "There should be no typos.", !PARAM_SET_isTypoFailure(set));

	buf[0] = '\0';
	PARAM_SET_unknownsToString(set, NULL, buf, sizeof(buf));
	CuAssert(tc, "Four unknown should be detected.", strcmp(buf, "Unknown parameter 'f'.\n") == 0);

	PARAM_SET_free(set);
}

static void Test_param_last_parameter_without_mandatory_value(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *argv[] = {
		"<path>", "-ab", "-c", NULL};
	int argc = 0;
	int count = 0;

	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{a}{b}{c}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res += PARAM_SET_setParseOptions(set, "{a}{b}", PST_PRSCMD_DEFAULT);
	res += PARAM_SET_setParseOptions(set, "{c}", PST_PRSCMD_HAS_VALUE);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);

	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 0);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	res = PARAM_SET_getValueCount(set, "{a}{b}{c}", NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 3);

	assert_value(tc, set, "a", 0, __FILE__, __LINE__, NULL, 0);
	assert_value(tc, set, "a", 1, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "b", 0, __FILE__, __LINE__, NULL, 0);
	assert_value(tc, set, "b", 1, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "c", 0, __FILE__, __LINE__, NULL, 0);
	assert_value(tc, set, "c", 1, __FILE__, __LINE__, NULL, 1);

	/**
	 * Check for unknown and typos.
	 */
	CuAssert(tc, "There should be no typos.", !PARAM_SET_isTypoFailure(set));
	CuAssert(tc, "There should be no typos.", !PARAM_SET_isUnknown(set));

	PARAM_SET_free(set);
}

static void Test_param_last_token_bunch_of_flags(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *argv[] = {
		"<path>", "-c", "c_value", "-ab", NULL};
	int argc = 0;
	int count = 0;

	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{a}{b}{c}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res += PARAM_SET_setParseOptions(set, "{a}{b}", PST_PRSCMD_DEFAULT);
	res += PARAM_SET_setParseOptions(set, "{c}", PST_PRSCMD_HAS_VALUE);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);

	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 0);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	res = PARAM_SET_getValueCount(set, "{a}{b}{c}", NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 3);

	assert_value(tc, set, "a", 0, __FILE__, __LINE__, NULL, 0);
	assert_value(tc, set, "a", 1, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "b", 0, __FILE__, __LINE__, NULL, 0);
	assert_value(tc, set, "b", 1, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "c", 0, __FILE__, __LINE__, "c_value", 0);
	assert_value(tc, set, "c", 1, __FILE__, __LINE__, NULL, 1);

	/**
	 * Check for unknown and typos.
	 */
	CuAssert(tc, "There should be no typos.", !PARAM_SET_isTypoFailure(set));
	CuAssert(tc, "There should be no typos.", !PARAM_SET_isUnknown(set));


//	printf("\n%s\n", PARAM_SET_toString(set, buf, sizeof(buf)));
//	printf("\n%s\n", PARAM_SET_unknownsToString(set, NULL, buf, sizeof(buf)));
//	printf("\n%s\n", PARAM_SET_typosToString(set, PST_TOSTR_DOUBLE_HYPHEN, NULL, buf, sizeof(buf)));

	PARAM_SET_free(set);
}

static void Test_command_line_short_and_long(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *argv[] = {
		"<path>", "-a", "a1", "--a", "a2", "---a", "a3", "----a", "a4", NULL};
	int argc = 0;
	int count = 0;

	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{a}{b}{c}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res += PARAM_SET_setParseOptions(set, "{a}", PST_PRSCMD_DEFAULT);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);

	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 0);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	res = PARAM_SET_getValueCount(set, "{a}", NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 2);

	assert_value(tc, set, "a", 0, __FILE__, __LINE__, "a1", 0);
	assert_value(tc, set, "a", 1, __FILE__, __LINE__, "a2", 0);
	assert_value(tc, set, "a", 2, __FILE__, __LINE__, NULL, 1);


	PARAM_SET_free(set);
}

static void Test_command_line_limited_loose_collect(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *argv[] = {
		"<path>", "a1", "a2", "a3", NULL};
	int argc = 0;
	int count = 0;
	char buf[1024];

	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{c}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res += PARAM_SET_setParseOptions(set, "{c}", PST_PRSCMD_HAS_VALUE | PST_PRSCMD_COLLECT_LOOSE_VALUES | PST_PRSCMD_COLLECT_LIMITER_ON | (PST_PRSCMD_COLLECT_LIMITER_1X * 2));
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);

	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 0);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	res = PARAM_SET_getValueCount(set, "{c}", NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 2);

	assert_value(tc, set, "c", 0, __FILE__, __LINE__, "a1", 0);
	assert_value(tc, set, "c", 1, __FILE__, __LINE__, "a2", 0);
	assert_value(tc, set, "c", 2, __FILE__, __LINE__, NULL, 1);


	/**
	 * Check for unknown and typos.
	 */
	CuAssert(tc, "There should be no typos.", !PARAM_SET_isTypoFailure(set));

	buf[0] = '\0';
	PARAM_SET_unknownsToString(set, NULL, buf, sizeof(buf));
	CuAssert(tc, "Four unknown should be detected.", strcmp(buf, "Unknown parameter 'a3'.\n") == 0);

	PARAM_SET_free(set);
}

static void Test_command_line_collect_lower_priority(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *argv[] = {
		"<path>", "a1", "a2", "-c", "a3", "a4", "a5", NULL};
	int argc = 0;
	int count = 0;

	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{c}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res += PARAM_SET_setParseOptions(set, "{c}", PST_PRSCMD_HAS_VALUE | PST_PRSCMD_COLLECT_LOOSE_VALUES | PST_PRSCMD_COLLECT_HAS_LOWER_PRIORITY);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);

	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 3);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	res = PARAM_SET_getValueCount(set, "{c}", NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 5);

	res = PARAM_SET_getValueCount(set, "{c}", NULL, 3, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 1);

	res = PARAM_SET_getValueCount(set, "{c}", NULL, 2, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 4);

	assert_value(tc, set, "c", 0, __FILE__, __LINE__, "a1", 0);
	assert_value(tc, set, "c", 1, __FILE__, __LINE__, "a2", 0);
	assert_value(tc, set, "c", 2, __FILE__, __LINE__, "a3", 0);
	assert_value(tc, set, "c", 3, __FILE__, __LINE__, "a4", 0);
	assert_value(tc, set, "c", 4, __FILE__, __LINE__, "a5", 0);
	assert_value(tc, set, "c", 5, __FILE__, __LINE__, NULL, 1);


	/**
	 * Check for unknown and typos.
	 */
	CuAssert(tc, "There should be no typos.", !PARAM_SET_isTypoFailure(set));
	CuAssert(tc, "There should be no unknown.", !PARAM_SET_isUnknown(set));

	PARAM_SET_free(set);
}

static void Test_command_line_collect_has_no_flag_no_ignore_typos(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *argv[] = {
		"<path>", "a1", "a2", "-c", "a3", "a4", "a5", NULL};
	int argc = 0;
	int count = 0;
	char buf[1024];

	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{c}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res += PARAM_SET_setParseOptions(set, "{c}", PST_PRSCMD_HAS_VALUE | PST_PRSCMD_COLLECT_LOOSE_VALUES | PST_PRSCMD_HAS_NO_FLAG | PST_PRSCMD_NO_TYPOS);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);

	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 3);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	res = PARAM_SET_add(set, "c", "c_value", "code", 0);

	res = PARAM_SET_getValueCount(set, "{c}", NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 6);

	assert_value(tc, set, "c", 0, __FILE__, __LINE__, "a1", 0);
	assert_value(tc, set, "c", 1, __FILE__, __LINE__, "a2", 0);
	assert_value(tc, set, "c", 2, __FILE__, __LINE__, "a3", 0);
	assert_value(tc, set, "c", 3, __FILE__, __LINE__, "a4", 0);
	assert_value(tc, set, "c", 4, __FILE__, __LINE__, "a5", 0);
	assert_value(tc, set, "c", 5, __FILE__, __LINE__, "c_value", 0);
	assert_value(tc, set, "c", 6, __FILE__, __LINE__, NULL, 1);


	/**
	 * Check for unknown and typos.
	 */

	CuAssert(tc, "There should be no typos.", !PARAM_SET_isTypoFailure(set));

	buf[0] = '\0';
	PARAM_SET_unknownsToString(set, NULL, buf, sizeof(buf));
	CuAssert(tc, "Four unknown should be detected.", strcmp(buf, "Unknown parameter 'c'.\n") == 0);

	PARAM_SET_free(set);
}

int control_format_i_j(const char* a) {
	if (a == NULL || a[0] == 'c') return 0;
	else return 1;
}

static void Test_command_line_check_for_highest_priority_last_element_errors(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *argv[] = {
		"<path>", "-i", "a", "-i", "b", "-i", "c", "-j", "c", NULL};
	int argc = 0;
	char buf[1024];

	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{i}{j}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res = PARAM_SET_addControl(set,"{i}{j}", control_format_i_j, NULL, NULL, NULL);
	res += PARAM_SET_setParseOptions(set, "{i}", PST_PRSCMD_FORMAT_CONTROL_ONLY_FOR_LAST_HIGHST_PRIORITY_VALUE | PST_PRSCMD_DEFAULT);
	//CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);

	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 3);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	CuAssert(tc, "There should not be format errors.", PARAM_SET_isFormatOK(set) == 1);

	res = PARAM_SET_add(set, "j", "a", NULL, 2);
	CuAssert(tc, "Unable to add a new value.", res == PST_OK);

	CuAssert(tc, "There should be format erros.", PARAM_SET_isFormatOK(set) == 0);

	buf[0] = '\0';
	PARAM_SET_invalidParametersToString(set, NULL, NULL, buf, sizeof(buf));
	CuAssert(tc, "Four unknown should be detected.", strcmp(buf, "Error: 0x1. Parameter -j 'a'.\n") == 0);

	PARAM_SET_free(set);
}

static void Test_command_line_parameter_with_value_in_the_end_2(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *argv[] = {
		"<path>", "-i", "-o", NULL};
	int argc = 0;
	int count = 0;

	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{i}{o}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res += PARAM_SET_setParseOptions(set, "{i}{o}", PST_PRSCMD_HAS_VALUE | PST_PRSCMD_BREAK_VALUE_WITH_EXISTING_PARAMETER_MATCH);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);

	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 3);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	res = PARAM_SET_getValueCount(set, "{i}{o}", NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 2);

	assert_value(tc, set, "i", 0, __FILE__, __LINE__, NULL, 0);
	assert_value(tc, set, "i", 1, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "o", 0, __FILE__, __LINE__, NULL, 0);
	assert_value(tc, set, "o", 1, __FILE__, __LINE__, NULL, 1);

	/**
	 * Check for unknown and typos.
	 */

	CuAssert(tc, "There should be no typos.", !PARAM_SET_isTypoFailure(set));
	CuAssert(tc, "There should be no unknown.", !PARAM_SET_isUnknown(set));


	PARAM_SET_free(set);
}

static void Test_command_last_value_after_flag_type_parameteter(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *argv[] = {
		"<path>", "-v", "test", NULL};
	int argc = 0;
	int count = 0;
	char buf[1024];

	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{v}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res += PARAM_SET_setParseOptions(set, "{v}", PST_PRSCMD_HAS_NO_VALUE);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);

	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 3);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	res = PARAM_SET_getValueCount(set, "{v}", NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 1);

	assert_value(tc, set, "v", 0, __FILE__, __LINE__, NULL, 0);
	assert_value(tc, set, "v", 1, __FILE__, __LINE__, NULL, 1);


	/**
	 * Check for unknown and typos.
	 */

	CuAssert(tc, "There should be no typos.", !PARAM_SET_isTypoFailure(set));
	buf[0] = '\0';
	PARAM_SET_unknownsToString(set, NULL, buf, sizeof(buf));
	CuAssert(tc, "One unknown should be detected.", strcmp(buf, "Unknown parameter 'test'.\n") == 0);

	PARAM_SET_free(set);
}

static void Test_command_test_1(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *argv[] = {
		"<path>", "-o", "test_1", "-d", "-h", NULL};
	int argc = 0;
	int count = 0;
	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{o}{d}{h}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res += PARAM_SET_setParseOptions(set, "{o}", PST_PRSCMD_HAS_VALUE);
	res += PARAM_SET_setParseOptions(set, "{d}{h}", PST_PRSCMD_DEFAULT);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);

	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 3);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	res = PARAM_SET_getValueCount(set, "{o}{d}{h}", NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 3);

	assert_value(tc, set, "o", 0, __FILE__, __LINE__, "test_1", 0);
	assert_value(tc, set, "o", 1, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "d", 0, __FILE__, __LINE__, NULL, 0);
	assert_value(tc, set, "d", 1, __FILE__, __LINE__, NULL, 1);

	assert_value(tc, set, "h", 0, __FILE__, __LINE__, NULL, 0);
	assert_value(tc, set, "h", 1, __FILE__, __LINE__, NULL, 1);


	/**
	 * Check for unknown and typos.
	 */

	CuAssert(tc, "There should be no typos.", !PARAM_SET_isTypoFailure(set));
	CuAssert(tc, "There should be no typos.", !PARAM_SET_isUnknown(set));

	PARAM_SET_free(set);
}

static int expand_wildcard_len3str(PARAM_VAL *param_value, void *ctx, int *value_shift) {
	 const char *input = NULL;
	 int res;
	 char **accepted_strs = (char**)ctx;
	 char *str = NULL;
	 char *src = NULL;
	 int prio = 0;
	 int i = 0;
	 PARAM_VAL *tmp = NULL;
	 int count = 0;

	 if (param_value == NULL || ctx == NULL || value_shift == NULL) {
		 res = PST_INVALID_ARGUMENT;
		 goto cleanup;
	 }

	 res = PARAM_VAL_extract(param_value, &input, &src, &prio);
	 if (res != PST_OK) return res;

	 if (strlen(input) != 3) return PST_INVALID_FORMAT;

	 while ((str = accepted_strs[i++]) != NULL) {
		if ((input[0] == '?' || input[0] == str[0])
			&& (input[1] == '?' || input[1] == str[1])
			&& (input[2] == '?' || input[2] == str[2])){

			res = PARAM_VAL_new(str, src, prio, &tmp);
			if (res != PST_OK) goto cleanup;

			res = PARAM_VAL_insert(param_value, NULL, PST_PRIORITY_NONE, count, tmp);
			if (res != PST_OK) goto cleanup;

			count++;
			tmp = NULL;
		}
	 }

	*value_shift = count;
	res = PST_OK;

 cleanup:

	 PARAM_VAL_free(tmp);
	 return res;
 }

static void Test_expand_WC_on_CMD_WC_not_configured_no_WC_input(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *argv[] = {
		"<path>", "xxx", "zzz", "abc", "dbg", NULL};
	int argc = 0;
	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{i}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res += PARAM_SET_setParseOptions(set, "{i}", PST_PRSCMD_COLLECT_LOOSE_VALUES | PST_PRSCMD_EXPAND_WILDCARD);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);

	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 3);
	CuAssert(tc, "Unable to parse command line.", res == PST_PARAMETER_UNIMPLEMENTED_WILDCARD);

	PARAM_SET_free(set);
}

static void Test_expand_WC_on_CMD_WC_configured_WC_as_input(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *data[] = {
		"fbg", "xzy", "hgj", "xzz", NULL};
	char *argv[] = {
		"<path>", "xxx", "f?g", "zzz", "abc", "xz?", NULL};
	int argc = 0;
	int count = 0;
	char buf[1024];

	while(argv[argc] != NULL) argc++;

	res = PARAM_SET_new("{i}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res = PARAM_SET_setParseOptions(set, "{i}", PST_PRSCMD_COLLECT_LOOSE_VALUES | PST_PRSCMD_EXPAND_WILDCARD);
	CuAssert(tc, "Unable to set parameter set command line parsing options.", res == PST_OK);

	res = PARAM_SET_wildcardExpander(set, "i", data, expand_wildcard_len3str);
	CuAssert(tc, "Unable to configure wildcard expander.", res == PST_OK);

	res = PARAM_SET_parseCMD(set, argc, argv, NULL, 3);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	res = PARAM_SET_getValueCount(set, "{i}", NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 6);

	assert_value(tc, set, "i", 0, __FILE__, __LINE__, "xxx", 0);
	assert_value(tc, set, "i", 1, __FILE__, __LINE__, "fbg", 0);
	assert_value(tc, set, "i", 2, __FILE__, __LINE__, "zzz", 0);
	assert_value(tc, set, "i", 3, __FILE__, __LINE__, "abc", 0);
	assert_value(tc, set, "i", 4, __FILE__, __LINE__, "xzy", 0);
	assert_value(tc, set, "i", 5, __FILE__, __LINE__, "xzz", 0);
	assert_value(tc, set, "i", 6, __FILE__, __LINE__, NULL, 1);

	PARAM_SET_free(set);
}


CuSuite* Command_LineTest_getSuite(void) {
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, Test_param_set_from_cmd_flags_bacward_compatibility);
	SUITE_ADD_TEST(suite, Test_param_set_cmd_special);
	SUITE_ADD_TEST(suite, Test_param_set_cmd_special_array_break);
	SUITE_ADD_TEST(suite, Test_param_set_loose_parameters_end_of_commands);
	SUITE_ADD_TEST(suite, Test_param_set_loose_parameters_collect_no_dashes);
	SUITE_ADD_TEST(suite, Test_param_set_loose_parameters_collect_dashes);
	SUITE_ADD_TEST(suite, Test_param_last_parameter_without_mandatory_value);
	SUITE_ADD_TEST(suite, Test_param_last_token_bunch_of_flags);
	SUITE_ADD_TEST(suite, Test_command_line_short_and_long);
	SUITE_ADD_TEST(suite, Test_command_line_limited_loose_collect);
	SUITE_ADD_TEST(suite, Test_command_line_collect_lower_priority);
	SUITE_ADD_TEST(suite, Test_command_line_collect_has_no_flag_no_ignore_typos);
	SUITE_ADD_TEST(suite, Test_command_line_parameter_with_value_in_the_end_2);
	SUITE_ADD_TEST(suite, Test_command_last_value_after_flag_type_parameteter);
	SUITE_ADD_TEST(suite, Test_command_test_1);
	SUITE_ADD_TEST(suite, Test_command_line_check_for_highest_priority_last_element_errors);
	SUITE_ADD_TEST(suite, Test_expand_WC_on_CMD_WC_not_configured_no_WC_input);
	SUITE_ADD_TEST(suite, Test_expand_WC_on_CMD_WC_configured_WC_as_input);
	return suite;
}


