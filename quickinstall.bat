@echo off
:START
adb wait-for-device root
adb remount
adb push u3gmonitor /system/bin/
IF ERRORLEVEL=1 GOTO START
adb shell chown 0.2000 /system/bin/u3gmonitor;chmod 755 /system/bin/u3gmonitor;stop u3gmonitor;start u3gmonitor
pause
