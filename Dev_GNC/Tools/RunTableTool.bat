@echo off
chcp 65001 >NUL 2>NUL
cd /d "%~dp0"

echo ================================================
echo  CardGame Table Tool (UE5)
echo  Excel - Protobuf - DataTable JSON
echo ================================================
echo.

REM Check Python (py launcher preferred — avoids Microsoft Store stub)
set "PY_CMD=py -3"
%PY_CMD% --version >NUL 2>&1
if errorlevel 1 (
    set "PY_CMD=python"
    where python >NUL 2>&1
    if errorlevel 1 (
        echo [ERROR] Python is not installed or not in PATH.
        goto :END
    )
)

REM Check packages
%PY_CMD% -c "import openpyxl, jinja2, google.protobuf, yaml" >NUL 2>&1
if errorlevel 1 (
    echo [INFO] Installing required packages...
    %PY_CMD% -m pip install -r requirements.txt
    echo.
)

REM Run pipeline
echo Pipeline start...
echo.
%PY_CMD% main.py %*

if errorlevel 1 (
    echo.
    echo [ERROR] Pipeline failed.
    echo         Run "python main.py --debug" for details.
    goto :END
)

echo.
echo ================================================
echo  Done! Import output/json/ JSON files
echo  as DataTable in UE5 Editor.
echo ================================================

:END
echo.
pause
