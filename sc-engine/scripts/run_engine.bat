@echo off
set SCRIPT_DIR=%~dp0
set ENGINE_ROOT=%SCRIPT_DIR%..
for %%I in ("%ENGINE_ROOT%\..") do set REPO_ROOT=%%~fI
set MODDAW_HOST_PORT=57130
set MODDAW_ENGINE_PORT=57131

:parse_args
if "%~1"=="" goto args_done
if "%~1"=="--host-port" (
  set MODDAW_HOST_PORT=%~2
  shift
  shift
  goto parse_args
)
if "%~1"=="--engine-port" (
  set MODDAW_ENGINE_PORT=%~2
  shift
  shift
  goto parse_args
)
shift
goto parse_args

:args_done

set SCLANG_PATH=

if not "%SCLANG_BIN%"=="" if exist "%SCLANG_BIN%" set SCLANG_PATH=%SCLANG_BIN%
if "%SCLANG_PATH%"=="" if exist "%REPO_ROOT%\third_party\supercollider\build\lang\sclang.exe" set SCLANG_PATH=%REPO_ROOT%\third_party\supercollider\build\lang\sclang.exe
if "%SCLANG_PATH%"=="" if exist "%REPO_ROOT%\third_party\supercollider\build\install\bin\sclang.exe" set SCLANG_PATH=%REPO_ROOT%\third_party\supercollider\build\install\bin\sclang.exe
if "%SCLANG_PATH%"=="" if exist "%REPO_ROOT%\third_party\supercollider\cmake-build-debug\lang\sclang.exe" set SCLANG_PATH=%REPO_ROOT%\third_party\supercollider\cmake-build-debug\lang\sclang.exe
if "%SCLANG_PATH%"=="" if exist "%ProgramFiles%\SuperCollider\sclang.exe" set SCLANG_PATH=%ProgramFiles%\SuperCollider\sclang.exe
if "%SCLANG_PATH%"=="" for %%I in (sclang.exe) do set SCLANG_PATH=%%~$PATH:I

if "%SCLANG_PATH%"=="" (
  echo Could not locate sclang. 1>&2
  echo Checked SCLANG_BIN, local SuperCollider builds, Program Files, and PATH. 1>&2
  exit /b 1
)

"%SCLANG_PATH%" "%ENGINE_ROOT%\boot.scd"
