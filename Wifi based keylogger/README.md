![image-device](https://i.imgur.com/1yqzook.png)

# Overview  
It has the same functionality as [wifi_keylogger](https://github.com/spacehuhn/wifi_keylogger) made by spacehuhn but instead of reinterpretting keys it is simply passing the raw HID reports from keyboard to PC so the keyboard types exactly the same keys as if there was no keylogger.  


# Parts used:  
-CJMCU Beetle  
> SS Micro could be used instead. It has 3.3V regulator so there would be no need to use zener diodes.  

-Esp12-E module  
-[Ft232rl](https://www.aliexpress.com/item/Free-shipping-1pcs-FT232RL-FT232-FTDI-USB-3-3V-5-5V-to-TTL-Serial-Adapter-Module/32460118879.html) serial adapter 
> This adapter will be used only to flash Esp12-E module (you could use alternative ways to do that, e.g. NodeMCU does not require any adapter because it has in-built one)  

-[USB Host Mini V2](http://www.hobbytronics.co.uk/usb-host-mini) board by Hobbytronics  
> Make sure to pick "USB Keyboard" before purchase  

-2x 3.6V zener diodes  
-resistors (2K Ohm, 3x 4.7K Ohm)  
-solid core wires (including some thin ones like [these](https://www.ebay.co.uk/itm/30AWG-Insulated-Silver-Plated-Single-Core-Copper-PCB-0-25mm-Kynar-Wrapping-Wire/263504549866))  
-soldering equipment  
-clips/hooks like [these](https://www.aliexpress.com/item/10pcs-lot-20cm-Test-probe-jumper-wire-cable-with-Hook-pulg-Clip-SMD-IC-Logic-Analyzer/32862677867.html) (to grab the pins without soldering)  
-[power breakout board](https://www.ebay.co.uk/itm/Breadboard-Power-Supply-Module-3-3V-5V-USB-for-Arduino-Raspberry-Pi-Board-MB102/131668810841)  
> Needed to power Esp12-E module with 3.3V during flashing with Ft232rl which by itself does not provide enough power (according to online tutorials about it).

> I strongly recommend to buy [NodeMCU](https://www.aliexpress.com/item/NodeMcu-Lua-WIFI-Internet-of-Things-development-board-based-ESP8266-module/32448461056.html) and possibly additional SS Micro or CJMCU Beetle board with pre-soldered pins. NodeMCU can be easily reprogrammed and reconnected which also applies to SS-Micro/CJMCU-Beetle. It is very difficult and frustrating to reflash or apply corrections on the soldered setup (especially with the plain Esp12-E where GPIO 0 pin has to be connected to ground for uploading).  


# Setup process

1. Connecting + initializing USB Host Mini V2 board.  
-connect the board to CJMCU Beetle using the following connections:   

![image-wiring](https://i.imgur.com/3HBcuRI.png)

-connect CJMCU Beetle to PC  
-upload the "tools/Serial1_communication.ino" code to CJMCU Beetle using Arduino IDE (pick "Arduino Leonardo" board setting)  
> It will allow to communicate with USB Host Mini V2 board, it will be used to set its baud rate to a higher one (default is 9600 and it will be set to 115200).  

-open serial monitor, set "Both NL & CR" and "9600" in the bottom-right corner of it  
-send "HELP" command through Serial Monitor, the response should appear  
-see the digit corresponding to 115200 baud rate and use it to send "BAUD digit" command  
> The baud rate should be set to a different one. Further communication should not be possible unless you reupload "Serial1_commuication.ino" with 115200 BAUD_RATE setting instead of 9600 which it has right now.  

2. Set up Esp12-E module for flashing with Ft232rl.  
The following connections were used for the Esp12-E module:  
![image-wiring](https://i.imgur.com/fgeTDur.jpg)
> I didn't connect Ft232rl to ground and the code still was uploaded well.  

3. Upload "Keylogger_wifi_hobbytronics_ESP" code to Esp12-E using Arduino IDE and Ft232rl.  
> I used "NodeMCU V1.0 (Esp12-E module)" board setting with 3M SPIFFS.   

4. Reconnect GPIO 0 pin from GND to VCC of Esp12-E for "Running" (as suggested on the wiring image above).  

5. Connect Esp12-E to CJMCU Beetle in the following way:  

| Esp12-E | CJMCU Beetle |
| --- | --- |
| VCC | <-zener_diode<-zener_diode<-5V |
| GND | GND |
| RX | SCK |
| TX | MO |

> Zener diodes should be pointing towards the Esp12-E (with their black endings).  

6. Connect CJMCU Beetle to PC and flash it with the "Keylogger_wifi_hobbytronics_Arduino" code.

7. Make the general test of the whole device that should be working at this point:  
-disconnect CJMCU Beetle  
-connect some keyboard to the USB Host Mini V2  
-connect CJMCU Beetle back to PC  
-type few characters using the keyboard and see whether they are visible on screen  
-try to find and connect the newly created wifi access point ("BTHub3-537", password: 37373737)  
-open browser and go to 192.168.4.1/ and see whether the typed characters are displayed  



# Credits  
Thanks to [spacehuhn](https://github.com/spacehuhn/wifi_keylogger) for sharing the proof of concept of his wifi_keylogger under MIT license. This implementation builds on the his idea and uses the code for Esp8266 written by him.
