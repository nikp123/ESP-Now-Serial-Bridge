# ESP-Now-Serial-Bridge

### What's this?

This is a simple UART bridge which utilizes two ESP8266-based (in my case, ESP-01) boards. The wireless communication is Espressif's
own ESP-Now propriatery solution which boasts way shorter latencies and larger signal ranges, which is exactly what's desired
for this type of application. Bonus point is that this wireless protocol is entirely peer to peer so it does not depend on local Wi-Fi
or 2.4 GHz load.


### Applications?

I am currently using this as a Octoprint serial bridge between my server and the home server I have set up in my room.

As someone who does not like very loud 3D printer noises this is a desired feature. 

It's potentially any other DIY project which uses serial for communication but would like to have extremely small communication latencies.


### Security?

NONE, **PLEASE DO NOT USE THIS FOR ANY SENSITIVE INFO**. THE WIRELESS DATA IS NOT ENCRYPTED AT ALL.

This was done on purpose to increase the throughput of this solution as speed and latency is what's most important here.


### How do I use this?

 - Get two ESP8266 based boards, in my case two ESP-01s and it's USB serial programmer.

 - Install Arduino IDE 1.6 (haven't tried on 2.0 but please report if successful)

 - From within Arduino IDE install the [ESP8266 core](https://github.com/esp8266/Arduino). This is typically done by pasting the json url into the Board manager.

 - Open this project in Arduino IDE and change values as desired.

   - The thing to look out for is whether or not you want packet resending. For 3D printing applications I honestly found it works better if you disable resending
   as Octoprint will do that itself.

 - Hit compile and upload

   - During the upload you can see what MAC address your ESP8266-based board has. This is important, put that into the code and plug in the other board.

 - Set the BOARD1 variable and compile and upload again. Grab the MAC address of the other board.

 - Unplug the 2nd board and plug in the first one, remembering to unset BOARD1, hit compile and upload once more and you should be done.

 - Wire your ESP8266 board accordingly (use Google) and try it out. It should be a pretty responsive UART solution.


### Derivative work?

Based off of [Yuri's own ESP serial bridge](https://github.com/yuri-rage/ESP-Now-Serial-Bridge), 
this is the same thing but HEAVILY modified to suit some needs.

Changes from the original:

 - Added packet resending (configurable)

 - Removed ESP reset on connection failure as it would result in junk being outputted in the serial, which you definitely DO NOT WANT.

 - Using ESP8266 (ESP-01) as the target device instead of the ESP32s because the ESP-01 board is MUUUUUUUUUUCH cheaper in my country.


### License

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
