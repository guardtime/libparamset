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

/**
 * PARAM_SET_new
 * PARAM_SET_add
 * PARAM_SET_clearParameter
 * PARAM_SET_free
 *
 * PARAM_SET_addControl
 * PARAM_SET_clearParameter
 * PARAM_SET_clearValue
 *
 * PARAM_SET_typosToString
 * PARAM_SET_unknownsToString
 * PARAM_SET_invalidParametersToString
 *
 * PARAM_SET_isFormatOK
 * PARAM_SET_isTypoFailure
 * PARAM_SET_isSetByName
 */

static void Test_param_add_count_clear(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;

	/**
	 * Create set and add control functions.
     */
	res = PARAM_SET_new("{1}{2}{3|three}{4|four}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	CuAssert(tc, "1 must not be set.", !PARAM_SET_isSetByName(set, "1"));

	/**
	 * Add all kind of valid parameters.
     */
	res = PARAM_SET_add(set, "1", "1.1_0", NULL, 0);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	res = PARAM_SET_add(set, "1", "1.2_B0", "B", 0);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	res = PARAM_SET_add(set, "1", "1.3_B1", "B", 1);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	res = PARAM_SET_add(set, "3", "3.1_A1", "A", 1);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	res = PARAM_SET_add(set, "three", "3.2_B0", "B", 0);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	res = PARAM_SET_add(set, "4", "4.1_0", NULL, 0);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	res = PARAM_SET_add(set, "four", "4.2_0", NULL, 0);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	CuAssert(tc, "1 must be set.", PARAM_SET_isSetByName(set, "1"));
	/**
	 * Count the values.
     */
	assert_param_set_value_count(tc, set, NULL, NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 7);
	assert_param_set_value_count(tc, set, NULL, "B", PST_PRIORITY_NONE, __FILE__, __LINE__, 3);
	assert_param_set_value_count(tc, set, NULL, "B", 0, __FILE__, __LINE__, 2);
	assert_param_set_value_count(tc, set, NULL, "B", 1, __FILE__, __LINE__, 1);
	assert_param_set_value_count(tc, set, "{1}", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 3);
	assert_param_set_value_count(tc, set, "{2}", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 0);
	assert_param_set_value_count(tc, set, "{3}", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 2);
	assert_param_set_value_count(tc, set, "{three}", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 2);
	assert_param_set_value_count(tc, set, "{4}", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 2);
	assert_param_set_value_count(tc, set, "{four}", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 2);
	assert_param_set_value_count(tc, set, "four", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 2);
	assert_param_set_value_count(tc, set, "{1}{4}", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 5);
	assert_param_set_value_count(tc, set, "1,4", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 5);


	/**
	 * Remove parameters.
     */
	res = PARAM_SET_clearParameter(set, "1");
	CuAssert(tc, "Unable to remove.", res == PST_OK);
	assert_param_set_value_count(tc, set, "{1}", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 0);
	assert_param_set_value_count(tc, set, NULL, NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 4);
	CuAssert(tc, "1 must not be set.", !PARAM_SET_isSetByName(set, "1"));

	res = PARAM_SET_clearParameter(set, "{2}");
	CuAssert(tc, "Unable to remove.", res == PST_OK);
	assert_param_set_value_count(tc, set, "{2}", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 0);
	assert_param_set_value_count(tc, set, NULL, NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 4);
	CuAssert(tc, "2 must not be set.", !PARAM_SET_isSetByName(set, "2"));

	res = PARAM_SET_clearParameter(set, "{three}");
	CuAssert(tc, "Unable to remove.", res == PST_OK);
	assert_param_set_value_count(tc, set, "{3}", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 0);
	assert_param_set_value_count(tc, set, NULL, NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 2);
	CuAssert(tc, "3 must not be set.", !PARAM_SET_isSetByName(set, "3"));

	res = PARAM_SET_clearParameter(set, "{4}");
	CuAssert(tc, "Unable to remove.", res == PST_OK);
	assert_param_set_value_count(tc, set, "{4}", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 0);
	assert_param_set_value_count(tc, set, NULL, NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 0);
	CuAssert(tc, "4 must not be set.", !PARAM_SET_isSetByName(set, "4"));

	PARAM_SET_free(set);
}

static void Test_param_remove_element(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	int i = 0;

	/**
	 * Create set and add control functions.
     */
	res = PARAM_SET_new("{1}{2}{3|three}{4|four}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	/**
	 * Add all kind of valid parameters.
     */
	res = PARAM_SET_add(set, "1", "1.1_0", NULL, 0);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	res = PARAM_SET_add(set, "1", "1.2_B0", "B", 0);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	res = PARAM_SET_add(set, "1", "1.3_B1", "B", 1);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	res = PARAM_SET_add(set, "3", "3.1_A1", "A", 1);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	res = PARAM_SET_add(set, "three", "3.2_B0", "B", 0);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	res = PARAM_SET_add(set, "4", "4.1_0", NULL, 0);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	res = PARAM_SET_add(set, "four", "4.2_0", NULL, 0);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	/**
	 * Remove parameters.
     */
	assert_param_set_value_count(tc, set, NULL, NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 7);
	assert_param_set_value_count(tc, set, "1 three", "B", 0, __FILE__, __LINE__, 2);
	for (i = 0; PARAM_SET_clearValue(set, "1 three", "B", 0, 0) == PST_OK; i++);

	assert_param_set_value_count(tc, set, "{1}{three}", "B", 0, __FILE__, __LINE__, 0);
	assert_param_set_value_count(tc, set, NULL, NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 5);

	assert_param_set_value_count(tc, set, "{1}{three}", "B", PST_PRIORITY_NONE, __FILE__, __LINE__, 1);
	for (i = 0; PARAM_SET_clearValue(set, "{1}{three}", "B", PST_PRIORITY_NONE, 0) == PST_OK; i++);
	assert_param_set_value_count(tc, set, NULL, NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 4);

	assert_param_set_value_count(tc, set, "{four}", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 2);
	for (i = 0; PARAM_SET_clearValue(set, "{four}", NULL, PST_PRIORITY_NONE, 0) == PST_OK; i++);
	assert_param_set_value_count(tc, set, NULL, NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 2);

	PARAM_SET_free(set);
}


enum {
	ERROR_TOO_LONG = 1,
	ERROR_NOT_ALPHA,
	ERROR_NULL
};

static const char*errToString(int err) {
	switch(err) {
	case ERROR_NULL:
		return "Format error: Parameter must have value";
	case ERROR_TOO_LONG:
		return "Content error: Too many characters";
	case ERROR_NOT_ALPHA:
		return "Format error: Not alpha char";
	}
	return "Unknown";
}

static int controlFormat_isAlpha(const char *value) {
	int i = 0;
	if (value == NULL) return ERROR_NULL;
	while (value[i]) {
		if (!isalpha(value[i])) return ERROR_NOT_ALPHA;
		i++;
	}
	return 0;
}

static int controlContent_notOver_one_char(const char *value) {
	if (value == NULL) return ERROR_NULL;
	if (strlen(value) > 1) return ERROR_TOO_LONG;
	return 0;
}

static void Test_param_invalid(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char buf[1024];
	char expected1[] = "Error: 0x2. Parameter --str '1234'.\n"
					"Error: 0x3. Parameter --str ''.\n"
					"Error: 0x1. Parameter --str_short 'toolong'.\n"
					"Error: 0x3. Parameter --str_short ''.\n"
					"Error: 0x1. Parameter (from 'conf') --str_short 'toolong'.\n";
	char expected2[] = "Format error: Not alpha char. Parameter --str '1234'.\n"
					"Format error: Parameter must have value. Parameter --str ''.\n"
					"Content error: Too many characters. Parameter --str_short 'toolong'.\n"
					"Format error: Parameter must have value. Parameter --str_short ''.\n"
					"Content error: Too many characters. Parameter (from 'conf') --str_short 'toolong'.\n";

	/**
	 * Create set and add control functions.
     */
	res = PARAM_SET_new("{str}{str_short}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res = PARAM_SET_addControl(set, "{str}", controlFormat_isAlpha, NULL, NULL, NULL);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	res = PARAM_SET_addControl(set, "{str_short}", controlFormat_isAlpha, controlContent_notOver_one_char, NULL, NULL);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	CuAssert(tc, "Parameter set must have no errors.", PARAM_SET_isFormatOK(set));
	CuAssert(tc, "Parameter set must have no typos.", !PARAM_SET_isTypoFailure(set));


	/**
	 * Add all kind of invalid values.
     */
	res = PARAM_SET_add(set, "str", "1234", NULL, 0);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	res = PARAM_SET_add(set, "str", NULL, NULL, 0);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	res = PARAM_SET_add(set, "str_short", "toolong", NULL, 0);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	res = PARAM_SET_add(set, "str_short", NULL, NULL, 0);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	res = PARAM_SET_add(set, "str_short", "toolong", "conf", 0);
	CuAssert(tc, "Unable to add functions.", res == PST_OK);

	CuAssert(tc, "Parameter set must be invalid.", !PARAM_SET_isFormatOK(set));
	CuAssert(tc, "Parameter set must have no typos.", !PARAM_SET_isTypoFailure(set));

	/**
	 * Check for invalid values.
     */
	PARAM_SET_invalidParametersToString(set, NULL, NULL, buf, sizeof(buf));
	CuAssert(tc, "Invalid string generated.", strcmp(buf, expected1) == 0);

	PARAM_SET_invalidParametersToString(set, NULL, errToString, buf, sizeof(buf));
	CuAssert(tc, "Invalid string generated.", strcmp(buf, expected2) == 0);

	PARAM_SET_free(set);
}

static void Test_param_set_typos(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char buf[1024];
	char expected[] = "Typo: Did You mean '--test' instead of 'est'.\n"
					"Typo: Did You mean '--sest' instead of 'est'.\n"
					"Typo: Did You mean '--kest' instead of 'est'.\n"
					"Typo: Did You mean '--mest' instead of 'est'.\n"
					"Typo: Did You mean '--mest' instead of 'met'.\n"
					"Typo: Did You mean '--abc' instead of 'ac'.\n";

	res = PARAM_SET_new("{ab}{abc}{abcd}{test}{sest}{kest}{mest}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res = PARAM_SET_add(set, "est", NULL, NULL, 0);
	CuAssert(tc, "Unable to add to set.", res == PST_PARAMETER_IS_TYPO);

	res = PARAM_SET_add(set, "met", NULL, NULL, 0);
	CuAssert(tc, "Unable to add to set.", res == PST_PARAMETER_IS_TYPO);

	res = PARAM_SET_add(set, "ac", NULL, NULL, 0);
	CuAssert(tc, "Unable to add to set.", res == PST_PARAMETER_IS_TYPO);

	CuAssert(tc, "Ther should be typos.", PARAM_SET_isTypoFailure(set));


	PARAM_SET_typosToString(set, "Typo: ", buf, sizeof(buf));
	CuAssert(tc, "Invalid string generated.", strcmp(buf, expected) == 0);



	PARAM_SET_free(set);
}

static void Test_param_set_unknown(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char buf[1024];
	char expected[] = "Warning: Unknown parameter 'unknown 1'.\n"
					"Warning: Unknown parameter 'argument'.\n"
					"Warning: Unknown parameter 'unknown 2'.\n"
					"Warning: Unknown parameter 'unknown 3'.\n";

	res = PARAM_SET_new("{ab}{abc}{abcd}{test}{sest}{kest}{mest}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	CuAssert(tc, "There should be no unknown parameters.", !PARAM_SET_isUnknown(set));

	res = PARAM_SET_add(set, "unknown 1", "argument", NULL, 0);
	CuAssert(tc, "Unable to add to set.", res == PST_PARAMETER_IS_UNKNOWN);

	res = PARAM_SET_add(set, "unknown 2", NULL, NULL, 0);
	CuAssert(tc, "Unable to add to set.", res == PST_PARAMETER_IS_UNKNOWN);

	res = PARAM_SET_add(set, "unknown 3", NULL, NULL, 0);
	CuAssert(tc, "Unable to add to set.", res == PST_PARAMETER_IS_UNKNOWN);

	CuAssert(tc, "There should be some unknown parameters.", PARAM_SET_isUnknown(set));

	PARAM_SET_unknownsToString(set, "Warning: ", buf, sizeof(buf));
	CuAssert(tc, "Invalid string genrated.", strcmp(buf, expected) == 0);
	PARAM_SET_free(set);
}

static void Test_param_set_from_cmd_flags(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *path = "<path>";
	char *p1 = "-abc";
	char *p2 = "-x";
	char *p3 = "-e";
	char *p4 = "e_value";
	char *argv[5];
	int argc = 5;
	int count = 0;
	char *value = NULL;

	argv[0] = path;
	argv[1] = p1;
	argv[2] = p2;
	argv[3] = p3;
	argv[4] = p4;

	res = PARAM_SET_new("{a}{b}{c}{d}{x}{e}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res = PARAM_SET_readFromCMD(set, argc, argv, NULL, 0);
	CuAssert(tc, "Unable to parse command line.", res == PST_OK);

	res = PARAM_SET_getValueCount(set, "{a}{b}{c}{d}{x}{e}", NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Unable to count values set from cmd.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", count == 5);

	res = PARAM_SET_getObj(set, "e", NULL, PST_PRIORITY_NONE, 0, &value);
	CuAssert(tc, "Invalid value extracted.", res == PST_OK && strcmp(value, p4) == 0);

	res = PARAM_SET_getObj(set, "a", NULL, PST_PRIORITY_NONE, 0, &value);
	CuAssert(tc, "Invalid value extracted.", res == PST_OK && value == NULL);

	res = PARAM_SET_getObj(set, "d", NULL, PST_PRIORITY_NONE, 0, &value);
	CuAssert(tc, "Invalid value extracted.", res == PST_PARAMETER_EMPTY);

	PARAM_SET_free(set);
}

static int wrapper_return_str_append_a_value(void *extra, const char* str, void** obj){
	int res;
	void **extra_array = extra;
	PARAM_SET *set = (PARAM_SET*)(extra_array[0]);
	char *m = NULL;
	char *tmp = NULL;

	m = (char*)malloc(sizeof(char) * 1024);
	if (m == NULL) {
		return PST_OUT_OF_MEMORY;
	}

	res = PARAM_SET_getObj(set, "a", NULL, PST_PRIORITY_NONE, 0, &tmp);
	if (res != PST_OK) return res;

	snprintf(m, 1024, "%s:%s", str, tmp);

	*obj = m;
	return PST_OK;
}

static int wrapper_return_error(void *extra, const char* str, void** obj){
	if (extra || str || obj);
	return PST_UNDEFINED_BEHAVIOUR;
}

static void Test_set_get_object(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *value;
	char *d_value = NULL;


	res = PARAM_SET_new("{a}{b}{c}{d}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res = PARAM_SET_add(set, "a", "a_value", NULL, 0);
	CuAssert(tc, "Unable to add a value.", res == PST_OK);

	res = PARAM_SET_add(set, "b", "b_value", NULL, 0);
	CuAssert(tc, "Unable to add a value.", res == PST_OK);

	res = PARAM_SET_add(set, "d", "d_value", NULL, 0);
	CuAssert(tc, "Unable to add a value.", res == PST_OK);


	res = PARAM_SET_addControl(set, "d", NULL, NULL, NULL, wrapper_return_str_append_a_value);
	CuAssert(tc, "Unable to object extractor to 'd'.", res == PST_OK);

	res = PARAM_SET_addControl(set, "b", NULL, NULL, NULL, wrapper_return_error);
	CuAssert(tc, "Unable to object extractor to 'd'.", res == PST_OK);


	res = PARAM_SET_getObj(set, "a", NULL, PST_PRIORITY_NONE, 0, &value);
	CuAssert(tc, "Invalid value extracted.", res == PST_OK && value != NULL);

	value = NULL;
	res = PARAM_SET_getObj(set, "b", NULL, PST_PRIORITY_NONE, 0, &value);
	CuAssert(tc, "Invalid value extracted.", res == PST_UNDEFINED_BEHAVIOUR && value == NULL);

	res = PARAM_SET_getObj(set, "d", NULL, PST_PRIORITY_NONE, 0, &d_value);
	CuAssert(tc, "Invalid value extracted.", res == PST_OK && strcmp(d_value, "d_value:a_value") == 0);

	free(d_value);
	PARAM_SET_free(set);
}

static int isFormatOk_isNull(const char *str) {
	if (str == NULL) return 0;
	else return 1;
}

static void Test_set_get_str(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	char *value;


	res = PARAM_SET_new("{a}{b}{c}{d}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res = PARAM_SET_add(set, "a", "a_value", NULL, 0);
	CuAssert(tc, "Unable to add a value.", res == PST_OK);

	res = PARAM_SET_add(set, "b", NULL, NULL, 0);
	CuAssert(tc, "Unable to add a value.", res == PST_OK);

	res = PARAM_SET_add(set, "d", "not null", NULL, 0);
	CuAssert(tc, "Unable to add a value.", res == PST_OK);

	res = PARAM_SET_addControl(set, "d", isFormatOk_isNull, NULL, NULL, NULL);


	res = PARAM_SET_getStr(set, "a", NULL, PST_PRIORITY_NONE, 0, &value);
	CuAssert(tc, "Invalid value extracted.", res == PST_OK && value != NULL);
	CuAssert(tc, "Invalid value extracted.", strcmp(value, "a_value") == 0);

	res = PARAM_SET_getStr(set, "b", NULL, PST_PRIORITY_NONE, 0, &value);
	CuAssert(tc, "Invalid value extracted.", res == PST_OK && value == NULL);

	res = PARAM_SET_getStr(set, "c", NULL, PST_PRIORITY_NONE, 0, &value);
	CuAssert(tc, "Invalid value extracted.", res == PST_PARAMETER_EMPTY);

	res = PARAM_SET_getStr(set, "d", NULL, PST_PRIORITY_NONE, 0, &value);
	CuAssert(tc, "Invalid value extracted.", res == PST_PARAMETER_INVALID_FORMAT || strcmp(value, "not null") == 0);

	PARAM_SET_free(set);
}

static void Test_key_value_pairs(CuTest* tc) {
	int res;
	char key[2048];
	char value[2048];

	/**
	 * Test all kind of empty results.
     */
	res = parse_key_value_pair("", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && key[0] == '\0' && value[0] == '\0');

	res = parse_key_value_pair("\n", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && key[0] == '\0' && value[0] == '\0');

	res = parse_key_value_pair("  \n", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && key[0] == '\0' && value[0] == '\0');

	res = parse_key_value_pair("   ", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && key[0] == '\0' && value[0] == '\0');

	res = parse_key_value_pair("\t\t", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && key[0] == '\0' && value[0] == '\0');

	res = parse_key_value_pair("\t\t\n", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && key[0] == '\0' && value[0] == '\0');

	res = parse_key_value_pair("\n\n", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && key[0] == '\0' && value[0] == '\0');


	/**
	 * Test some invalid results
     */
	res = parse_key_value_pair(NULL, key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_INVALID_ARGUMENT && key[0] == '\0' && value[0] == '\0');

	res = parse_key_value_pair(" #--aaa test", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_INVALID_FORMAT && key[0] == '\0' && value[0] == '\0');

	/**
	 * Test some valid values.
     */
	res = parse_key_value_pair("\ntest aa\n", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && strcmp(key, "test") == 0 && strcmp(value, "aa") == 0);

	res = parse_key_value_pair("\ntest \"a b c d\" \n", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && strcmp(key, "test") == 0 && strcmp(value, "a b c d") == 0);

	res = parse_key_value_pair("\ntest \\\"a\n", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && strcmp(key, "test") == 0 && strcmp(value, "\"a") == 0);

	res = parse_key_value_pair("test a", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && strcmp(key, "test") == 0 && strcmp(value, "a") == 0);

	res = parse_key_value_pair(" test a1", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && strcmp(key, "test") == 0 && strcmp(value, "a1") == 0);

	res = parse_key_value_pair("-test a2", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && strcmp(key, "-test") == 0 && strcmp(value, "a2") == 0);

	res = parse_key_value_pair("--test a3", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && strcmp(key, "--test") == 0 && strcmp(value, "a3") == 0);

	res = parse_key_value_pair("--test      a4", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && strcmp(key, "--test") == 0 && strcmp(value, "a4") == 0);

	res = parse_key_value_pair("test1 = a1", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && strcmp(key, "test1") == 0 && strcmp(value, "a1") == 0);

	res = parse_key_value_pair("test2= a2", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && strcmp(key, "test2") == 0 && strcmp(value, "a2") == 0);

	res = parse_key_value_pair("test3 =a3", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && strcmp(key, "test3") == 0 && strcmp(value, "a3") == 0);

	res = parse_key_value_pair("test4=a4", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && strcmp(key, "test4") == 0 && strcmp(value, "a4") == 0);

	res = parse_key_value_pair("test5=\\\\a5", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && strcmp(key, "test5") == 0 && strcmp(value, "\\a5") == 0);

	res = parse_key_value_pair(" test6 \"\\\\a6 b6\" ", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && strcmp(key, "test6") == 0 && strcmp(value, "\\a6 b6") == 0);

	res = parse_key_value_pair(" test7 \" \\\"a \\\\ b\\\"\" ", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && strcmp(key, "test7") == 0 && strcmp(value, " \"a \\ b\"") == 0);

	res = parse_key_value_pair(" test8 \" \" ", key, value, sizeof(key));
	CuAssert(tc, "Invalid value.", res == PST_OK && strcmp(key, "test8") == 0 && strcmp(value, " ") == 0);
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

static void Test_set_read_from_file(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;

	res = PARAM_SET_new("{a}{b}{c}{test-test}{cnstr}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res = PARAM_SET_readFromFile(set, getFullResourcePath("ok-conf.conf"), NULL, 0);
	CuAssert(tc, "Unable to read conf file.", res == PST_OK);

	assert_value(tc, set, "a", 0, __FILE__, __LINE__, NULL);
	assert_value(tc, set, "b", 0, __FILE__, __LINE__, NULL);
	assert_value(tc, set, "c", 0, __FILE__, __LINE__, NULL);

	assert_value(tc, set, "test-test", 0, __FILE__, __LINE__, "Test");
	assert_value(tc, set, "test-test", 1, __FILE__, __LINE__, "a b c d");
	assert_value(tc, set, "test-test", 2, __FILE__, __LINE__, "a b \\c d");
	assert_value(tc, set, "test-test", 3, __FILE__, __LINE__, "a \"b\" c d");
	assert_value(tc, set, "test-test", 4, __FILE__, __LINE__, NULL);
	assert_value(tc, set, "test-test", 5, __FILE__, __LINE__, "\\");
	assert_value(tc, set, "test-test", 6, __FILE__, __LINE__, NULL);
	assert_value(tc, set, "test-test", 7, __FILE__, __LINE__, "  ");

	assert_value(tc, set, "cnstr", 0, __FILE__, __LINE__, "O=Guardtime AS");

	PARAM_SET_free(set);
}

static void Test_set_read_from_file_weird_format(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;

	res = PARAM_SET_new("{a}{b}{c}{test-test}{cnstr}{x}{y}{z}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res = PARAM_SET_readFromFile(set, getFullResourcePath("ok-conf-weird-format.conf"), NULL, 0);
	CuAssert(tc, "Unable to read conf file.", res == PST_OK);

	assert_value(tc, set, "a", 0, __FILE__, __LINE__, NULL);
	assert_value(tc, set, "b", 0, __FILE__, __LINE__, NULL);
	assert_value(tc, set, "c", 0, __FILE__, __LINE__, NULL);
	assert_value(tc, set, "x", 0, __FILE__, __LINE__, NULL);
	assert_value(tc, set, "y", 0, __FILE__, __LINE__, NULL);

	assert_value(tc, set, "test-test", 0, __FILE__, __LINE__, "Test");
	assert_value(tc, set, "test-test", 1, __FILE__, __LINE__, "a b c d");
	assert_value(tc, set, "test-test", 2, __FILE__, __LINE__, "a b \\c d");
	assert_value(tc, set, "test-test", 3, __FILE__, __LINE__, "a \"b\" c d");
	assert_value(tc, set, "test-test", 4, __FILE__, __LINE__, NULL);
	assert_value(tc, set, "test-test", 5, __FILE__, __LINE__, "\\");
	assert_value(tc, set, "test-test", 6, __FILE__, __LINE__, NULL);
	assert_value(tc, set, "test-test", 7, __FILE__, __LINE__, "  ");
	assert_value(tc, set, "test-test", 8, __FILE__, __LINE__, "\t");
	assert_value(tc, set, "test-test", 9, __FILE__, __LINE__, "#/&}{[]@");
	assert_value(tc, set, "test-test", 10, __FILE__, __LINE__, "-");
	assert_value(tc, set, "test-test", 11, __FILE__, __LINE__, "--");
	assert_value(tc, set, "test-test", 12, __FILE__, __LINE__, "-long way-");

	assert_value(tc, set, "cnstr", 0, __FILE__, __LINE__, "O=Guardtime AS");

	CuAssert(tc, "-z must be commented out.", !PARAM_SET_isSetByName(set, "z"));

	PARAM_SET_free(set);
}

static void Test_set_include_other_set(CuTest* tc) {
	int res;
	PARAM_SET *set_1 = NULL;
	PARAM_SET *set_2 = NULL;
	PARAM_SET *set_3 = NULL;
	PARAM_SET *set_4 = NULL;

	/**
	 * Create the sets.
     */
	res = PARAM_SET_new("{a}{b}{c}{d}", &set_1);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res = PARAM_SET_new("{a}{b}", &set_2);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res = PARAM_SET_new("{a}{x}{y}", &set_3);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res = PARAM_SET_new("{x}{y}", &set_4);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);


	/**
	 * Add the data to the sets.
     */
	res += PARAM_SET_add(set_1, "a", "a1.0", NULL, 1);
	res += PARAM_SET_add(set_1, "d", "d1.0", NULL, 1);

	res += PARAM_SET_add(set_2, "a", "a2.0", NULL, 2);
	res += PARAM_SET_add(set_2, "b", "b2.0", "2B", 2);
	res += PARAM_SET_add(set_2, "b", "b2.1", NULL, 2);

	res += PARAM_SET_add(set_3, "x", "x3.0", NULL, 3);
	CuAssert(tc, "There was errors in adding values to the list.", res == PST_OK);

	/**
	 * Merge sets.
     */
	res = PARAM_SET_IncludeSet(set_1, set_2);
	CuAssert(tc, "Unable to include set 2 to set 1.", res == PST_OK);
	res = PARAM_SET_IncludeSet(set_1, set_3);
	CuAssert(tc, "Unable to include set 3 to set 1.", res == PST_OK);
	res = PARAM_SET_IncludeSet(set_1, set_4);
	CuAssert(tc, "Unable to include set 4 to set 1.", res == PST_OK);

//	printf("\n-----\n%s\n-----\n", PARAM_SET_toString(set_1, buf, sizeof(buf)));

	/**
	 * Check the counts.
     */
	assert_param_set_value_count(tc, set_1, "{a}", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 2);
	assert_param_set_value_count(tc, set_1, "{b}", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 2);
	assert_param_set_value_count(tc, set_1, "{c}", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 0);
	assert_param_set_value_count(tc, set_1, "{d}", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 1);

	assert_param_set_value_count(tc, set_1, "{b}", "2B", 2, __FILE__, __LINE__, 1);
	assert_param_set_value_count(tc, set_1, "{b}", NULL, 2, __FILE__, __LINE__, 2);
	assert_param_set_value_count(tc, set_1, "{a}", NULL, 1, __FILE__, __LINE__, 1);
	assert_param_set_value_count(tc, set_1, "{a}", NULL, 2, __FILE__, __LINE__, 1);


	PARAM_SET_free(set_1);
	PARAM_SET_free(set_2);
	PARAM_SET_free(set_3);
	PARAM_SET_free(set_4);
}

static void Test_set_param_atr(CuTest* tc) {
	int res;
	PARAM_SET *set = NULL;
	PARAM_ATR atr;

	res = PARAM_SET_new("{a|aaa}{b}{c}{d}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res = PARAM_SET_add(set, "a", NULL, NULL, 2);
	CuAssert(tc, "Unable to add a value.", res == PST_OK);

	res = PARAM_SET_add(set, "a", "a_value", "A", 3);
	CuAssert(tc, "Unable to add a value.", res == PST_OK);

	res = PARAM_SET_getAtr(set, "a", NULL, PST_PRIORITY_HIGHEST, PST_INDEX_LAST, &atr);
	CuAssert(tc, "Unable to get attributes.", res == PST_OK);

	CuAssert(tc, "Invalid atr.", atr.priority == 3);
	CuAssert(tc, "Invalid atr.", strcmp(atr.name, "a") == 0);
	CuAssert(tc, "Invalid atr.", strcmp(atr.alias, "aaa") == 0);
	CuAssert(tc, "Invalid atr.", strcmp(atr.cstr_value, "a_value") == 0);
	CuAssert(tc, "Invalid atr.", strcmp(atr.source, "A") == 0);
	CuAssert(tc, "Invalid atr.", atr.contentStatus == PST_CONTENT_STATUS_OK);
	CuAssert(tc, "Invalid atr.", atr.contentStatus == PST_FORMAT_STATUS_OK);

	PARAM_SET_free(set);
}

CuSuite* ParamSetTest_getSuite(void) {
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, Test_param_add_count_clear);
	SUITE_ADD_TEST(suite, Test_param_invalid);
	SUITE_ADD_TEST(suite, Test_param_set_typos);
	SUITE_ADD_TEST(suite, Test_param_set_unknown);
	SUITE_ADD_TEST(suite, Test_param_remove_element);
	SUITE_ADD_TEST(suite, Test_param_set_from_cmd_flags);
	SUITE_ADD_TEST(suite, Test_set_get_object);
	SUITE_ADD_TEST(suite, Test_set_get_str);
	SUITE_ADD_TEST(suite, Test_key_value_pairs);
	SUITE_ADD_TEST(suite, Test_set_read_from_file);
	SUITE_ADD_TEST(suite, Test_set_read_from_file_weird_format);
	SUITE_ADD_TEST(suite, Test_set_include_other_set);
	SUITE_ADD_TEST(suite, Test_set_param_atr);
	return suite;
}

