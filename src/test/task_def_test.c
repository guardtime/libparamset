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
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include "cutest/CuTest.h"
#include "all_tests.h"
#include "../param_set/param_set_obj_impl.h"
#include "../param_set/param_set.h"
#include "../param_set/task_def.h"

#define LIFO_SIZE 32

#ifdef _WIN32
#define snprintf _snprintf
#endif

static int isValidNameChar(int c) {
	if (c == ',' || isspace(c)) return 0;
	else return 1;
}

static const char* category_extract_name(const char* category, char *buf, short len, int *flags){
	int cat_i = 0;
	int buf_i = 0;
	int tmp_flags = 0;
	int isNameOpen = 0;
	int isFlagsOpen = 0;


	if (category == NULL || category[0] == 0 || buf == NULL) {
		return NULL;
	}

	while (category[cat_i] != 0) {
		/* If buf is going to be full, return NULL. */
		if (buf_i >= len - 1) {
			buf[len - 1] = 0;
			return NULL;
		}

		/**
		 * Extract the name and extra flags.
         */
		if (buf_i == 0 && !isNameOpen && !isFlagsOpen && isValidNameChar(category[cat_i])) {
			isNameOpen = 1;
		} else if (isNameOpen && buf_i > 0 && !isValidNameChar(category[cat_i]) && category[cat_i] != '-') {
			isNameOpen = 0;
		}


		if (buf_i > 0 && !isNameOpen && !isFlagsOpen && category[cat_i] == '[') {
			isFlagsOpen = 1;
		} else if (isFlagsOpen && category[cat_i] == ']') {
			isFlagsOpen = 0;
		}

		/**
		 * Extract the data fields.
         */
		if (isNameOpen) {
			buf[buf_i++] = category[cat_i];
		} else if (isFlagsOpen) {
			/*TODO: extract flags*/
		} else if (buf_i > 0){
			break;
		}

		cat_i++;
	}

	if (buf[0] == '0') {
		return NULL;
	}

	buf[buf_i] = 0;
	if (flags != NULL) {
		*flags = tmp_flags;
	}

	return &category[cat_i];
}

static int param_set_add(CuTest* tc, PARAM_SET *set, const char *names, const char *file, int line) {
	int res;
	char buf[1024];
	char name_buf[1024];
	size_t count = 0;
	const char *name = NULL;

	count += snprintf(buf + count, sizeof(buf) - count, "Error at: %i in file: %s. ", line, file);

	if (names == NULL) {
		count += snprintf(buf + count, sizeof(buf) - count, " names == NULL");
		CuAssert(tc, buf, 0);
	}

	name = names;
	while((name = category_extract_name(name ,name_buf, sizeof(name_buf), NULL)) != NULL) {
		if (strlen(name_buf) != 0) {
			res = PARAM_SET_add(set, name_buf, NULL, NULL, 0);
			if (res != PST_OK) {
				count += snprintf(buf + count, sizeof(buf) - count, " Unable to add '%s' to param set. Error 0x%x.", name_buf, res);
				CuAssert(tc, buf, 0);
			}
		}
	}

	return PST_OK;
}

void assert_getConsistancy(CuTest* tc, TASK_DEFINITION *task_def, PARAM_SET *set, const char *file, int line, double cons) {
	double c = 0.0 ;
	char buf[1024];
	TASK_DEFINITION_analyzeConsistency(task_def, set, &c);
	if (fabs(c - cons) >= 0.001) {
		snprintf(buf, sizeof(buf), "Error in file '%s' at line %i: Invalid consistency (Task id %i) %f but expected %f. Error %i/100.", file, line, task_def->id, c, cons, abs((int)(1000.0*(c-cons))));
		printf("%s\n", buf);
		CuAssert(tc, buf, 0);
	}
}


#define NEW_TASK_DEF(_man, _atl, _forb, _expected) \
	expected_cons[n] = _expected; \
	res = TASK_DEFINITION_new(n, "Task", _man, _atl, _forb, NULL, &(array[n])); \
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && array[n] != NULL); \
	n++\

#define MAX_TASK_COUNT 64

static void Test_task_definition_consistency(CuTest* tc) {
	int res;
	int i;
	int n = 0;
	double expected_cons[MAX_TASK_COUNT];
	TASK_DEFINITION *array[MAX_TASK_COUNT];
	PARAM_SET *set = NULL;
	int task_count = 0;

	for (i = 0; i < MAX_TASK_COUNT; i++) {
		array[i] = NULL;
		expected_cons[i] = -100;
	}

	/**
	 * Configure parameter set _ before indicates that the flag is never set.
     */
	res = PARAM_SET_new("{a}{d}{x}{y}{z}{f}{g}{l}"
						"{_b}{_c}{_h}{_m}{_n}",
			&set);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && set != NULL);

	param_set_add(tc, set, "a,d,x,y,z,f,g,l", __FILE__, __LINE__);

	/**
	 * Configure tasks
     */

	/* MAN, ATL, FORB, EXPECTED CONSISTANCY*/
	NEW_TASK_DEF("a", NULL, NULL, 1.0);
	NEW_TASK_DEF("a", "_c,d", NULL, 1.0);
	NEW_TASK_DEF("_b", "_c,d", NULL, 0.5);
	NEW_TASK_DEF("_b", "_c", NULL, 0);
	NEW_TASK_DEF("_b", "d", NULL, 0.5);
	NEW_TASK_DEF("_b", "f,g", NULL, 0.666);
	NEW_TASK_DEF("_b", "f,g,x,y", NULL, 0.8);
	NEW_TASK_DEF("_b", "f,g,x,y", "d", 0.6);
	NEW_TASK_DEF("_b", "f,g,x,y", "d,l", 0.4);
	NEW_TASK_DEF("_b", "a,d,x,y,z,f,g,l", NULL, 0.888);

	NEW_TASK_DEF("_b", NULL, NULL, 0.0);
	NEW_TASK_DEF("_b", "_c", NULL, 0.0);
	NEW_TASK_DEF("_b", "_c", "a", -0.5);
	NEW_TASK_DEF("_b", "_c", "a,d", -1);
	NEW_TASK_DEF("_b", "_c,y", "a,d,x", -1.0);
	NEW_TASK_DEF("_b,z,f", "_c,y", "a,d,x", 0);

	task_count = n;
	for (n = 0; n < task_count; n++) {
		assert_getConsistancy(tc, array[n], set, __FILE__, __LINE__, expected_cons[n]);
	}


	for (i = 0; i < MAX_TASK_COUNT; i++) {
		TASK_DEFINITION_free(array[i]);
	}

	PARAM_SET_free(set);
}

static void assert_similar_consistency_tasks(CuTest* tc, TASK_DEFINITION *A, TASK_DEFINITION *B, PARAM_SET *set, const char *file, int line, TASK_DEFINITION *exp) {
	int res;
	char buf[1024];
	size_t count = 0;
	TASK_DEFINITION *tmp = NULL;

	count += snprintf(buf + count, sizeof(buf) - count, "Error at: %i in file: %s. ", line, file);

	if (A == NULL || B == NULL ) {
		count += snprintf(buf + count, sizeof(buf) - count, " invalid argument A or B.");
		CuAssert(tc, buf, 0);
	}

	res = TASK_DEFINITION_getMoreConsistent(A, B, set, 0.1, &tmp);
	if (res != PST_OK) {
		count += snprintf(buf + count, sizeof(buf) - count, " Unable to compare task definitions. Error 0x%x.", res);
		CuAssert(tc, buf, 0);
	}

	if (tmp != exp) {
		if (exp == NULL) {
			count += snprintf(buf + count, sizeof(buf) - count, "Expected (null) ");
		} else {
			count += snprintf(buf + count, sizeof(buf) - count, "Expected %s %i ", exp->name, exp->id);
		}

		if (tmp == NULL) {
			count += snprintf(buf + count, sizeof(buf) - count, "but is(null).");
		} else {
			count += snprintf(buf + count, sizeof(buf) - count, "but is %s %i.", tmp->name, tmp->id);
		}
		printf("%s\n", buf);
		CuAssert(tc, buf, 0);
	}

	return;
}

#define COMPARE_TASKS(_man1, _atl1, _forb1, _man2, _atl2, _forb2, _expected) \
	TASK_DEFINITION_new(n++, "Task A", _man1, _atl1, _forb1, NULL, &A); \
	TASK_DEFINITION_new(n++, "Task B", _man2, _atl2, _forb2, NULL, &B); \
	assert_similar_consistency_tasks(tc, A, B, set, __FILE__, __LINE__, _expected); TASK_DEFINITION_free(A); TASK_DEFINITION_free(B); A = NULL; B = NULL;

static void Test_task_definition_similar_consistancy(CuTest* tc) {
	int res;
	int n = 0;
	TASK_DEFINITION *A;
	TASK_DEFINITION *B;
	PARAM_SET *set = NULL;

	/**
	 * Configure parameter set _ before indicates that the flag is never set.
     */
	res = PARAM_SET_new("{a}{d}{x}{y}{z}{f}{g}{l}"
						"{_b}{_c}{_h}{_m}{_n}",
			&set);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && set != NULL);

	param_set_add(tc, set, "a,d,x,y,z,f,g,l", __FILE__, __LINE__);

	/**
	 * Configure similar tasks.
	 * TODO: add more.
     */

	COMPARE_TASKS("a,d,_b,_c", NULL, NULL,
					"a,_b", NULL, NULL, A);

	COMPARE_TASKS("a,_b,_c", "d", NULL,
					"a,_b", NULL, NULL, A);

	COMPARE_TASKS("a", NULL, NULL,
					"a", NULL, NULL, NULL);

	COMPARE_TASKS("a,d,_b,_c", NULL, NULL,
					"a,_b,_c", "d", NULL, NULL);


	PARAM_SET_free(set);
}

static void Test_task_definition_toString(CuTest* tc) {
	int res;
	TASK_DEFINITION *task_def_1 = NULL;
	TASK_DEFINITION *task_def_2 = NULL;
	TASK_DEFINITION *task_def_3 = NULL;
	char buf[1024];
	char *ret = NULL;

	char expected1[] = "-a\n";
	char expected2[] = "-a one or more of (-x -y)\n";
	char expected3[] = "-a -b one or more of (-x -y -z)\n";


	res = TASK_DEFINITION_new(0, "Task 1", "a", NULL, NULL, NULL, &task_def_1);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && task_def_1 != NULL);

	res = TASK_DEFINITION_new(1, "Task 2", "a", "x,y", NULL, NULL, &task_def_2);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && task_def_2 != NULL);

	res = TASK_DEFINITION_new(2, "Task 3", "a,b", "x,y,z", NULL, NULL, &task_def_3);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && task_def_3 != NULL);


	ret = TASK_DEFINITION_toString(task_def_1, buf, sizeof(buf));
	CuAssert(tc, "TASK_DEFINITION_toString.", ret != NULL && strcmp(ret, expected1) == 0);

	ret = TASK_DEFINITION_toString(task_def_2, buf, sizeof(buf));
	CuAssert(tc, "Unable to create new task definition.", ret != NULL && strcmp(ret, expected2) == 0);

	ret = TASK_DEFINITION_toString(task_def_3, buf, sizeof(buf));
	CuAssert(tc, "Unable to create new task definition.", ret != NULL && strcmp(ret, expected3) == 0);

	TASK_DEFINITION_free(task_def_1);
	TASK_DEFINITION_free(task_def_2);
	TASK_DEFINITION_free(task_def_3);
}

static void Test_task_definition_repair_messages(CuTest* tc) {
	int res;
	TASK_DEFINITION *task_def_1 = NULL;
	TASK_DEFINITION *task_def_2 = NULL;
	TASK_DEFINITION *task_def_3 = NULL;
	TASK_DEFINITION *task_def_4 = NULL;
	TASK_DEFINITION *task_def_5 = NULL;
	PARAM_SET *set = NULL;
	char buf[1024];

	char exp_1[] = "Error: You have to define flag(s) '--_b', '--_c'.\n";
	char exp_2[] = "Error: You have to define at least one the flag(s) '--_b', '--_c'.\n";
	char exp_3[] = "Error: You must not use flag(s) '-x'.\n";
	char exp_4[] = "You have to define flag(s) '--_b'.\nYou must not use flag(s) '-x', '-g'.\n";
	char exp_5[] = "Error: You have to define flag(s) '--_b'.\nError: You have to define at least one the flag(s) '--_h'.\nError: You must not use flag(s) '-x'.\n";

	res = PARAM_SET_new("{a}{_b}{_c}{d}{x}{y}{z}{f}{g}{_h}", &set);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && set != NULL);

	param_set_add(tc, set, "a,d,x,y,z,f,g", __FILE__, __LINE__);

	res = TASK_DEFINITION_new(0, "Task 1", "a,_b,_c", "x,y", NULL, NULL, &task_def_1);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && task_def_1 != NULL);

	res = TASK_DEFINITION_new(1, "Task 3", "a", "_b,_c", NULL, NULL, &task_def_2);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && task_def_2 != NULL);

	res = TASK_DEFINITION_new(2, "Task 4", "a", "z", "x,_h", NULL, &task_def_3);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && task_def_3 != NULL);

	res = TASK_DEFINITION_new(3, "Task 5", "a,_b", "z", "x,g", NULL, &task_def_4);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && task_def_4 != NULL);

	res = TASK_DEFINITION_new(3, "Task 5", "a,_b", "_h", "x", NULL, &task_def_5);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && task_def_5 != NULL);


	CuAssert(tc, "Invalid repair suggestions.",
			TASK_DEFINITION_howToRepiar_toString(task_def_1, set, "Error: ", buf, sizeof(buf)) != NULL && strcmp(buf, exp_1) == 0);

	CuAssert(tc, "Invalid repair suggestions.",
			TASK_DEFINITION_howToRepiar_toString(task_def_2, set, "Error: ", buf, sizeof(buf)) != NULL && strcmp(buf, exp_2) == 0);

	CuAssert(tc, "Invalid repair suggestions.",
			TASK_DEFINITION_howToRepiar_toString(task_def_3, set, "Error: ", buf, sizeof(buf)) != NULL && strcmp(buf, exp_3) == 0);

	CuAssert(tc, "Invalid repair suggestions.",
			TASK_DEFINITION_howToRepiar_toString(task_def_4, set, NULL, buf, sizeof(buf)) != NULL && strcmp(buf, exp_4) == 0);

	CuAssert(tc, "Invalid repair suggestions.",
			TASK_DEFINITION_howToRepiar_toString(task_def_5, set, "Error: ", buf, sizeof(buf)) != NULL && strcmp(buf, exp_5) == 0);

	PARAM_SET_free(set);
	TASK_DEFINITION_free(task_def_1);
	TASK_DEFINITION_free(task_def_2);
	TASK_DEFINITION_free(task_def_3);
	TASK_DEFINITION_free(task_def_4);
	TASK_DEFINITION_free(task_def_5);
}

static void Test_task_definition_ignoring_messages(CuTest* tc) {
	int res;
	TASK_DEFINITION *task_def_1 = NULL;
	TASK_DEFINITION *task_def_2 = NULL;
	TASK_DEFINITION *task_def_3 = NULL;
	TASK_DEFINITION *task_def_4 = NULL;
	PARAM_SET *set = NULL;
	char buf[1024];

	char exp_1[] = "Warning: Ignoring following flag(s) '-z'.\n";
	char exp_2[] = "Ignoring following flag(s) '-z', '-x'.\n";
	char exp_3[] = "";
	char exp_4[] = "";

	res = PARAM_SET_new("{a}{_b}{_c}{d}{x}{y}{z}{f}{g}{_h}", &set);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && set != NULL);

	param_set_add(tc, set, "a,d,x,y,z,f,g", __FILE__, __LINE__);

	res = TASK_DEFINITION_new(0, "Task 1", "a", "x,y", NULL, "z", &task_def_1);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && task_def_1 != NULL);

	res = TASK_DEFINITION_new(1, "Task 3", "a", "_b,_c", NULL, "z,x,_h", &task_def_2);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && task_def_2 != NULL);

	res = TASK_DEFINITION_new(2, "Task 4", "a", "z", "x,_h", NULL, &task_def_3);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && task_def_3 != NULL);

	res = TASK_DEFINITION_new(3, "Task 5", "a,_b", "z", "x,g", "_h", &task_def_4);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && task_def_4 != NULL);


	CuAssert(tc, "Invalid ignoring message.",
			TASK_DEFINITION_ignoredParametersToString(task_def_1, set, "Warning: ", buf, sizeof(buf)) != NULL && strcmp(buf, exp_1) == 0);
	buf[0] = '\0';

	CuAssert(tc, "Invalid ignoring message.",
			TASK_DEFINITION_ignoredParametersToString(task_def_2, set, NULL, buf, sizeof(buf)) != NULL && strcmp(buf, exp_2) == 0);
	buf[0] = '\0';

	CuAssert(tc, "Invalid ignoring message.",
			TASK_DEFINITION_ignoredParametersToString(task_def_3, set, "Warning: ", buf, sizeof(buf)) != NULL && buf[0] == '\0');
	buf[0] = '\0';

	CuAssert(tc, "Invalid ignoring message.",
			TASK_DEFINITION_ignoredParametersToString(task_def_4, set, NULL, buf, sizeof(buf)) != NULL && strcmp(buf, exp_4) == 0);
	buf[0] = '\0';

	PARAM_SET_free(set);
	TASK_DEFINITION_free(task_def_1);
	TASK_DEFINITION_free(task_def_2);
	TASK_DEFINITION_free(task_def_3);
	TASK_DEFINITION_free(task_def_4);
}


static void Test_task_set_add(CuTest* tc) {
	int res;
	TASK_SET *tasks = NULL;

	res = TASK_SET_new(&tasks);
	CuAssert(tc, "Unable to create new task set.", res == PST_OK && tasks != NULL);

	res = TASK_SET_add(tasks, 1, "Task 1", "a", "x,y", NULL, NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK && tasks->count == 1);

	res = TASK_SET_add(tasks, 2, "Task 2", "a", NULL, "x", "y");
	CuAssert(tc, "Unable to add task.", res == PST_OK && tasks->count == 2);

	CuAssert(tc, "Invalid task at pos 0.", tasks->array[0] != NULL && tasks->array[0]->id == 1);
	CuAssert(tc, "Invalid task at pos 1.", tasks->array[1] != NULL && tasks->array[1]->id == 2);

	TASK_SET_free(tasks);
}

static void Test_task_set_lifecycle(CuTest* tc) {
	int res;
	TASK_SET *tasks = NULL;
	PARAM_SET *set = NULL;
	TASK *cons_task = NULL;
	char buf[1024];

	/**
	 * Create and configure TASK set.
     */
	res = PARAM_SET_new("{a}{_b}{_c}{d}{e}{x}{y}{z}{f}{g}{_h}{i}", &set);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && set != NULL);

	param_set_add(tc, set, "a,d,e,x,y,z,f,g,i", __FILE__, __LINE__);

	/**
	 * Create and configure tasks.
     */
	res = TASK_SET_new(&tasks);
	CuAssert(tc, "Unable to create new task set.", res == PST_OK && tasks != NULL);

	res = TASK_SET_analyzeConsistency(tasks, set, 0.2);
	CuAssert(tc, "Analyze can't be performed!.", res == PST_TASK_SET_HAS_NOD_DEFINITIONS);

	res = TASK_SET_getConsistentTask(tasks, &cons_task);
	CuAssert(tc, "Unable to add task.", res == PST_TASK_SET_HAS_NOD_DEFINITIONS && cons_task == NULL);


	/* Add only invalid tasks. */
	res = TASK_SET_add(tasks, 0, "Task 0", "a,_b", NULL, NULL, NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	res = TASK_SET_add(tasks, 1, "Task 1", "a,_b,_c,d", NULL, NULL, NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	res = TASK_SET_add(tasks, 2, "Task 2", "a,_b,d", NULL, NULL, NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	res = TASK_SET_add(tasks, 3, "Task 3", "a,_b,d", "x", NULL, NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	/* Try to get a task from not analyzed task set. */
	res = TASK_SET_getConsistentTask(tasks, &cons_task);
	CuAssert(tc, "Unable to add task.", res == PST_TASK_SET_NOT_ANALYZED && cons_task == NULL);

	res = TASK_SET_analyzeConsistency(tasks, set, 0.2);
	CuAssert(tc, "Unable to analyze.", res == PST_OK);


	/* Try to get a task from analyzed task set. There should not be no consistent tasks. */
	res = TASK_SET_getConsistentTask(tasks, &cons_task);
	CuAssert(tc, "Unable to add task.", res == PST_TASK_ZERO_CONSISTENT_TASKS && cons_task == NULL);

	res = TASK_SET_add(tasks, 4, "Task 4", "a", NULL, NULL, NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	/* Try to get a task from not analyzed task set as one task is added. */
	res = TASK_SET_getConsistentTask(tasks, &cons_task);
	CuAssert(tc, "Unable to add task.", res == PST_TASK_SET_NOT_ANALYZED && cons_task == NULL);

	res = TASK_SET_add(tasks, 5, "Task 5", "a,d", "_b", NULL, NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	res = TASK_SET_add(tasks, 6, "Task 6", "a,d,g,f,x,y,z,e,i", "_b", NULL, NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	res = TASK_SET_add(tasks, 8, "Task 8", "a,d_c", "_b", "x,y,z,i", NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	res = TASK_SET_analyzeConsistency(tasks, set, 0.2);
	CuAssert(tc, "Unable to analyze.", res == PST_OK);

	/* Try to get a single consistent task (id == 4). */
	res = TASK_SET_getConsistentTask(tasks, &cons_task);
	CuAssert(tc, "Unable to get one signle consistent tasks.", res == PST_OK && cons_task->id == 4);
	cons_task = NULL;

	res = TASK_SET_add(tasks, 7, "Task 7", "a,d", NULL, NULL, NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	/* Try to get a task from not analyzed task set as one task is added. */
	res = TASK_SET_getConsistentTask(tasks, &cons_task);
	CuAssert(tc, "Unable to add task.", res == PST_TASK_SET_NOT_ANALYZED && cons_task == NULL);

	res = TASK_SET_analyzeConsistency(tasks, set, 0.1);
	CuAssert(tc, "Unable to analyze.", res == PST_OK);

	res = TASK_SET_getConsistentTask(tasks, &cons_task);
	CuAssert(tc, "Unable to add task.", res == PST_TASK_MULTIPLE_CONSISTENT_TASKS && cons_task == NULL);


	PARAM_SET_free(set);
	TASK_SET_free(tasks);
}

static void Test_task_set_remove_ignored_parameters(CuTest* tc) {
	int res;
	TASK_SET *tasks = NULL;
	PARAM_SET *set = NULL;
	TASK *cons_task = NULL;
	char buf[1024];
	int removed = 0;

	/**
	 * Create and configure TASK set.
     */
	res = PARAM_SET_new("{a}{_b}{_c}{d}{e}{x}{y}{z}{f}{g}{_h}{i}", &set);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && set != NULL);

	param_set_add(tc, set, "a,d,e,x,y,z,f,g,i", __FILE__, __LINE__);

	/**
	 * Create and configure tasks.
     */
	res = TASK_SET_new(&tasks);
	CuAssert(tc, "Unable to create new task set.", res == PST_OK && tasks != NULL);

	res = TASK_SET_add(tasks, 0, "Task 0", "a,_b", "_c", NULL, "x");
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	res = TASK_SET_add(tasks, 1, "Task 1", "a,d", "e,_b", NULL, "i,x,z");
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	res = TASK_SET_analyzeConsistency(tasks, set, 0.2);
	CuAssert(tc, "Unable to analyze.", res == PST_OK);

	res = TASK_SET_getConsistentTask(tasks, &cons_task);
	CuAssert(tc, "Unable to get one signle consistent tasks.", res == PST_OK && cons_task->id == 1);
	CuAssert(tc, "i, x and z must be still set.", PARAM_SET_isSetByName(set, "i") && PARAM_SET_isSetByName(set, "x") && PARAM_SET_isSetByName(set, "z"));

	res = TASK_SET_cleanIgnored(tasks, cons_task, &removed);
	CuAssert(tc, "Unable to remove ignored parameters.", res == PST_OK && removed == 3);
	CuAssert(tc, "i, x and z must be cleand.", !PARAM_SET_isSetByName(set, "i") && !PARAM_SET_isSetByName(set, "x") && !PARAM_SET_isSetByName(set, "z"));


	PARAM_SET_free(set);
	TASK_SET_free(tasks);
}

static void Test_task_set_suggestions(CuTest* tc) {
	int res;
	TASK_SET *tasks = NULL;
	PARAM_SET *set = NULL;
	TASK *cons_task = NULL;
	char buf[1024];

	/**
	 * Create and configure TASK set.
     */
	res = PARAM_SET_new("{a}{_b}{_c}{d}{e}{x}{y}{z}{f}{g}{_h}{i}", &set);
	CuAssert(tc, "Unable to create new task definition.", res == PST_OK && set != NULL);

	param_set_add(tc, set, "a,d,e,x,y,z,f,g,i", __FILE__, __LINE__);

	/**
	 * Create and configure tasks.
     */
	res = TASK_SET_new(&tasks);
	CuAssert(tc, "Unable to create new task set.", res == PST_OK && tasks != NULL);

	res = TASK_SET_analyzeConsistency(tasks, set, 0.2);
	CuAssert(tc, "Analyze can't be performed!.", res == PST_TASK_SET_HAS_NOD_DEFINITIONS);

	res = TASK_SET_getConsistentTask(tasks, &cons_task);
	CuAssert(tc, "Unable to add task.", res == PST_TASK_SET_HAS_NOD_DEFINITIONS && cons_task == NULL);


	/* Add only invalid tasks. */
	res = TASK_SET_add(tasks, 0, "Task 0", "a,_b", NULL, NULL, NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	res = TASK_SET_add(tasks, 1, "Task 1", "a,_b,_c,d", NULL, NULL, NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	res = TASK_SET_add(tasks, 2, "Task 2", "a,_b,d", NULL, NULL, NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	res = TASK_SET_add(tasks, 3, "Task 3", "a,_b,d", "x", NULL, NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	/* Try to get a task from not analyzed task set. */
	res = TASK_SET_getConsistentTask(tasks, &cons_task);
	CuAssert(tc, "Unable to add task.", res == PST_TASK_SET_NOT_ANALYZED && cons_task == NULL);

	res = TASK_SET_analyzeConsistency(tasks, set, 0.2);
	CuAssert(tc, "Unable to analyze.", res == PST_OK);


	/* Try to get a task from analyzed task set. There should not be no consistent tasks. */
	res = TASK_SET_getConsistentTask(tasks, &cons_task);
	CuAssert(tc, "Unable to add task.", res == PST_TASK_ZERO_CONSISTENT_TASKS && cons_task == NULL);

	res = TASK_SET_add(tasks, 4, "Task 4", "a", NULL, NULL, NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	/* Try to get a task from not analyzed task set as one task is added. */
	res = TASK_SET_getConsistentTask(tasks, &cons_task);
	CuAssert(tc, "Unable to add task.", res == PST_TASK_SET_NOT_ANALYZED && cons_task == NULL);

	res = TASK_SET_add(tasks, 5, "Task 5", "a,d", "_b", NULL, NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	res = TASK_SET_add(tasks, 6, "Task 6", "a,d,g,f,x,y,z,e,i", "_b", NULL, NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	res = TASK_SET_add(tasks, 8, "Task 8", "a,d_c", "_b", "x,y,z,i", NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	res = TASK_SET_analyzeConsistency(tasks, set, 0.2);
	CuAssert(tc, "Unable to analyze.", res == PST_OK);

	/* Try to get a single consistent task (id == 4). */
	res = TASK_SET_getConsistentTask(tasks, &cons_task);
	CuAssert(tc, "Unable to get one signle consistent tasks.", res == PST_OK && cons_task->id == 4);
	cons_task = NULL;

	res = TASK_SET_add(tasks, 7, "Task 7", "a,d", NULL, NULL, NULL);
	CuAssert(tc, "Unable to add task.", res == PST_OK);

	/* Try to get a task from not analyzed task set as one task is added. */
	res = TASK_SET_getConsistentTask(tasks, &cons_task);
	CuAssert(tc, "Unable to add task.", res == PST_TASK_SET_NOT_ANALYZED && cons_task == NULL);

	res = TASK_SET_analyzeConsistency(tasks, set, 0.1);
	CuAssert(tc, "Unable to analyze.", res == PST_OK);

	res = TASK_SET_getConsistentTask(tasks, &cons_task);
	CuAssert(tc, "Unable to add task.", res == PST_TASK_MULTIPLE_CONSISTENT_TASKS && cons_task == NULL);

	printf("%s", TASK_SET_suggestions_toString(tasks, 6, buf, sizeof(buf)));


	PARAM_SET_free(set);
	TASK_SET_free(tasks);
}



CuSuite* TaskDefTest_getSuite(void) {
	CuSuite* suite = CuSuiteNew();


	SUITE_ADD_TEST(suite, Test_task_definition_consistency);
	SUITE_ADD_TEST(suite, Test_task_definition_similar_consistancy);
	SUITE_ADD_TEST(suite, Test_task_definition_toString);
	SUITE_ADD_TEST(suite, Test_task_definition_repair_messages);
	SUITE_ADD_TEST(suite, Test_task_definition_ignoring_messages);
	SUITE_ADD_TEST(suite, Test_task_set_add);
	SUITE_ADD_TEST(suite, Test_task_set_lifecycle);
	SUITE_ADD_TEST(suite, Test_task_set_remove_ignored_parameters);
	SUITE_ADD_TEST(suite, Test_task_set_suggestions);

	return suite;
}

