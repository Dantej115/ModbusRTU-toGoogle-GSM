#ifndef DATATYPES_H
#define DATATYPES_H
#include <Arduino.h>
#include <ArduinoJson.h>

struct DataRecived
{
    int16_t windDir;
    int16_t windCnt;
    int16_t inc_XVal;
    int16_t inc_YVal;

    DataRecived()
    {
        windDir = INT16_MIN;
        windCnt = INT16_MIN;
        inc_XVal = INT16_MIN;
        inc_YVal = INT16_MIN;   
    }
};

struct dateTime_t
{
    char mDate[15];
    char mTime[15];

    dateTime_t()
    {
        strcpy(mDate, "");
        strcpy(mTime, "");
    }

    dateTime_t(const char* date, const char* time)
    {
        strcpy(mDate, date);
        strcpy(mTime, time);
    }

};

struct Measurements_t
{
    DataRecived _dataRecived;
    dateTime_t _timeStamp;  
    Measurements_t()
    {
        _dataRecived = DataRecived();
        _timeStamp = dateTime_t();
    }      
};

struct messageBuffer_t{
    char _buffer[4800];    
};

#endif