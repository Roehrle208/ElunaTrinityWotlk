@echo off
setlocal enabledelayedexpansion

:: Basispfade setzen
set SCRIPT_DIR=%~dp0
set PATCH_DIR=%SCRIPT_DIR%patches
set CORE_PATCH=%PATCH_DIR%\core.patch
set BOT_PATCH=%PATCH_DIR%\bot-engine.patch
set LOG_FILE=.\patch.log

:: Standardwerte
set DRYRUN=false
if "%2"=="--dry-run" (
    set DRYRUN=true
)

if "%1"=="make" (
    echo [MAKE] Erzeuge core.patch ...
    git diff master...feature/core-adjustments > %CORE_PATCH%

    echo [MAKE] Erzeuge bot-engine.patch ...
    git diff feature/core-adjustments...feature/bot-engine > %BOT_PATCH%

    echo [MAKE] Fertig. Patches gespeichert unter %PATCH_DIR%
    exit /b
)

if "%1"=="apply" (
    echo [APPLY] wende core.patch an ...
    if "%DRYRUN%"=="true" (
        git apply --check %CORE_PATCH% > %LOG_FILE% 2>&1
    ) else (
        git apply %CORE_PATCH% > %LOG_FILE% 2>&1
    )

    echo [APPLY] wende bot-engine.patch an ...
    if "%DRYRUN%"=="true" (
        git apply --check %BOT_PATCH% >> %LOG_FILE% 2>&1
    ) else (
        git apply %BOT_PATCH% >> %LOG_FILE% 2>&1
    )

    echo [APPLY] Fertig. Log siehe %LOG_FILE%
    exit /b
)

if "%1"=="clean" (
    echo [CLEAN] Entferne core.patch ...
    git apply -R %CORE_PATCH%

    echo [CLEAN] Entferne bot-engine.patch ...
    git apply -R %BOT_PATCH%

    echo [CLEAN] Patches rückgängig gemacht.
    exit /b
)

echo [INFO] Verfügbare Befehle:
echo   patch-helper.bat make
echo   patch-helper.bat apply
echo   patch-helper.bat apply --dry-run
echo   patch-helper.bat clean
exit /b