
set MSBuild="%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\amd64\MSBuild.exe"
if not exist %MSBuild% (
	set MSBuild="%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe"
)
call %MSBuild%
call "pkggen\bin\x64\Debug\pkggen.exe"
if exist output\PKG_TypeIdMappings.cs (
	xcopy output\PKG_TypeIdMappings.cs pkggen_template_PKG\ /y
	call %MSBuild%
	call "pkggen\bin\x64\Debug\pkggen.exe"
	del output\PKG_TypeIdMappings.cs
)
call %MSBuild%
call "cfg_maker\bin\x64\Debug\cfg_maker.exe"
xcopy output\cfg.bin ..\x64\Debug /y
xcopy output\cfg.bin ..\x64\Release /y
xcopy output\cfg.bin ..\..\xxlib_cocos_cpp\res /y
xcopy output\PKG_class_filter.h ..\..\xxlib_cocos_cpp\Classes\PKG_class.h /y
xcopy output\PKG_class_filter.hpp ..\..\xxlib_cocos_cpp\Classes\PKG_class.hpp /y
pause