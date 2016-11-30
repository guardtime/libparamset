#
# Copyright 2013-2015 Guardtime, Inc.
#
# This file is part of the Guardtime client SDK.
#
# Licensed under the Apache License, Version 2.0 (the "License").
# You may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#     http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES, CONDITIONS, OR OTHER LICENSES OF ANY KIND, either
# express or implied. See the License for the specific language governing
# permissions and limitations under the License.
# "Guardtime" and "KSI" are trademarks or registered trademarks of
# Guardtime, Inc., and no license to trademarks is granted; Guardtime
# reserves and retains all trademark rights.
#

AC_INIT([libparamset], m4_esyscmd([tr -d [:space:] <VERSION]), [support@guardtime.com])
# AC_CONFIG_FILES([libparamset.pc])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([src/param_set/config.h])
AC_CANONICAL_TARGET

# Before making a release, the LTVER string should be modified. The
# string is of the form c:r:a. Follow these instructions sequentially:
#   1. If the library source code has changed at all since the last update, then
#      increment revision ('c:r:a' becomes 'c:r+1:a').
#   2. If any interfaces have been added, removed, or changed since the last
#      update, increment current, and set revision to 0 ('c:r:a' becomes 'c+1:0:a').
#   3. If any interfaces have been added since the last public release, then
#      increment age ('c:r:a' becomes 'c:r:a+1').
#   4. If any interfaces have been removed or changed since the last public
#      release, then set age to 0 ('c:r:a' becomes 'c:r:0'). 
LTVER="1:0:0"
AC_SUBST(LTVER)

AM_INIT_AUTOMAKE([subdir-objects foreign -Wall -Werror tar-ustar])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_PROG_LIBTOOL
AC_CHECK_PROG(git_installed,git,"yes", "no")

# Define the git hash of the current version.
AC_MSG_CHECKING([for commit id])
commit_id=""
if test "$git_installed" = "yes" -a -x ".git"; then
    commit_id=$(git log -n 1 --format="%H")
    AC_MSG_RESULT([$commit_id])
    AC_DEFINE_UNQUOTED(COMMIT_ID, "$commit_id", [Commit id])
else
    AC_MSG_RESULT([no])
fi


# Checks for libraries.



# To ensure compatibility with Microsoft compiler.
CFLAGS+= " -Wdeclaration-after-statement"
# Add more warnings
CFLAGS+=" -Wall"

AC_MSG_NOTICE([Update version.h])
rm -f src/param_set/version.h
VER=($(echo $PACKAGE_VERSION | tr "." " "))
VER_MAJOR=${VER[[0]]}
VER_MINOR=${VER[[1]]}
VER_BUILD=${VER[[2]]}
AC_SUBST(VER_MAJOR)
AC_SUBST(VER_MINOR)
AC_SUBST(VER_BUILD)
AC_CONFIG_FILES([src/param_set/version.h])

AC_CONFIG_FILES([GNUmakefile src/param_set/GNUmakefile src/test/GNUmakefile packaging/redhat/libparamset.spec])
AC_OUTPUT
