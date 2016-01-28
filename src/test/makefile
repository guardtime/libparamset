!IF "$(RTL)" != "MT" && "$(RTL)" != "MTd" && "$(RTL)" != "MD" && "$(RTL)" != "MDd"
!ERROR "TEST MAKE: You Must specify RTL as MT, MTd, MD or MDd"
!ENDIF

!IF "$(ENG_LIB)" != "lib" && "$(ENG_LIB)" != "dll"
!ERROR "TEST MAKE: You Must specify ENG_LIB as lib or dll"
!ENDIF

TOOL_NAME = test

PARAM_SET_LIB_NAME = libparamset
PARAM_SET_LIB_DIR = ..\..\out\$(ENG_LIB)

SRC_DIR = .
SRC_CUTEST_DIR = $(SRC_DIR)\cutest
OBJ_DIR = ..\..\obj\$(TOOL_NAME)
OBJ_CUTEST_DIR = ..\..\obj\cutest
BIN_DIR = ..\..\out\bin


#Objects for making command-line tool

TOOL_OBJ = \
	$(OBJ_DIR)\all_tests.obj \
	$(OBJ_DIR)\param_value_test.obj \
	$(OBJ_DIR)\parameter_test.obj \
	$(OBJ_DIR)\param_set_test.obj \
	$(OBJ_DIR)\support_tests.obj \
	$(OBJ_DIR)\task_def_test.obj
	
CUTEST_OBJ = $(OBJ_CUTEST_DIR)\CuTest.obj	
	


#Compiler and linker configuration
EXT_LIB = $(PARAM_SET_LIB_NAME)$(RTL).lib \
	user32.lib gdi32.lib advapi32.lib Ws2_32.lib Kernel32.lib
	  
	
CCFLAGS = /nologo /W4 /D_CRT_SECURE_NO_DEPRECATE  /I$(SRC_DIR) /I$(SRC_CUTEST_DIR)
LDFLAGS = $(LDFLAGS) /NOLOGO /LIBPATH:$(PARAM_SET_LIB_DIR)

!IF "$(ENG_LIB)" == "dll"
CCFLAGS = $(CCFLAGS) /DLINKEAGAINSTDLLENGINE
!MESSAGE LNINKING AGAINST DLL
!ENDIF

!IF "$(RTL)" == "MT" || "$(RTL)" == "MD"
CCFLAGS = $(CCFLAGS) /DNDEBUG /O2
LDFLAGS = $(LDFLAGS) /RELEASE
!ELSE
CCFLAGS = $(CCFLAGS) /D_DEBUG /Od /RTC1 /Zi
LDFLAGS = $(LDFLAGS) /DEBUG
!ENDIF


CCFLAGS = $(CCFLAGS) $(CCEXTRA)
LDFLAGS = $(LDFLAGS) $(LDEXTRA)


#Making

default: $(BIN_DIR)\$(TOOL_NAME).exe 

#Linking 
$(BIN_DIR)\$(TOOL_NAME).exe: $(BIN_DIR) $(OBJ_DIR) $(OBJ_CUTEST_DIR) $(TOOL_OBJ) $(CUTEST_OBJ)
	link $(LDFLAGS) /OUT:$@ $(TOOL_OBJ) $(CUTEST_OBJ) $(EXT_LIB) 

	
#C file compilation  	
{$(SRC_DIR)\}.c{$(OBJ_DIR)\}.obj:
	cl /c /$(RTL) $(CCFLAGS) /Fo$@ $<

{$(SRC_CUTEST_DIR)\}.c{$(OBJ_CUTEST_DIR)\}.obj:
	cl /c /$(RTL) $(CCFLAGS) /Fo$@ $<
	

#Folder factory	
$(OBJ_DIR) $(OBJ_CUTEST_DIR) $(BIN_DIR):
	@if not exist $@ mkdir $@
	
clean:
	@for %i in ($(OBJ_DIR) $(OBJ_CUTEST_DIR) $(BIN_DIR)) do @if exist .\%i rmdir /s /q .\%i
	