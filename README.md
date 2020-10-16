[![noswpatv3](http://zoobab.wdfiles.com/local--files/start/noupcv3.jpg)](https://ffii.org/donate-now-to-save-europe-from-software-patents-says-ffii/)
Source code and documentation for a version of tjtag that connects to a router's JTAG port through an Arduino board.

Original source code was posted here:

http://www.dd-wrt.com/phpBB2/viewtopic.php?p=435578&sid=0c7c7b114dc7239be2620514cecbe2d8



### Use case: Erasing nvram on Linksys WAP54G with dd-wrt running

I had fucked up the dd-wrt installation, forgetting to set nvram version to three, that caused the reset button not to work. That combined to bad configuration on WAP port i had the box unreachable. The only way to reset the box to factory defaults was to use jtag connection.

* Arduino: duemilanove/atmega 328  - worked out of the box, except that i had to retry the launch few times before "reseting arduino" went ok.
* OS: Ubunutu 14.04.3 / 3.15.0-031500rc2
* Target: WAP54G 3.1, running dd-wrt v24 - image on dd-wrt wiki page is from older version, same pins worked though. 
* Command: ```./tjtag -erase:nvram /wiggler /noreset``` 
* Notes: It did not work without ``` /wiggle /noreset ```. Chip identification worked fine.

### Links

* See a similar project using UrJTAG (available for Linux, OSX, or Windows) and an Arduino: https://github.com/zoobab/urjtag_arduiggler
* See also JTAGduino: https://github.com/balau/JTAGduino
* See also FUJI: http://hsbp.org/fuji
