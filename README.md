# macro-keyboard-atmega328p
4x3 keyboard with programmable macros using atmega328p and microchip studio.<br>

This program is meant to run on an ATMEGA328P for taking input from a custom 4x3 keyboard where each key is a programmable macro with 20 actions.<br>
Each key also has an RGB LED. The macro and colour of each key can be customised using the custom PC software (not available to use).<br>
The keyboard also contains a small LCD display which displays the name of the macro being executed.<br>
It also contains brightness control options (as well as light sensor) where you can adjust the brightness of the LCD display and RGB LEDs.<br>

Used ws2812 library from cpldcpu for the RGB LEDs, and used st7735 library from matiasus for the LCD display.
