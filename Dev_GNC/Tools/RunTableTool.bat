@echo off
chcp 65001 >NUL 2>NUL
cd /d "%~dp0"

echo ================================================
echo  CardGame Table Tool (UE5)
echo  Excel - Protobuf - DataTable JSON
echo ================================================
echo.

REM Check Python
where python >NUL 2>&1
if errorlevel 1 (
    echo [ERROR] python is not in PATH.
    goto :END
)

REM Check packages
python -c "import openpyxl, jinja2, google.protobuf, yaml" >NUL 2>&1
if errorlevel 1 (
    echo [INFO] Installing required packages...
    pip install -r requirements.txt
    echo.
)

REM Run pipeline
echo Pipeline start...
echo.
python main.py %*

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
