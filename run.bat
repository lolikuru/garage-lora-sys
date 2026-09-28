@echo off
echo Compiling project...
arduino-cli compile --fqbn esp32:esp32:esp32s2 --output-dir ./build_output .
if %errorlevel% neq 0 (
    echo Compilation failed.
    pause
    exit /b %errorlevel%
)
echo.
echo Uploading to board...
echo Note: Change COM14 to your actual port if different.
arduino-cli upload -p COM14 --fqbn esp32:esp32:esp32s2:CDCOnBoot=cdc .
if %errorlevel% neq 0 (
    echo Upload failed.
    pause
    exit /b %errorlevel%
)
echo Done!
pause
