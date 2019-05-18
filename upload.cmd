@echo off

REM so ist upload per USB
REM C:\Users\Stefan.DESKTOP-GFTTGH4\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.5.1/tools/upload.py --chip esp8266 --port COM7 --baud 115200 --trace version --end --chip esp8266 --port COM7 --baud 115200 --trace write_flash 0x0 C:\Users\STEFAN~1.DES\AppData\Local\Temp\arduino_build_269258/LichtWecker.ino.bin --end 


REM Cmd-File fuer OTA

REM Lese Passwort
set /p pw=OTA Passwort: 
REM python.exe C:\Users\Stefan.DESKTOP-GFTTGH4\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.4.2/tools/espota.py -i 192.168.2.138 -p 8266 --auth=%pw% -f C:\Users\STEFAN~1.DES\AppData\Local\Temp\arduino_build_425799/LichtWecker.ino.bin
REM python.exe C:\Users\Stefan.DESKTOP-GFTTGH4\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.4.2/tools/espota.py -i 192.168.2.150 -p 8266 --auth=%pw% -f C:\Users\STEFAN~1.DES\AppData\Local\Temp\arduino_build_425799/LichtWecker.ino.bin
@echo on
python.exe C:\Users\Stefan.DESKTOP-GFTTGH4\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.5.1\tools\espota.py -i 192.168.2.135	 -p 8266 --auth=%pw% -f C:\Users\STEFAN~1.DES\AppData\Local\Temp\arduino_build_269258/LichtWecker.ino.bin

