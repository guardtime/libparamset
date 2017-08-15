PARAM SET Overview {#mainpage}
============
The libparamset is a software development kit for handling command-line parameters and program tasks. Parameters can be read from command-line and a task can be extrated that matches with the given input. Process is covered with error detection and functions that generates helpful feedback messages to the user.

libparamset
-----------

The libparamset provides the following functionality:
* Parameters can be parsed from command-line, read from configuration file or inserted from the code.
* Short and long parameters (-a and --long).
* Concatenating of flags with length 1 (-ab instead of -a -b).
* One alias for the parameters names (e.g. --load and -l).
* Individual parsing options for parameters ([PARAM_PARS_OPTIONS](@ref PARAM_PARS_OPTIONS_enum)).
  * Parameter with multiple coexisting values (-a v1 -a v2 ... -a vn, where -a = {"v1", "v2", ..., "vn"}).
  * Parameter that never takes a value (-a v1 -a -a, where -a = {NULL, NULL, NULL} and "v1" is unknown token).
  * Parameter that always takes a value (-a -a, where -a = {"-a"}).
  * Existing parameter break that takes next token as it's value only if it's not an existing parameter.
  * Possible parameter break that takes next token as it's value only if it does not look like possible parameter (-a v1 -a -b, where -a = {"v1", NULL})
  * Parameter "--", that will redirect every next token to a specified parameter(s).
  * Parameter that collects values that are not bound with any parameter ("-i x y ...", where "x" is bound with "-i" but "y" is not).
  * Parameter value sequence with (existing or possible) parameter break ("-a v1 v2 ... vn -b", where -a = {"v1", "v2", ..., "vn"}).
  * Parameter that is hidden from the command-line but can be inserted from the code or configuration file.
  * Parameter that do not generate any typo errors (useful when hiding a parameter from command-line).
* Values can be filtered by, name (e.g. -i as "i" and  --long as "long" ), source (e.g. "default"), priority (e.g. 3) and index (0 - n).
* Values can be counted by, name, source and priority (e.g. 3).
* Values can be filtered as the last or the first with the highest or the lowest priority.
* Values can be counted as the highest or the lowest priority.
* Parameter default name that is shown in error messages (print name) can be replaced with custom string (constant or generated).
* Auto-generated typo suggestions (e.g. Did You mean "--long" instead of "--song"?).
* Auto-generated unknown parameter error messages.
* Abstract format and content check functionality with auto-generated error messages.
* Abstract parameter transformation or repair functionality.
* Abstract Wildcard expander (can be used to make -i * work on Windows).
  * Implemented wildcards for Windows file system.
* Abstract object parsing functionality (e.g. extract double or file).
* Multiple parameter sets can be merged.
* Task set, composed of multiple tasks, where from a task can be extracted by specified parameter set.
* Auto-generated suggestions between multiple tasks for the user when it is not impossible to resolve which task the user wants to perform.
* Auto-generated suggestions how to fix the task user tries to perform.
* Parameters can be bound with description and formatted to human readable list for help text.

Main objects
------------
[PARAM_SET](@ref param_set.h) contains user defined parameters that can be parsed from command-line, read from configuration file or added from the code.

[TASK_SET](@ref task_def.h) contains multiple task definitions (parameters that are mandatory, ignored or restricted for defined task) that can be analyzed against specified [PARAM_SET](@ref param_set.h) to extract a signle consistent task that matches the input.

[TASK](@ref task_def.h) is object returned by successful [TASK_SET](@ref task_def.h) analyze that contains the matching task \c ID and \c PARAM_SET object.

Memory Management
-----------------

The memory management obeys the following rules:
* Every object you create, belongs to you.
* Every object you own, must be freed by you.
* Using free function on \c NULL does nothing and won't crash.
* Input strings are copied and freed by parent object.

Workflow
-----------------
- Include param_set.h and task_def.h.
- Configure parameters.
  + Create new \c PARAM_SET object with set of parameters [PARAM_SET_new](@ref PARAM_SET_new).
  + Add implementations for format, control (and convert) functions with [PARAM_SET_addControl](@ref PARAM_SET_addControl).
  + Add implementation for wildcard expander (e.g. to implement wildcards when getting file input on Windows "-i *.txt") [PARAM_SET_setWildcardExpander](@ref PARAM_SET_setWildcardExpander).
  + Add parsing options for parameters with [PARAM_SET_setParseOptions](@ref PARAM_SET_setParseOptions).
- Configure tasks.
  + Create new \c TASK_SET object with [TASK_SET_new](@ref TASK_SET_new).
  + Add task definitions to the \c TASK_SET with [TASK_SET_add](@ref TASK_SET_add).
- Get parameters.
  + Parse command-line with [PARAM_SET_parseCMD](@ref PARAM_SET_parseCMD).
  + Read parameters from configuration file with [PARAM_SET_readFromFile](@ref PARAM_SET_readFromFile).
  + Check and report user if configuration file contained some syntax errors with [PARAM_SET_isSyntaxError](@ref PARAM_SET_isSyntaxError) and [PARAM_SET_syntaxErrorsToString](@ref PARAM_SET_syntaxErrorsToString).
  + Merge different \c PARAM_SET objects with [PARAM_SET_IncludeSet](@ref PARAM_SET_IncludeSet).
  + Check for unknown parameters and typos with [PARAM_SET_isUnknown](@ref PARAM_SET_isUnknown) and [PARAM_SET_isTypoFailure](@ref PARAM_SET_isTypoFailure).
  + Check for invalid parameters that failed format or content checks with [PARAM_SET_isFormatOK](@ref PARAM_SET_isFormatOK).
  + If there are some errors help user with error messages by using following functions [PARAM_SET_unknownsToString](@ref PARAM_SET_unknownsToString), [PARAM_SET_typosToString](@ref PARAM_SET_typosToString) and [PARAM_SET_invalidParametersToString](@ref PARAM_SET_invalidParametersToString).
- Analyze task set against given input.
  + Analyze the \c TASK_SET against \c PARAM_SET [TASK_SET_analyzeConsistency](@ref TASK_SET_analyzeConsistency).
  + Extract consistent task from \c TASK_SET with [TASK_SET_getConsistentTask](@ref TASK_SET_getConsistentTask).
  + In case of failure check if there is a invalid task that stands out from the others and give user some hints how to fix it with functions [TASK_SET_isOneFromSetTheTarget](@ref TASK_SET_isOneFromSetTheTarget) and [TASK_SET_howToRepair_toString](@ref TASK_SET_howToRepair_toString). When it's hard to distinguish which task user is trying to accomplish give some suggestions that do have the most similar pattern to the given input with [TASK_SET_suggestions_toString](@ref TASK_SET_suggestions_toString).
- Use functions [TASK_getID](@ref TASK_getID) and [TASK_getSet](@ref TASK_getSet) and perform the selected task.
  * To check if a parameter is set, use function [PARAM_SET_isSetByName](@ref PARAM_SET_isSetByName).
  + To get parameters value from the set, use functions [PARAM_SET_getStr](@ref PARAM_SET_getStr) and [ PARAM_SET_getObj](@ref PARAM_SET_getObj).
  + To get parameter value count, use function [PARAM_SET_getValueCount](@ref PARAM_SET_getValueCount).
- Release object with [TASK_SET_free](@ref TASK_SET_free) and [PARAM_SET_free](@ref PARAM_SET_free).

Code example
-----------------
```C
#include <stdio.h>
#include <string.h>
#include <param_set/param_set.h>
#include <param_set/task_def.h>

#ifdef _WIN32
#   include <param_set/wildcardexpanders.h>
#endif

enum {
	VALUE_OK = 0,
	VALUE_IS_NULL = 0x01,
	VALUE_IS_EMPTY = 0x02,
	VALUE_FILE_DOES_NOT_EXIST = 0x03,
};

int convertRepair_path(const char* arg, char* buf, unsigned len);
int isContentOk_path(const char* fname);
int isFormatOk_path(const char* fname);
const char *parameter_error_to_string(int err);

int main(int argc, char** argv, char **envp) {
	PARAM_SET *set = NULL;
	TASK_SET *task_set = NULL;
	TASK *pTask = NULL;
	char buf[1024];


	/**
	 * Configure parameter set and its parameters.
	 * 1) Both o and i must not be NULL or empty string.
	 * 2) Both o and i converts '\' into '/'.
	 * 3) i must be a path to a file that must exist.
	 * 4) Next token after o and i is always bound with the flag.
	 * 5) All options with true or false do not bound anything.
	 */
	PARAM_SET_new("{i}{o}{dump}{debug}{r}{help|h}", &set);
	PARAM_SET_addControl(set, "o", isFormatOk_path, NULL, convertRepair_path, NULL);
	PARAM_SET_addControl(set, "i", isFormatOk_path, isContentOk_path, convertRepair_path, NULL);
	PARAM_SET_setParseOptions(set, "o", PST_PRSCMD_HAS_VALUE);
	PARAM_SET_setParseOptions(set, "dump,debug,h,r", PST_PRSCMD_HAS_NO_VALUE);

	/**
	 * To enable Windows file system wildcards, specify the wildcard expander
	 * function implementation and enable parsing option that enable wildcard
	 * processor.
	 */
#ifdef _WIN32
	PARAM_SET_setWildcardExpander(set, "i", NULL, NULL, NULL, PST_WCF_Win32FileWildcard);
	PARAM_SET_setParseOptions(set, "i", PST_PRSCMD_HAS_VALUE |  PST_PRSCMD_EXPAND_WILDCARD);
#else
	PARAM_SET_setParseOptions(set, "i", PST_PRSCMD_HAS_VALUE);
#endif

	/**
	 * Describe different tasks:
	 * util -h
	 * util -i file_in --dump [-o file_out]
	 * util -i file_in -o file_out -r
	 */
	TASK_SET_new(&task_set);
	TASK_SET_add(task_set, 0, "Help", "h", NULL, NULL, NULL);
	TASK_SET_add(task_set, 1, "Dump file", "i,dump", NULL, "h,r", NULL);
	TASK_SET_add(task_set, 2, "Reverse file", "i,r,o", NULL, "h,dump", NULL);

	/* Parse command-line. */
	PARAM_SET_parseCMD(set, argc, argv, NULL, 0);

	/* Check for typos and unknown parameters. */
	if (PARAM_SET_isTypoFailure(set)) {
			printf("%s\n", PARAM_SET_typosToString(set, NULL, buf, sizeof(buf)));
			goto cleanup;
	} else if (PARAM_SET_isUnknown(set)){
			printf("%s\n", PARAM_SET_unknownsToString(set, "Error: ", buf, sizeof(buf)));
			goto cleanup;
	}

	/* Check for invalid values. */
	if (!PARAM_SET_isFormatOK(set)) {
		printf("%s\n", PARAM_SET_invalidParametersToString(set, NULL, parameter_error_to_string, buf, sizeof(buf)));
		goto cleanup;
	}

	/* As input parameter passed all checks try to extract a consistent task. */
	TASK_SET_analyzeConsistency(task_set, set, 0.2);
	TASK_SET_getConsistentTask(task_set, &pTask);

	/* If task is not extracted try to give user some suggestions what to do to make things work. */
	if (pTask == NULL) {
		int ID;
		if (TASK_SET_isOneFromSetTheTarget(task_set, 0.1, &ID)) {
			printf("%s", TASK_SET_howToRepair_toString(task_set, set, ID, NULL, buf, sizeof(buf)));
		} else {
			printf("%s", TASK_SET_suggestions_toString(task_set, 3, buf, sizeof(buf)));
		}

		goto cleanup;
	}

	/* Task is extracted, check which one to run. */
	switch(TASK_getID(pTask)) {
		case 0:
			printf("Print help.\n");
		break;

		case 1:
			printf("Dump File.\n");
		break;

		case 2:
			printf("Reverse File.\n");
		break;

		default:
			printf("Error.\n");
		break;
	}

cleanup:
	TASK_SET_free(task_set);
	PARAM_SET_free(set);
}

const char *parameter_error_to_string(int err) {
	switch(err) {
		case VALUE_OK: return "OK";
		case VALUE_IS_NULL: return "Parameter must have value";
		case VALUE_IS_EMPTY: return "Parameter must have content";
		case VALUE_FILE_DOES_NOT_EXIST: return "File does not exist";
		default: return "Unknown error";
	}
}

int isFormatOk_path(const char* fname) {
	if (fname == NULL) return VALUE_IS_NULL;
	else if (*fname == '\0');
	else return VALUE_OK;
}

int isContentOk_path(const char* fname) {
	int result = 1;
	FILE *f = NULL;
	f = fopen(fname, "r");
	result = f == NULL ? VALUE_FILE_DOES_NOT_EXIST : VALUE_OK;
	if (f != NULL) fclose(f);
	return result;
}

int convertRepair_path(const char* arg, char* buf, unsigned len){
	char *toBeReplaced = NULL;

	if (arg == NULL || buf == NULL) return PST_INVALID_ARGUMENT;
	strncpy(buf, arg, len - 1);


	toBeReplaced = buf;
	while ((toBeReplaced = strchr(toBeReplaced, '\\')) != NULL){
		*toBeReplaced = '/';
		toBeReplaced++;
	}

	return PST_OK;
}

```
Third party components
------------

The SDK is using the following third party components:
* CuTest [cutest.sourceforge.net](http://cutest.sourceforge.net/)
