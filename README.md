DigitalDice1d6
==============

Digital dice based on attiny13 microcontroller. Generates random value from 1 to 6 when you press a button.

All you need to build device is atTiny13 microcontroller, 7 LEDs, a button and a battery.

Pin configuration:
pin4: ground
pin8: VCC
pin1: RESET (doesn't used)
pin5: PB0 input for next value button
pin2,3,6,7: PB1-PB4 outputs for connecting LEDS.

For more information please look at layout (file DDice.lay, can be opened in Sprint Layout).
In my layout I added power switcher which is optional element.