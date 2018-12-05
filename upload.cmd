REM Cmd-File fuer OTA

REM Lese Passwort
set /p pw=OTA Passwort: 

echo python.exe C:\Users\Stefan.DESKTOP-GFTTGH4\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.4.2/tools/espota.py -i 192.168.2.150 -p 8266 --auth=%pw% -f C:\Users\STEFAN~1.DES\AppData\Local\Temp\arduino_build_678010/LichtWecker.ino.bin
