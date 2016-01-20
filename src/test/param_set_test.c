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

#ifdef _WIN32
#define snprintf _snprintf
#endif

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
	assert_param_set_value_count(tc, set, "{1}{4}", NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 5);


	/**
	 * Remove parameters.
     */
	res = PARAM_SET_clearParameter(set, "{1}");
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
	assert_param_set_value_count(tc, set, "{1}{three}", "B", 0, __FILE__, __LINE__, 2);
	for (i = 0; PARAM_SET_clearValue(set, "{1}{three}", "B", 0, 0) == PST_OK; i++);

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
		return "Format error: Parameter must have value.";
	case ERROR_TOO_LONG:
		return "Content error: Too many characters.";
	case ERROR_NOT_ALPHA:
		return "Format error: Not alpha char.";
	}
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
					"Warning: Unknown parameter 'unknown 2'.\n"
					"Warning: Unknown parameter 'unknown 3'.\n";

	res = PARAM_SET_new("{ab}{abc}{abcd}{test}{sest}{kest}{mest}", &set);
	CuAssert(tc, "Unable to create new parameter set.", res == PST_OK);

	res = PARAM_SET_add(set, "unknown 1", NULL, NULL, 0);
	CuAssert(tc, "Unable to add to set.", res == PST_PARAMETER_IS_UNKNOWN);

	res = PARAM_SET_add(set, "unknown 2", NULL, NULL, 0);
	CuAssert(tc, "Unable to add to set.", res == PST_PARAMETER_IS_UNKNOWN);

	res = PARAM_SET_add(set, "unknown 3", NULL, NULL, 0);
	CuAssert(tc, "Unable to add to set.", res == PST_PARAMETER_IS_UNKNOWN);

	PARAM_SET_unknownsToString(set, "Warning: ", buf, sizeof(buf));
	CuAssert(tc, "Invalid string genrated.", strcmp(buf, expected) == 0);
	PARAM_SET_free(set);
}




CuSuite* ParamSetTest_getSuite(void) {
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, Test_param_add_count_clear);
	SUITE_ADD_TEST(suite, Test_param_invalid);
	SUITE_ADD_TEST(suite, Test_param_set_typos);
	SUITE_ADD_TEST(suite, Test_param_set_unknown);
	SUITE_ADD_TEST(suite, Test_param_remove_element);
	return suite;
}

