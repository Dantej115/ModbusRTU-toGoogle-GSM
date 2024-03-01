#ifndef MYRTC_H
#define MYRTC_H

#include <Wire.h>
#include <SPI.h>
#include "RTClib.h"
#include "dataTypes.h"

#define SDA_PIN 27
#define SCL_PIN 25
#define countof(a) (sizeof(a) / sizeof(a[0]))
namespace RTC
{
    static RTC_DS3231  Rtc;
    dateTime_t getDateTime();
    static DateTime lastValid;

    void setupRTC()
    {
        Wire.begin(SDA_PIN, SCL_PIN);
        if (!Rtc.begin())
        {
            SerialMon.println("Couldn't find RTC");
            SerialMon.flush();
            while (1)
                delay(10);
        }

        if (Rtc.lostPower())
        {
            SerialMon.println("RTC lost power, let's set the time!");
            Rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }
    }

    dateTime_t getDateTime()
    {        
        DateTime now = Rtc.now();        
        for (size_t i = 0; i < 5; i++)
        {
            if(now.isValid())
                break;
            now = Rtc.now();
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        
        char date[15];
        char time[15];

        snprintf_P(date,
                   countof(date),
                   PSTR("%04u/%02u/%02u"),
                   now.year(),
                   now.month(),
                   now.day());

        snprintf_P(time,
                   countof(time),
                   PSTR("%02u:%02u:%02u"),
                   now.hour(),
                   now.minute(),
                   now.second());
        
        if(now.isValid()) lastValid = now;
        else 
        {
            if(lastValid.isValid()) Rtc.adjust(lastValid);
        }
        return dateTime_t(date, time);
    }
}

#endif