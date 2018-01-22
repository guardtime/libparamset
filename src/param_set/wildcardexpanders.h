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

#ifndef WILDCARDEXPANDER_H
#define	WILDCARDEXPANDER_H
#include <stddef.h>
#include <stdarg.h>

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * This Wildcard expander implementation enables file and directory matching with
 * wildcard for Windows platform. It takes #PARAM_VAL as input, interprets it as a
 * file system path and matches it with file system directories as files. The
 * original value is removed and replaced with expanded values. If no match is found,
 * original value is still removed.
 *
 * When setting this function with #PARAM_SET_setWildcardExpander, set \c charList
 * (may also be '<tt>?*</tt>') and \c ctx  as \c NULL.
 *
 * \param param_value	PARAM_VAL object.
 * \param ctx			Additional ctx. Set as \c NULL.
 * \param value_shift	The output parameter for the count of expanded values.
 * \return #PST_OK if successful, error code otherwise. When not on Windows platform
 * #PST_PARAMETER_UNIMPLEMENTED_WILDCARD is returned.
 * \note Only available on Windows platform.
 * \see #PARAM_setWildcardExpander and #PARAM_SET_setWildcardExpander.
 */
int PST_WCF_Win32FileWildcard(PARAM_VAL *param_value, void *ctx, int *value_shift);

#ifdef	__cplusplus
}
#endif

#endif	/* WILDCARDEXPANDER_H */
