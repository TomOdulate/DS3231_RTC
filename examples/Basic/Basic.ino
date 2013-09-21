/* RTC_DS3231 example
 
    Demonstrates basic usage of the RTC_DS3231 library.

 
   Created  by Tom The Tinkerer http://www.tomthetinkerer.com
 */

#include <RTC_DS3231.h>
#include <Wire.h>

RTC_DS3231 rtc;


void setup()
{
    Serial.begin(4800);    	
    rtc.init();                                      // initialise the RTC    
    rtc.SetDateTime(DateTime(__DATE__, __TIME__));   // Sets the RTC to the date & time this sketch was compiled
}

void loop()
{
  DateTime dt = rtc.now();
  char c[20];  
  Serial.println( dt.toString(c,20) );
  delay(1000);
}
