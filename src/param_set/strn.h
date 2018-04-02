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

#ifndef STRN_H
#define	STRN_H

#include <stddef.h>
#include <stdarg.h>

#ifdef	__cplusplus
extern "C" {
#endif


/**
 * Platform independent version of \c snprintf.
 * \param		buf		Pointer to buffer.
 * \param		n		Maximum number of bytes to be written into buffer. Includes terminating \c NULL character.
 * \param		format	Format string.
 * \param		...		Extra parameters for formatting.
 * \return The number of characters written, not including terminating \c NULL character. On error \c  0 is returned.
 */
size_t PST_snprintf(char *buf, size_t n, const char *format, ... );

/**
 * Platform independent version of \c strncpy that guarantees \c NULL terminated
 * destination. To copy \c N characters from source to destination \c n and size of
 * destination must be <tt>N + 1</tt>.
 * \param		destination Pointer to destination.
 * \param		source		Pointer to source.
 * \param		n			Maximum number of characters to be copied including terminating \c NULL.
 * \return The pointer to destination. On error \c NULL is returned.
 */
char *PST_strncpy (char *destination, const char *source, size_t n);

/**
 * Similar to the #PST_snprintf but is used to print and format text for
 * command-line help. Formatter can work in two similar modes: command-line option (CO)
 * and regular text (RT) mode. To enter CM mode, specify \c paramName, otherwise set
 * it to \c NULL.
 *
 * 1) CO mode with specified indentation and row length:
 * \code{.txt}
 *         <arg><del>     <txt>
 * |      --myopt - This is my option
 * |                with description.
 * |      --my-long-opt
 * |     |        - This is my another
 * |     |          useful option with
 * |     |          description       |
 * |<-I->|         |                  |
 * |<------H------>|                  |
 * |<----------------R--------------->|
 * \endcode
 *
 * 2) RT mode with specified indentation and row length:
 * \code{.txt}
 * |     This is my sample text. This
 * |     is my sample text. This is my
 * |     sample text.
 * |
 * |        * This is my sample list
 * |          first element.
 * |        * This is my sample list
 * |          second element.
 * |
 * |      This is my sample text. This
 * |      is my sample text.          |
 * |<-I->|                            |
 * |<----------------R--------------->|
 * \endcode
 *
 * In examples above:
 *  + \c I   - Indentation from the left border. Is specified with \c indent.
 *  + \c H   - The size of the header where only command-line parameters can be printed. Is specified with \c headerLen only in CO mode.
 *  + \c R   - The overall length of the row. Is specified with \c rowLen.
 *  + \c \<arg\> - The name of the command-line options. Is specified with \c paramName only in CO mode.
 *  + \c \<del\> - Delimiter between option and its description (above '-' is used). Is specified with \c delimiter only in CO mode.
 *  + \c \<txt\> - The description of the command-line option. Is specified with \c txt.
 *
 *
 * When text is being formatted, it is broken into tokens, where whitespace characters
 * are used as delimiters. When there is a need to change parsers behaviour, escape
 * character "\" can be used.
 *
 *
 * + Embed whitespace or '\' to the text:
 *   - "\\"  - backslash.
 *   - "\ "   - space.
 * + Change indentation on the fly:
 *   - "\>nr" - change indentation relative to the base. In RT mode base is \c indent
 *               and in CO mode it is \c headerLen. Change in indentation is applied after
 *               next new line. Note that "\>" is same as "\>0". Also negative value
 *               can be used. If overall indentation is negative, it is set to \c 0.
 *               If it's larger than \c rowLen, it is set to <tt>rowLen - 1</tt>.
 *
 *
 * Limitations
 *
 * Words are not split. If word (+ indentation) is larger than \c rowLen, it is just put on the line.
 *
 * Examples
 *
 * + \c rowLen = \c 20, \c indent = \c 2, \c headerLen = \c 10, \c paramName = \c "-i", \c delimiter = \c '-' and \c txt = \c "text text text text!"
 * \code{.txt}
 * |  -i    - text text |
 * |          text text!|
 * \endcode
 *
 * + \c rowLen = \c 20, \c indent = \c 2, \c headerLen = \c 10, \c paramName = \c "--lon-param", \c delimiter = \c '-'  and \c txt = \c "text text text text!"
 * \code{.txt}
 * |  --lon-param       |
 * |  -i    - text text |
 * |          text text!|
 * \endcode
 *
 * + \c rowLen = \c 20, \c indent = \c 2, \c headerLen = \c 7, \c paramName = \c "-i", \c delimiter = \c '-'  and \c
 * txt = \c "txt txt txt t txt txt txt txt:\\>2\n*\\>4 list item list item\\>2\n*\\>4 list item list item\\>\ntxt txt"
 * \code{.txt}
 * |  -i - txt txt txt t|
 * |       txt txt txt  |
 * |       txt:         |
 * |         * list item|
 * |           list item|
 * |         * list item|
 * |           list item|
 * |       txt txt      |
 * \endcode
 *
 * + \c rowLen = \c 20, \c indent = \c 2, \c headerLen = \c X, \c paramName = \c NULL and \c
 * txt = \c "text text text text text\n\\>2\n+\\>4 txt txt txt txt\n\\>\ntext text text text text text"
 * \code{.txt}
 * |  text text text    |
 * |  text text         |
 * |                    |
 * |    + txt txt txt   |
 * |      txt           |
 * |                    |
 * |  text text text    |
 * |  text text text    |
 * \endcode
 *
 *
 * \param buf			Pointer to buffer where the formatted text is stored.
 * \param buf_len		The size of the buffer.
 * \param rowLen		The overall size of the row.
 * \param indent		The size of indentation. Can be \c 0.
 * \param headerLen		The size of the header. Available only in CO mode. Can be \c 0.
 * \param paramName		Parameter name, if NOT \NULL function works in CO mode.
 * \param delimiter		Delimiter character used to separates parameter name from description. Available only in CO mode.
 * \param txt			Format string: parameter description in CO mode or regular text RT mode.
 * \param ...			Extra parameters for formatting.
 * \return The number of characters written, not including terminating \c NULL character. On error \c 0 is returned.
 */
size_t PST_snhiprintf(char *buf, size_t buf_len, unsigned rowLen, unsigned indent, unsigned headerLen, const char *paramName, const char delimiter, const char *txt, ...);

/**
 * Same as #PST_snhiprintf but with <tt>va_list</tt>.
 *
 * \param buf			Pointer to buffer where the formatted text is stored.
 * \param buf_len		The size of the buffer.
 * \param rowLen		The overall size of the row.
 * \param indent		The size of indentation. Can be \c 0.
 * \param headerLen		The size of the header. Available only in CO mode. Can be \c 0.
 * \param paramName		Parameter name, if NOT \NULL function works in CO mode.
 * \param delimiter		Delimiter character used to separates parameter name from description. Available only in CO mode.
 * \param txt			Format string: parameter description in CO mode or regular text RT mode.
 * \param va			Variable list.
 * \return The number of characters written, not including terminating \c NULL character. On error \c 0 is returned.
 */
size_t PST_vsnhiprintf(char *buf, size_t buf_len, unsigned indent, unsigned headerLen, unsigned rowLen, const char *paramName, const char delimiter, const char *txt, va_list va);

/*
 * @}
 */

#ifdef	__cplusplus
}
#endif

#endif	/* STRN_H */
