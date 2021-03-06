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

#ifndef PARAM_SET_H
#define	PARAM_SET_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "param_value.h"
#include "parameter.h"

#ifndef PARAM_SET_ERROR_BASE
#	define PARAM_SET_ERROR_BASE 0x30001
#endif

/**
 * List of error codes returned by the functions.
 */
enum PARAM_SET_ERR_enum {
	/** Everything is OK! */
	PST_OK = 0,

	/** Function parameters are invalid, check for \c NULL pointers or out of range values. */
	PST_INVALID_ARGUMENT = PARAM_SET_ERROR_BASE,

	/** Input content or syntax is wrong (e.g. parsing command line or configuration file). */
	PST_INVALID_FORMAT,

	/** Index out of boundaries. */
	PST_INDEX_OVF,

	/** Parameter with the given name does not exist in the given set. */
	PST_PARAMETER_NOT_FOUND,

	/** Parameter value with the given constraints does not exist. */
	PST_PARAMETER_VALUE_NOT_FOUND,

	/** The parameter value count is zero.*/
	PST_PARAMETER_EMPTY,

	/** Parameter added to the set is possibly a typo. */
	PST_PARAMETER_IS_TYPO,

	/** Parameter added to the set is unknown. */
	PST_PARAMETER_IS_UNKNOWN,

	/** Object extractor function is not implemented. */
	PST_PARAMETER_UNIMPLEMENTED_OBJ,

	/** The function for extracting wildcard is not implemented. */
	PST_PARAMETER_UNIMPLEMENTED_WILDCARD,

	/** PST out of memory. */
	PST_OUT_OF_MEMORY,

	/** PST IO error (e.g. when reading configuration file). */
	PST_IO_ERROR,

	/** Priority value is negative. */
	PST_PRIORITY_NEGATIVE,

	/** Priority value is too large. */
	PST_PRIORITY_TOO_LARGE,

	/** The format of the parameter is invalid. */
	PST_PARAMETER_INVALID_FORMAT,

	/** It was not possible to extract a consistent task from the task set. */
	PST_TASK_ZERO_CONSISTENT_TASKS,

	/** There are two consistent tasks while only one is expected. */
	PST_TASK_MULTIPLE_CONSISTENT_TASKS,

	/** Task set is empty.*/
	PST_TASK_SET_HAS_NO_DEFINITIONS,

	/** Task set is not analyzed with #PARAM_SET object. */
	PST_TASK_SET_NOT_ANALYZED,

	/** Task set is reanalyzed with different #PARAM_SET object. */
	PST_TASK_UNABLE_TO_ANALYZE_PARAM_SET_CHANGED,

	/** Wildcard processor error occurred. */
	PST_WILDCARD_ERROR,

	/** Undefined behaviour occurred (e.g. a bug, unrecognised memory error). */
	PST_UNDEFINED_BEHAVIOUR,

	/** Unable to set the combination of command-line parser options. */
	PST_PRSCMD_INVALID_COMBINATION,

	/** Parameter conversion is not performed, original input must be used. (See #PARAM_addControl and #PARAM_SET_addControl). */
	PST_PARAM_CONVERT_NOT_PERFORMED,

	/** Parameter alias is not specified and it is not possible to work with it. */
	PST_ALIAS_NOT_SPECIFIED,

	/** Unknown error. */
	PST_UNKNOWN_ERROR,
};

/**
 * Parameter set object. Holds multiple parameters, performs parsing and access
 * to the parameter values.
 */
typedef struct PARAM_SET_st PARAM_SET;

/**
 * \return A constant pointer to a constant string describing the
 * version number of the package.
 */
const char *PST_getVersion(void);

/**
 * Creates new #PARAM_SET object using parameter names.
 * Parameter names are defined using string '<tt>{name|alias}*{name|alias}*...</tt>' where
 *
 * \c name - parameter name,
 *
 * \c alias - alias for the name (optional),
 *
 * \c * - indicates the possibility of multiple values (optional), for usage instructions, see #PARAM_SET_isConstraintViolation and #PARAM_SET_constraintErrorToString.
 *
 * Example: <tt>{h|help}{file}*{o}*{n}</tt>
 *
 * \param	names	Pointer to parameter names.
 * \param	set		Pointer to receiving pointer to #PARAM_SET object.
 * \return #PST_OK if successful, error code otherwise.
 */
int PARAM_SET_new(const char *names, PARAM_SET **set);

/**
 * Frees the  #PARAM_SET.
 * \param	set	#PARAM_SET object.
 */
void PARAM_SET_free(PARAM_SET *set);

/**
 * Adds several optional functions to a set of parameters. Each function takes
 * one parameter as C-string value (must not fail if is \c NULL). All the
 * functions except \c extractObject are applied when adding the value to the
 * #PARAM_SET object. Function \c extractObject is applied when calling #PARAM_SET_getObj
 * or #PARAM_SET_getObjExtended.
 *
 * Function \c controlFormat is to check the format. Return \c 0 if format is ok,
 * error code otherwise.
 *
 * <tt>int (*controlFormat)(const char *str)</tt>
 *
 * Function \c controlContent is to check the content. Return 0 if content is
 * ok, error code otherwise.
 *
 * <tt>int (*controlContent)(const char *str)</tt>
 *
 * Function \c convert is used to repair/convert the C-string value before any
 * content or format check is performed. Takes two extra parameters for buffer and its
 * size. Return #PST_OK if conversion is successful or #PST_PARAM_CONVERT_NOT_PERFORMED
 * to skip conversion. Any other error code will break adding the value.
 *
 * <tt>int (*convert)(const char *value, char *buf, unsigned *buf_len)</tt>
 *
 * Function \c extractObject is used to extract an object from the parameter value.
 * Function \c extractObject affects #PARAM_SET_getObj and  #PARAM_SET_getObjExtended
 * behavior. If not specified, the default function is used that extracts the
 * C-string value. Parameter \c extra is <tt>void**</tt> pointer to array with
 * 2 elements, \c str is parameter value and \c obj is pointer to receiving pointer.
 * Calling #PARAM_SET_getObj both array elements in \c extra are pointing to #PARAM_SET
 * itself (<tt>void **extra = {set, set}</tt>), when calling #PARAM_SET_getObjExtended,
 * the second value is determined by the function call and \c extra parameter given
 * (<tt>void **extra = {set, extra}</tt>). If extracting of the object is successful,
 * #PST_OK is returned, error code otherwise.
 *
 * <tt>int (*extractObject)(void **extra, const char *str, void **obj)</tt>
 *
 * \param	set				#PARAM_SET object.
 * \param	names			List of names to add the functions for.
 * \param	controlFormat	Function for format checking.
 * \param	controlContent	Function for content checking.
 * \param	convert			Function for value conversion.
 * \param	extractObject	Function for object extraction.
 * \return #PST_OK if successful, error code otherwise.
 * \note Note that \c controlFormat and \c controlContent may return any error code
 * but \c convert function should return #PST_OK and #PST_PARAM_CONVERT_NOT_PERFORMED
 * as any other error code will break adding the simple value, parsing configuration
 * file or command line.
 * \see #PARAM_SET_setParseOptions, #PARAM_SET_setPrintName and #PARAM_SET_setWildcardExpander.
 * To get error reports related to functions \c controlFormat and \c controlContent,
 * see #PARAM_SET_isFormatOK and #PARAM_SET_invalidParametersToString.
 */
int PARAM_SET_addControl(PARAM_SET *set, const char *names,
		int (*controlFormat)(const char *),
		int (*controlContent)(const char *),
		int (*convert)(const char*, char*, unsigned),
		int (*extractObject)(void **, const char *, void**));

/**
 * Alters the way the parameter is represented in (error)
 * messages, help text and returned by #PARAM_getPrintName.
 *
 * If \c constv is not \c NULL, a user specified constant value is used. If \c constv is
 * \c NULL, an abstract function \c getPrintName must be specified that formats the string.
 * Default print format for long and short parameters are  \c '\-\-long' and \c '-a'.
 *
 * <tt>const char* (*getPrintName)(PARAM *param, char *buf, unsigned buf_len)</tt>
 *
 * <ul>
 * <li>\c param   - this PARAM object.</li>
 * <li>\c buf     - Internal buffer with constant size. May be left unused.</li>
 * <li>\c buf_len - The size of the internal buffer.</li>
 * <li>Returns string that is the string representation of parameter.</li>
 * </ul>
 *
 * \param	set				#PARAM_SET object.
 * \param	names			List of names to add the functions for.
 * \param	constv			Constant string representation of the parameter, may be \c NULL.
 * \param	getPrintName	Abstract function implementation. Has effect only when \c constv is \c NULL. May be \c NULL.
 * \return #PST_OK when successful, error code otherwise.
 * \see #PARAM_SET_addControl, #PARAM_SET_setParseOptions and #PARAM_SET_setWildcardExpander.
 */
int PARAM_SET_setPrintName(PARAM_SET *set, const char *names,
							const char *constv, const char* (*getPrintName)(PARAM *param, char *buf, unsigned buf_len));

/**
 * Same as #PARAM_SET_setPrintName but works with alias.
 * \param	set				#PARAM_SET object.
 * \param	names			List of names to add the functions for.
 * \param	constv			Constant string representation of the parameter alias. Can be \c NULL.
 * \param	getPrintName	Abstract function implementation. Has effect only when \c constv is \c NULL. Can be \c NULL.
 * \return #PST_OK when successful, error code otherwise.
 * \see #PARAM_SET_addControl, #PARAM_SET_setParseOptions and #PARAM_SET_setWildcardExpander.
 */
int PARAM_SET_setPrintNameAlias(PARAM_SET *set, const char *names,
							const char *constv, const char* (*getPrintName)(PARAM *param, char *buf, unsigned buf_len));

/**
 * Specify help text for a parameter.
 *
 * Overall help text is composed of:
 * \code{.txt}
 * General format:
 *   <print name> [<arg>] <delimiter> <description text>
 *
 * Examples:
 *   --myoption [arg] - description where arg is "[arg]".
 *   --input file     - description where arg is "file".
 *   --debug          - description where arg is "" or NULL.
 * \endcode
 *
 * \param	set		#PARAM_SET object.
 * \param	names	List of names to add the help text for.
 * \param	arg		Description of argument. Can be \c NULL.
 * \param	txt		Help text for a parameter. Value is copied. Must NOT be \c NULL.
 * \return #PST_OK when successful, error code otherwise.
 * \see #PARAM_SET_setPrintName and #PARAM_SET_helpToString.
 */
int PARAM_SET_setHelpText(PARAM_SET *set, const char *names, const char *arg, const char *txt);

/**
 * Generates help text for parameters. Before any help text
 * can be generated it must be configured for all the parameters with function
 * #PARAM_SET_setHelpText. The way the parameter is represented can be modified
 * with #PARAM_SET_setPrintName function.
 *
 * \param	set			#PARAM_SET object.
 * \param	names		List of names to generate help for.
 * \param	indent		Help text indention.
 * \param	header		The size of the header (all the text and space before 'parameter' in '  -a - parameter A.').
 * \param	rowWidth	The size of the row.
 * \param	buf			Buffer.
 * \param	buf_len	The size of \c buf.
 * \return Returns \c buf if successful, \c NULL otherwise.
 */
char* PARAM_SET_helpToString(const PARAM_SET *set, const char *names, int indent, int header, int rowWidth, char *buf, size_t buf_len);

/**
 * Appends value to the set. Invalid value format or content is not handled
 * as error, but the state is saved. Internal format, content or count errors can
 * be detected - see #PARAM_SET_isFormatOK. If parameter name dose not exist,
 * function will fail. When parameter is not found it is examined as a typo or
 * unknown (see #PARAM_SET_isTypoFailure and #PARAM_SET_isUnknown).
 *
 *
 * \param	set			#PARAM_SET object.
 * \param	name		Parameter name.
 * \param	value		Parameter value as c-string. Can be \c NULL.
 * \param	source		Source description as c-string. Can be \c NULL.
 * \param	priority	Priority that can be #PST_PRIORITY_VALID_BASE (<tt>0</tt>) or higher.
 * \return #PST_OK if successful, error code otherwise. When parameter is not
 * part of the set it is pushed to the unknown or typo list and error code
 * #PST_PARAMETER_IS_UNKNOWN or #PST_PARAMETER_IS_TYPO is returned.
 * \see #PARAM_SET_unknownsToString, #PARAM_SET_typosToString and #PARAM_SET_invalidParametersToString to display errors.
 */
int PARAM_SET_add(PARAM_SET *set, const char *name, const char *value, const char *source, int priority);

/**
 * Extracts strings from the #PARAM_SET (see #PARAM_SET_add). If object
 * extractor is set, a string value is always extracted. The user <b>MUST  NOT</b> free
 * the returned string.
 *
 * Values are filtered by constraints. If multiple names (e.g. <tt>name1,name2,name3</tt>)
 * are specified, process is started with the first name and \c at is used to index
 * over all specified values. If some parameters in the name list do not contain any values matching
 * the constraints, the next name is selected without an error (if there exists a next value).
 * If all parameters are empty #PST_PARAMETER_EMPTY is returned, if some
 * values are found and the end of the list is reached #PST_PARAMETER_VALUE_NOT_FOUND
 * is returned.
 *
 * \param	set			#PARAM_SET object.
 * \param	name		Parameter name.
 * \param	source		Constraint for the source, can be \c NULL.
 * \param	priority	Priority that can be #PST_PRIORITY_VALID_BASE (<tt>0</tt>) or higher.
 * \param	at			Parameter index in the matching set composed with the constraints.
 * \param	value		Pointer to receiving pointer to string returned.
 * \return #PST_OK when successful, error code otherwise. Some more common error
 * codes: #PST_INVALID_ARGUMENT, #PST_PARAMETER_NOT_FOUND, #PST_PARAMETER_EMPTY,
 * #PST_PARAMETER_INVALID_FORMAT and #PST_PARAMETER_VALUE_NOT_FOUND.
 * \see PARAM_SET_getObj.
 */
int PARAM_SET_getStr(PARAM_SET *set, const char *name, const char *source, int priority, int at, char **value);

/**
 * Extracts object from the #PARAM_SET. If no object extractor (see #PARAM_SET_addControl)
 * is set, a string (see #PARAM_SET_getStr instead) value is returned. By default the user
 * must not free the returned object. If a custom object extractor function
 * is used, object must be freed if implementation requires it.
 *
 * Values are filtered by constraints. If multiple names (e.g. <tt>name1,name2,name3</tt>)
 * are specified, process is started with the first name and \c at is used to index
 * over all specified values. If some parameters in the name list do not contain any values matching
 * the constraints, the next name is selected without an error (if there exists a next value).
 * If all parameters are empty #PST_PARAMETER_EMPTY is returned, if some
 * values are found and the end of the list is reached #PST_PARAMETER_VALUE_NOT_FOUND
 * is returned.
 *
 * \param	set			#PARAM_SET object.
 * \param	name		Parameter name.
 * \param	source		Constraint for the source, can be \c NULL.
 * \param	priority	Priority that can be #PST_PRIORITY_VALID_BASE (<tt>0</tt>) or higher.
 * \param	at			Parameter index in the matching set composed with the constraints.
 * \param	obj			Pointer to receiving pointer to \c object returned.
 * \return #PST_OK when successful, error code otherwise. If object extractor implementation
 * returns an error code, it is returned by this functionSome more common error
 * codes: 	#PST_INVALID_ARGUMENT,  #PST_PARAMETER_NOT_FOUND, #PST_PARAMETER_EMPTY,
 * #PST_PARAMETER_INVALID_FORMAT, #PST_PARAMETER_UNIMPLEMENTED_OBJ, #PST_PARAMETER_VALUE_NOT_FOUND.
 * \note Note that if format or content status is invalid, it is not possible to extract
 * the object. Custom object extractor may return error values that overlaps with
 * local error codes!
 * \see #PARAM_SET_add, #PARAM_SET_addControl and #PARAM_SET_getObjExtended.
 */
int PARAM_SET_getObj(PARAM_SET *set, const char *name, const char *source, int priority, int at, void **obj);

/**
 * Same as #PARAM_SET_getObj, but the \c ctx fed to object extractor contains two
 * pointers <tt>void **extra = {set, ctx}</tt>.
 * \param	set			#PARAM_SET object.
 * \param	name		Parameter name.
 * \param	source		Constraint for the source, can be \c NULL.
 * \param	priority	Priority that can be #PST_PRIORITY_VALID_BASE (<tt>0</tt>) or higher.
 * \param	at			Parameter index in the matching set composed with the constraints.
 * \param	ctx		Pointer to extra context.
 * \param	obj			Pointer to receiving pointer to \c object returned.
 * \return #PST_OK when successful, error code otherwise. Some more common error
 * codes: 	#PST_INVALID_ARGUMENT,  #PST_PARAMETER_NOT_FOUND, #PST_PARAMETER_EMPTY,
 * #PST_PARAMETER_INVALID_FORMAT, #PST_PARAMETER_UNIMPLEMENTED_OBJ and #PST_PARAMETER_VALUE_NOT_FOUND.
 */
int PARAM_SET_getObjExtended(PARAM_SET *set, const char *name, const char *source, int priority, int at, void *ctx, void **obj);

/**
 * Extract attributes with the given constraints.
 * \param	set			#PARAM_SET object.
 * \param	name		Parameter name.
 * \param	source		Constraint for the source, can be \c NULL.
 * \param	priority	Priority that can be #PST_PRIORITY_VALID_BASE (<tt>0</tt>) or higher.
 * \param	at			Parameter index in the matching set composed with the constraints.
 * \param	atr			Output parameter filled with attributes.
 * \return #PST_OK when successful, error code otherwise. Some more common error
 * codes: 	#PST_INVALID_ARGUMENT,  #PST_PARAMETER_NOT_FOUND, #PST_PARAMETER_EMPTY,
 * #PST_PARAMETER_VALUE_NOT_FOUND.
 */
int PARAM_SET_getAtr(PARAM_SET *set, const char *name, const char *source, int priority, int at, PARAM_ATR *atr);

/**
 * Extracts parameter's print name that is also displayed in (error)
 * messages and help text.
 *
 * \param	set			#PARAM_SET object.
 * \param	name		Parameter name.
 * \param	print_name	Pointer to receiving pointer.
 * \return #PST_OK when successful, error code otherwise. Some more common error
 * codes: #PST_INVALID_ARGUMENT,  #PST_PARAMETER_NOT_FOUND;
 * \see To change the print name value see #PARAM_SET_setPrintName.
 */
int PARAM_SET_getPrintName(PARAM_SET *set, const char *name, const char **print_name);

/**
 * Same as #PARAM_SET_getPrintName but works with alias.
 * \param	set			#PARAM_SET object.
 * \param	name		Parameters alias name.
 * \param	print_name	Pointer to receiving pointer.
 * \return #PST_OK when successful, error code otherwise. Some more common error
 * codes: #PST_INVALID_ARGUMENT,  #PST_PARAMETER_NOT_FOUND or #PST_ALIAS_NOT_SPECIFIED;
 * \see #PARAM_SET_setPrintNameAlias to change the print name value.
 */
int PARAM_SET_getPrintNameAlias(PARAM_SET *set, const char *name, const char **print_name);

/**
 * Removes all values from the specified parameter list. Parameter list is defined
 * as '<tt>p1,p2,p3 ...</tt>'.
 * \param set	#PARAM_SET object.
 * \param names	Parameter name list.
 * \return #PST_OK if successful, error code otherwise.
 * \note Parameter is not deleted but only its values leaving it empty.
 */
int PARAM_SET_clearParameter(PARAM_SET *set, const char *names);

/**
 * Removes a value specified by the constraints from the specified parameter list.
 * Parameter list is defined as '<tt>p1,p2,p3 ...</tt>'.
 *
 * \param	set			#PARAM_SET object.
 * \param	names		Parameter name list.
 * \param	source		Constraint for the source, can be \c NULL.
 * \param	priority	Priority that can be #PST_PRIORITY_VALID_BASE (<tt>0</tt>) or higher.
 * \param	at			Parameter index in the matching set composed with the constraints.
 * \return #PST_OK if successful, error code otherwise.
 */
int PARAM_SET_clearValue(PARAM_SET *set, const char *names, const char *source, int priority, int at);

/**
 * Counts all the existing parameter values in the list composed by the parameter
 * list and constraints specified. Parameter list is defined as '<tt>p1,p2,p3 ...</tt>'.
 *
 * \param	set			#PARAM_SET object.
 * \param	names		Parameter name list.
 * \param	source		Constraint for the source, can be \c NULL.
 * \param	priority	Priority that can be #PST_PRIORITY_VALID_BASE (<tt>0</tt>) or higher.
 * \param	count		Pointer to integer for storing the value count.
 * \return #PST_OK if successful, error code otherwise. If parameter does not exist
 * #PST_PARAMETER_NOT_FOUND is returned.
 */
int PARAM_SET_getValueCount(PARAM_SET *set, const char *names, const char *source, int priority, int *count);

/**
 * Searches for a parameter defined in the list by name and checks if <b>all</b> the
 * parameters have at least one values set. Even if the value format or content is
 * invalid, true is returned. Parameter list is defined as '<tt>p1,p2,p3 ...</tt>'.
 *
 * \param	set		#PARAM_SET object.
 * \param	names	Parameter name list.
 * \return 1 if is set, 0 otherwise.
 */
int PARAM_SET_isSetByName(const PARAM_SET *set, const char *names);

/**
 * Searches for a parameter defined in the list by name and checks if <b>at least one</b>
 * of the parameters have at least on values set. Even if the value format or
 * content is invalid, true is returned. Parameter list is defined as '<tt>p1,p2,p3 ...</tt>'.
 *
 * \param	set		#PARAM_SET object.
 * \param	names	Parameter name list.
 * \return 1 if is at least one is set, 0 otherwise.
 */
int PARAM_SET_isOneOfSetByName(const PARAM_SET *set, const char *names);

/**
 * Controls if the format and content of the parameters are OK.
 * \param	set		#PARAM_SET object.
 * \return 0 if format is invalid, 1 otherwise.
 * \see #PARAM_SET_addControl and #PARAM_SET_invalidParametersToString.
 */
int PARAM_SET_isFormatOK(const PARAM_SET *set);

/**
 * Controls if the the constraints are violated.
 * \param	set		#PARAM_SET object.
 * \return 0 if constraints are OK, 1 otherwise. <tt>-1</tt> if an error occurred.
 * \see #PARAM_SET_new.
 */
int PARAM_SET_isConstraintViolation(const PARAM_SET *set);

/**
 * Controls if there are some undefined parameters read from command line or
 * file, similar to the defined ones - possible typos. Typos are detected using
 * the \c difference value calculated as specified below:
 *
 * \code{.txt}
 *                    levenshtein_distance(token, param_name)
 * difference = 100 * --------------------------------------- - (is_sub_str * 15 + is_sub_str_at_beginning * 15)  , where
 *                                param_name_len
 *
 *   levenshtein_distance - function that calculates edit distance for two strings.
 *   token - unknown token whose similarity is compared with existing parameters.
 *   param_name - parameter name.
 *   param_name_len - parameter name length in characters.
 *   is_sub_str - 1 if token is a substring, 0 otherwise.
 *   is_sub_str_at_beginning - 1 if token matches with the beginning of param_name, 0 otherwise.
 * \endcode
 *
 * The \c difference value with the unknown \c token is calculated for every known
 * parameter in the \c set and the smallest value is saved as \c smdiff. An unknown
 * token is interpreted as typo if there exists at least one parameter so that:
 * \code{.txt}
 * difference < 90 && difference < (smdiff + 10)
 * \endcode
 *
 * \param	set		#PARAM_SET object.
 * \return 0 if set contains possible typos, 1 otherwise.
 * \see #PARAM_SET_typosToString, #PARAM_SET_add, #PARAM_SET_parseCMD and #PARAM_SET_readFromFile.
 */
int PARAM_SET_isTypoFailure(const PARAM_SET *set);

/**
 * Controls if there are some syntax errors after reading configuration file.
 * \param	set		#PARAM_SET object.
 * \return 0 if set contains possible typos, 1 otherwise.
 * \see #PARAM_SET_readFromFile.
 */
int PARAM_SET_isSyntaxError(const PARAM_SET *set);

/**
 * Controls if there are some undefined parameters read from command line or file.
 * \param	set		#PARAM_SET object.
 * \return 0 if set contains unknown parameters, 1 otherwise.
 * \see #PARAM_SET_unknownsToString, #PARAM_SET_add, #PARAM_SET_parseCMD and #PARAM_SET_readFromFile.
 */
int PARAM_SET_isUnknown(const PARAM_SET *set);

/**
 * Reads parameter values from file into predefined #PARAM_SET. File must be
 * formatted one parameter (and its possible value) per line. To add a comment '<tt>#</tt>'
 * must be inserted at the beginning of the line. To learn how the key-value pairs
 * are precisely extracted see #parse_key_value_pair and #read_line.
 * Format of parameters:
 * \code{.txt}
 * # comment    - a line with comment.
 * --long       - Long parameter without value.
 * --long <arg> - Long parameter with value.
 * -i <arg>     - Short parameter with value.
 * -vxn         - Bunch of flags.
 * \endcode
 *
 * \param	set			#PARAM_SET object.
 * \param	fname		File path.
 * \param	source		Source description as c-string. Can be \c NULL.
 * \param	priority	Priority that can be #PST_PRIORITY_VALID_BASE (<tt>0</tt>) or higher.
 * \return #PST_OK if successful, error code otherwise. If file format is invalid,
 * #PST_INVALID_FORMAT is returned.
 */
int PARAM_SET_readFromFile(PARAM_SET *set, const char *fname, const char* source, int priority);

/**
 * Reads parameter values from command line into predefined #PARAM_SET. Parameters
 * are stored in internal data structures where one parameter can have multiple values.
 * If configured, all values read are checked against checking functions (see
 * #PARAM_SET_addControl). See #PARAM_SET_isSetByName, #PARAM_SET_getStr and
 * #PARAM_SET_getObj to get command-line parameter values. If there are some
 * misspelled and/or unknown parameters, it is detected and it is possible to get
 * error reports. See #PARAM_SET_isTypoFailure and #PARAM_SET_isUnknown to check
 * for errors. See #PARAM_SET_unknownsToString and #PARAM_SET_typosToString to get
 * error reports.
 *
 * Command-line format:
 * \code{.txt}
 * --long       - Long parameter without value.
 * --long <arg> - Long parameter with value.
 * -i <arg>     - Short parameter with value.
 * -vxn         - Bunch of flags.
 * \endcode
 * \param	set			#PARAM_SET object.
 * \param	argc		Count of command line strings.
 * \param	argv		Array of command line strings.
 * \param	source		Source description as c-string. Can be \c NULL.
 * \param	priority	Priority that can be #PST_PRIORITY_VALID_BASE (<tt>0</tt>) or higher.
 * \return #PST_OK if successful, error code otherwise.
 * \see #PARAM_SET_readFromCMD for more advanced behaviour.
 */
int PARAM_SET_readFromCMD(PARAM_SET *set, int argc, char **argv, const char *source, int priority);


/**
 * Parses command-line parameters. It is similar to
 * #PARAM_SET_readFromCMD but extends its functionality. For example it is possible
 * to have a parameter that always interprets the next token as its value, even
 * if it is identical to some command-line parameter. To redirect all tokens after
 * '\-\-' to specified parameter, or process some parameters with configured wildcard
 * expander see #PARAM_SET_setParseOptions.
 *
 * To specify parse option for each command line parameter:
 * \code{.txt}
 * --long       - Long parameter without value.
 * --long <arg> - Long parameter with value.
 * -i <arg>     - Short parameter with value.
 * -vxn         - Bunch of flags.
 * \endcode
 *
 * \param	set			#PARAM_SET object.
 * \param	argc		Count of command line strings.
 * \param	argv		Array of command line strings.
 * \param	source		Source description as c-string. Can be \c NULL.
 * \param	priority	Priority that can be #PST_PRIORITY_VALID_BASE (<tt>0</tt>) or higher.
 * \return #PST_OK if successful, error code otherwise.
 * \see [PARAM_PARSE_OPTIONS](@ref PARAM_PARSE_OPTIONS_enum), #PARAM_SET_addControl, #PARAM_SET_isSetByName, #PARAM_SET_getStr,
 * #PARAM_SET_getObj, #PARAM_SET_isTypoFailure, #PARAM_SET_isUnknown, #PARAM_SET_unknownsToString and
 * #PARAM_SET_typosToString.
 */
int PARAM_SET_parseCMD(PARAM_SET *set, int argc, char **argv, const char *source, int priority);

/**
 * Specifies the parsing options ([PARAM_PARSE_OPTIONS](@ref PARAM_PARSE_OPTIONS_enum)) used
 * by #PARAM_SET_parseCMD.
 *
 * \param set			#PARAM_SET object.
 * \param names			Parameter name list.
 * \param options		Parsing options.
 * \return #PST_OK if successful, error code otherwise.
 * \see #PARAM_SET_addControl, #PARAM_SET_setPrintName, and #PARAM_SET_setWildcardExpander.
 */
int PARAM_SET_setParseOptions(PARAM_SET *set, const char *names, int options);

/**
 * Extracts all parameters from \c src known to \c target and appends all the
 * values to the target #PARAM_SET. Values are added via #PARAM_SET_add and all
 * check and extract functions that are used are from the target set. After
 * successful operation both sets <b>must be freed separately</b> and operations applied
 * to each set fo not affect the other one.
 *
 * \param	target			Target #PARAM_SET.
 * \param	src				Source #PARAM_SET.
 * \return #PST_OK if successful, error code otherwise.
 */
int PARAM_SET_IncludeSet(PARAM_SET *target, PARAM_SET *src);

/**
 * Generates #PARAM_SET string representation for debugging.
 * \param	set		#PARAM_SET object.
 * \param	buf		Receiving buffer.
 * \param	buf_len	Receiving buffer size.
 * \return \c buf if successful, \c NULL otherwise.
 */
char* PARAM_SET_toString(PARAM_SET *set, char *buf, size_t buf_len);

/**
 * Generates typo failure report.
 * \param	set		#PARAM_SET object.
 * \param	prefix	Prefix to each typo failure string. Can be \c NULL.
 * \param	buf		Receiving buffer.
 * \param	buf_len	Receiving buffer size.
 * \return \c buf if successful, \c NULL otherwise.
 * \see #PARAM_SET_isTypoFailure.
 */
char* PARAM_SET_typosToString(PARAM_SET *set, const char *prefix, char *buf, size_t buf_len);

/**
 * Generates unknown parameter report.
 * \param set		#PARAM_SET object.
 * \param prefix	Prefix to each unknown failure string. Can be \c NULL.
 * \param buf		Receiving buffer.
 * \param buf_len	Receiving buffer size.
 * \return \c buf if successful, \c NULL otherwise.
 * \see #PARAM_SET_isUnknown.
 */
char* PARAM_SET_unknownsToString(const PARAM_SET *set, const char *prefix, char *buf, size_t buf_len);

/**
 * Generates a string from invalid parameter list (see #PARAM_SET_addControl).
 * By default error strings generated contain only error code. To make the messages
 * more human-readable define function \c getErrString that takes error code as
 * input and returns <tt>const string</tt> describing the failure.
 *
 * \param	set				#PARAM_SET object.
 * \param	prefix			Prefix for each failure string. Can be \c NULL.
 * \param	getErrString	Function pointer to make error codes to string. Can be \c NULL.
 * \param	buf				Receiving buffer.
 * \param	buf_len			Receiving buffer size.
 * \return \c buf if successful, \c NULL otherwise.
 * \see #PARAM_SET_isFormatOK.
 */
char* PARAM_SET_invalidParametersToString(const PARAM_SET *set, const char *prefix, const char* (*getErrString)(int), char *buf, size_t buf_len);

/**
 * Generates constraint error report. See #PARAM_CONSTRAINTS and #PARAM_SET_new.
 * \param	set				#PARAM_SET object.
 * \param	prefix			Prefix for each constraint error string. Can be \c NULL.
 * \param	buf				Receiving buffer.
 * \param	buf_len			Receiving buffer size.
 * \return \c buf if successful, \c NULL otherwise.
 */
char* PARAM_SET_constraintErrorToString(const PARAM_SET *set, const char *prefix, char *buf, size_t buf_len);

/**
 * Converts PST_* error codes to string.
 * \param err	Error code from #PARAM_SET_ERR_enum.
 * \return Error string mapped from specified error code.
 */
const char* PARAM_SET_errorToString(int err);

/**
 * Generates syntax error report.
 * \param	set				#PARAM_SET object.
 * \param	prefix			Prefix for each constraint error string. Can be \c NULL.
 * \param	buf				Receiving buffer.
 * \param	buf_len			Receiving buffer size.
 * \return \c buf if successful, \c NULL otherwise.
 * \see #PARAM_SET_readFromFile and #PARAM_SET_isSyntaxError.
 */
char* PARAM_SET_syntaxErrorsToString(const PARAM_SET *set, const char *prefix, char *buf, size_t buf_len);

/**
 * Separates names from a string. A function \c isValidNameChar
 * must be defined to separate valid name characters from the separators.
 *
 * \param	name_string		A string full of names that are separated from each other.
 * \param	isValidNameChar	Function that defines valid name characters.
 * \param	buf				Buffer for storing extracted name.
 * \param	len				Buffer size.
 * \param	flags			Can be \c NULL.
 * \return Pointer inside \c name_string that points to next character after name
 * extracted or \c NULL if end of string reached or no name can be extracted.
 */
const char* extract_next_name(const char* name_string, int (*isValidNameChar)(int), char *buf, short len, int *flags);

/**
 * Extracts a key value pair from the line. Value part can be wrapped inside
 * double quote marks (<tt>\"</tt>) to include whitespace characters. Use back slash
 * (<tt>\\</tt>) as escape character for itself and for double quotes. Some examples:
 *
 * \code{.txt}
 * key = value
 * key = "value 1"
 *   key = value
 * key value
 * key "value 1"
 *   key value
 * key "value=\"test\""
 * \endcode
 *
 * \param line		Input line.
 * \param key		Pointer to receiving key.
 * \param value		Pointer to receiving value.
 * \param buf_len	Maximum size of both \c key and \c value buffer.
 * \return #PST_OK if successful, error code otherwise. Some more common error
 * codes: #PST_INVALID_FORMAT.
 */
int parse_key_value_pair(const char *line, char *key, char *value, size_t buf_len);

/**
 * Reads a line from a file (opened with \c fopen in mode \c r) and tracks the lines.
 * Supported line endings:
 * \code{.txt}
 * MAC  \r      CR      0x0d
 * Unix \n      LF      0x0a
 * Win  \r\n    CRLF    0x0d0a
 * \endcode
 * \param	file		A file pointer that is us used for reading from.
 * \param	buf			A buffer to store the line.
 * \param	len			Size of the buffer.
 * \param	row_pointer	Pointer to the row counting value. Initial value pointed to must be 0. If not used can be \c NULL.
 * \param	read_count	Pointer to character counting value. If not used can be \c NULL.
 * \return Return 0 if successful, \c EOF if end of file.
 */
int read_line(FILE *file, char *buf, size_t len, size_t *row_pointer, size_t *read_count);


/**
 * Specifies a function to expand tokens that contain wildcard character (<tt>WC</tt>)
 * to array of new values. By default characters '<tt>?</tt>' and '<tt>*</tt>' are \c WC.
 * Values containing \c WC are removed and replaced with the expanded values. To
 * use default \c WC set \c charList as \c NULL.
 *
 * <tt>int (*expand_wildcard)(PARAM_VAL *param_value, void *ctx, int *value_shift)</tt>
 *
 * <ul>
 * <li>\c param_value - The value of the current parameter that contains <tt>WC</tt>.</li>
 * <li>\c ctx - Additional data structure (same object as #PARAM_SET_setWildcardExpander input parameter <tt>ctx</tt>).</li>
 * <li>\c value_shift - Output parameter for the count of values expanded.</li>
 * </ul>
 *
 * \param	set				#PARAM_SET object.
 * \param	names			List of names to add the functionality.
 * \param	charList		List of wildcard characters used. When set to \c NULL, '<tt>*?</tt>' is used.
 * \param	ctx				Data structure used by Wildcard expander, can be \c NULL.
 * \param	ctx_free		Data structure release function, can be \c NULL.
 * \param	expand_wildcard	Function pointer to Wildcard Expander function.
 * \return #PST_OK if successful, error code otherwise.
 * \note #PARAM_SET_parseCMD must be used and parsing option #PST_PRSCMD_EXPAND_WILDCARD
 * must be set using #PARAM_SET_setParseOptions.
 * \see #PARAM_SET_setPrintName, #PARAM_SET_addControl, #PARAM_expandWildcard,
 * #PARAM_setWildcardExpander and [Implemented wildcard expanders](@ref wildcardexpanders.h).
 */
int PARAM_SET_setWildcardExpander(PARAM_SET *set, const char *names,
		const char* charList,
		void *ctx,
		void (*ctx_free)(void*),
		int (*expand_wildcard)(PARAM_VAL *param_value, void *ctx, int *value_shift));

#ifdef	__cplusplus
}
#endif

#endif
