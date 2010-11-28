JTMOD patches to tjtag by Jeremy Trimble
----------------------------------------

A few quick notes on how to use the JTMOD version of tjtag:

- The JTMOD patches only work on linux right now, but you could probably apply the same idea to Windows (I'm just not much of a Windows programmer at the moment).

- Build the "tjtag" executable:
    $ make tjtag

- You'll want to program your Arduino board with the "arduiggler.pde" sketch in the arduino/ directory.

- Follow the schematic in the arduino directory when connecting your Arduino board to the JTAG port.

- Make sure that your Arduino board is connected to the computer before trying to run tjtag.
    - If your board is connected to a serial port other than "/dev/ttyUSB0", you can set the "JTMOD_SER_PORT" environment variable to the appropriate serial port device:
        $ set JTMOD_SER_PORT=/dev/ttyS0
        - Otherwise, you'll get an error message that looks like "Failed to open '/dev/ttyUSB0': No such file or directory".

- Run tjtag with your favorite set of switches.  Be sure to include the "/wiggler" switch.


Note:  JTAG communication with the WRT54G through the arduiggler seems to be VERY SLOW.  I presume this is due to the fact that data is being transferred to/from the arduiggler essentially one bit at a time.  There are probably ways to speed this up, but since I was only trying to do a flash erase on my router, I didn't need the speed.



- The "test" executable can be built by invoking:
    $ make test
    - I used the "test" executable to make sure that my circuitry was working properly before hooking up my arduiggler to the JTAG port on my WRT54G.  The "test" executable basically just exercises the code in jt_mod.c.  I connected the arduino pins to LEDs to ensure that the pins were putting out the right bit patterns.
      

