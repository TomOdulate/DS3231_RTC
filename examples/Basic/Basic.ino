/* RTC_DS3231 example
 
    Demonstrates basic usage of the RTC_DS3231 library.
    Created  by Tom The Tinkerer http://www.tomthetinkerer.com
 			
    Board               I2C / TWI pins
    Uno, Ethernet       A4 (SDA), A5 (SCL)
    Mega2560            20 (SDA), 21 (SCL)
    Leonardo             2 (SDA),  3 (SCL)
    Due                 20 (SDA), 21 (SCL), SDA1, SCL1
 */

#include <RTC_DS3231.h>
#include <Wire.h>

RTC_DS3231 rtc;


void setup()
{
    Serial.begin(4800);    	 
    rtc.SetDateTime(DateTime(__DATE__, __TIME__));   // Sets the RTC to the date & time this sketch was compiled
}

void loop()
{
  DateTime dt = rtc.now();
  char c[20];  
  Serial.println( dt.toString(c,20) );
  delay(1000);
}
