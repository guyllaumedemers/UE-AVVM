@echo off

rem ## environment changes made after you run setlocal are local to the batch file.
setlocal

rem ## dir path assertion
set BatchFileDir=%~dp0..\BatchFiles
set ScriptFileDir=%~dp0..\Scripts
set EngineDir=%~dp0..\..\Engine

rem ## environement globals
set DirectoryLookup=
set SymlinkPath=
set Script=%ScriptFileDir%

rem ## Parse batch file argument list.
:GetParameterArguments
set Delim=%~2
rem ## TODO stip possible explicit file extension
if /i "%~1" == "-Script" (if not "%Delim:~0,1%" == "-" ((set Script=%Script%\%Delim%) & shift) else (goto Assert_BadArgumentPassing))
REM rem ## Shift the position of the Parameters list so 
shift
if not "%~1" == "" goto GetParameterArguments

rem ## validate batch process execution path.
:GetSetRelevantDirectories
if not exist %BatchFileDir% (set DirectoryLookup=%BatchFileDir% & goto Assert_DirectoryLookup)
if not exist %ScriptFileDir% (set DirectoryLookup=%ScriptFileDir% & goto Assert_DirectoryLookup)

:CheckEngineDir
if not exist %EngineDir%\BatchFiles (set DirectoryLookup=%EngineDir%\BatchFiles & goto Warning_CheckIfShortcutExist)
pushd %~dp0..\..\Engine\BatchFiles
goto Execute
	
:Warning_CheckIfShortcutExist
echo.
echo WARNING: Directory lookup failed. Engine relative path must exist somewhere. Checking available Symlink.
echo.
rem ## move to project root.
pushd %~dp0..\..
rem ## check existing symlink under root.
for %%g in (*.lnk) do (call :Subroutine_Symlink "%cd%\%%~g")
goto EOF

:Subroutine_Symlink
set SymlinkPath=%1
call set SymlinkPath=%%SymlinkPath:\=\\%%
for /f "delims=" %%h in ('wmic path win32_shortcutfile where 'name^=%SymlinkPath%' get target /value') do (
	for /f "tokens=2,* delims=^=" %%i in ("%%~h") do (call :Subroutine_MoveToSymlink %%i)
)
goto EOF

:Subroutine_MoveToSymlink
if not exist %1\Engine\Build\BatchFiles (set DirectoryLookup=%1\Engine\Build\BatchFiles & goto Assert_DirectoryLookup)
pushd %1\Engine\Build\BatchFiles
goto Execute

:Assert_DirectoryLookup
echo.
echo ERROR: Directory lookup failed. Running process doesn't appear to be executed from within the expected directory path %DirectoryLookup%.
echo.
goto Exit

:Assert_BadArgumentPassing
rem ## Output a blank line
echo.
echo ERROR: The Parameter passing arguments are invalid. Argument passing should be provided like so: ./RunTest_Demo.bat -Arg0 Value0 -Arg1 Value1 etc...
echo.
goto Exit

rem ## run BuildGraph test process.
:Execute
echo Executing RunUAT...
call .\RunUAT BuildGraph -Script="%Script%.xml"

:Exit
rem ## Suspend the process of the batch program and request user input for program closure.
pause
rem ## Exit the current bash script instead of the cmd.exe. <exitcode> set the EnvVAR:ERRORLEVEL.
exit /b 1

:EOF