##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=dapcxx
ConfigurationName      :=Debug
WorkspaceConfiguration := $(ConfigurationName)
WorkspacePath          :=C:/src/gdbd
ProjectPath            :=C:/src/gdbd/dap
IntermediateDirectory  :=../build-$(ConfigurationName)/dap
OutDir                 :=../build-$(ConfigurationName)/dap
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Eran
Date                   :=21/01/2020
CodeLitePath           :="C:/Program Files/CodeLite"
LinkerName             :=C:/compilers/mingw64/bin/g++.exe
SharedObjectLinkerName :=C:/compilers/mingw64/bin/g++.exe -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=..\build-$(ConfigurationName)\lib\lib$(ProjectName).a
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :=$(IntermediateDirectory)/ObjectsList.txt
PCHCompileFlags        :=
RcCmpOptions           := 
RcCompilerName         :=C:/compilers/mingw64/bin/windres.exe
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := C:/compilers/mingw64/bin/ar.exe rcu
CXX      := C:/compilers/mingw64/bin/g++.exe
CC       := C:/compilers/mingw64/bin/gcc.exe
CXXFLAGS :=  -g -std=c++11 $(Preprocessors)
CFLAGS   :=  -g $(Preprocessors)
ASFLAGS  := 
AS       := C:/compilers/mingw64/bin/as.exe


##
## User defined environment variables
##
>CodeLiteDir:=C:\src\codelite\Runtime32
WXWIN:=C:\src\wxWidgets
WXCFG:=gcc_dll\mswu
CodeLiteDir:=C:\Program Files\CodeLite
Objects0=../build-$(ConfigurationName)/dap/dap.cpp$(ObjectSuffix) ../build-$(ConfigurationName)/dap/json.cpp$(ObjectSuffix) ../build-$(ConfigurationName)/dap/cJSON.cpp$(ObjectSuffix) ../build-$(ConfigurationName)/dap/Process.cpp$(ObjectSuffix) ../build-$(ConfigurationName)/dap/JsonRPC.cpp$(ObjectSuffix) ../build-$(ConfigurationName)/dap/SocketServer.cpp$(ObjectSuffix) ../build-$(ConfigurationName)/dap/SocketClient.cpp$(ObjectSuffix) ../build-$(ConfigurationName)/dap/SocketBase.cpp$(ObjectSuffix) ../build-$(ConfigurationName)/dap/ConnectionString.cpp$(ObjectSuffix) ../build-$(ConfigurationName)/dap/StringUtils.cpp$(ObjectSuffix) \
	../build-$(ConfigurationName)/dap/msw.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: MakeIntermediateDirs ../build-$(ConfigurationName)/dap/$(OutputFile)

../build-$(ConfigurationName)/dap/$(OutputFile): $(Objects)
	@if not exist "..\build-$(ConfigurationName)\dap" mkdir "..\build-$(ConfigurationName)\dap"
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(AR) $(ArchiveOutputSwitch)$(OutputFile) @$(ObjectsFileList)
	@echo rebuilt > $(IntermediateDirectory)/dapcxx.relink

MakeIntermediateDirs:
	@if not exist "..\build-$(ConfigurationName)\dap" mkdir "..\build-$(ConfigurationName)\dap"
	@if not exist ""..\build-$(ConfigurationName)\lib"" mkdir ""..\build-$(ConfigurationName)\lib""

:
	@if not exist "..\build-$(ConfigurationName)\dap" mkdir "..\build-$(ConfigurationName)\dap"

PreBuild:


##
## Objects
##
../build-$(ConfigurationName)/dap/dap.cpp$(ObjectSuffix): dap.cpp ../build-$(ConfigurationName)/dap/dap.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/src/gdbd/dap/dap.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/dap.cpp$(ObjectSuffix) $(IncludePath)
../build-$(ConfigurationName)/dap/dap.cpp$(DependSuffix): dap.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../build-$(ConfigurationName)/dap/dap.cpp$(ObjectSuffix) -MF../build-$(ConfigurationName)/dap/dap.cpp$(DependSuffix) -MM dap.cpp

../build-$(ConfigurationName)/dap/dap.cpp$(PreprocessSuffix): dap.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../build-$(ConfigurationName)/dap/dap.cpp$(PreprocessSuffix) dap.cpp

../build-$(ConfigurationName)/dap/json.cpp$(ObjectSuffix): json.cpp ../build-$(ConfigurationName)/dap/json.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/src/gdbd/dap/json.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/json.cpp$(ObjectSuffix) $(IncludePath)
../build-$(ConfigurationName)/dap/json.cpp$(DependSuffix): json.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../build-$(ConfigurationName)/dap/json.cpp$(ObjectSuffix) -MF../build-$(ConfigurationName)/dap/json.cpp$(DependSuffix) -MM json.cpp

../build-$(ConfigurationName)/dap/json.cpp$(PreprocessSuffix): json.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../build-$(ConfigurationName)/dap/json.cpp$(PreprocessSuffix) json.cpp

../build-$(ConfigurationName)/dap/cJSON.cpp$(ObjectSuffix): cJSON.cpp ../build-$(ConfigurationName)/dap/cJSON.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/src/gdbd/dap/cJSON.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/cJSON.cpp$(ObjectSuffix) $(IncludePath)
../build-$(ConfigurationName)/dap/cJSON.cpp$(DependSuffix): cJSON.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../build-$(ConfigurationName)/dap/cJSON.cpp$(ObjectSuffix) -MF../build-$(ConfigurationName)/dap/cJSON.cpp$(DependSuffix) -MM cJSON.cpp

../build-$(ConfigurationName)/dap/cJSON.cpp$(PreprocessSuffix): cJSON.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../build-$(ConfigurationName)/dap/cJSON.cpp$(PreprocessSuffix) cJSON.cpp

../build-$(ConfigurationName)/dap/Process.cpp$(ObjectSuffix): Process.cpp ../build-$(ConfigurationName)/dap/Process.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/src/gdbd/dap/Process.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Process.cpp$(ObjectSuffix) $(IncludePath)
../build-$(ConfigurationName)/dap/Process.cpp$(DependSuffix): Process.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../build-$(ConfigurationName)/dap/Process.cpp$(ObjectSuffix) -MF../build-$(ConfigurationName)/dap/Process.cpp$(DependSuffix) -MM Process.cpp

../build-$(ConfigurationName)/dap/Process.cpp$(PreprocessSuffix): Process.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../build-$(ConfigurationName)/dap/Process.cpp$(PreprocessSuffix) Process.cpp

../build-$(ConfigurationName)/dap/JsonRPC.cpp$(ObjectSuffix): JsonRPC.cpp ../build-$(ConfigurationName)/dap/JsonRPC.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/src/gdbd/dap/JsonRPC.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/JsonRPC.cpp$(ObjectSuffix) $(IncludePath)
../build-$(ConfigurationName)/dap/JsonRPC.cpp$(DependSuffix): JsonRPC.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../build-$(ConfigurationName)/dap/JsonRPC.cpp$(ObjectSuffix) -MF../build-$(ConfigurationName)/dap/JsonRPC.cpp$(DependSuffix) -MM JsonRPC.cpp

../build-$(ConfigurationName)/dap/JsonRPC.cpp$(PreprocessSuffix): JsonRPC.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../build-$(ConfigurationName)/dap/JsonRPC.cpp$(PreprocessSuffix) JsonRPC.cpp

../build-$(ConfigurationName)/dap/SocketServer.cpp$(ObjectSuffix): SocketServer.cpp ../build-$(ConfigurationName)/dap/SocketServer.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/src/gdbd/dap/SocketServer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/SocketServer.cpp$(ObjectSuffix) $(IncludePath)
../build-$(ConfigurationName)/dap/SocketServer.cpp$(DependSuffix): SocketServer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../build-$(ConfigurationName)/dap/SocketServer.cpp$(ObjectSuffix) -MF../build-$(ConfigurationName)/dap/SocketServer.cpp$(DependSuffix) -MM SocketServer.cpp

../build-$(ConfigurationName)/dap/SocketServer.cpp$(PreprocessSuffix): SocketServer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../build-$(ConfigurationName)/dap/SocketServer.cpp$(PreprocessSuffix) SocketServer.cpp

../build-$(ConfigurationName)/dap/SocketClient.cpp$(ObjectSuffix): SocketClient.cpp ../build-$(ConfigurationName)/dap/SocketClient.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/src/gdbd/dap/SocketClient.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/SocketClient.cpp$(ObjectSuffix) $(IncludePath)
../build-$(ConfigurationName)/dap/SocketClient.cpp$(DependSuffix): SocketClient.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../build-$(ConfigurationName)/dap/SocketClient.cpp$(ObjectSuffix) -MF../build-$(ConfigurationName)/dap/SocketClient.cpp$(DependSuffix) -MM SocketClient.cpp

../build-$(ConfigurationName)/dap/SocketClient.cpp$(PreprocessSuffix): SocketClient.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../build-$(ConfigurationName)/dap/SocketClient.cpp$(PreprocessSuffix) SocketClient.cpp

../build-$(ConfigurationName)/dap/SocketBase.cpp$(ObjectSuffix): SocketBase.cpp ../build-$(ConfigurationName)/dap/SocketBase.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/src/gdbd/dap/SocketBase.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/SocketBase.cpp$(ObjectSuffix) $(IncludePath)
../build-$(ConfigurationName)/dap/SocketBase.cpp$(DependSuffix): SocketBase.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../build-$(ConfigurationName)/dap/SocketBase.cpp$(ObjectSuffix) -MF../build-$(ConfigurationName)/dap/SocketBase.cpp$(DependSuffix) -MM SocketBase.cpp

../build-$(ConfigurationName)/dap/SocketBase.cpp$(PreprocessSuffix): SocketBase.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../build-$(ConfigurationName)/dap/SocketBase.cpp$(PreprocessSuffix) SocketBase.cpp

../build-$(ConfigurationName)/dap/ConnectionString.cpp$(ObjectSuffix): ConnectionString.cpp ../build-$(ConfigurationName)/dap/ConnectionString.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/src/gdbd/dap/ConnectionString.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ConnectionString.cpp$(ObjectSuffix) $(IncludePath)
../build-$(ConfigurationName)/dap/ConnectionString.cpp$(DependSuffix): ConnectionString.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../build-$(ConfigurationName)/dap/ConnectionString.cpp$(ObjectSuffix) -MF../build-$(ConfigurationName)/dap/ConnectionString.cpp$(DependSuffix) -MM ConnectionString.cpp

../build-$(ConfigurationName)/dap/ConnectionString.cpp$(PreprocessSuffix): ConnectionString.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../build-$(ConfigurationName)/dap/ConnectionString.cpp$(PreprocessSuffix) ConnectionString.cpp

../build-$(ConfigurationName)/dap/StringUtils.cpp$(ObjectSuffix): StringUtils.cpp ../build-$(ConfigurationName)/dap/StringUtils.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/src/gdbd/dap/StringUtils.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/StringUtils.cpp$(ObjectSuffix) $(IncludePath)
../build-$(ConfigurationName)/dap/StringUtils.cpp$(DependSuffix): StringUtils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../build-$(ConfigurationName)/dap/StringUtils.cpp$(ObjectSuffix) -MF../build-$(ConfigurationName)/dap/StringUtils.cpp$(DependSuffix) -MM StringUtils.cpp

../build-$(ConfigurationName)/dap/StringUtils.cpp$(PreprocessSuffix): StringUtils.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../build-$(ConfigurationName)/dap/StringUtils.cpp$(PreprocessSuffix) StringUtils.cpp

../build-$(ConfigurationName)/dap/msw.cpp$(ObjectSuffix): msw.cpp ../build-$(ConfigurationName)/dap/msw.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/src/gdbd/dap/msw.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/msw.cpp$(ObjectSuffix) $(IncludePath)
../build-$(ConfigurationName)/dap/msw.cpp$(DependSuffix): msw.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../build-$(ConfigurationName)/dap/msw.cpp$(ObjectSuffix) -MF../build-$(ConfigurationName)/dap/msw.cpp$(DependSuffix) -MM msw.cpp

../build-$(ConfigurationName)/dap/msw.cpp$(PreprocessSuffix): msw.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../build-$(ConfigurationName)/dap/msw.cpp$(PreprocessSuffix) msw.cpp


-include ../build-$(ConfigurationName)/dap//*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


