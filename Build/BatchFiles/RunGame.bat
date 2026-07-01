@echo off
setlocal enabledelayedexpansion

:: Generate a timestamp (YYYY-MM-DD HH:MM)
for /f "tokens=2-4 delims=/ " %%a in ('date /t') do set "mydate=%%c-%%a-%%b"
for /f "tokens=1-2 delims=: " %%a in ('time /t') do set "mytime=%%a:%%b"
set "TIMESTAMP=[%mydate% %mytime%]"

:: 1. Setup paths and log file
set "LOG_FILE=%CD%\launch_errors.log"
set "ENGINE_ROOT=%~dp0..\..\..\.."
set "UBT_EXE=%ENGINE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe"
set "UNREAL_EDITOR=%ENGINE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"
set "PROJECT_NAME=UISample"
set "PROJECT_PATH=%~dp0..\..\%PROJECT_NAME%.uproject"
set "STARTUP_MAP=/Game/UE-AVVM/Content/ThirdPerson/Lvl_ThirdPerson.umap"

echo %TIMESTAMP% [0/3] Search/Validate project requirements...

:: Clean up the path (resolves the ".." into a clean, readable path)
for %%I in ("%UNREAL_EDITOR%") do set "UNREALED=%%~fI"
for %%I in ("%PROJECT_PATH%") do set "UPROJECT=%%~fI"

:: Verify executable & files exist before trying to run them
if not exist "%UNREALED%" (
    echo %TIMESTAMP% [CRITICAL] Editor executable missing at: "%UNREALED%" >> "%LOG_FILE%"
    echo Error: Editor executable not found! Check launch_errors.log.
    goto :fail
)

if not exist "%UPROJECT%" (
    echo %TIMESTAMP% [CRITICAL] .uproject file missing at: "%UPROJECT%" >> "%LOG_FILE%"
    echo Error: Project file not found! Check launch_errors.log.
    goto :fail
)

:: 2. Navigate to your project folder
cd /d "%~dp0..\.."

echo %TIMESTAMP% [1/3] Cleaning project and plugin binaries...
:: 3. This forces a cleanup of binary files, and object files
if exist "Binaries" rmdir /s /q "Binaries"
if exist "Intermediate" rmdir /s /q "Intermediate"

:: 4. Find every .uplugin file under the Plugins directory and clean its binaries
for /r "%CD%\Plugins" %%P in (*.uplugin) do (
    set "PLUGIN_DIR=%%~dpP"
	
    if exist "!PLUGIN_DIR!Binaries" (
        echo Cleaning: !PLUGIN_DIR!Binaries
        rmdir /s /q "!PLUGIN_DIR!Binaries"
    )
	
    if exist "!PLUGIN_DIR!Intermediate" (
        rmdir /s /q "!PLUGIN_DIR!Intermediate"
    )
)

echo %TIMESTAMP% [2/3] Regenerating Project Manifest Assemblies
:: 5. This forces UBT to regenerate Project Manifest Assemblies
"%UBT_EXE%" -projectfiles -project="%PROJECT_PATH%" -game -rocket -progress

echo %TIMESTAMP% [3/3] Compiling Discovered Plugins Individually
:: 6. Loop through every plugin and run UBT separately to avoid argument conflicts
::for /r "%CD%\Plugins" %%P in (*.uplugin) do (
::    echo.
::    echo [BUILDING PLUGIN] %%~nP...
::    
::    "%UBT_EXE%" %PROJECT_NAME%Editor Win64 Development -Project="%PROJECT_PATH%" -Plugin="%%P" -WaitMutex
::    
::    if !ERRORLEVEL! NEQ 0 (
::        echo [ERROR] Failed compiling plugin: %%~nP
::        goto :fail
::    )
::)

echo %TIMESTAMP% [FINAL STEP] Building the Main Project Game Target
::7. Final build invocation for the project core logic now that all plugin .dlls exist
"%UBT_EXE%" %PROJECT_NAME%Editor Win64 Development -Project="%PROJECT_PATH%" -WaitMutex

if %ERRORLEVEL% NEQ 0 (
    echo %TIMESTAMP% [ERROR] UnrealBuildTool failed to compile project.
    goto :fail
)

echo [SUCCESS] Entire environment compiled perfectly without conflicts!

echo %TIMESTAMP% [3/3] Launching Unreal Editor...
:: 3. Launch and WAIT for the exit code, redirecting command-line errors to the log
start /wait %UNREAL_EDITOR% %PROJECT_PATH% %STARTUP_MAP% -game -windowed -ResX=640 -ResY=400 -log 2>> "%LOG_FILE%"

:: 4. Check the return code from Unreal Editor
set "EXIT_CODE=%ERRORLEVEL%"

if %EXIT_CODE% NEQ 0 (
    echo %TIMESTAMP% [FAILURE] UnrealEditor closed with Exit Code: %EXIT_CODE% >> "%LOG_FILE%"
    echo --------------------------------------------------------------------- >> "%LOG_FILE%"
    
    echo [ERROR] Editor failed to launch or crashed.
    echo Exit Code: %EXIT_CODE%
    echo Details have been logged to: %LOG_FILE%
    goto :fail
)

echo [SUCCESS] Editor closed normally.
exit /b 0

:fail
pause
exit /b 1