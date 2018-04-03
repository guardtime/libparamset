/*
 * Copyright 2013-2018 Guardtime, Inc.
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
#include "../src/param_set/strn.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>



static void Test_help_text_with_parameter(CuTest* tc) {
	size_t len = 0;
	char buf[1024] = "";
	char input[] = "1234567890 this is sample";

	char *expected_help_1 = "  -i    - 1234567890\n"
						    "          this is\n"
						    "          sample";
	char *expected_help_2 = "  --lon-param\n"
							"        - 1234567890\n"
						    "          this is\n"
						    "          sample";

	len = PST_snhiprintf(buf, sizeof(buf), 20, 2, 10, "-i", '-', input);
	CuAssert(tc, "Unexpected output for PST_snhiprintf!", strcmp(buf, expected_help_1) == 0);
	CuAssert(tc, "Unexpected len!", len == strlen(expected_help_1));
	buf[0] = '\0';

	len = PST_snhiprintf(buf, sizeof(buf), 20, 2, 10, "--lon-param", '-', input);
	CuAssert(tc, "Unexpected output for PST_snhiprintf!", strcmp(buf, expected_help_2) == 0);
	CuAssert(tc, "Unexpected len!", len == strlen(expected_help_2));
}

static void Test_help_text_description_only_one_word(CuTest* tc) {
	size_t len = 0;
	char buf[1024] = "";
	char *expected_help = "  -i    - word";

	len = PST_snhiprintf(buf, sizeof(buf), 20, 2, 10, "-i", '-', "word");
	CuAssert(tc, "Unexpected output for PST_snhiprintf!", strcmp(buf, expected_help) == 0);
	CuAssert(tc, "Unexpected len!", len == strlen(expected_help));
}

static void Test_help_text_description_change_indent(CuTest* tc) {
	size_t len = 0;
	char buf[1024] = "";
	char *input = "txt txt txt t txt txt txt txt:\\>2\n*\\>4 list item list item\\>2\n*\\>4 list item list item\\>\ntxt txt";
	char *expected_help = "  -i - txt txt txt t\n"
						  "       txt txt txt\n"
						  "       txt:\n"
						  "         * list item\n"
						  "           list item\n"
						  "         * list item\n"
						  "           list item\n"
						  "       txt txt";

	len = PST_snhiprintf(buf, sizeof(buf), 20, 2, 7, "-i", '-', input);
	CuAssert(tc, "Unexpected output for PST_snhiprintf!", strcmp(buf, expected_help) == 0);
	CuAssert(tc, "Unexpected len!", len == strlen(expected_help));
}

static void Test_help_regular_text_without_changing_indentation(CuTest* tc) {
	size_t len = 0;
	char buf[1024] = "";
	char input[] = "1234567890 regular text";
	char *expected_help = "          1234567890\n"
						  "          regular\n"
						  "          text";

	len = PST_snhiprintf(buf, sizeof(buf), 20, 10, 0, NULL, '\0', input);
	CuAssert(tc, "Unexpected output for PST_snhiprintf!", strcmp(buf, expected_help) == 0);
	CuAssert(tc, "Unexpected len!", len == strlen(expected_help));
}

static void Test_help_regular_text_change_indentation(CuTest* tc) {
	size_t len = 0;
	char buf[1024] = "";
	char input[] = "1234567890\\>2 regular text\\>extension\\>4\na\\>0\nto 0";
	char *expected_help = "          1234567890\n"
						  "            regular\n"
						  "            text\n"
						  "          extension\n"
						  "              a\n"
						  "          to 0";

	len = PST_snhiprintf(buf, sizeof(buf), 20, 10, 0, NULL, '\0', input);
	CuAssert(tc, "Unexpected output for PST_snhiprintf!", strcmp(buf, expected_help) == 0);
	CuAssert(tc, "Unexpected len!", len == strlen(expected_help));
}

static void Test_help_regular_text_change_indentation_go_over_limits(CuTest* tc) {
	size_t len = 0;
	char buf[1024] = "";
	char input[] = "1234567890\\>2 text tex text te\\>-2 text text text text\\>-24text text text text text text text text\\>24\na aa aaa\\>0\nxx x";
	char *expected_help = "          1234567890\n"
						  "            text tex\n"
						  "            text te\n"
						  "        text text\n"
						  "        text text\n"
						  "text text text text\n"
						  "text text text text\n"
						  "                   a\n"
						  "                   aa\n"
						  "                   aaa\n"
						  "          xx x";
	len = PST_snhiprintf(buf, sizeof(buf), 20, 10, 0, NULL, '\0', input);
	CuAssert(tc, "Unexpected output for PST_snhiprintf!", strcmp(buf, expected_help) == 0);
	CuAssert(tc, "Unexpected len!", len == strlen(expected_help));
}

static void Test_help_regular_text_invalid_arguments(CuTest* tc) {
	size_t len = 0;
	char buf[1024] = "";
	char input[] = "text text text text";

	len = PST_snhiprintf(NULL, sizeof(buf), 0, 0, 0, NULL, '\0', input);
	CuAssert(tc, "This call must fail due to invalid arguments", len == 0 && buf[0] == '\0');

	len = PST_snhiprintf(buf, 0, 0, 0, 0, NULL, '\0', input);
	CuAssert(tc, "This call must fail due to invalid arguments", len == 0 && buf[0] == '\0');

	len = PST_snhiprintf(buf, sizeof(buf), 5, 5, 0, NULL, '\0', input);
	CuAssert(tc, "This call must fail due to invalid arguments", len == 0 && buf[0] == '\0');

	len = PST_snhiprintf(buf, sizeof(buf), 5, 0, 5, "--plah", '\0', input);
	CuAssert(tc, "This call must fail due to invalid arguments", len == 0 && buf[0] == '\0');

	len = PST_snhiprintf(buf, sizeof(buf), 5, 3, 3, "--plah", '\0', input);
	CuAssert(tc, "This call must fail due to invalid arguments", len == 0 && buf[0] == '\0');
}

static void Test_snhiprintf_parse_errors(CuTest* tc) {
	size_t len = 0;
	char buf[1024] = "";

	len = PST_snhiprintf(buf, sizeof(buf), 20, 10, 0, NULL, '\0', "Test\\k");
	CuAssert(tc, "Unexpected parsing error!", strcmp(buf, "          Test<Parse error: '\\k'>") == 0);
	CuAssert(tc, "Unexpected len!", len == 33);
}


CuSuite* StrnTest_getSuite(void) {
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, Test_help_text_with_parameter);
	SUITE_ADD_TEST(suite, Test_help_text_description_only_one_word);
	SUITE_ADD_TEST(suite, Test_help_text_description_change_indent);
	SUITE_ADD_TEST(suite, Test_help_regular_text_without_changing_indentation);
	SUITE_ADD_TEST(suite, Test_help_regular_text_change_indentation);
	SUITE_ADD_TEST(suite, Test_help_regular_text_change_indentation_go_over_limits);
	SUITE_ADD_TEST(suite, Test_help_regular_text_invalid_arguments);
	SUITE_ADD_TEST(suite, Test_snhiprintf_parse_errors);
	return suite;
}

