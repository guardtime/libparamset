!IF "$(RTL)" != "MT" && "$(RTL)" != "MTd" && "$(RTL)" != "MD" && "$(RTL)" != "MDd"
!ERROR "ROOT MAKE: You Must specify RTL as MT, MTd, MD or MDd"
!ENDIF

#Selecting of C Run-Time library and output format. 
!IF "$(DLL)" != "lib" && "$(DLL)" != "dll"
!ERROR "ROOT MAKE: You Must specify DLL as lib or dll"
!ENDIF

SRC_DIR = src
PARAM_SET_DIR = $(SRC_DIR)\param_set
TEST_DIR = $(SRC_DIR)\test



#Making
test: param_set
	cd $(TEST_DIR)
	nmake RTL=$(RTL) ENG_LIB=$(DLL)
	cd ..\..\

param_set:
	cd $(PARAM_SET_DIR)/
	nmake RTL=$(RTL) DLL=$(DLL)
	cd ..\..\


clean:
	cd $(TEST_DIR)
	nmake RTL=$(RTL) ENG_LIB=$(DLL) clean
	cd ..\..\
	
	cd $(PARAM_SET_DIR)
	nmake RTL=$(RTL) DLL=$(DLL) clean
	cd ..\..\
