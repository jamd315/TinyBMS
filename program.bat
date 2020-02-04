@echo off
"C:\Program Files (x86)\BitSuite Productions\BitBurner AVR Programmer\avrdude" -p atmega328p -b 115200 -c arduino -P COM3 -U "flash:w:C:\Users\Aidan\Documents\Arduino\ArduinoISP\build\ArduinoISP.ino.hex:a"
if errorlevel 1 (
    echo Failed to program ArduinoISP sketch
    exit \b 1
)
"C:\Program Files (x86)\BitSuite Productions\BitBurner AVR Programmer\avrdude" -p t85 -b 19200 -c stk500v1 -P COM3 -U "flash:w:C:\Users\Aidan\Documents\Atmel Studio\7.0\TinyBMS\TinyBMS\Debug\TinyBMS.hex:a"
if errorlevel 1 (
    echo Failed to program ATTINY
    exit \b 1
)
"C:\Program Files (x86)\Arduino\arduino" --upload --port COM3 -v C:\Users\Aidan\Documents\Arduino\I2C_diag\I2C_diag.ino
if errorlevel 1 (
    echo Failed to program I2C_diag sketch
    exit \b 1
)