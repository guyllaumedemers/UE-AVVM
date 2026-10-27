@echo off
setlocal

REM Get the directory of this batch file, then go up one level to your project root
set "PROJECT_ROOT=%~dp5..\.."

:: Find the .uproject file in the current or parent directory
for %%f in (*.uproject) do (
    set "PROJECT_NAME=%%~nf"
)

echo Found Project Name: %PROJECT_NAME%

:: Start searching from the folder where this batch file lives
set "CURRENT_DIR=%~dp0"

:FindEngineLoop
:: Check if the Engine folder exists in the current directory path
if exist "%CURRENT_DIR%Engine" (
    goto FoundEngine
)

:: Otherwise, go up one directory level
for %%I in ("%CURRENT_DIR%..") do set "PARENT_DIR=%%~fI"

:: Prevent infinite loop if we hit the root of the drive (e.g., C:\)
if "%CURRENT_DIR%"=="%PARENT_DIR%" (
    echo Error: Could not find the Engine directory!
    exit /b 1
)

set "CURRENT_DIR=%PARENT_DIR%\Engine"
goto FindEngineLoop

:FoundEngine
:: Strip the trailing backslash for clean paths
set "ENGINE_ROOT=%CURRENT_DIR:~0,-1%"
echo Found Engine at: %ENGINE_ROOT%

:: Set default values in case arguments aren't provided
set "CONFIG=Development"
set "PLATFORM=Win64"
set "BUILD_SCRIPT=%PROJECT_ROOT%\Build\Graphs\GauntletPipelineWithGit.xml"

:: Loop through all arguments passed to the batch file
:ParseLoop
if "%~1"=="" goto EndParseLoop

:: Split the argument by '=' or ':' into Key and Value
for /f "tokens=1,* delims==:" %%a in ("%~1") do (
    set "PARAM_KEY=%%a"
    set "PARAM_VAL=%%b"
)

:: Match the key (case-insensitive) and assign the value
if /i "%PARAM_KEY%"=="-configuration" set "CONFIG=%PARAM_VAL%"
if /i "%PARAM_KEY%"=="configuration"  set "CONFIG=%PARAM_VAL%"

if /i "%PARAM_KEY%"=="-platform" set "PLATFORM=%PARAM_VAL%"
if /i "%PARAM_KEY%"=="platform"  set "PLATFORM=%PARAM_VAL%"

if /i "%PARAM_KEY%"=="-script" set "BUILD_SCRIPT=%PARAM_VAL%"
if /i "%PARAM_KEY%"=="script"  set "BUILD_SCRIPT=%PARAM_VAL%"

shift
goto ParseLoop

:EndParseLoop
echo Configuration: %CONFIG%
echo Platform: %PLATFORM%
echo Script Path: %BUILD_SCRIPT%

REM Call Unreal's RunUAT and pass the variable or use it
"%ENGINE_ROOT%\Build\BatchFiles\RunUAT.bat" BuildGraph ^
	-Script="%PROJECT_ROOT%\Build\Graphs\%BUILD_SCRIPT%.xml" ^
	-Set:ProjectDir="%PROJECT_ROOT%" ^
	-Set:ProjectName="%PROJECT_NAME%" ^
	-Set:EngineDir="%ENGINE_ROOT%" ^
	-set:SharedDDCPath="\\YourNetworkShare\UnrealSharedDDC" ^
	-Set:Configuration="%CONFIG%" ^
	-Set:Platform="%PLATFORM%" ^
	-Target="CI_FullPipeline"

endlocal