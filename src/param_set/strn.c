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
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include "strn.h"


static size_t param_set_vsnprintf(char *buf, size_t n, const char *format, va_list va){
	size_t ret = 0;
	int tmp;
	if (buf == NULL || n > INT_MAX || n == 0 || format == NULL) goto cleanup;
#ifdef _WIN32
	/* NOTE: If there is empty space in buf, it will be filled with 0x00 or 0xfe. */
	tmp = vsnprintf_s(buf, n, _TRUNCATE, format, va);
	if (tmp < 0) {
		ret = n - 1;
		goto cleanup;
	}
	ret = (size_t) tmp;
#else
	(void)tmp;
	ret = vsnprintf(buf, n, format, va);
	if (ret >= n) {
		ret = n - 1;
		goto cleanup;
	}
#endif

cleanup:

	return ret;
}

size_t PST_snprintf(char *buf, size_t n, const char *format, ... ){
	size_t ret;
	va_list va;
	va_start(va, format);
	ret = param_set_vsnprintf(buf, n, format, va);
	va_end(va);
	return ret;
}

char *PST_strncpy (char *destination, const char *source, size_t n){
	char *ret = NULL;
	if (destination == NULL || source == NULL || n == 0) {
		goto cleanup;
	}
	ret = strncpy(destination, source, n - 1);
	destination[n - 1] = 0;

cleanup:

	return ret;
}

/**
 * This function is token parser for #PST_vsnhiprintf. It breaks string to words,
 * striping all whitespace (new line character is interpreted as individual token!).
 * It has a break character '\' that can be used for:
 * Embedding whitespace or \ to the output token.
 *   "\\"   - backslash.
 *   "\ "   - space (not stripped).
 * Changing indentation on the fly.
 *   "\>nr" - request to change indentation. Note that "\>" is same as "\>0".
 *
 *
 * If string token is retrieved, \c indent will output -1. If indentation change
 * is extracted \c buf will be empty string and \c indent will output the indentation
 * offset.
 *
 * In case of empty string, function returns \c 0 and \c next will output <tt>NULL</tt>.
 * In case of parsing error, \c buf contains concatenated error message to indicate
 * where the failure occurred (only if there is enough space for the message).
 *
 * \param	buf			The buffer to store the token.
 * \param	buf_len		The size of the buffer.
 * \param	str			The pointer to a string wherefrom the token is extracted.
 * \param	indnt		Output parameter for indentation. Can be \c NULL.
 * \param	next		Pointer to the character next where the parsing should be continued.
 * \return Character count in the buffer. Note that when error message is inserted
 * it is also counted!
 */
static size_t parseNextToken(char *buf, size_t buf_len, const char *str, int* indnt, const char **next) {
	size_t i = 0;
	size_t n = 0;
	int C = '\0';
	int isBreak = 0;
	int isIdent = 0;
	int tmp_indent = -1;
	int indent_sign = 1;

	if (buf == NULL || buf_len < 2 || str == NULL || next == NULL) {
		return 0;
	}

	/* Remove some white space. In case of empty string indicate that there is no more data available. */
	while(*str != '\0' && *str != '\n' && isspace((int)(*str))) str++;
	if (*str == '\0') {
		*next = NULL;
		return 0;
	}

	/* In case on new line character handle it as a token! */
	if (*str == '\n') {
		buf[0] = '\n';
		buf[1] = '\0';
		*next = str + 1;
		return 1;
	}

	/* Parse next token. */
	buf[0] = '\0';
	for (i = 0, C = str[0]; C != '\0' && (!isspace(C) || isBreak) && i < (buf_len - 1); i++, C = str[i]) {
		if (!isBreak && !isIdent && C == '\\') {
			isBreak = 1;
			continue;
		} else if (isBreak) {
			/* If there is unknown break condition terminate with error. */
			if (C != '\\' && !isspace(C) && C != '>') {
				n += PST_snprintf(buf + n, buf_len - n, "<Parse error: '\\%c'>", (char)C);
				*next = NULL;
				return n;
			}
			/* Break command detected indentation change request. */
			isBreak = 0;
			if (C == '>') {
				/* If there is at least 1 character extracted, return that token and rewind buffer. */
				if (n > 0) {
					i--;
					break;
				}
				isIdent = 1;
				tmp_indent = 0;
				continue;
			} else {
				buf[n] = (char) (0xff & C);
				n++;
			}
		} else if (isIdent) {
			if (isdigit(C)) {
				tmp_indent = tmp_indent * 10 + (int)(C - '0');
			} else if (tmp_indent == 0 && C == '-') {
				indent_sign = -1;
			} else {
				break;
			}
		} else {
			buf[n] = (char) (0xff & C);
			n++;
		}
	}

	if (indnt) *indnt = indent_sign * tmp_indent;

	/* Set pointer to the next value and return the size of the token extracted. */
	buf[n] = '\0';
	*next = &str[i];
	return n;
}

size_t PST_vsnhiprintf(char *buf, size_t buf_len, unsigned indent, unsigned headerLen, unsigned rowLen, const char *paramName, const char delimiter, const char *txt, va_list va) {
	char *description = NULL;
	int calculated = 0;
	size_t current_row_len = 0;
	size_t count = 0;
	size_t c = 0;
	int spaceNeeded = 0;
	const char *next = NULL;
	int ioffs = 0;

	if (buf == NULL || buf_len == 0 || txt == NULL ||
		(indent >= rowLen) ||
		(indent >= headerLen && paramName != NULL) ||
		(headerLen >= rowLen && paramName != NULL)) {
		return 0;
	}

	/* Create buff value for preprocessing. */
	description = (char*)malloc(buf_len * sizeof(*description));
	if (description == NULL) return 0;

	param_set_vsnprintf(description, buf_len, txt, va);
	buf[buf_len - 1] = 0;

	if (headerLen > 0 && paramName != NULL) {
		/* Get calculated size of the header, if it is too large insert a line break. */
		calculated = (headerLen - indent - (int)strlen(paramName) - 3);
		calculated = calculated < 0 ? 0 : calculated;

		/* Print the header of the help row (indentation, parameter, delimiter and description. */
		count += PST_snprintf(buf + count, buf_len - count, "%*s%s%*s", indent, "", paramName, calculated, "");
		current_row_len = count;
		if (current_row_len > (headerLen - 3)) {
			c = PST_snprintf(buf + count, buf_len - count, "\n%*s %c ", headerLen - 3, "", delimiter);
			count += c;
			current_row_len = c - 1;
			spaceNeeded = 0;
		} else {
			c = PST_snprintf(buf + count, buf_len - count, " %c ", delimiter);
			current_row_len += c;
			count += c;
			spaceNeeded = 0;
		}
		indent = headerLen;
	} else {
		c = PST_snprintf(buf + count, buf_len - count, "%*s", indent, "");
		current_row_len += c;
		count += c;
		spaceNeeded = 0;
	}


	next = description;
	while (next != NULL) {
		size_t word_len = 0;
		char wordBuffer[1024];
		int tmp_offs = -1;

		word_len = parseNextToken(wordBuffer, sizeof(wordBuffer), next, &tmp_offs, &next);
		if (next == NULL ) {
			if (word_len != 0) count += PST_snprintf(buf + count, buf_len - count, "%s", wordBuffer);
			break;
		}

		if (tmp_offs != -1) {
			ioffs = tmp_offs;
			if ((int)indent + ioffs < 0) ioffs = -(int)indent;
			else if (indent + ioffs >= rowLen) {
				ioffs = rowLen - 1 -indent;
			}
			continue;
		}

		/**
		 * If word is a new line character, force the print function to change
		 * the line and handle indentation.
		 */
		if (*wordBuffer == '\n') {

			spaceNeeded = 0;
			if (next != NULL && *next != '\0') {
				current_row_len = rowLen + 1;
				wordBuffer[0] = '\0';
			}
		}

		if (current_row_len + word_len + (spaceNeeded ? 1 : 0) > rowLen) {
			c = PST_snprintf(buf + count, buf_len - count, "\n%*s%s", indent + ioffs, "", wordBuffer);
			count += c;
			current_row_len = c - 1;
			/* In case of empty string, the space is not needed. */
			if (wordBuffer[0] != '\0') spaceNeeded = 1;
			continue;
		}

		c = PST_snprintf(buf + count, buf_len - count, "%s%s", spaceNeeded ? " " : "", wordBuffer);
		spaceNeeded = 1;
		current_row_len += c;
		count += c;
	}

	if (description != NULL) free(description);
	return count;
}

size_t PST_snhiprintf(char *buf, size_t buf_len, unsigned rowLen, unsigned indent, unsigned headerLen, const char *paramName, const char delimiter, const char *txt, ...) {
	va_list va;
	size_t count = 0;

	va_start(va, txt);
	count = PST_vsnhiprintf(buf, buf_len, indent, headerLen, rowLen, paramName, delimiter, txt, va);
	va_end(va);

	return count;
}

