![image-device](https://raw.githubusercontent.com/michalmonday/Hardware-keyloggers/master/images/pic.jpg)

![image-device-2](https://raw.githubusercontent.com/michalmonday/Hardware-keyloggers/master/images/pic_2.jpg)

![image-wiring](https://i.imgur.com/Zm0vHTz.png)

# Overview  
Comparing to the original [KEYVILBOARD](https://github.com/RedBulletTooling/KEYVILBOARD) it had the following differences:  
-Arduino Pro Micro 5V used instead of Teensy  
-passing of raw HID reports instead of reinterpretting the keys before sending them to PC which solves multiple issues like:  
- key combinations not working  
- unability to hold keys  

-NPN transistor added to allow automatic resetting of the Sim800L if it misbehaves  
-debugging and some error handling  
-different wiring  

**The KEYVILBOARD project was updated since this repository was created, it has fixed the key-combinations/key-holding issues and implemented some other cool features.**
 
# Parts used:  
-Arduino Pro Micro 5V 16Mhz  
-Sim800L (the one with blue board, not red one)  
-[USB Host Mini V2](http://www.hobbytronics.co.uk/usb-host-mini) board by Hobbytronics  
> Make sure to pick "USB Keyboard" before purchase  

-BD137 NPN transistor  
-resistors (1K, 2K, 4.7K)  
-solid core wires (including some thin ones like [these](https://www.ebay.co.uk/itm/30AWG-Insulated-Silver-Plated-Single-Core-Copper-PCB-0-25mm-Kynar-Wrapping-Wire/263504549866))  
-soldering equipment  

# Setup process  
Wiring instructions are provided in the comments of the "keylogger_sim800l.ino" sketch.
Once the correct wiring is used baud rates of each module will have to be set.  
 
To set the baud rate of Sim800L:  
-upload "tools/setting_Sim800L_baud_rate.ino" code to Arduino Pro Micro using Arduino IDE (pick "Arduino Leonardo" board setting)  
-once uploaded open serial monitor, wait 1 second and close it (baud rate should be set automatically)  

To set the baud rate of USB Host Mini V2:  
-upload "tools/setting_USB_Host_Mini_V2_baud_rate.ino" code to Arduino Pro Micro  
> It will allow to communicate with USB Host Mini V2 board  

-open serial monitor, set "Both NL & CR" and "9600" in the bottom-right corner of it  
-send "HELP" command through Serial Monitor, the response should appear  
-see the digit corresponding to 115200 baud rate and use it to send "BAUD digit" command  
> The baud rate should be set to a different one. Further communication should not be possible unless you reupload "Serial1_commuication.ino" with 115200 BAUD_RATE setting instead of 9600 which it has right now.  

Baud rates of both modules should be set correctly now and the main code (keylogger_sim800l.ino) should be uploaded.


# Credits  
Thanks to [Helmmen](https://github.com/RedBulletTooling/KEYVILBOARD) for publishing the working prototype of keylogger that would use SMS communication and the cool Hobbytronics USB host board. That was the origin and the base for this project.  
