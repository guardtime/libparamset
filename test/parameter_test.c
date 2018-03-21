/*
 * Copyright 2013-2017 Guardtime, Inc.
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

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "cutest/CuTest.h"
#include "all_tests.h"
#include "../src/param_set/param_value.h"
#include "../src/param_set/parameter.h"
#include "../src/param_set/param_set_obj_impl.h"



/**
 * PARAM_new
 * PARAM_addArgument
 * PARAM_getValueCount - mostly tested by PARAM_VAL.
 * PARAM_checkConstraints
 * PARAM_addControl
 * PARAM_getInvalidCount
 * PARAM_getInvalid
 * PARAM_free
 */
static void Test_parameterConstraints(CuTest* tc) {
	int res;
	PARAM *multiple = NULL;
	PARAM *single = NULL;
	PARAM *single_per_prio = NULL;
	int count = 0xf;

	res = PARAM_new("test1", NULL, 0, 0, &multiple);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	res = PARAM_new("test2", NULL, PARAM_SINGLE_VALUE, 0, &single);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	res = PARAM_new("test3", NULL, PARAM_SINGLE_VALUE_FOR_PRIORITY_LEVEL, 0, &single_per_prio);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	/* It is mostly covered by PARAM_VAL tests*/
	res = PARAM_getValueCount(multiple, NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Count must be 0.", res == PST_OK && count == 0);

	/**
	 * Add valid amount of parameters to multiple.
     */
	res = PARAM_addValue(multiple, "1", NULL, 0);
	CuAssert(tc, "Unable to add argument.", res == PST_OK);

	res = PARAM_addValue(multiple, "2", NULL, 1);
	CuAssert(tc, "Unable to add argument.", res == PST_OK);

	res = PARAM_addValue(multiple, "3", NULL, 0);
	CuAssert(tc, "Unable to add argument.", res == PST_OK);

	res = PARAM_getValueCount(multiple, NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Invalid count.", res == PST_OK && count == 3);

	res = PARAM_getValueCount(multiple, NULL, 1, &count);
	CuAssert(tc, "Invalid count.", res == PST_OK && count == 1);

	res = PARAM_getValueCount(multiple, NULL, 0, &count);
	CuAssert(tc, "Invalid count.", res == PST_OK && count == 2);
	CuAssert(tc, "Must be free from duplicate errors.", PARAM_checkConstraints(multiple, PARAM_SINGLE_VALUE | PARAM_SINGLE_VALUE_FOR_PRIORITY_LEVEL) == 0);

	/**
	 * Add valid amount of parameters to single priority level.
     */
	res = PARAM_addValue(single_per_prio, "1", NULL, 0);
	CuAssert(tc, "Unable to add argument.", res == PST_OK);

	res = PARAM_addValue(single_per_prio, "2", NULL, 1);
	CuAssert(tc, "Unable to add argument.", res == PST_OK);

	res = PARAM_addValue(single_per_prio, "3", NULL, 2);
	CuAssert(tc, "Unable to add argument.", res == PST_OK);

	res = PARAM_getValueCount(single_per_prio, NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Invalid count.", res == PST_OK && count == 3);

	res = PARAM_getValueCount(single_per_prio, NULL, 0, &count);
	CuAssert(tc, "Invalid count.", res == PST_OK && count == 1);

	res = PARAM_getValueCount(single_per_prio, NULL, 1, &count);
	CuAssert(tc, "Invalid count.", res == PST_OK && count == 1);

	res = PARAM_getValueCount(single_per_prio, NULL, 2, &count);
	CuAssert(tc, "Invalid count.", res == PST_OK && count == 1);

	CuAssert(tc, "Must be free from duplicate errors.", PARAM_checkConstraints(single_per_prio, PARAM_SINGLE_VALUE | PARAM_SINGLE_VALUE_FOR_PRIORITY_LEVEL) == 0);

	/**
	 * Add valid amount of parameters to single.
     */
	res = PARAM_addValue(single, "1", NULL, 0);
	CuAssert(tc, "Unable to add argument.", res == PST_OK);

	res = PARAM_getValueCount(single, NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Invalid count.", res == PST_OK && count == 1);

	res = PARAM_getValueCount(single, NULL, 0, &count);
	CuAssert(tc, "Invalid count.", res == PST_OK && count == 1);

	CuAssert(tc, "Must be free from duplicate errors.", PARAM_checkConstraints(single, PARAM_SINGLE_VALUE | PARAM_SINGLE_VALUE_FOR_PRIORITY_LEVEL) == 0);

	/**
	 * Brake the rules!
     */
	res = PARAM_addValue(single_per_prio, "4", NULL, 2);
	CuAssert(tc, "Unable to add argument.", res == PST_OK);
	CuAssert(tc, "Must contain duplicate errors.", PARAM_checkConstraints(single_per_prio, PARAM_SINGLE_VALUE_FOR_PRIORITY_LEVEL) == PARAM_SINGLE_VALUE_FOR_PRIORITY_LEVEL);

	res = PARAM_addValue(single, "2", NULL, 1);
	CuAssert(tc, "Unable to add argument.", res == PST_OK);
	CuAssert(tc, "Must contain duplicate errors.", PARAM_checkConstraints(single, PARAM_SINGLE_VALUE) == PARAM_SINGLE_VALUE);

	PARAM_free(multiple);
	PARAM_free(single);
	PARAM_free(single_per_prio);
}

/**
 * PARAM_getValue - mostly tested by PARAM_VAL.
 * PARAM_getInvalidCount - mostly tested by PARAM_VAL
 */
static void Test_parameterGetValue(CuTest* tc) {
	int res;
	PARAM *multiple = NULL;
	PARAM_VAL *val = NULL;
	const char *value = NULL;
	int prio = 0xffff;

	res = PARAM_new("test1", NULL, 0, 0, &multiple);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	res = PARAM_getValue(multiple, NULL, PST_PRIORITY_NONE, 0, &val);
	CuAssert(tc, "Parameters must not have a value.", res == PST_PARAMETER_EMPTY);

	/**
	 * Add valid amount of parameters to multiple.
     */
	res = PARAM_addValue(multiple, "1", NULL, 3);
	CuAssert(tc, "Unable to add argument.", res == PST_OK);

	res = PARAM_getValue(multiple, NULL, PST_PRIORITY_NONE, 0, &val);
	CuAssert(tc, "Unable to get value.", res == PST_OK);

	res = PARAM_VAL_extract(val, &value, NULL, &prio);
	CuAssert(tc, "Parameters must not have a value.", res == PST_OK && strcmp(value, "1") == 0 && prio == 3);

	PARAM_free(multiple);
}

static int controlFormat_isAlpha(const char *value) {
	int i = 0;
	while (value[i]) {
		if (!isalpha(value[i])) return 1;
		i++;
	}
	return 0;
}

static int controlContent_notOver_one_char(const char *value) {
	if (value == NULL) return 1;
	if (strlen(value) > 1) return 1;
	return 0;
}

static int convert_replaceNonAlpha(const char *value, char *buf, unsigned buf_len) {
	unsigned i = 0;
	unsigned j = 0;
	while (value[i]) {
		if (isalpha(value[i])) buf[j++] = value[i];
		if (j == buf_len - 1) break;
		i++;
	}
	buf[j] = '\0';
	return PST_OK;
}

static void Test_SetValuesAndControl(CuTest* tc) {
	int res;
	PARAM *p1 = NULL;
	PARAM *p2 = NULL;
	PARAM *p3 = NULL;
	PARAM *p4 = NULL;
	PARAM_VAL *value_1 = NULL;
	PARAM_VAL *value_2 = NULL;
	int count = 0xffff;

	res = PARAM_new("test1", NULL, 0, 0, &p1);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	res = PARAM_new("test2", NULL, 0, 0, &p2);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	res = PARAM_new("test3", NULL, 0, 0, &p3);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	res = PARAM_new("test4", NULL, 0, 0, &p4);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);


	/**
     * Add control functions.
     */
	res = PARAM_addControl(p1, controlFormat_isAlpha, NULL, NULL);
	CuAssert(tc, "Unable to set control.", res == PST_OK);

	res = PARAM_addControl(p2, controlFormat_isAlpha, controlContent_notOver_one_char, NULL);
	CuAssert(tc, "Unable to set control.", res == PST_OK);

	res = PARAM_addControl(p3, NULL, controlContent_notOver_one_char, NULL);
	CuAssert(tc, "Unable to set control.", res == PST_OK);

	res = PARAM_addControl(p4, controlFormat_isAlpha, NULL, convert_replaceNonAlpha);
	CuAssert(tc, "Unable to set control.", res == PST_OK);

	/**
	 * Add valid parameters.
     */
	res = PARAM_addValue(p1, "abcd", NULL, 0);
	CuAssert(tc, "Unable to at valid parameters.", res == PST_OK);

	res = PARAM_addValue(p2, "a", NULL, 0);
	CuAssert(tc, "Unable to at valid parameters.", res == PST_OK);

	res = PARAM_addValue(p3, "1", NULL, 0);
	CuAssert(tc, "Unable to at valid parameters.", res == PST_OK);

	res = PARAM_addValue(p4, "a1b2c3", NULL, 0);
	CuAssert(tc, "Unable to at valid parameters.", res == PST_OK);

	/**
	 * Validate by invalid count.
     */
	res = PARAM_getInvalidCount(p1, NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Invalid count.", res == PST_OK && count == 0);

	res = PARAM_getInvalidCount(p2, NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Invalid count.", res == PST_OK && count == 0);

	res = PARAM_getInvalidCount(p3, NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Invalid count.", res == PST_OK && count == 0);

	res = PARAM_getInvalidCount(p4, NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Invalid count.", res == PST_OK && count == 0);

	/**
	 * Add invalid parameters.
     */
	res = PARAM_addValue(p1, "1234", NULL, 0);
	CuAssert(tc, "Unable to at valid parameters.", res == PST_OK);

	res = PARAM_addValue(p2, "1", NULL, 0);
	CuAssert(tc, "Unable to at valid parameters.", res == PST_OK);

	res = PARAM_addValue(p2, "aa", NULL, 0);
	CuAssert(tc, "Unable to at valid parameters.", res == PST_OK);

	res = PARAM_addValue(p3, "12", NULL, 0);
	CuAssert(tc, "Unable to at valid parameters.", res == PST_OK);

	/**
	 * Validate by invalid count.
     */
	res = PARAM_getInvalidCount(p1, NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Invalid count.", res == PST_OK && count == 1);

	res = PARAM_getInvalidCount(p2, NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Invalid count.", res == PST_OK && count == 2);

	res = PARAM_getInvalidCount(p3, NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "Invalid count.", res == PST_OK && count == 1);

	/**
	 * Get some invalid values.
     */
	res = PARAM_getInvalid(p2, NULL, PST_PRIORITY_NONE, 0, &value_1);
	CuAssert(tc, "Unable to get Invalid value.", res == PST_OK && value_1->formatStatus == 1 && value_1->contentStatus == 0);

	res = PARAM_getInvalid(p2, NULL, PST_PRIORITY_NONE, 1, &value_2);
	CuAssert(tc, "Unable to get Invalid value.", res == PST_OK && value_2->formatStatus == 0 && value_2->contentStatus == 1);

	res = PARAM_getInvalid(p2, NULL, PST_PRIORITY_NONE, 2, &value_2);
	CuAssert(tc, "There should be only 2 invalid values.", res == PST_PARAMETER_VALUE_NOT_FOUND);

	PARAM_free(p1);
	PARAM_free(p2);
	PARAM_free(p3);
	PARAM_free(p4);
}

static int wrapper_returnStr(void **extra, const char* str, void** obj){
	VARIABLE_IS_NOT_USED(extra);
	*obj = (void*)str;
	return PST_OK;
}

static int wrapper_returnInt(void **extra, const char* str,  void** obj){
	int *pI = (int*)obj;
	VARIABLE_IS_NOT_USED(extra);
	*pI = atoi(str);
	return PST_OK;
}

static int wrapper_returnDouble(void **extra, const char* str,  void** obj){
	double *pd = (double*)obj;
	VARIABLE_IS_NOT_USED(extra);
	*pd = atof(str);
	return PST_OK;
}

static void Test_ObjectGetter(CuTest* tc) {
	int res;
	PARAM *p1 = NULL;
	PARAM *p2 = NULL;
	PARAM *p3 = NULL;
	PARAM *p4 = NULL;
	PARAM *p5 = NULL;
	char *string = NULL;
	int integer = 98;
	double floating = 10.0;

	/**
	 * Create some parameter objects.
     */
	res = PARAM_new("string", NULL, 0, 0, &p1);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	res = PARAM_new("int", NULL, 0, 0, &p2);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	res = PARAM_new("double", NULL, 0, 0, &p3);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	res = PARAM_new("default", NULL, 0, 0, &p4);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	res = PARAM_new("null", NULL, 0, 0, &p5);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	/**
	 * Set extractor methods.
     */
	res = PARAM_setObjectExtractor(p1, wrapper_returnStr);
	CuAssert(tc, "Unable set object extractor.", res == PST_OK);

	res = PARAM_setObjectExtractor(p2, wrapper_returnInt);
	CuAssert(tc, "Unable set object extractor.", res == PST_OK);

	res = PARAM_setObjectExtractor(p3, wrapper_returnDouble);
	CuAssert(tc, "Unable set object extractor.", res == PST_OK);

	/**
	 * Set valid values according to the extractor method.
     */
	res = PARAM_addValue(p1, "text", NULL, 0);
	CuAssert(tc, "Unable to add argument.", res == PST_OK);

	res = PARAM_addValue(p2, "1234", NULL, 0);
	CuAssert(tc, "Unable to add argument.", res == PST_OK);

	res = PARAM_addValue(p2, "-567", NULL, 0);
	CuAssert(tc, "Unable to add argument.", res == PST_OK);

	res = PARAM_addValue(p3, "12.3", NULL, 0);
	CuAssert(tc, "Unable to add argument.", res == PST_OK);

	res = PARAM_addValue(p4, "default.txt", NULL, 0);
	CuAssert(tc, "Unable to add argument.", res == PST_OK);

	res = PARAM_addValue(p5, NULL, NULL, 0);
	CuAssert(tc, "Unable to add argument.", res == PST_OK);

	/**
	 * Extract object valid objects.
     */
	res = PARAM_getObject(p1, NULL, PST_PRIORITY_NONE, 0, NULL, (void**)&string);
	CuAssert(tc, "Unable to extract string.", res == PST_OK && strcmp(string, "text") == 0);

	res = PARAM_getObject(p2, NULL, PST_PRIORITY_NONE, 0, NULL, (void**)&integer);
	CuAssert(tc, "Unable to extract int.", res == PST_OK && integer == 1234);

	res = PARAM_getObject(p2, NULL, PST_PRIORITY_NONE, 1, NULL, (void**)&integer);
	CuAssert(tc, "Unable to extract int.", res == PST_OK && integer == -567);

	res = PARAM_getObject(p3, NULL, PST_PRIORITY_NONE, 0, NULL, (void**)&floating);
	CuAssert(tc, "Unable to extract double.", res == PST_OK && floating == 12.3);

	res = PARAM_getObject(p4, NULL, PST_PRIORITY_NONE, 0, NULL, (void**)&string);
	CuAssert(tc, "Unable to extract default string.", res == PST_OK && strcmp(string, "default.txt") == 0);

	res = PARAM_getObject(p5, NULL, PST_PRIORITY_NONE, 0, NULL, (void**)&string);
	CuAssert(tc, "Unable to extract default string.", res == PST_OK && string == NULL);

	PARAM_free(p1);
	PARAM_free(p2);
	PARAM_free(p3);
	PARAM_free(p4);
	PARAM_free(p5);
}

static void Test_ParseOptionSetter(CuTest* tc) {
	int res;
	PARAM *p1 = NULL;

	/**
	 * Create some parameter objects.
     */
	res = PARAM_new("string", NULL, 0, PST_PRSCMD_DEFAULT, &p1);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	CuAssert(tc, "Parsing option is not default.", PARAM_isParseOptionSet(p1, PST_PRSCMD_DEFAULT));
	CuAssert(tc, "Invalid parsing flags also set.", !PARAM_isParseOptionSet(p1, PST_PRSCMD_HAS_VALUE | PST_PRSCMD_HAS_VALUE_SEQUENCE | PST_PRSCMD_HAS_NO_VALUE));

	res = PARAM_setParseOption(p1, PST_PRSCMD_NONE);
	CuAssert(tc, "Unable set parsing options.", res == PST_OK && PARAM_isParseOptionSet(p1, PST_PRSCMD_NONE));
	CuAssert(tc, "Invalid parsing flags also set.", !PARAM_isParseOptionSet(p1, PST_PRSCMD_HAS_VALUE | PST_PRSCMD_HAS_VALUE_SEQUENCE | PST_PRSCMD_HAS_NO_VALUE | PST_PRSCMD_DEFAULT));

	res = PARAM_setParseOption(p1, PST_PRSCMD_HAS_VALUE | PST_PRSCMD_BREAK_WITH_POTENTIAL_PARAMETER);
	CuAssert(tc, "Unable set parsing options.", res == PST_OK && PARAM_isParseOptionSet(p1, PST_PRSCMD_HAS_VALUE | PST_PRSCMD_BREAK_WITH_POTENTIAL_PARAMETER));
	CuAssert(tc, "Invalid parsing flags also set.", !PARAM_isParseOptionSet(p1, PST_PRSCMD_HAS_VALUE_SEQUENCE | PST_PRSCMD_HAS_NO_VALUE | PST_PRSCMD_BREAK_WITH_EXISTING_PARAMETER_MATCH | PST_PRSCMD_DEFAULT));

	res = PARAM_setParseOption(p1, PST_PRSCMD_HAS_VALUE_SEQUENCE | PST_PRSCMD_BREAK_WITH_EXISTING_PARAMETER_MATCH);
	CuAssert(tc, "Unable set parsing options.", res == PST_OK && PARAM_isParseOptionSet(p1, PST_PRSCMD_HAS_VALUE_SEQUENCE | PST_PRSCMD_BREAK_WITH_EXISTING_PARAMETER_MATCH));
	CuAssert(tc, "Invalid parsing flags also set.", !PARAM_isParseOptionSet(p1, PST_PRSCMD_HAS_NO_VALUE | PST_PRSCMD_BREAK_WITH_POTENTIAL_PARAMETER | PST_PRSCMD_DEFAULT | PST_PRSCMD_HAS_VALUE));

	/**
	 * Some invalid cases.
     */
	res = PARAM_setParseOption(p1, PST_PRSCMD_HAS_VALUE_SEQUENCE | PST_PRSCMD_HAS_NO_VALUE);
	CuAssert(tc, "This combinations should not be possible to be applied.", res == PST_PRSCMD_INVALID_COMBINATION);

	res = PARAM_setParseOption(p1, PST_PRSCMD_HAS_VALUE | PST_PRSCMD_HAS_NO_VALUE);
	CuAssert(tc, "This combinations should not be possible to be applied.", res == PST_PRSCMD_INVALID_COMBINATION);

	res = PARAM_setParseOption(p1, PST_PRSCMD_DEFAULT | PST_PRSCMD_HAS_NO_VALUE);
	CuAssert(tc, "This combinations should not be possible to be applied.", res == PST_PRSCMD_INVALID_COMBINATION);

	res = PARAM_setParseOption(p1, PST_PRSCMD_DEFAULT | PST_PRSCMD_HAS_VALUE_SEQUENCE);
	CuAssert(tc, "This combinations should not be possible to be applied.", res == PST_PRSCMD_INVALID_COMBINATION);

	res = PARAM_setParseOption(p1, PST_PRSCMD_DEFAULT | PST_PRSCMD_HAS_VALUE);
	CuAssert(tc, "This combinations should not be possible to be applied.", res == PST_PRSCMD_INVALID_COMBINATION);

	CuAssert(tc, "Parameter must not be changed after multiple unsuccessful function calls.", PARAM_isParseOptionSet(p1, PST_PRSCMD_HAS_VALUE_SEQUENCE | PST_PRSCMD_BREAK_WITH_EXISTING_PARAMETER_MATCH));
	CuAssert(tc, "Invalid parsing flags also set.", !PARAM_isParseOptionSet(p1, PST_PRSCMD_HAS_NO_VALUE | PST_PRSCMD_BREAK_WITH_POTENTIAL_PARAMETER | PST_PRSCMD_DEFAULT | PST_PRSCMD_HAS_VALUE));

	PARAM_free(p1);
}

/* A wildcard character! Must be initialized! */
char wc = ' ';

 static int expand_wildcard_len2str(PARAM_VAL *param_value, void *ctx, int *value_shift) {
	 const char *input = NULL;
	 int res;
	 char **accepted_strs = (char**)ctx;
	 char *str = NULL;
	 const char *src = NULL;
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
		if ((input[0] == wc || input[0] == str[0])
			&& (input[1] == wc || input[1] == str[1])
			&& (input[2] == wc || input[2] == str[2])){

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


static void test_wildcarc_expander(CuTest* tc, const char *charList, int (*expand_wildcard)(PARAM_VAL *param_value, void *ctx, int *value_shift), const char **inputList) {
	int res;
	PARAM *param = NULL;
	PARAM_VAL *value = NULL;
	char *argv[] = {"abc", "cba", "efx", "ebc", "eee", NULL};
	int value_count = 0;
	int expand_count = 0;
	size_t i = 0;

	/**
	 * Create some parameter objects.
     */
	res = PARAM_new("string", NULL, 0, PST_PRSCMD_DEFAULT | PST_PRSCMD_EXPAND_WILDCARD, &param);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	while (inputList[i] != NULL) {
		res += PARAM_addValue(param, inputList[i], NULL, 0);
		i++;
	}

	CuAssert(tc, "Unable to add valid parameters.", res == PST_OK);

	res = PARAM_getValueCount(param, NULL, PST_PRIORITY_NONE, &value_count);
	CuAssert(tc, "Unable to get value count.", res == PST_OK);
	CuAssert(tc, "Invalid value count.", value_count == 6);

	res = PARAM_setWildcardExpander(param, charList, argv, NULL, expand_wildcard);
	CuAssert(tc, "Unable to set wildcard expander.", res == PST_OK);

	res = PARAM_expandWildcard(param, &expand_count);
	CuAssert(tc, "Unable to expand wildcard.", res == PST_OK);
	CuAssert(tc, "Wrong expand count.", expand_count == 7);

	res = PARAM_getValue(param, NULL, PST_PRIORITY_NONE, 0, &value);
	CuAssert(tc, "Unable to get value.", res == PST_OK);
	CuAssert(tc, "Wrong value.", strcmp(value->cstr_value, "efx") == 0);

	res = PARAM_getValue(param, NULL, PST_PRIORITY_NONE, 1, &value);
	CuAssert(tc, "Unable to get value.", res == PST_OK);
	CuAssert(tc, "Wrong value.", strcmp(value->cstr_value, "xxx") == 0);

	res = PARAM_getValue(param, NULL, PST_PRIORITY_NONE, 2, &value);
	CuAssert(tc, "Unable to get value.", res == PST_OK);
	CuAssert(tc, "Wrong value.", strcmp(value->cstr_value, "abc") == 0);

	res = PARAM_getValue(param, NULL, PST_PRIORITY_NONE, 3, &value);
	CuAssert(tc, "Unable to get value.", res == PST_OK);
	CuAssert(tc, "Wrong value.", strcmp(value->cstr_value, "cba") == 0);

	res = PARAM_getValue(param, NULL, PST_PRIORITY_NONE, 4, &value);
	CuAssert(tc, "Unable to get value.", res == PST_OK);
	CuAssert(tc, "Wrong value.", strcmp(value->cstr_value, "ebc") == 0);

	res = PARAM_getValue(param, NULL, PST_PRIORITY_NONE, 5, &value);
	CuAssert(tc, "Unable to get value.", res == PST_OK);
	CuAssert(tc, "Wrong value.", strcmp(value->cstr_value, "yyy") == 0);

	res = PARAM_getValue(param, NULL, PST_PRIORITY_NONE, 6, &value);
	CuAssert(tc, "Unable to get value.", res == PST_OK);
	CuAssert(tc, "Wrong value.", strcmp(value->cstr_value, "efx") == 0);

	res = PARAM_getValue(param, NULL, PST_PRIORITY_NONE, 7, &value);
	CuAssert(tc, "Unable to get value.", res == PST_OK);
	CuAssert(tc, "Wrong value.", strcmp(value->cstr_value, "ebc") == 0);

	res = PARAM_getValue(param, NULL, PST_PRIORITY_NONE, 8, &value);
	CuAssert(tc, "Unable to get value.", res == PST_OK);
	CuAssert(tc, "Wrong value.", strcmp(value->cstr_value, "eee") == 0);

	res = PARAM_getValue(param, NULL, PST_PRIORITY_NONE, 9, &value);
	CuAssert(tc, "There should not be more values.", res == PST_PARAMETER_VALUE_NOT_FOUND);

	PARAM_free(param);
}

static void Test_WildcarcExpander_defaultWC(CuTest* tc) {
	char *values[] = {"ef?", "xxx", "?x?", "?b?", "yyy", "e??", NULL};
	wc = '?'; /* Global variable to change  expand_wildcard_len2str behaviour. */
	test_wildcarc_expander(tc, NULL, expand_wildcard_len2str, (const char**)values);
}

static void Test_WildcarcExpander_defaultSpecifiedWC(CuTest* tc) {
	char *values[] = {"ef*", "xxx", "*x*", "*b*", "yyy", "e**", NULL};
	wc = '*'; /* Global variable to change  expand_wildcard_len2str behaviour. */
	test_wildcarc_expander(tc, "*", expand_wildcard_len2str, (const char**)values);
}

static void Test_root_and_get_values(CuTest* tc) {
	int res;
	PARAM *param = NULL;
	PARAM_VAL *value = NULL;

	/**
	 * Create some parameter objects.
	 */
	res = PARAM_new("string", NULL, 0, PST_PRSCMD_DEFAULT | PST_PRSCMD_EXPAND_WILDCARD, &param);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	res += PARAM_addValue(param, "a", NULL, 0);
	res += PARAM_addValue(param, "b", NULL, 0);
	res += PARAM_addValue(param, "c", NULL, 0);
	res += PARAM_addValue(param, "d", NULL, 0);
	res += PARAM_addValue(param, "e", NULL, 0);
	res += PARAM_addValue(param, "f", NULL, 0);
	CuAssert(tc, "Unable to add valid parameters.", res == PST_OK);

	res = PARAM_getValue(param, NULL, PST_PRIORITY_NONE, 0, &value);
	CuAssert(tc, "Unable to get value from parameter.", res == PST_OK);
	CuAssert(tc, "Wrong value extracted.", res == PST_OK && strcmp(value->cstr_value, "a") == 0);

	res = PARAM_getValue(param, NULL, PST_PRIORITY_NONE, 1, &value);
	CuAssert(tc, "Unable to get value from parameter.", res == PST_OK);
	CuAssert(tc, "Wrong value extracted.", res == PST_OK && strcmp(value->cstr_value, "b") == 0);

	res = PARAM_clearValue(param, NULL, PST_PRIORITY_NONE, 0);
	CuAssert(tc, "Unable to clear first (root) value.", res == PST_OK);

	res = PARAM_getValue(param, NULL, PST_PRIORITY_NONE, 0, &value);
	CuAssert(tc, "Unable to get value from parameter.", res == PST_OK);
	CuAssert(tc, "Wrong value extracted.", res == PST_OK && strcmp(value->cstr_value, "b") == 0);

	PARAM_free(param);
}

static void Test_defaultPrintName(CuTest* tc) {
	int res;
	PARAM *param_1 = NULL;
	PARAM *param_2 = NULL;
	PARAM *param_3 = NULL;
	PARAM *param_4 = NULL;
	const char *expected_print_name_1 = "--string";
	const char *expected_print_name_2 = "-s";
	const char *expected_print_name_3 = "--long";
	const char *expected_print_name_4 = "-l";

	/* Create a single parameter without any special steps to alter its print name. */
	res = PARAM_new("string", NULL, 0, 0, &param_1);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	res = PARAM_new("s", NULL, 0, 0, &param_2);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	res = PARAM_new("long", "l", 0, 0, &param_3);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	res = PARAM_new("l", "long", 0, 0, &param_4);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);


	CuAssert(tc, "Unexpected print name.", strcmp(PARAM_getPrintName(param_1), expected_print_name_1) == 0);
	CuAssert(tc, "Unexpected print name.", strcmp(PARAM_getPrintName(param_2), expected_print_name_2) == 0);

	CuAssert(tc, "Unexpected print name.", strcmp(PARAM_getPrintName(param_3), expected_print_name_3) == 0);
	CuAssert(tc, "Unexpected print name.", strcmp(PARAM_getPrintNameAlias(param_3), expected_print_name_4) == 0);

	CuAssert(tc, "Unexpected print name.", strcmp(PARAM_getPrintName(param_4), expected_print_name_4) == 0);
	CuAssert(tc, "Unexpected print name.", strcmp(PARAM_getPrintNameAlias(param_4), expected_print_name_3) == 0);

	PARAM_free(param_1);
	PARAM_free(param_2);
	PARAM_free(param_3);
	PARAM_free(param_4);
}

static const char* dummy_print_name_implementation(PARAM *param, char *buf, unsigned buf_len){
	VARIABLE_IS_NOT_USED(param);
	VARIABLE_IS_NOT_USED(buf);
	VARIABLE_IS_NOT_USED(buf_len);
	return NULL;
}

static void Test_PrintNameAlias_alias_do_not_exist(CuTest* tc) {
	int res;
	PARAM *param_1 = NULL;

	/* Create a single parameter without any special steps to alter its print name. */
	res = PARAM_new("string", NULL, 0, 0, &param_1);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	res = PARAM_setPrintNameAlias(param_1, "const", NULL);
	CuAssert(tc, "Unexpected return code. Adding print name must fail!", res == PST_ALIAS_NOT_SPECIFIED);

	res = PARAM_setPrintNameAlias(param_1, NULL, dummy_print_name_implementation);
	CuAssert(tc, "Unexpected return code. Adding print name must fail!", res == PST_ALIAS_NOT_SPECIFIED);

	CuAssert(tc, "Alias must be NULL.", PARAM_getPrintNameAlias(param_1) == NULL);

	PARAM_free(param_1);
}

static void Test_constantPrintName(CuTest* tc) {
	int res;
	PARAM *param = NULL;
	char *expected_print_name = "constant";
	char *expected_print_name_alias = "alias constant";

	/* Create a single parameter without any special steps to alter its print name. */
	res = PARAM_new("string", "s", 0, 0, &param);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	res = PARAM_setPrintName(param, expected_print_name, NULL);
	CuAssert(tc, "Unable to set print name.", res == PST_OK);

	res = PARAM_setPrintNameAlias(param, expected_print_name_alias, NULL);
	CuAssert(tc, "Unable to set print name alias.", res == PST_OK);

	CuAssert(tc, "Unexpected print name.", strcmp(PARAM_getPrintName(param), expected_print_name) == 0);
	CuAssert(tc, "Unexpected print name.", strcmp(PARAM_getPrintNameAlias(param), expected_print_name_alias) == 0);

	PARAM_free(param);
}

static void Test_getAttributes(CuTest* tc) {
	int res;
	PARAM *param = NULL;
	PARAM_ATR attributes;

	res = PARAM_new("p", NULL, 0, PST_PRSCMD_DEFAULT | PST_PRSCMD_EXPAND_WILDCARD, &param);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	res += PARAM_addValue(param, "a", "A", 0);
	res += PARAM_addValue(param, "b", NULL, 1);
	res += PARAM_addValue(param, "c", NULL, 0);
	CuAssert(tc, "Unable to add valid parameters.", res == PST_OK);

	res = PARAM_getAtr( param, "A", 0, PST_INDEX_LAST, &attributes);
	CuAssert(tc, "Unable to get attributes.", res == PST_OK);
	CuAssert(tc, "Wrong value extracted.", strcmp(attributes.source, "A") == 0 && attributes.priority == 0);

	res = PARAM_getAtr(param, NULL, 1, PST_INDEX_LAST, &attributes);
	CuAssert(tc, "Unable to get attributes.", res == PST_OK);
	CuAssert(tc, "Wrong value extracted.", attributes.source == NULL && attributes.priority == 1);

	res = PARAM_getAtr(param, NULL, 0, PST_INDEX_LAST, &attributes);
	CuAssert(tc, "Unable to get attributes.", res == PST_OK);
	CuAssert(tc, "Wrong value extracted.", attributes.source == NULL && attributes.priority == 0);

	PARAM_free(param);
}

static void Test_setHelpText(CuTest* tc) {
	int res;
	PARAM *param = NULL;
	const char *expectedHelpText = "This is parameter p description.";

	res = PARAM_new("p", NULL, 0, 0, &param);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);
	CuAssert(tc, "Help text must be NULL.", PARAM_getHelpText(param) == NULL);

	res = PARAM_setHelpText(param, expectedHelpText);
	CuAssert(tc, "Unable to add help text.", res == PST_OK);
	CuAssert(tc, "Unexpected help text.", strcmp(PARAM_getHelpText(param), expectedHelpText) == 0);

	PARAM_free(param);
}

static void Test_getName(CuTest* tc) {
	int res;
	PARAM *param = NULL;
	const char *name = NULL;
	const char *alias = NULL;

	res = PARAM_new("p", "P", 0, PST_PRSCMD_DEFAULT | PST_PRSCMD_EXPAND_WILDCARD, &param);
	CuAssert(tc, "Unable to create PARAM obj.", res == PST_OK);

	res = PARAM_getName(param, &name, &alias);
	CuAssert(tc, "Unable to get name and alias.", res == PST_OK);
	CuAssert(tc, "Name is not correct.", strcmp(name, "p") == 0);
	CuAssert(tc, "Alias is not correct.", strcmp(alias, "P") == 0);

	PARAM_free(param);
}

CuSuite* ParameterTest_getSuite(void) {
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, Test_parameterConstraints);
	SUITE_ADD_TEST(suite, Test_parameterGetValue);
	SUITE_ADD_TEST(suite, Test_SetValuesAndControl);
	SUITE_ADD_TEST(suite, Test_ObjectGetter);
	SUITE_ADD_TEST(suite, Test_ParseOptionSetter);
	SUITE_ADD_TEST(suite, Test_WildcarcExpander_defaultWC);
	SUITE_ADD_TEST(suite, Test_WildcarcExpander_defaultSpecifiedWC);
	SUITE_ADD_TEST(suite, Test_root_and_get_values);
	SUITE_ADD_TEST(suite, Test_defaultPrintName);
	SUITE_ADD_TEST(suite, Test_PrintNameAlias_alias_do_not_exist);
	SUITE_ADD_TEST(suite, Test_constantPrintName);
	SUITE_ADD_TEST(suite, Test_getAttributes);
	SUITE_ADD_TEST(suite, Test_getName);
	SUITE_ADD_TEST(suite, Test_setHelpText);

	return suite;
}

