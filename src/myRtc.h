#ifndef MYRTC_H
#define MYRTC_H
#include <Wire.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>

#define I2C_SDA 21
#define I2C_SCL 22
#define RTC_RST 27
#define countof(a) (sizeof(a) / sizeof(a[0]))

 ThreeWire myWire(I2C_SDA, I2C_SCL, RTC_RST); // IO, SCLK, CE
 RtcDS1302<ThreeWire> Rtc(myWire);

struct dateTime_t
{
    char date[25];
    char time[25];

    // dateTime_t()
    // {
    //     getDateTime();
    // }

    static void printDateTime(const RtcDateTime &dt)
    {
        char datestring[20];

        snprintf_P(datestring,
                   countof(datestring),
                   PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
                   dt.Month(),
                   dt.Day(),
                   dt.Year(),
                   dt.Hour(),
                   dt.Minute(),
                   dt.Second());
        Serial.print(datestring);
    }

    void getDateTime()
    {
        RtcDateTime now = Rtc.GetDateTime();
        if (!now.IsValid())
        {
            // Common Causes:
            //    1) the battery on the device is low or even missing and the power line was disconnected
            Serial.println("RTC lost confidence in the DateTime!");
        }
        snprintf_P(date,
                   countof(date),
                   PSTR("%02u/%02u/%04u"),
                   now.Month(),
                   now.Day(),
                   now.Year());

        snprintf_P(time,
                   countof(time),
                   PSTR("%02u:%02u:%02u"),
                   now.Hour(),
                   now.Minute(),
                   now.Second());
    }
};


namespace myRtc
{
    void setupRtc()
    {
        Wire.begin(I2C_SDA, I2C_SCL);
        Serial.print("compiled: ");
        Serial.print(__DATE__);
        Serial.println(__TIME__);

        Rtc.Begin();
        RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
        dateTime_t::printDateTime(compiled);
        Serial.println();

        if (!Rtc.IsDateTimeValid())
        {
            // Common Causes:
            //    1) first time you ran and the device wasn't running yet
            //    2) the battery on the device is low or even missing

            Serial.println("RTC lost confidence in the DateTime!");
            Rtc.SetDateTime(compiled);
            RtcDateTime firstTime(__DATE__, __TIME__);
            Rtc.SetDateTime(firstTime);
        }

        if (Rtc.GetIsWriteProtected())
        {
            Serial.println("RTC was write protected, enabling writing now");
            Rtc.SetIsWriteProtected(false);
        }

        if (!Rtc.GetIsRunning())
        {
            Serial.println("RTC was not actively running, starting now");
            Rtc.SetIsRunning(true);
        }

        RtcDateTime now = Rtc.GetDateTime();
        if (now < compiled)
        {
            Serial.println("RTC is older than compile time!  (Updating DateTime)");
            Rtc.SetDateTime(compiled);
        }
        else if (now > compiled)
        {
            Serial.println("RTC is newer than compile time. (this is expected)");
        }
        else if (now == compiled)
        {
            Serial.println("RTC is the same as compile time! (not expected but all is fine)");
        }
    }
}

#endif