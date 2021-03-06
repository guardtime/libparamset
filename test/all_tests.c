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

#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include<stdlib.h>

#include "cutest/CuTest.h"
#include "all_tests.h"
#include "support_tests.h"
#include <ctype.h>

#ifndef UNIT_TEST_OUTPUT_XML
#  define UNIT_TEST_OUTPUT_XML "_testsuite.xml"
#endif


static CuSuite* initSuite(void) {
	CuSuite *suite = CuSuiteNew();

	addSuite(suite, ParamValueTest_getSuite);
	addSuite(suite, ParameterTest_getSuite);
	addSuite(suite, ParamSetTest_getSuite);
	addSuite(suite, TaskDefTest_getSuite);
	addSuite(suite, Command_LineTest_getSuite);
	addSuite(suite, StrnTest_getSuite);

	return suite;
}

static int RunAllTests() {
	int failCount;
	CuSuite* suite = initSuite();


	CuSuiteRun(suite);

	printStats(suite, "==== PARAM_SET TEST RESULTS ====");

	writeXmlReport(suite, UNIT_TEST_OUTPUT_XML);

	failCount = suite->failCount;

	CuSuiteDelete(suite);

	return failCount;
}


int main(int argc, char** argv) {
	int ret = EXIT_FAILURE;
#ifdef _WIN32
#ifdef _DEBUG
//	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
//	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
//	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDERR );
#endif
#endif

	if (argc != 2) {
		printf("Usage:\n %s <path to test root>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	initFullResourcePath(argv[1]);
	ret = RunAllTests();
//	_CrtDumpMemoryLeaks();

	return ret;
}
