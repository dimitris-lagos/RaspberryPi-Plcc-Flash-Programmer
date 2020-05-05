# RaspberryPi-Plcc-Flash-Programmer

Ported from [Arduino-Plcc-Flash-Programmer](https://github.com/dimitris-lagos/Arduino-Plcc-Flash-Programmer#arduino-plcc-flash-programmer).  
Simple C program that implements a Plcc Flash Programmer for flashroms that use the LPC Protocol.
Should work with any Raspbian Distro. I've tested it with Retropie on PI 2 MODEL B.
##### Dependencies  
-Uses 6 GPIO pins from the Raspberry Pi pin header (GPIO16, GPIO22, GPIO23, GPIO24, GPIO25, GPIO27 )  
-Git-Core should be preinstalled  for WiringPi library clone.  
-[WiringPi library](https://github.com/WiringPi/WiringPi) should be preinstalled in order for the program to compile.  

Clone the repository cd into the repo folder and then compile with:  
```sh
$ gcc flasher.c -l wiringPi -o flasher
```

Then to run:
```sh
$ ./flasher
```
The output shows the usage help and parameters:
```sh
pi@retropie:~/flasher $ ./flasher
LPC PLCC Flash Programmer
Usage: ./flasher -[switch] [parameter]
Switch:
 -w   [filename]      Write to flash (flash should be erased)
 -es                  Erase flash sector by sector
 -eb                  Erase flash block by block
 -sec [Sector Number] Erase specific sector
 -r   [filename]      Save the flash to file
 -id                  Read the Chip ID
 -jid                 Read the JDEC Chip ID
 -fs                  Read the flash to screen
 ```
 
 Examples:
 -To read the flash into a file (backup.bin).
 ```sh
 $ ./flasher -r backup.bin
 ```
 
-To program the flash with bin file (bios.bin).  
 ```sh
 $ ./flasher -w bios.bin
 ```
 
 ~Work In Progress!~
