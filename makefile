#
# Copyright 2013-2016 Guardtime, Inc.
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

!IF "$(RTL)" != "MT" && "$(RTL)" != "MTd" && "$(RTL)" != "MD" && "$(RTL)" != "MDd"
!ERROR "ROOT MAKE: You Must specify RTL as MT, MTd, MD or MDd"
!ENDIF

#Selecting of C Run-Time library and output format.
!IF "$(DLL)" != "lib" && "$(DLL)" != "dll"
!ERROR "ROOT MAKE: You Must specify DLL as lib or dll"
!ENDIF

SRC_DIR = src
PARAM_SET_DIR = $(SRC_DIR)\param_set
TEST_DIR = test
VERSION_FILE = VERSION

VER = \
!INCLUDE <$(VERSION_FILE)>

#Making
test: param_set
	cd $(TEST_DIR)
	nmake RTL=$(RTL) ENG_LIB=$(DLL)
	cd ..

param_set:
	cd $(PARAM_SET_DIR)/
	nmake RTL=$(RTL) DLL=$(DLL) VERSION=$(VER)
	cd ..\..\


clean:
	cd $(TEST_DIR)
	nmake RTL=$(RTL) ENG_LIB=$(DLL) clean
	cd ..

	cd $(PARAM_SET_DIR)
	nmake RTL=$(RTL) DLL=$(DLL) clean
	cd ..\..\
