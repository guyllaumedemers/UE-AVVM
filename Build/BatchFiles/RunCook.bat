@echo off

rem ## Attempt to scripting a bat file that will run Unreal cooking process.
setlocal enableextensions
echo Running Cook process...

rem ## Define a set of global variables to be injected by the run command.
set ProjectName=
set CookPlatforms=
set CookMaps=
set UnrealEdDir=

rem ## Parse batch file argument list.
:GetParameterArguments
set Delim=%~2
if /i "%~1" == "-Project" (if not "%Delim:~0,1%" == "-" (set ProjectName=%~2 & shift) else (goto Error_BadArgumentPassing))
if /i "%~1" == "-Platforms" (if not "%Delim:~0,1%" == "-" (set CookPlatforms=%~2 & shift) else (goto Error_BadArgumentPassing))
if /i "%~1" == "-Maps" (if not "%Delim:~0,1%" == "-" (set CookMaps=%~2 & shift) else (goto Error_BadArgumentPassing))
REM rem ## Shift the position of the Parameters list so 
shift
if not "%~1" == "" goto GetParameterArguments

rem ## Verify the BatchFile location.
if not exist "%~dp0..\..\Source" goto Error_BatchFileInWrongLocation

rem ## batch file parameters
echo %ProjectName% %CookPlatforms% %CookMaps%

rem ## Check if UnrealEditor.exe can be accessed from your %ProjectName%\Engine\Binaries\%CookPlatforms%\ directory, if thats how the engine setup is done. 
pushd "%~dp0..\..\Source"
if not exist "%~dp0Engine\Binaries\$(Platform)\UnrealEditor.exe" goto Error_CheckAlternativePath

goto UnrealEdFound

:Error_BadArgumentPassing
rem ## Output a blank line
echo.
echo RunCook ERROR: The Parameter passing arguments are invalid. Argument passing should be done as follow : ./RunCook.bat -Arg0 Value0 -Arg1 Value1. Exiting process...
echo.
pause
goto Exit

:Error_BatchFileInWrongLocation
rem ## Output a blank line
echo.
echo RunCook ERROR: The batch file does not appear to be located in the \$(ProjectName)\Build\BatchFiles directory. Process must be run from withing this location.
echo.
rem ## Suspend the process of the batch program and request user input for program closure.
pause
goto Exit

:Error_CheckAlternativePath
echo.
echo RunCook ERROR: UnrealEditor.exe not found. Searching in alternative locations.
echo.
rem ## fsutils fsinfo drives return a single line -> Drives: C:\ ... which is why we are using token to create an additional variable who's input the drive list.
for /f "tokens=1,*" %%g in ('fsutil fsinfo drives') do (
	for %%c in (%%h) do (echo %%~dc)
	)
pause
goto Exit

:Error_CannotExecuteRunCook
echo.
echo RunCook ERROR: Command cannot be executed properly. Exiting process...
echo.
pause
goto Exit

:UnrealEdFound
pushd %UnrealEdDir%
rem ## Run the CMD for cooking.
call UnrealEditor %ProjectName% -run=cook -targetplatform=%CookPlatforms% -map=%CookMaps% -NODEV
goto Exit

:Exit
rem ## Restore the command prompt directory to its previous state.
popd
rem ## Exit the current bash script instead of the cmd.exe. <exitcode> set the EnvVAR:ERRORLEVEL.
exit /b 1