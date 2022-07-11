# 8080 Emulator

A 8080 microprocessor emulator that can run classic space invaders and some COM programs, I mainly chose this emulator to learn more about emulator development and low-level programming, all 255 opcodes have been implemented and tested, also some dedicated shift hardware has been emulated as space invaders use this for moving pixels on the screen and thus adds a hardware shift register to help with the math.
Also as the 8080 is a microprocessor and not a complete machine like a Gameboy, it does not have a standard input, only some ports that can receive and send signals.
So for sending keyboard signals and playing audio I used SDL as these are not really part of emulating an 8080 but emulating a sound card used in old arcade machines might be a fun project for the future.

 
<br>

## How to install:
will be added soon

## Controlls (Space invaders):


## Sources:

http://www.emulator101.com/reference/8080-by-opcode.html<br>
https://computerarcheology.com/Arcade/SpaceInvaders/Code.html<br>
https://computerarcheology.com/Arcade/SpaceInvaders/<br>
https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html<br>
8080-Programmers-Manual (see resources folder)<br>
https://en.wikipedia.org/wiki/Intel_8080<br>
