@echo off

rem ## Attempt to scripting a bat file that will run Unreal cooking process.
setlocal
echo Running Cook process...
echo.

rem ## Define a set of global variables to be injected by the run command.
set ProjectName=
set CookPlatforms=
set CookMaps=

echo Parsing CommandLine Arguments...

rem ## Parse batch file argument list.
:GetParameterArguments
set Delim=%~2
if /i "%~1" == "-Project" (if not "%Delim:~0,1%" == "-" ((set ProjectName=%Delim%) & shift) else (goto Error_BadArgumentPassing))
if /i "%~1" == "-Platforms" (if not "%Delim:~0,1%" == "-" ((set CookPlatforms=%Delim%) & shift) else (goto Error_BadArgumentPassing))
if /i "%~1" == "-Maps" (if not "%Delim:~0,1%" == "-" ((set CookMaps=%Delim%) & shift) else (goto Error_BadArgumentPassing))
REM rem ## Shift the position of the Parameters list so 
shift
if not "%~1" == "" goto GetParameterArguments

rem ## Verify the BatchFile location.
if not exist "%~dp0..\..\Source" (goto Error_BatchFileInWrongLocation)
pushd "%~dp0..\.."

set ProjectDirSingleSlash=%~dp0
set ProjectDirDoubleSlash=%ProjectDirSingleSlash:\=\\%

echo Searching UnrealEditor.exe in "%~dp0Engine\Binaries"...

rem ## Check if UnrealEditor.exe can be accessed from your %ProjectName%\Engine\Binaries\%CookPlatforms%\ directory, if thats how the engine setup is done.
for %%g in (%CookPlatforms%) do (
	if exist "%~dp0Engine\Binaries\%%~g\UnrealEditor.exe" (pushd "%~dp0Engine\Binaries\%%~g" & goto UnrealEdFound)
)

goto Error_CheckAlternativePath

:Error_BadArgumentPassing
rem ## Output a blank line
echo.
echo RunCook ERROR: The Parameter passing arguments are invalid. Argument passing should be provided like so: ./RunCook.bat -Arg0 Value0 -Arg1 Value1.
echo.
pause
goto Exit

:Error_CheckAlternativePath
echo.
echo RunCook ERROR: UnrealEditor.exe cannot be executed from "%~dp0Engine\Binaries" due to missing directory.
echo.
echo Searching matching directories based on available Drives...
echo.
rem ## fsutils fsinfo drives return a single line -> Drives: C:\ ... which is why we are using token to create an additional variable who's input the drive list.
for /f "tokens=1,*" %%g in ('fsutil fsinfo drives') do (
	for %%c in (%%~h) do (
		if exist "%%~dc\Documents\UnrealEngine" (pushd "%%~dc\Documents\UnrealEngine" & goto UnrealEdFound) else (echo RunCook ERROR: "%%~dc\Documents\UnrealEngine" is not a valid directory on your local machine.)
	)
)

goto Error_CheckIfShortcutExist
	
:Error_CheckIfShortcutExist
echo.
echo RunCook ERROR: UnrealEditor.exe cannot be executed from "*:\Documents\UnrealEngine".
echo.
echo Searching for Shortcut define in "%~dp0"...
echo.
rem ## Move directory to project ROOT.
for %%g in (*.lnk) do (
	rem ## Disable default Delim options so we don't split fetch shortcuts.
	for /f "delims=" %%h in ('wmic path win32_shortcutfile where 'name^="%ProjectDirDoubleSlash%%%~g"' get target /value') do (
		for /f "tokens=2,* delims=^=" %%i in ("%%~h") do (
			rem ## TODO Our ROOT project could have multiple .lnk which imply that validating the directory path is required before calling pushd and goto
			rem ## add missing checks.
			if not "%%~i" == "" (pushd "%%~i\Engine\Binaries\%CookPlatforms%" & goto UnrealEdFound)
		)
	)
)

goto Error_CannotExecuteRunCook

:Error_BatchFileInWrongLocation
rem ## Output a blank line
echo.
echo RunCook ERROR: The batch file does not appear to be located in the relative directory path "%ProjectName%\Build\BatchFiles". Process must be run from within this location.
echo.
rem ## Suspend the process of the batch program and request user input for program closure.
pause
goto Exit

:Error_CannotExecuteRunCook
echo.
echo RunCook ERROR: No shortcut .lnk available. 
echo.
pause
goto Exit

:UnrealEdFound
echo Found UnrealEditor.exe in: "%~dp0"
rem ## Run the CMD for cooking.
call UnrealEditor %ProjectDirDoubleSlash%%ProjectName% -run=cook
popd
goto Exit

:Exit
rem ## Restore the command prompt directory to its previous state.
popd
rem ## Exit the current bash script instead of the cmd.exe. <exitcode> set the EnvVAR:ERRORLEVEL.
exit /b 1