/*
 * Copyright 2013-2016 Guardtime, Inc.
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
#include <stdio.h>
#include <stdlib.h>
#include "param_set.h"
#include "strn.h"

#ifdef _WIN32
	#include <windows.h>
#endif

#ifdef _WIN32
int PST_WCF_Win32FileWildcard(PARAM_VAL *param_value, void *ctx, int *value_shift) {
	int res;
	HANDLE hfile = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA found;
	const char *value;
	const char *source;
	int prio = 0;
	PARAM_VAL *tmp = NULL;
	PARAM_VAL *insertTo = param_value;
	int count = 0;
	char buf[1024] = "";
	char path[1024] = "";
	char *toBeReplaced = "";

	char *endOfPath = NULL;

	if (param_value == NULL || ctx != NULL || value_shift == NULL) {
		return PST_INVALID_ARGUMENT;
	}

	res = PARAM_VAL_extract(param_value, &value, &source, &prio);
	if (res != PST_OK) goto cleanup;

	/**
	 * Search for a files and directories matching the wildcard.
	 * Ignore "." and "..". If the current value is t and a, b and c are expanded
	 * value, the resulting array is [... t, a, b, c ...].
	 */


	PST_strncpy(buf, value, sizeof(buf));

	toBeReplaced = buf;
	while ((toBeReplaced = strchr(toBeReplaced, '\\')) != NULL){
		*toBeReplaced = '/';
		toBeReplaced++;
	}

	path[0] = '\0';
	endOfPath = strrchr(buf, '/');
	if (endOfPath != NULL) {
		PST_strncpy(path, buf, 1 + endOfPath - buf);
	}

	/**
	 * In the case nothing was found, return as OK.
	 */
	hfile = FindFirstFile(value, &found);
	if (hfile == INVALID_HANDLE_VALUE) {
		res = PST_OK;
		goto cleanup;
	}

	do {
		if (strcmp(found.cFileName, ".") == 0 || strcmp(found.cFileName, "..") == 0) continue;

		PST_snprintf(buf, sizeof(buf), "%s%s%s",
				path[0] == '\0' ? "" : path,
				path[0] == '\0' ? "" : "/",
				found.cFileName);

		res = PARAM_VAL_new(buf, source, prio, &tmp);
		if (res != PST_OK) goto cleanup;

		res = PARAM_VAL_insert(insertTo, NULL, PST_PRIORITY_NONE, 0, tmp);
		if (res != PST_OK) goto cleanup;

		insertTo = tmp;
		tmp = NULL;
		count++;
	} while (FindNextFile(hfile, &found) != 0);

	*value_shift = count;
	res = PST_OK;

cleanup:

	if (hfile != INVALID_HANDLE_VALUE) FindClose(hfile);
	PARAM_VAL_free(tmp);
	return res;
}
#else
int PST_WCF_Win32FileWildcard(PARAM_VAL *param_value, void *ctx, int *value_shift) {
	return PST_PARAMETER_UNIMPLEMENTED_WILDCARD;
}
#endif

