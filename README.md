# Andruino-Shutter-Timer
A camera shutter timer using Arduino UNO, laser and laser sensor. Optionally with a UCTronics OLED display.
I purchased mine at Amazon... https://www.amazon.com/UCTRONICS-SSD1306-Self-Luminous-Display-Raspberry/dp/B072Q2X2LL

I want to give credit to the YouTube fellow CAMERADACTYL Cameras who wrote the initial code that I started with.
    
Everyone has their own programming style. I spent almost 40 years coding with most of it dealing with interfacing 
with hardware. ISR routines have some coding rules that help keep multi-threading safer. All of the critical timing
components to the formula for shutter speed calculations are done in the ISR here. Mainly for reliability and
also to keep the Andurino code base for adding in extra overhead, if possible.

One other feature I added was the ability to average a specific number of consecutive shutter clicks. The program
defaults to 5 clicks. But that can be changed to go from 1 (no averaging), to however many you feel like doing.
To change the number of snaps used for averaging, just change the value in the #define AVERAGES

Another feature that I added was the ability to use a "UCTRONICS 0.96 Inch OLED Module 12864". This allows the
Adruino setup to be used with or without a serial connection for logging data. Perhaps running off of a 9 volt battery.
This UCTRONICS feature can be used at the same time as the serial interface. And it can also be turned off
completely to keep from getting compile errors. Just comment out the #define UCTRONICS line of code at the top
of this file.

 On my Laser sensor module, the output is HIGH when the no laser is sensed (laser is off). And it is LOW when it senses
 the laser is on and hitting the sensor. The original code was just the opposite. So if your implementation of this code
 acts weird, just swap the if/else code in the ISR.
