// RTC_DS3231.h
/******************************************************************************
	NAME   : RTC_DS3231.cpp
	Version: 0.2
	Created: 06/09/2013
	Author : TomTheTinkerer.com
	NOTES  : RTC based on the DS3231 RTC chip. Please ensure that you refer to
			datasheet, http://datasheets.maximintegrated.com/en/ds/DS3231.pdf
			Communication is via I2C using wire.h.  Credit where its due! The 
			DateTime class adapted from RTCLib from jeelabs.
			
			Board			I2C / TWI pins
			Uno, Ethernet	A4 (SDA), 	A5 (SCL)
			Mega2560		20 (SDA), 	21 (SCL)
			Leonardo		 2 (SDA), 	 3 (SCL)
			Due				20 (SDA), 	21 (SCL), SDA1, SCL1

******************************************************************************/

#ifndef _RTC_DS3231_h
#define _RTC_DS3231_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
	#define SEND(x) write(static_cast<uint8_t>(x))
	#define RECEIVE(x) read(x)
#else
	#include "WProgram.h"
	#define SEND(x) send(x) 
	#define RECEIVE(x) receive(x) 
#endif

#include <Wire.h>

/* Register addresses. */
#define DS3231_ADDRESS			0x68	// The I2C address of the DS1307 chip. Default is 1101000 (104 decimal)
#define DS3231_TIME_ADDRESS		0x00	// Start of time register addresses, there are 7 in total (0x00 to 0x06)
#define DS3231_TIME_HOURS		0x02	// Start of the Hours register address.
#define DS3231_TIME_ALARM1		0x07	// Start of the alarm 1 addresses, there are 5 in total.
#define DS3231_TIME_ALARM2		0x0B	// Start of the alarm 2 addresses, there are 4 in total.
#define DS3231_SPECIAL_CONTROL	0x0E	// Control register
#define DS3231_SPECIAL_STATUS	0x0F	// Control / Status register
#define DS3231_SPECIAL_AGEOFF	0X10	// Aging offset
#define DS3231_SPECIAL_TMPMSB	0X11	// MSB of temperature
#define DS3231_SPECIAL_TMPLSB	0X12	// LSB of temperature

/* Control bits. */
#define DS3231_BIT_EOSC			128		// Enable oscillalator	
#define DS3231_BIT_BBSQW		64		// Battery-Backed Square-Wave Enable
#define DS3231_BIT_CONV			32		// Convert Temperature
#define DS3231_BIT_RS2			16		// Rate Select
#define DS3231_BIT_RS1			8		// Rate Select
#define DS3231_BIT_INTCN		4		// Interrupt Control
#define DS3231_BIT_A2IE			2		// Alarm 2 Interrupt Enable
#define DS3231_BIT_A1IE			1		// Alarm 1 Interrupt Enable
#define DS3231_BIT_SQW_1Hz		0		// Square-wave output frequency mask
#define DS3231_BIT_SQW_1024Hz	8		// Square-wave output frequency mask
#define DS3231_BIT_SQW_4096Hz	16		// Square-wave output frequency mask
#define DS3231_BIT_SQW_8192Hz	24		// Square-wave output frequency mask
#define DS3231_BIT_AM_PM        32		// AM / PM bit.
#define DS3231_BIT_12_24        64		// 12 / 24 hour mode
#define DS3231_BIT_DYDT         64		// Mask for setting DYDT bit to Day of the week.
#define DS3231_BIT_ALARM		128		// Mask used to set the Alarm range bits.

#define SECONDS_PER_DAY 86400L				// Used by DateTime class
#define SECONDS_FROM_1970_TO_2000 946684800	// Used by DateTime class

/* AlarmTriggerType enumerations just to simplify setting the alarms. */
enum Alarm1TriggerType {EverySecond,				// Alarm once per second 
						SecsMatch,					// Alarm when seconds match 
						MinsSecsMatch,				// Alarm when minutes and seconds match 
						HoursMinsSecsMatch, 		// Alarm when hours, minutes, and seconds match 
						DateHoursMinsSecsMatch, 	// Alarm when date, hours, minutes, and seconds match 
						DayHoursMinsSecsMatch };	// Alarm when day, hours, minutes, and seconds match 

enum Alarm2TriggerType {EveryMinute,				// Alarm once per minute (00 seconds of every minute) 
						MinsMatch, 					// Alarm when minutes match 
						HoursMinsMatch, 			// Alarm when hours and minutes match
						DateHoursMinsMatch, 		// Alarm when date, hours, and minutes match 
						DayHoursMinsMatch };		// Alarm when day, hours, and minutes match 

extern uint8_t bcd2bin (uint8_t val);				// Conversion from bcd to decimal
extern uint8_t bin2bcd (uint8_t val);				// Conversion from decimal to bcd

static const uint8_t daysInMonth[] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };

/*  Simple general-purpose date/time class (no TZ / DST / leap second handling!) */
class DateTime
{
	public:
		DateTime (uint32_t t =0);
		DateTime (uint16_t year, uint8_t month, uint8_t day, uint8_t hour =0, uint8_t min =0, uint8_t sec =0);
		DateTime (const char* date, const char* time);
		uint16_t year() const { return 2000 + yOff; }
		uint8_t month() const { return m; }
		uint8_t day() const { return d; }
		uint8_t hour() const { return hh; }
		uint8_t minute() const { return mm; }
		uint8_t second() const { return ss; }
		uint8_t dayOfWeek() const;

		long secondstime() const;						// 32-bit times as seconds since 1/1/2000    
		uint32_t unixtime(void) const;					// 32-bit times as seconds since 1/1/1970    
		char* toString(char* buf, int maxlen) const;	// As a string
		void operator+=(uint32_t);						// Add additional time

	protected:
		uint8_t yOff, m, d, hh, mm, ss;
};

class RTC_DS3231
{
	public:
		RTC_DS3231();										// Initialises the wire library, so must be called once	
		void SetDateTime(const DateTime& dt);				// Set the RTC time (and date!)
		DateTime now();										// Returns the current time.
		DateTime GetAlarm1();								// Returns the current Alarm 1 time setting
		DateTime GetAlarm2();								// Returns the current Alarm 2 time setting
		uint8_t GetRegister(uint8_t);						// Returns a particular register byte
		uint8_t SwitchRegisterBit(uint8_t, uint8_t, bool );	// Set an individual register bit on or off
		void SetRegister(uint8_t, uint8_t);					// Write to a register.
		void SetAlarm1( DateTime ,bool = true, Alarm1TriggerType = HoursMinsSecsMatch );// Set alarm 1
		void SetAlarm2( DateTime ,bool = true, Alarm2TriggerType = HoursMinsMatch );	// Set alarm 2
		double GetTemperature();							// Returns the temperature
};

#endif