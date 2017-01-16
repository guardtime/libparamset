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

#include <string.h>

#include "cutest/CuTest.h"
#include "all_tests.h"
#include "../param_set/param_value.h"
#include "../param_set/param_set_obj_impl.h"

#ifdef _WIN32
#define snprintf _snprintf
#endif

/**
 * Functions covered with tests.
 * PARAM_VAL_new
 * PARAM_VAL_free
 * PARAM_VAL_getElement
 * PARAM_VAL_getInvalid
 * PARAM_VAL_extract
 * PARAM_VAL_getElementCount
 * PARAM_VAL_getPriority
 */

static void assert_value_abstact(CuTest* tc,
		void *value_container,
		int (*getValue)(void*, const char*, int, int, PARAM_VAL **),
		const char *S, int P, int at,
		const char *file, int line,
		const char *expected) {
	int res;
	PARAM_VAL *tmp = NULL;
	const char *V = NULL;
	char buf[2048];
	int count = 0;

	count += snprintf(buf + count, sizeof(buf) - count, "Invalid value '%s' at line %i. ", file, line);

	res = getValue(value_container, S, P, at, &tmp);
	if (res != PST_OK) {
		count += snprintf(buf + count, sizeof(buf) - count, " Unable to get next.");
		CuAssert(tc, buf, 0);
	}

	res = PARAM_VAL_extract(tmp, &V, NULL, NULL);
	if (res != PST_OK || strcmp(V, expected) != 0) {
		count += snprintf(buf + count, sizeof(buf) - count, " Value expected '%s' but is '%s'.", expected, V);
		CuAssert(tc, buf, 0);
	}
}

static void assert_value_unable_to_extract_abstract(CuTest* tc,
		void *value_container,
		int (*getValue)(void*, const char*, int, int, PARAM_VAL **),
		const char *S, int P, int at,
		const char *file, int line) {
	int res1;
	int res2;
	PARAM_VAL *tmp = NULL;
	const char *V = NULL;
	char buf[2048];
	int count = 0;

	count += snprintf(buf + count, sizeof(buf) - count, "It should be the last value '%s' at line %i. ", file, line);

	res1 = getValue(value_container, S, P, at, &tmp);

	if (res1 != PST_PARAMETER_VALUE_NOT_FOUND) {
		res2 = PARAM_VAL_extract(tmp, &V, NULL, NULL);
		CuAssert(tc, buf, res2 == PST_OK);

		count += snprintf(buf + count, sizeof(buf) - count, " Value extracted '%s'.", V);
		CuAssert(tc, buf, 0);
	}
}

static void assert_value(CuTest* tc,
		PARAM_VAL *value, const char *S, int P, int at,
		const char *file, int line,
		const char *expected) {
		assert_value_abstact(tc,
			(void*)value,
			(int (*)(void*, const char*, int, int, PARAM_VAL **))PARAM_VAL_getElement,
			S, P, at, file, line, expected);
}

static void assert_value_unable_to_extract(CuTest* tc,
		PARAM_VAL *value, const char *S, int P, int at,
		const char *file, int line) {
		assert_value_unable_to_extract_abstract(tc,
			(void*)value,
			(int (*)(void*, const char*, int, int, PARAM_VAL **))PARAM_VAL_getElement,
			S, P, at, file, line);
}

static void assert_value_count(CuTest* tc,
		PARAM_VAL *value, const char *S, int P,
		const char *file, int line, int C) {
	int res;
	int count = 0;
	char buf[2048];

	res = PARAM_VAL_getElementCount(value, S, P, &count);
	if (res != PST_OK || count != C) {
		snprintf(buf, sizeof(buf), "Invalid count %i, expected %i in file '%s' at line %i.", count, C, file, line);
		CuAssert(tc, buf, 0);
	}
}



static void Test_param_value(CuTest* tc) {
	int res;
	PARAM_VAL *value = NULL;
	int i;

	/**
	 * Create linked list.
     */
	res = PARAM_VAL_new("1_p0", NULL, 0, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("b1_p0", "B", 0, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("2_p0", NULL, 0, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("b2_p1", "B", 1, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("3_p2", NULL, 2, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("b3_p0", "B", 0, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("b4_p2", "B", 2, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("c1_p2", "C", 2, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	/**
	 * Validate the linked list extraction and counting.
     */

	/* Extract from PST_PRIORITY_NONE, no source. */
	i = 0;
	assert_value(tc, value, NULL, PST_PRIORITY_NONE, i++, __FILE__, __LINE__ , "1_p0");
	assert_value(tc, value, NULL, PST_PRIORITY_NONE, i++, __FILE__, __LINE__ , "b1_p0");
	assert_value(tc, value, NULL, PST_PRIORITY_NONE, i++, __FILE__, __LINE__ , "2_p0");
	assert_value(tc, value, NULL, PST_PRIORITY_NONE, i++, __FILE__, __LINE__ , "b2_p1");
	assert_value(tc, value, NULL, PST_PRIORITY_NONE, i++, __FILE__, __LINE__ , "3_p2");
	assert_value(tc, value, NULL, PST_PRIORITY_NONE, i++, __FILE__, __LINE__ , "b3_p0");
	assert_value(tc, value, NULL, PST_PRIORITY_NONE, i++, __FILE__, __LINE__ , "b4_p2");
	assert_value(tc, value, NULL, PST_PRIORITY_NONE, i++, __FILE__, __LINE__ , "c1_p2");
	assert_value_count(tc, value, NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, i);
	assert_value_unable_to_extract(tc, value, NULL, PST_PRIORITY_NONE, i++, __FILE__, __LINE__ );

	/* Extract from priority level 0, no source. */
	i = 0;
	assert_value(tc, value, NULL, 0, i++, __FILE__, __LINE__ , "1_p0");
	assert_value(tc, value, NULL, 0, i++, __FILE__, __LINE__ , "b1_p0");
	assert_value(tc, value, NULL, 0, i++, __FILE__, __LINE__ , "2_p0");
	assert_value(tc, value, NULL, 0, i++, __FILE__, __LINE__ , "b3_p0");
	assert_value_count(tc, value, NULL, 0, __FILE__, __LINE__, i);
	assert_value_unable_to_extract(tc, value, NULL, 0, i++, __FILE__, __LINE__ );

	/* Extract from priority level 1, no source. */
	i = 0;
	assert_value(tc, value, NULL, 1, i++, __FILE__, __LINE__ , "b2_p1");
	assert_value_count(tc, value, NULL, 1, __FILE__, __LINE__, i);
	assert_value_unable_to_extract(tc, value, NULL, 1, i++, __FILE__, __LINE__ );

	/* Extract from priority level 2, no source. */
	i = 0;
	assert_value(tc, value, NULL, 2, i++, __FILE__, __LINE__ , "3_p2");
	assert_value(tc, value, NULL, 2, i++, __FILE__, __LINE__ , "b4_p2");
	assert_value(tc, value, NULL, 2, i++, __FILE__, __LINE__ , "c1_p2");
	assert_value_count(tc, value, NULL, 2, __FILE__, __LINE__, i);
	assert_value_unable_to_extract(tc, value, NULL, 2, i++, __FILE__, __LINE__ );

	/* Extract from PST_PRIORITY_NONE, source B. */
	i = 0;
	assert_value(tc, value, "B", PST_PRIORITY_NONE, i++, __FILE__, __LINE__ , "b1_p0");
	assert_value(tc, value, "B", PST_PRIORITY_NONE, i++, __FILE__, __LINE__ , "b2_p1");
	assert_value(tc, value, "B", PST_PRIORITY_NONE, i++, __FILE__, __LINE__ , "b3_p0");
	assert_value(tc, value, "B", PST_PRIORITY_NONE, i++, __FILE__, __LINE__ , "b4_p2");
	assert_value_count(tc, value, "B", PST_PRIORITY_NONE, __FILE__, __LINE__, i);
	assert_value_unable_to_extract(tc, value, "B", PST_PRIORITY_NONE, i++, __FILE__, __LINE__ );

	/* Extract from priority level 0, source B. */
	i = 0;
	assert_value(tc, value, "B", 0, i++, __FILE__, __LINE__ , "b1_p0");
	assert_value(tc, value, "B", 0, i++, __FILE__, __LINE__ , "b3_p0");
	assert_value_count(tc, value, "B", 0, __FILE__, __LINE__, i);
	assert_value_unable_to_extract(tc, value, "B", 0, i++, __FILE__, __LINE__ );

	/* Extract from priority level 1, source B. */
	i = 0;
	assert_value(tc, value, "B", 1, i++, __FILE__, __LINE__ , "b2_p1");
	assert_value_count(tc, value, "B", 1, __FILE__, __LINE__, i);
	assert_value_unable_to_extract(tc, value, "B", 1, i++, __FILE__, __LINE__ );

	/* Extract from priority level 2, source B. */
	i = 0;
	assert_value(tc, value, "B", 2, i++, __FILE__, __LINE__ , "b4_p2");
	assert_value_count(tc, value, "B", 2, __FILE__, __LINE__, i);
	assert_value_unable_to_extract(tc, value, "B", 2, i++, __FILE__, __LINE__ );

	/* Extract from priority level 2, source C. */
	i = 0;
	assert_value(tc, value, "C", PST_PRIORITY_NONE, i++, __FILE__, __LINE__ , "c1_p2");
	assert_value_count(tc, value, "C", PST_PRIORITY_NONE, __FILE__, __LINE__, i);
	assert_value_unable_to_extract(tc, value, "C", PST_PRIORITY_NONE, i++, __FILE__, __LINE__ );

	/* Extract from priority level PST_PRIORITY_LOWEST, no source. */
	i = 0;
	assert_value(tc, value, NULL, PST_PRIORITY_LOWEST, i++, __FILE__, __LINE__ , "1_p0");
	assert_value(tc, value, NULL, PST_PRIORITY_LOWEST, i++, __FILE__, __LINE__ , "b1_p0");
	assert_value(tc, value, NULL, PST_PRIORITY_LOWEST, i++, __FILE__, __LINE__ , "2_p0");
	assert_value(tc, value, NULL, PST_PRIORITY_LOWEST, i++, __FILE__, __LINE__ , "b3_p0");
	assert_value_count(tc, value, NULL, PST_PRIORITY_LOWEST, __FILE__, __LINE__, i);
	assert_value_unable_to_extract(tc, value, NULL, PST_PRIORITY_LOWEST, i++, __FILE__, __LINE__ );

	/* Extract from priority level PST_PRIORITY_LOWEST, source B. */
	i = 0;
	assert_value(tc, value, "B", PST_PRIORITY_LOWEST, i++, __FILE__, __LINE__ , "b1_p0");
	assert_value(tc, value, "B", PST_PRIORITY_LOWEST, i++, __FILE__, __LINE__ , "b3_p0");
	assert_value_count(tc, value, "B", PST_PRIORITY_LOWEST, __FILE__, __LINE__, i);
	assert_value_unable_to_extract(tc, value, "B", PST_PRIORITY_LOWEST, i++, __FILE__, __LINE__ );

	/* Extract from priority level PST_PRIORITY_HIGHEST, source B. */
	i = 0;
	assert_value(tc, value, "B", PST_PRIORITY_HIGHEST, i++, __FILE__, __LINE__ , "b4_p2");
	assert_value_count(tc, value, "B", PST_PRIORITY_HIGHEST, __FILE__, __LINE__, i);
	assert_value_unable_to_extract(tc, value, "B", PST_PRIORITY_HIGHEST, i++, __FILE__, __LINE__ );

	/* Extract from priority level PST_PRIORITY_HIGHEST, source C. */
	i = 0;
	assert_value(tc, value, "C", PST_PRIORITY_HIGHEST, i++, __FILE__, __LINE__ , "c1_p2");
	assert_value_count(tc, value, "C", PST_PRIORITY_HIGHEST, __FILE__, __LINE__, i);
	assert_value_unable_to_extract(tc, value, "C", PST_PRIORITY_HIGHEST, i++, __FILE__, __LINE__ );

	/* Extract from priority level PST_PRIORITY_HIGHER than 1, source NULL. */
	i = 0;
	assert_value(tc, value, NULL, PST_PRIORITY_HIGHER_THAN + 1, i++, __FILE__, __LINE__ , "3_p2");
	assert_value(tc, value, NULL, PST_PRIORITY_HIGHER_THAN + 1, i++, __FILE__, __LINE__ , "b4_p2");
	assert_value(tc, value, NULL, PST_PRIORITY_HIGHER_THAN + 1, i++, __FILE__, __LINE__ , "c1_p2");
	assert_value_unable_to_extract(tc, value, NULL, PST_PRIORITY_HIGHER_THAN + 1, i++, __FILE__, __LINE__ );

	/* Extract from priority level PST_PRIORITY_LOWER_THAN than 2, source NULL. */
	i = 0;
	assert_value(tc, value, NULL, PST_PRIORITY_LOWER_THAN + 2, i++, __FILE__, __LINE__ , "1_p0");
	assert_value(tc, value, NULL, PST_PRIORITY_LOWER_THAN + 2, i++, __FILE__, __LINE__ , "b1_p0");
	assert_value(tc, value, NULL, PST_PRIORITY_LOWER_THAN + 2, i++, __FILE__, __LINE__ , "2_p0");
	assert_value(tc, value, NULL, PST_PRIORITY_LOWER_THAN + 2, i++, __FILE__, __LINE__ , "b2_p1");
	assert_value(tc, value, NULL, PST_PRIORITY_LOWER_THAN + 2, i++, __FILE__, __LINE__ , "b3_p0");
	assert_value_unable_to_extract(tc, value, NULL, PST_PRIORITY_LOWER_THAN + 2, i++, __FILE__, __LINE__ );

	PARAM_VAL_free(value);
}

static void Test_param_value_priority_limits(CuTest* tc) {
	int res;
	PARAM_VAL *value = NULL;
	int i;

	/**
	 * Create linked list.
     */
	res = PARAM_VAL_new("error", NULL, PST_PRIORITY_NOTDEFINED, &value);
	CuAssert(tc, "It should be impossible to create value with negative priority.", res == PST_PRIORITY_NEGATIVE);

	res = PARAM_VAL_new("error", NULL, PST_PRIORITY_NOTDEFINED - 1, &value);
	CuAssert(tc, "It should be impossible to create value with negative priority.", res == PST_PRIORITY_NEGATIVE);

	res = PARAM_VAL_new("error", NULL, PST_PRIORITY_VALID_BASE - 1, &value);
	CuAssert(tc, "It should be impossible to create value with negative priority.", res == PST_PRIORITY_NEGATIVE);

	res = PARAM_VAL_new("error", NULL, PST_PRIORITY_VALID_ROOF + 1, &value);
	CuAssert(tc, "It should be impossible to create value higher priority than the valid roof.", res == PST_PRIORITY_TOO_LARGE);


	res = PARAM_VAL_new("base", NULL, PST_PRIORITY_VALID_BASE, &value);
	CuAssert(tc, "It should be possible to create a value with priority < valid roof and > valid base.", res == PST_OK);

	res = PARAM_VAL_new("roof-1", NULL, PST_PRIORITY_VALID_ROOF - 1, &value);
	CuAssert(tc, "It should be possible to create a value with priority <= valid roof.", res == PST_OK);

	res = PARAM_VAL_new("roof", NULL, PST_PRIORITY_VALID_ROOF, &value);
	CuAssert(tc, "It should be possible to create a value with priority <= valid roof.", res == PST_OK);

	/**
	 * Validate the linked list extraction and counting.
     */

	i = 0;
	assert_value(tc, value, NULL, PST_PRIORITY_LOWER_THAN + PST_PRIORITY_VALID_ROOF, i++, __FILE__, __LINE__ , "base");
	assert_value(tc, value, NULL, PST_PRIORITY_LOWER_THAN + PST_PRIORITY_VALID_ROOF, i++, __FILE__, __LINE__ , "roof-1");
	assert_value_count(tc, value, NULL, PST_PRIORITY_LOWER_THAN + PST_PRIORITY_VALID_ROOF, __FILE__, __LINE__, i);
	assert_value_unable_to_extract(tc, value, NULL, PST_PRIORITY_LOWER_THAN + PST_PRIORITY_VALID_ROOF, i++, __FILE__, __LINE__ );

	i = 0;
	assert_value(tc, value, NULL, PST_PRIORITY_HIGHER_THAN + PST_PRIORITY_VALID_BASE, i++, __FILE__, __LINE__ , "roof-1");
	assert_value(tc, value, NULL, PST_PRIORITY_HIGHER_THAN + PST_PRIORITY_VALID_BASE, i++, __FILE__, __LINE__ , "roof");
	assert_value_count(tc, value, NULL, PST_PRIORITY_HIGHER_THAN + PST_PRIORITY_VALID_BASE, __FILE__, __LINE__, i);
	assert_value_unable_to_extract(tc, value, NULL, PST_PRIORITY_HIGHER_THAN + PST_PRIORITY_VALID_BASE, i++, __FILE__, __LINE__ );

	/**
     * Check if priority borders do exists.
     */
	assert_value_unable_to_extract(tc, value, NULL, PST_PRIORITY_HIGHER_THAN + PST_PRIORITY_VALID_ROOF, 0, __FILE__, __LINE__ );
	assert_value_unable_to_extract(tc, value, NULL, PST_PRIORITY_LOWER_THAN + PST_PRIORITY_VALID_BASE, 0, __FILE__, __LINE__ );

	PARAM_VAL_free(value);
}

static void Test_param_extract_last_higest_priority(CuTest* tc) {
	int res;
	PARAM_VAL *value = NULL;
	int i;

	/**
	 * Create linked list.
     */
	res = PARAM_VAL_new("1_p3", NULL, 3, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("b1_p2", "B", 2, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("2_p1", NULL, 1, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);


	/* Extract from PST_PRIORITY_NONE, no source. */
	i = 0;
	assert_value(tc, value, NULL, PST_PRIORITY_NONE, i++, __FILE__, __LINE__ , "1_p3");
	assert_value(tc, value, NULL, PST_PRIORITY_NONE, i++, __FILE__, __LINE__ , "b1_p2");
	assert_value(tc, value, NULL, PST_PRIORITY_NONE, i++, __FILE__, __LINE__ , "2_p1");

	assert_value(tc, value, NULL, PST_PRIORITY_HIGHEST, PST_INDEX_LAST, __FILE__, __LINE__ , "1_p3");


	PARAM_VAL_free(value);
}

static void Test_param_extractPriority(CuTest* tc) {
	int res;
	PARAM_VAL *value = NULL;
	int prio = PST_PRIORITY_NOTDEFINED;

	/**
	 * Create linked list.
     */
	res = PARAM_VAL_new("1_p0", NULL, 0, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("b1_p3", "B", 3, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("2_p0", NULL, 0, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("b2_p1", "B", 1, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("3_p2", NULL, 2, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("b3_p0", "B", 0, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("b4_p2", "B", 2, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("c1_p2", "C", 2, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);


	/**
	 * Extract invalid priority values.
     */
	prio = 0xffff;
	res = PARAM_VAL_getPriority(value, PST_PRIORITY_NONE, &prio);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_INVALID_ARGUMENT && prio == 0xffff);

	res = PARAM_VAL_getPriority(value, PST_PRIORITY_NOTDEFINED, &prio);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_INVALID_ARGUMENT && prio == 0xffff);

	/**
	 * Extract valid priority values.
     */

	prio = PST_PRIORITY_NOTDEFINED;
	res = PARAM_VAL_getPriority(value, PST_PRIORITY_HIGHEST, &prio);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK && prio == 3);

	/* Get the first priority. */
	res = PARAM_VAL_getPriority(value, PST_PRIORITY_LOWEST, &prio);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK && prio == 0);

	/* Get the next priority. */
	res = PARAM_VAL_getPriority(value, prio, &prio);
	CuAssert(tc, "Invalid priority.", res == PST_OK && prio == 1);

	res = PARAM_VAL_getPriority(value, prio, &prio);
	CuAssert(tc, "Invalid priority.", res == PST_OK && prio == 2);

	res = PARAM_VAL_getPriority(value, prio, &prio);
	CuAssert(tc, "Invalid priority.", res == PST_OK && prio == 3);

	res = PARAM_VAL_getPriority(value, prio, &prio);
	CuAssert(tc, "Invalid priority.", res == PST_PARAMETER_VALUE_NOT_FOUND && prio == 3);


	PARAM_VAL_free(value);
}

static void Test_param_extractInvalid(CuTest* tc) {
	int res;
	PARAM_VAL *value = NULL;
	PARAM_VAL *tmp = NULL;
	int count = 0xffff;

	/**
	 * Create linked list.
     */
	res = PARAM_VAL_new("1_p0", NULL, 0, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("b1_p3", "B", 3, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("2_p0", NULL, 0, &value);	/* Invalid 0. */
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("b2_p1", "B", 1, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("3_p2", NULL, 2, &value); 	/* Invalid 1. */
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("b3_p0", "B", 0, &value); 	/* Invalid 2. */
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("b4_p2", "B", 2, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("c1_p2", "C", 2, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_getInvalid(value, NULL, PST_PRIORITY_NONE, 0, &tmp);
	CuAssert(tc, "All values must be valid.", res == PST_PARAMETER_VALUE_NOT_FOUND);

	res = PARAM_VAL_getInvalidCount(value, NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "All values must be valid.", res == PST_OK && count == 0);

	/**
	 * Make some values invalid.
	 */
	res = PARAM_VAL_getElement(value, NULL, PST_PRIORITY_NONE, 2, &tmp);
	CuAssert(tc, "Unable to get value.", res == PST_OK);
	tmp->contentStatus = 1;

	res = PARAM_VAL_getElement(value, NULL, PST_PRIORITY_NONE, 4, &tmp);
	CuAssert(tc, "Unable to get value.", res == PST_OK);
	tmp->formatStatus = 2;

	res = PARAM_VAL_getElement(value, NULL, PST_PRIORITY_NONE, 5, &tmp);
	CuAssert(tc, "Unable to get value.", res == PST_OK);
	tmp->formatStatus = 3;
	tmp = NULL;

	/**
	 * Extract invalid values.
     */
	res = PARAM_VAL_getInvalid(value, NULL, PST_PRIORITY_NONE, 0, &tmp);
	CuAssert(tc, "Unable to extract invalid value.", res == PST_OK && tmp->contentStatus == 1 && tmp->formatStatus == 0);

	res = PARAM_VAL_getInvalid(value, NULL, PST_PRIORITY_NONE, 1, &tmp);
	CuAssert(tc, "Unable to extract invalid value.", res == PST_OK && tmp->formatStatus == 2 && tmp->contentStatus == 0);

	res = PARAM_VAL_getInvalid(value, NULL, PST_PRIORITY_NONE, 2, &tmp);
	CuAssert(tc, "Unable to extract invalid value.", res == PST_OK && tmp->formatStatus == 3 && tmp->contentStatus == 0);

	res = PARAM_VAL_getInvalid(value, NULL, PST_PRIORITY_NONE, 3, &tmp);
	CuAssert(tc, "Unable to extract invalid value.", res == PST_PARAMETER_VALUE_NOT_FOUND);


	/**
	 * Count invalid values with constraints.
     */
	res = PARAM_VAL_getInvalidCount(value, "B", 1, &count);
	CuAssert(tc, "The count with given constraints must be 0.", res == PST_OK && count == 0);

	res = PARAM_VAL_getInvalidCount(value, NULL, PST_PRIORITY_NONE, &count);
	CuAssert(tc, "All values must be valid.", res == PST_OK && count == 3);

	res = PARAM_VAL_getInvalidCount(value, "B", PST_PRIORITY_NONE, &count);
	CuAssert(tc, "All values must be valid.", res == PST_OK && count == 1);

	res = PARAM_VAL_getInvalidCount(value, NULL, 0, &count);
	CuAssert(tc, "All values must be valid.", res == PST_OK && count == 2);


	PARAM_VAL_free(value);
}



static void assert_value_pop_and_clean(CuTest* tc,
		PARAM_VAL **value, const char *S, int P, int at,
		const char *file, int line, const char *popped, const char *root, const char *last) {
	int res;
	PARAM_VAL *pop = NULL;
	PARAM_VAL *L = NULL;
	char buf[2048];
	int count = 0;

	count += snprintf(buf + count, sizeof(buf) - count, "Unable to pop '%s' at line %i.", file, line);

	res = PARAM_VAL_popElement(value, S, P, at, &pop);
	if (res != PST_OK) {
		CuAssert(tc, buf, 0);
	}

	if (pop == NULL) {
		count += snprintf(buf + count, sizeof(buf) - count, "Poped value is NULL.");
		CuAssert(tc, buf, 0);
	}

	if (pop->next != NULL || pop->previous != NULL || strcmp(pop->cstr_value, popped) != 0) {
		count += snprintf(buf + count, sizeof(buf) - count,
				"Invalid pop next->%p, previous->%p, value '%s', but expected '%s'.",
				(void*)(pop->next), (void*)(pop->previous), pop->cstr_value, popped);
		CuAssert(tc, buf, 0);
	}

	if (strcmp((*value)->cstr_value, root) != 0) {
		count += snprintf(buf + count, sizeof(buf) - count, " Root value is invalid '%s', bu expected '%s'.",
				(*value)->cstr_value, root);
		CuAssert(tc, buf, 0);
	}

	res = PARAM_VAL_getElement(*value, NULL, PST_PRIORITY_NONE, PST_INDEX_LAST, &L);
	if (res != PST_OK) {
		count += snprintf(buf + count, sizeof(buf) - count, "Unable to get last value.");
		CuAssert(tc, buf, 0);
	}

	if (strcmp(L->cstr_value, last) != 0) {
		count += snprintf(buf + count, sizeof(buf) - count, " Last value is invalid '%s', bu expected '%s'.",
				L->cstr_value, last);
		CuAssert(tc, buf, 0);
	}


	PARAM_VAL_free(pop);
	pop = NULL;
}




static void Test_param_pop(CuTest* tc) {
	int res;
	PARAM_VAL *value = NULL;
	PARAM_VAL *pop = NULL;

	/**
	 * Create linked list.
     */
	res = PARAM_VAL_new("1_p0", NULL, 0, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("b1_p3", "B", 3, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("2_p0", NULL, 0, &value);	/* Invalid 0. */
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("b2_p1", "B", 1, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("3_p2", NULL, 2, &value); 	/* Invalid 1. */
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("b3_p0", "B", 0, &value); 	/* Invalid 2. */
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("b4_p2", "B", 2, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("c1_p2", "C", 2, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	assert_value_count(tc, value, NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 8);

	/**
	 * Free the first element
     */
	assert_value_pop_and_clean(tc, &value, NULL, PST_PRIORITY_NONE, 0, __FILE__, __LINE__, "1_p0", "b1_p3", "c1_p2");
	assert_value_count(tc, value, NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 7);

	assert_value_pop_and_clean(tc, &value, NULL, PST_PRIORITY_NONE, PST_INDEX_LAST, __FILE__, __LINE__, "c1_p2", "b1_p3", "b4_p2");
	assert_value_count(tc, value, NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 6);

	assert_value_pop_and_clean(tc, &value, "B", 1, 0, __FILE__, __LINE__, "b2_p1", "b1_p3", "b4_p2");
	assert_value_count(tc, value, NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 5);

	res = PARAM_VAL_popElement(&value, "B", 1, 0, &pop);
	CuAssert(tc, "There should be not possible to pop anay values.", res == PST_PARAMETER_VALUE_NOT_FOUND && pop == NULL);

	assert_value_pop_and_clean(tc, &value, "B", PST_PRIORITY_NONE, 1, __FILE__, __LINE__, "b3_p0", "b1_p3", "b4_p2");
	assert_value_count(tc, value, NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 4);

	assert_value_pop_and_clean(tc, &value, "B", PST_PRIORITY_NONE, 0, __FILE__, __LINE__, "b1_p3", "2_p0", "b4_p2");
	assert_value_count(tc, value, NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 3);

	assert_value_pop_and_clean(tc, &value, "B", PST_PRIORITY_NONE, 0, __FILE__, __LINE__, "b4_p2", "2_p0", "3_p2");
	assert_value_count(tc, value, NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 2);

	res = PARAM_VAL_popElement(&value, "B", PST_PRIORITY_NONE, 0, &pop);
	CuAssert(tc, "There should be not possible to pop anay values.", res == PST_PARAMETER_VALUE_NOT_FOUND && pop == NULL);

	assert_value_pop_and_clean(tc, &value, NULL, PST_PRIORITY_NONE, 0, __FILE__, __LINE__, "2_p0", "3_p2", "3_p2");
	assert_value_count(tc, value, NULL, PST_PRIORITY_NONE, __FILE__, __LINE__, 1);

	res = PARAM_VAL_popElement(&value, NULL, PST_PRIORITY_NONE, 0, &pop);
	CuAssert(tc, "Not possible to pop the last value.", res == PST_OK && pop != NULL && value == NULL);

	PARAM_VAL_free(pop);
}

static void itr_assert_value(CuTest* tc,
		ITERATOR *value, const char *S, int P, int at,
		const char *file, int line,
		const char *expected) {
		assert_value_abstact(tc,
			(void*)value,
			(int (*)(void*, const char*, int, int, PARAM_VAL **))ITERATOR_fetch,
			S, P, at, file, line, expected);
}

static void itr_assert_value_unable_to_extract(CuTest* tc,
		ITERATOR *value, const char *S, int P, int at,
		const char *file, int line) {
		assert_value_unable_to_extract_abstract(tc,
			(void*)value,
			(int (*)(void*, const char*, int, int, PARAM_VAL **))ITERATOR_fetch,
			S, P, at, file, line);
}
static void Test_iteratorLifecycle(CuTest* tc) {
	int res;
	PARAM_VAL *value = NULL;
	ITERATOR *itr = NULL;

	/**
	 * Create linked list.
     */
	res = PARAM_VAL_new("a", NULL, 0, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = PARAM_VAL_new("b", "B", 3, &value);
	CuAssert(tc, "Unable to create parameter value object.", res == PST_OK);

	res = ITERATOR_new(value, &itr);
	CuAssert(tc, "Unable to create iterator.", res == PST_OK);

	itr_assert_value(tc, itr, NULL, PST_PRIORITY_NONE, 0, __FILE__, __LINE__, "a");
	itr_assert_value(tc, itr, NULL, PST_PRIORITY_NONE, 1, __FILE__, __LINE__, "b");
	itr_assert_value_unable_to_extract(tc, itr, NULL, PST_PRIORITY_NONE, 2,  __FILE__, __LINE__);
	itr_assert_value(tc, itr, NULL, PST_PRIORITY_NONE, 0, __FILE__, __LINE__, "a");

	ITERATOR_free(itr);
	PARAM_VAL_free(value);
}

static void Test_iterator(CuTest* tc) {
	int res = 0;
	PARAM_VAL *value = NULL;
	ITERATOR *itr = NULL;

	/**
	 * Create linked list.
     */
	res = PARAM_VAL_new("a", NULL, 0, &value);
	res += PARAM_VAL_new("b", "B", 3, &value);
	res += PARAM_VAL_new("c", NULL, 2, &value);
	res += PARAM_VAL_new("d", "D", 3, &value);
	res += PARAM_VAL_new("e", NULL, 0, &value);
	res += PARAM_VAL_new("f", NULL, 1, &value);

	CuAssert(tc, "Unable to create parameter value objects.", res == PST_OK);

	res = ITERATOR_new(value, &itr);
	CuAssert(tc, "Unable to create iterator.", res == PST_OK);


	CuAssert(tc, "Iterator Internal index variable mismatch.", itr->i == 0);
	itr_assert_value(tc, itr, NULL, PST_PRIORITY_NONE, 0, __FILE__, __LINE__, "a");
	CuAssert(tc, "Iterator Internal index variable mismatch.", itr->i == 0);
	itr_assert_value(tc, itr, NULL, PST_PRIORITY_NONE, 0, __FILE__, __LINE__, "a");
	CuAssert(tc, "Iterator Internal index variable mismatch.", itr->i == 0);
	itr_assert_value(tc, itr, NULL, PST_PRIORITY_NONE, 1, __FILE__, __LINE__, "b");
	CuAssert(tc, "Iterator Internal index variable mismatch.", itr->i == 1);
	itr_assert_value(tc, itr, NULL, PST_PRIORITY_NONE, 1, __FILE__, __LINE__, "b");
	CuAssert(tc, "Iterator Internal index variable mismatch.", itr->i == 1);
	itr_assert_value(tc, itr, NULL, PST_PRIORITY_NONE, 2, __FILE__, __LINE__, "c");
	CuAssert(tc, "Iterator Internal index variable mismatch.", itr->i == 2);
	itr_assert_value(tc, itr, NULL, PST_PRIORITY_NONE, 3, __FILE__, __LINE__, "d");
	CuAssert(tc, "Iterator Internal index variable mismatch.", itr->i == 3);
	itr_assert_value(tc, itr, NULL, PST_PRIORITY_NONE, 4, __FILE__, __LINE__, "e");
	CuAssert(tc, "Iterator Internal index variable mismatch.", itr->i == 4);
	itr_assert_value(tc, itr, NULL, PST_PRIORITY_NONE, 5, __FILE__, __LINE__, "f");
	CuAssert(tc, "Iterator Internal index variable mismatch.", itr->i == 5);
	itr_assert_value_unable_to_extract(tc, itr, NULL, PST_PRIORITY_NONE, 6,  __FILE__, __LINE__);

	ITERATOR_free(itr);
	PARAM_VAL_free(value);
}

static void Test_iteratorWithSpecialAtValues(CuTest* tc) {
	int res = 0;
	PARAM_VAL *value = NULL;
	ITERATOR *itr = NULL;

	/**
	 * Create linked list.
     */
	res = PARAM_VAL_new("a", NULL, 0, &value);
	res += PARAM_VAL_new("b", "B", 3, &value);
	res += PARAM_VAL_new("c", NULL, 2, &value);
	res += PARAM_VAL_new("d", "D", 3, &value);
	res += PARAM_VAL_new("e", NULL, 0, &value);
	res += PARAM_VAL_new("f", NULL, 1, &value);

	CuAssert(tc, "Unable to create parameter value objects.", res == PST_OK);

	res = ITERATOR_new(value, &itr);
	CuAssert(tc, "Unable to create iterator.", res == PST_OK);


	CuAssert(tc, "Iterator Internal index variable mismatch.", itr->i == 0);
	itr_assert_value(tc, itr, NULL, PST_PRIORITY_NONE, PST_INDEX_FIRST, __FILE__, __LINE__, "a");

	CuAssert(tc, "Iterator Internal index variable mismatch.", itr->i == 0);
	itr_assert_value(tc, itr, "D", PST_PRIORITY_NONE, PST_INDEX_FIRST, __FILE__, __LINE__, "d");

	CuAssert(tc, "Iterator Internal index variable mismatch.", itr->i == 0);
	itr_assert_value(tc, itr, "B", PST_PRIORITY_NONE, PST_INDEX_FIRST, __FILE__, __LINE__, "b");

	CuAssert(tc, "Iterator Internal index variable mismatch.", itr->i == 0);
	itr_assert_value(tc, itr, NULL, 3, PST_INDEX_FIRST, __FILE__, __LINE__, "b");

	CuAssert(tc, "Iterator Internal index variable mismatch.", itr->i == 0);
	itr_assert_value(tc, itr, NULL, 3, PST_INDEX_LAST, __FILE__, __LINE__, "d");

	CuAssert(tc, "Iterator Internal index variable mismatch.", itr->i == PST_INDEX_LAST);
	itr_assert_value(tc, itr, NULL, PST_PRIORITY_NONE, PST_INDEX_LAST, __FILE__, __LINE__, "f");

	ITERATOR_free(itr);
	PARAM_VAL_free(value);
}

CuSuite* ParamValueTest_getSuite(void) {
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, Test_param_value);
	SUITE_ADD_TEST(suite, Test_param_value_priority_limits);
	SUITE_ADD_TEST(suite, Test_param_extract_last_higest_priority);
	SUITE_ADD_TEST(suite, Test_param_extractPriority);
	SUITE_ADD_TEST(suite, Test_param_extractInvalid);
	SUITE_ADD_TEST(suite, Test_param_pop);
	SUITE_ADD_TEST(suite, Test_iteratorLifecycle);
	SUITE_ADD_TEST(suite, Test_iterator);
	SUITE_ADD_TEST(suite, Test_iteratorWithSpecialAtValues);

	return suite;
}

