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
set "MAP_NAME=Lvl_ThirdPerson"
set "MAP_PATH=%~dp0..\..\Content\ThirdPerson\%MAP_NAME%.umap"

echo %TIMESTAMP% Search/Validate project requirements...

:: Clean up the path (resolves the ".." into a clean, readable path)
for %%I in ("%UNREAL_EDITOR%") do set "UNREALED=%%~fI"
for %%I in ("%PROJECT_PATH%") do set "UPROJECT=%%~fI"
for %%I in ("%MAP_PATH%") do set "MAP=%%~fI"

:: 2. Verify executable & files exist before trying to run them
if not exist "%UNREALED%" (
    echo %TIMESTAMP% [CRITICAL] Editor executable missing at: "%UNREALED%" >> "%LOG_FILE%"
    echo %TIMESTAMP% Error: Editor executable not found! Check launch_errors.log.
    goto :fail
)

if not exist "%UPROJECT%" (
    echo %TIMESTAMP% [CRITICAL] .uproject file missing at: "%UPROJECT%" >> "%LOG_FILE%"
    echo %TIMESTAMP% Error: Project file not found! Check launch_errors.log.
    goto :fail
)

if not exist "%MAP%" (
    echo %TIMESTAMP% [CRITICAL] .umap file missing at: "%MAP%" >> "%LOG_FILE%"
    echo %TIMESTAMP% Error: Map file not found! Check launch_errors.log.
    goto :fail
)

:: 3. Navigate to your project folder
cd /d "%~dp0..\.."

:: 3.1 Check if the first argument (%1) matches "clean" (case-insensitive via /i)
if /i "%~1"=="clean" (
    echo %TIMESTAMP% [CLEAN MODE] 'clean' flag detected. Flushing build caches...
    
    :: Wipe main project caches
    if exist "Intermediate" rmdir /s /q "Intermediate"
    if exist "Binaries" rmdir /s /q "Binaries"
    
    :: Recursively find and wipe all plugin Binaries and Intermediate folders
    for /r "%CD%\Plugins" %%P in (*.uplugin) do (
        set "PLUGIN_DIR=%%~dpP"
        if exist "!PLUGIN_DIR!Binaries" (
            echo %TIMESTAMP% Flushing: !PLUGIN_DIR!Binaries
            rmdir /s /q "!PLUGIN_DIR!Binaries"
        )
        if exist "!PLUGIN_DIR!Intermediate" (
            rmdir /s /q "!PLUGIN_DIR!Intermediate"
        )
    )

	echo %TIMESTAMP% Regenerating Project Manifest Assemblies
	:: 5. This forces UBT to regenerate Project Manifest Assemblies
	"%UBT_EXE%" -projectfiles -project="%PROJECT_PATH%" -game -rocket -progress
	
) else (
    echo %TIMESTAMP% [STANDARD MODE] Skipping directory flush. (Run with 'clean' argument to force a full rebuild)
)

echo %TIMESTAMP% [FINAL STEP] Building the Main Project Game Target
:: 3.2 Final build invocation for the project core logic now that all plugin .dlls exist
"%UBT_EXE%" "%PROJECT_NAME%Editor" Win64 Development -Project="%PROJECT_PATH%" -WaitMutex

if %ERRORLEVEL% NEQ 0 (
    echo %TIMESTAMP% [ERROR] UnrealBuildTool failed to compile project.
    goto :fail
)

echo %TIMESTAMP% [SUCCESS] Entire environment compiled perfectly without conflicts!
echo %TIMESTAMP% Launching Unreal Editor...
:: 4. Launch and WAIT for the exit code, redirecting command-line errors to the log
start /wait "%UNREAL_EDITOR%" "%PROJECT_PATH%" "%STARTUP_MAP%" -windowed -ResX=640 -ResY=400 -log 2>> "%LOG_FILE%"

if %ERRORLEVEL% NEQ 0 (
    echo %TIMESTAMP% [FAILURE] UnrealEditor closed with Exit Code: %ERRORLEVEL% >> "%LOG_FILE%"
    echo ---------------------------------------------------------------------- >> "%LOG_FILE%"
    
    echo %TIMESTAMP% [ERROR] Editor failed to launch or crashed.
    echo %TIMESTAMP% Exit Code: %ERRORLEVEL%
    echo %TIMESTAMP% Details have been logged to: %LOG_FILE%
    goto :fail
)

echo %TIMESTAMP% Closing...
pause
exit /b 0

:fail
pause
exit /b 1
