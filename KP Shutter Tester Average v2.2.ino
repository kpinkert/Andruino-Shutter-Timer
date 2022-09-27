/*

    I want to give credit to the YouTube fellow CAMERADACTYL Cameras who wrote the initial code that I started with.
    
    Everyone has their own programming style. I spent almost 40 years coding with most of it dealing with interfacing 
    with hardware. ISR routines have some coding rules that help keep multi-threading safer. All of the critical timing
    components to the formula for shutter speed calculations are done in the ISR here. Mainly for reliability and
    also to keep the Andurino code base for adding in extra overhead, if possible.

    One other feature I added was the ability to average a specific number of consecutive shutter clicks. The program
    defaults to 5 clicks. But that can be changed to go from 1 (no averaging), to however many you feel like doing.
    To change the number of snaps used for averaging, just change the value in the #define AVERAGES

    Another feature that I added was the ability to use a "UCTRONICS 0.96 Inch OLED Module 12864". This allows the
    Adruino setup to be used with without a serial connection for logging data. Perhaps running off of a 9 volt battery.
    This UCTRONICS feature can be used at the same time as the serial interface. And it can also be turned off
    completely to keep from getting compile errors. Just comment out the #define UCTRONICS line of code at the top
    of this file.

    On my Laser sensor module, the output is HIGH when the the laser is off. And it is LOW when it senses the laser.
    The original code was just the opposite. So if your implementation of this code acts weird, just swap the if/else
    code in the ISR.

    Kevin Pinkerton 
    */

#define UCTRONICS  // uncomment this line if you want to output to the UCTRONICS 0.96 Inch OLED Module 12864

#ifdef UCTRONICS
// Import and defines required libraries for the UCTRONICS 0.96 Inch OLED Module 12864

#include <ArducamSSD1306.h>  // Modification of Adafruit_SSD1306 for ESP8266 compatibility
#include <Adafruit_GFX.h>    // Needs a little change in original Adafruit library (See README.txt file)

/*
    HardWare I2C pins
    A4   SDA
    A5   SCL
    */

// Pin definitions
#define OLED_RESET 16  // Pin 15 -RESET digital signal
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH 16
ArducamSSD1306 display(OLED_RESET);  // FOR I2C
#endif                               // ***** UCTRONICS

static unsigned long ulStartTime = 0;  // this is the time in microseconds that the shutter opens (the arduino runs a microsecond clock in the background always - it is reasonably accurate for this purpose)
static unsigned long ulStopTime = 0;   // this is the time in microseconds that the shutter closes
volatile byte bTrigger = 0;
unsigned long ulTimeCount = 0;
#define AVERAGES 1
#define SENSOR_DIO_PIN 2

void displayShutterSpeed(unsigned long);

int main(void) {
  unsigned long ulElapsedTime;
  unsigned long ulElapsedTimeSum = 0;
  unsigned long ulElapsedTimeAvg;

  init();
#if defined(USBCON)
  USBDevice.attach();
#endif

  Serial.begin(115200);  //opens a serial connection.

  introduction();  // for the user

  // this interrupt is required for the most accurate way to get the shutter speed
  attachInterrupt(digitalPinToInterrupt(SENSOR_DIO_PIN), changeISR, CHANGE);  //run the function ChangeISR, every time the voltage on pin 2 changes.

  // once we get into this loop, we stay there. The ISR can interrupt this loop

  while (true) {

    if (bTrigger) {

      ulElapsedTime = ulStopTime - ulStartTime;
      ulElapsedTimeSum += ulElapsedTime;  // get a 5 count average
      ulTimeCount++;

      bTrigger = 0;  // once we use the already collected start and end time, we can allow the ISR to do it again

      // log the current snapshot count

      Serial.print(ulTimeCount);
      Serial.print(" ");

#ifdef UCTRONICS
      display.clearDisplay();
      display.setCursor(3, 25);
      display.print(" Snap # ");
      display.print(ulTimeCount);
      display.display();  // force output to appear on the display
#endif                    // UTRONICS

      // are we ready to get the average for this round of snaps?

      if (ulTimeCount == AVERAGES) {  // yes, time to process the average
        ulElapsedTimeAvg = ulElapsedTimeSum / ulTimeCount;
        displayShutterSpeed(ulElapsedTimeAvg);
        ulTimeCount = 0;
        ulElapsedTimeSum = 0;  // reset the summation
      }
    }
  }

  return 0;
}

void changeISR() {  //this is the interrupt function, which is called everytime the voltage on pin 2 changes

  // this is the meat of the software. This ISR is called within a few microseconds (I think) after the sensor
  // toggles states

  // Sensor state LOW : Laser goes on and sensor changes states to LOW.
  // Sensor state HIGH: Laser goes off and sensor goes back to HIGH, it's idle state.

  // get the state of the laser sensor. This the resulting state from the CHANGE ISR interrupt.
  // In other words: the timeline for the sensor change is : Start to go to a new state, fire off the interrupt,
  // and by the time the ISR is given control, the sensor state has settled into the new state.

  // if bTrigger is set, then we are getting a laser response without finishing the last one. Probably caused by multi-tasking (ISR vs loop()). So skip it

  unsigned long ulMicroseconds = micros();  // get the running microsecond reading here and NO where else. Important!

  if (bTrigger != 1) {  // if bTrigger is not set then...

    // Ok, safe to process this interrupt

    if (digitalRead(SENSOR_DIO_PIN) == HIGH) {   // sensor says laser is now ON
      ulStopTime = ulMicroseconds;  // save the stop time for the main loop
      bTrigger = 1;                 // set the trigger and do not respond to anymore ISR calls until the trigger has been cleared.

    } else {                         // sensor says laser is now OFF
      ulStartTime = ulMicroseconds;  // save the start time for the main loop
    }
  }
}

void displayShutterSpeed(unsigned long ulElapsedTimeAvg) {

  float fFractionOfaSecond = 1000000. / ulElapsedTimeAvg;
  float fFractionOfaSecondRounded = roundf( fFractionOfaSecond);

#ifdef UCTRONICS
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(" -Average-");
  display.setCursor(0, 25);
#endif  // UTRONICS

  if (ulElapsedTimeAvg < 1000000) {  // the shutter speed is less than a second
    Serial.print("microseconds ");
    Serial.print( ulElapsedTimeAvg);
    Serial.print(" Shutter Speed: 1/");
    Serial.print(fFractionOfaSecondRounded, 0);
    Serial.println(" second");

#ifdef UCTRONICS
    display.print("  1/");
    display.print(fFractionOfaSecondRounded, 0);
    display.setCursor(0, 48);
    display.print("  second ");
#endif  // UTRONICS

  } else {  // the shutter speed is greater than a second
    fFractionOfaSecond = ulElapsedTimeAvg / 1000000.;
    Serial.print("Shutter Speed: ");
    Serial.print(fFractionOfaSecondRounded, 1);
    Serial.println(" second(s)");

#ifdef UCTRONICS
    display.print("    ");
    display.print(fFractionOfaSecondRounded, 1);
    display.setCursor(0, 48);
    display.print(" second(s)");
#endif  // UTRONICS
  }
  // finish up
#ifdef UCTRONICS
  display.display();  // force output to appear on the display
#endif                // UTRONICS
}

void introduction() {
  // intro...

  Serial.println();
  Serial.println();
  Serial.print(" ***** Starting shutter speed testing, averaging on ");
  Serial.print(AVERAGES);
  Serial.println(" *****");
  Serial.println();
  Serial.println();

#ifdef UCTRONICS
  // SSD1306 Init
  display.begin();  // Switch OLED
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("This will average ");
  display.print(AVERAGES);
  display.println(" snaps");
  display.display();
#endif  // UTRONICS
}