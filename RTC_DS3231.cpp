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
#include "RTC_DS3231.h"

/*	Simply initialises the wire library */
RTC_DS3231::RTC_DS3231()
{
	Wire.begin();
}


/*	Set this Real Time Clocks initial time. */
void RTC_DS3231::SetDateTime(const DateTime& dt)
{
    Wire.beginTransmission(DS3231_ADDRESS);	// Wake the I2C device
    Wire.SEND(0);							// Advance its internal register pointer.
    Wire.SEND(bin2bcd(dt.second()));		// Send data...
    Wire.SEND(bin2bcd(dt.minute()));
    Wire.SEND(bin2bcd(dt.hour()));
    Wire.SEND(bin2bcd(0));					// Skip the day of week.
    Wire.SEND(bin2bcd(dt.day()));		
    Wire.SEND(bin2bcd(dt.month()));
    Wire.SEND(bin2bcd(dt.year() - 2000));
    Wire.endTransmission();					// End the transmission
}


/* Returns the current time as a DateTime object */
DateTime RTC_DS3231::now()
{
    Wire.beginTransmission(DS3231_ADDRESS);		// Wake the I2C device
    Wire.SEND(0);								// Advance its internal register pointer.
    Wire.endTransmission();						// End the transmission
    Wire.requestFrom(DS3231_ADDRESS, 7);		// Request 7 bytes from device
    uint8_t ss = bcd2bin(Wire.RECEIVE() & 0x7F);// Read the data...
    uint8_t mm = bcd2bin(Wire.RECEIVE());
    uint8_t hh = bcd2bin(Wire.RECEIVE());
    Wire.RECEIVE();								// Skip the day of week.
    uint8_t d = bcd2bin(Wire.RECEIVE());
    uint8_t m = bcd2bin(Wire.RECEIVE());
    uint16_t y = bcd2bin(Wire.RECEIVE()) + 2000;
    return DateTime (y, m, d, hh, mm, ss);
}


/* Returns the alarm 1 time as a DateTime object */
DateTime RTC_DS3231::GetAlarm1()
{
	Wire.beginTransmission(DS3231_ADDRESS);		// Wake the I2C device
	Wire.SEND(DS3231_TIME_ALARM1);				// Advance its internal register pointer.
    Wire.endTransmission();						// End the transmission
	Wire.requestFrom(DS3231_ADDRESS, 4);		// Request 5 bytes from device
    uint8_t ss = bcd2bin(Wire.RECEIVE() );		// Read the data...
    uint8_t mm = bcd2bin(Wire.RECEIVE());
    uint8_t hh = bcd2bin(Wire.RECEIVE());
	uint8_t d = bcd2bin( Wire.RECEIVE());	
	DateTime dt(2000, 0, d, hh, mm, ss);		// Create our return value   
	return dt;									// Return it
}


/* Returns the alarm 2 time as a DateTime object */
DateTime RTC_DS3231::GetAlarm2()
{
	Wire.beginTransmission(DS3231_ADDRESS);		// Wake the I2C device
	Wire.SEND(DS3231_TIME_ALARM2);				// Advance its internal register pointer.
    Wire.endTransmission();						// End the transmission
    Wire.requestFrom(DS3231_ADDRESS, 3);		// Request 3 bytes from device
    uint8_t mm = bcd2bin(Wire.RECEIVE());		// Read the data...
    uint8_t hh = bcd2bin(Wire.RECEIVE());
	uint8_t d = bcd2bin(Wire.RECEIVE());
	DateTime dt(2000, 0, 0, hh, mm, 00);		// Create our return value    	
	return dt;									// Return it
}


/*	Sets the alarm 1 time.  The on and type parameters default to true
	and HoursMinsSecsMatch when no value is provided for them. The type 
	prameter uses the Alarm1TriggerType enumeration to set the Alarm Rate.	*/
void RTC_DS3231::SetAlarm1(DateTime dt, bool on, Alarm1TriggerType type)
{	
	// Convert the required time/date values to BCD
	uint8_t secs	= bin2bcd( dt.second()	);	
	uint8_t mins	= bin2bcd( dt.minute()	);
	uint8_t hours	= bin2bcd( dt.hour()	);
	uint8_t date	= bin2bcd( dt.day()		);
	
	// Set the Alarm Mask bits, (Alarm rate) See DS3231 datasheet, table 2 p12.
	switch( type )
	{
		case EverySecond:
			secs	|= DS3231_BIT_ALARM;
			mins	|= DS3231_BIT_ALARM;
			hours	|= DS3231_BIT_ALARM;
			date	|= DS3231_BIT_ALARM;
			break;
		case SecsMatch:
			mins	|= DS3231_BIT_ALARM;
			hours	|= DS3231_BIT_ALARM;	
			date	|= DS3231_BIT_ALARM;
			break;
		case MinsSecsMatch:
			hours	|= DS3231_BIT_ALARM;
			date	|= DS3231_BIT_ALARM;
			break;
		case HoursMinsSecsMatch:
			date	|= DS3231_BIT_ALARM;
			break;
		case DateHoursMinsSecsMatch:
			// No additional bits are needed, i.e all 0
			break;
		case DayHoursMinsSecsMatch:
			date	|= DS3231_BIT_DYDT;
			break;
	}

	// Now all bits are set, transmit to DS3231
	Wire.beginTransmission(DS3231_ADDRESS);		// Wake the correct I2C device.
	Wire.SEND(DS3231_TIME_ALARM1);				// Advance its internal register pointer.
    Wire.SEND(bin2bcd(dt.second()));			// Send data bytes...
    Wire.SEND(bin2bcd(dt.minute()));			
    Wire.SEND(bin2bcd(dt.hour()));
	Wire.SEND(bin2bcd(dt.day()));
    Wire.endTransmission();						// End the transmission
	
	// Now the time of the alarm is set, turn it on (or off!)
	SwitchRegisterBit(DS3231_SPECIAL_CONTROL,DS3231_BIT_A1IE, on );
}


/*	Sets the alarm 2 time.  The on and type parameters default to true
	and HoursMinsMatch when no value is provided for them. The type 
	prameter uses the Alarm1TriggerType enumeration to set the Alarm Rate.	*/
void RTC_DS3231::SetAlarm2(DateTime dt, bool on, Alarm2TriggerType type)
{
	// Convert the required time/date values to BCD
	uint8_t mins	= bin2bcd( dt.minute()	);
	uint8_t hours	= bin2bcd( dt.hour()	);
	uint8_t date	= bin2bcd( dt.day()		);
	
	// Set the Alarm Mask bits, (Alarm rate) See DS3231 datasheet, table 2 p12.
	switch( type )
	{
		case EveryMinute:
			mins	|= DS3231_BIT_ALARM;
			hours	|= DS3231_BIT_ALARM;
			date	|= DS3231_BIT_ALARM;
			break;
		case MinsMatch:
			hours	|= DS3231_BIT_ALARM;
			date	|= DS3231_BIT_ALARM;
			break;
		case HoursMinsMatch:
			date	|= DS3231_BIT_ALARM;
			break;
		case DateHoursMinsMatch:
			// No additional bits are needed, i.e all 0
			break;
		case DayHoursMinsMatch:
			date	|= DS3231_BIT_DYDT;
			break;
	}
	
	// Now all bits are set, transmit to DS3231
	Wire.beginTransmission( DS3231_ADDRESS );		// Wake the correct I2C device.
	Wire.SEND( DS3231_TIME_ALARM2 );				// Advance its internal register pointer
	Wire.SEND( mins );								// Send data bytes...
	Wire.SEND( hours );
    Wire.SEND( date );
    Wire.endTransmission();							// End the transmission

	// Now the time of the alarm is set, turn it on (or off!)
	SwitchRegisterBit(DS3231_SPECIAL_CONTROL,DS3231_BIT_A2IE, on );
}


/*	Returns the value (8 bits) of a single register.  Remember that the 
	values of the first 14 registers 0x00 to 0x0D are in BCD format.  
	You can use the bin2bcd() or bcd2bin() methods to convert as necessary */
uint8_t RTC_DS3231::GetRegister(uint8_t reg)
{
	Wire.beginTransmission(DS3231_ADDRESS);		// Wake the I2C device
	Wire.SEND(reg);								// Advance its internal register pointer.
    Wire.endTransmission();						// End the transmission
    Wire.requestFrom(DS3231_ADDRESS, 1);		// Request 1 byte from device
	return Wire.RECEIVE();				
}


/*	Sets the value (8 bits) of a single register.  Remember that the 
	values of the first 14 registers 0x00 to 0x0D are in BCD format.  
	You can use the bin2bcd() or bcd2bin() methods to convert as necessary */
void RTC_DS3231::SetRegister(uint8_t reg, uint8_t byte)
{
	Wire.beginTransmission(DS3231_ADDRESS);		// Wake the I2C device
	Wire.SEND(reg);								// Advance its internal register pointer.	
	Wire.SEND(byte); 							// Send data bytes...
	Wire.endTransmission();						// End the transmission			
}


/*	Flips an individual register bit on or off. Ensure you use the correct 
	register & control bit.  For example, if you want to change the 12 or 
	24 hr bit; i.e set it to 1, you could...
	
		SetRegister( DS3231_TIME_HOURS,		// The register address.
					 DS3231_BIT_12_24,		// The bit you need to change
					 true					// Turn it on.
					);   */
uint8_t RTC_DS3231::SwitchRegisterBit(uint8_t reg, uint8_t bit, bool onOff )
{
	if(onOff)
		this->SetRegister( reg, (this->GetRegister( reg ) | bit) );
	else
		this->SetRegister( reg, (this->GetRegister( reg ) & ~bit) );

	return this->GetRegister(reg);
}


/*	Returns the converted temperature. If you want to read the temperature
	registers directly, use GetRegister() as shown instead;

	GetRegister(DS3231_SPECIAL_TMPMSB);
	GetRegister(DS3231_SPECIAL_TMPLSB);	*/
double RTC_DS3231::GetTemperature()
{
	uint8_t msb = this->GetRegister(DS3231_SPECIAL_TMPMSB);	// Read temp MSB
	uint8_t lsb = this->GetRegister(DS3231_SPECIAL_TMPLSB);	// Read temp LSB
	return  msb + lsb / 256.0;								// Convert & return
}


// DateTime stuff.....

/* Number of days since 2000/01/01, valid for 2001..2099 */
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d)
{
    if (y >= 2000)
        y -= 2000;
    uint16_t days = d;
    for (uint8_t i = 1; i < m; ++i)
        days += pgm_read_byte(daysInMonth + i - 1);
    if (m > 2 && y % 4 == 0)
        ++days;
    return days + 365 * y + (y + 3) / 4 - 1;
}

static long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s)
{
    return ((days * 24L + h) * 60 + m) * 60 + s;
}

DateTime::DateTime (uint32_t t)
{
    t -= SECONDS_FROM_1970_TO_2000;    // bring to 2000 timestamp from 1970

    ss = t % 60;
    t /= 60;
    mm = t % 60;
    t /= 60;
    hh = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;
    for (yOff = 0; ; ++yOff)
    {
        leap = yOff % 4 == 0;
        if (days < 365U + leap)
            break;
        days -= 365 + leap;
    }
    for (m = 1; ; ++m)
    {
        uint8_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);
        if (leap && m == 2)
            ++daysPerMonth;
        if (days < daysPerMonth)
            break;
        days -= daysPerMonth;
    }
    d = days + 1;
}

DateTime::DateTime (uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
{
    if (year >= 2000)
        year -= 2000;
    yOff = year;
    m = min(month,12);
    d = min(day,31);
    hh = min(hour,23);
    mm = min(min,60);
    ss = min(sec,60);
}

static uint8_t conv2d(const char* p)
{
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}

/* A convenient constructor for using "the compiler's time":
   DateTime now (__DATE__, __TIME__);  */
DateTime::DateTime (const char* date, const char* time)
{
    // sample input: date = "Dec 26 2009", time = "12:34:56"
    yOff = conv2d(date + 9);
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    switch (date[0])
    {
    case 'J':
        if ( date[1] == 'a' )
	    m = 1;
	else if ( date[2] == 'n' )
	    m = 6;
	else
	    m = 7;
        break;
    case 'F':
        m = 2;
        break;
    case 'A':
        m = date[2] == 'r' ? 4 : 8;
        break;
    case 'M':
        m = date[2] == 'r' ? 3 : 5;
        break;
    case 'S':
        m = 9;
        break;
    case 'O':
        m = 10;
        break;
    case 'N':
        m = 11;
        break;
    case 'D':
        m = 12;
        break;
    }
    d = conv2d(date + 4);
    hh = conv2d(time);
    mm = conv2d(time + 3);
    ss = conv2d(time + 6);
}

uint8_t DateTime::dayOfWeek() const
{
    uint16_t day = date2days(yOff, m, d);
    return (day + 6) % 7; // Jan 1, 2000 is a Saturday, i.e. returns 6
}

uint32_t DateTime::unixtime(void) const
{
    uint32_t t;
    uint16_t days = date2days(yOff, m, d);
    t = time2long(days, hh, mm, ss);
    t += SECONDS_FROM_1970_TO_2000;  // seconds from 1970 to 2000

    return t;
}

char* DateTime::toString(char* buf, int maxlen) const
{
	snprintf(buf,maxlen,"%02u-%02u-%04u %02u:%02u:%02u",
			d,    
			m,
            2000 + yOff,
            hh,
            mm,
            ss
        );
    return buf;
}

void DateTime::operator+=(uint32_t additional)
{
    DateTime after = DateTime( unixtime() + additional );
    *this = after;
}

uint8_t bcd2bin (uint8_t val)
{
    return val - 6 * (val >> 4);
}

uint8_t bin2bcd (uint8_t val)
{
    return val + 6 * (val / 10);
}