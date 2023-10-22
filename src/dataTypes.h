#ifndef DATATYPES_H
#define DATATYPES_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <myRtc.h>

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

struct Measurements_t{
    DataRecived _dataRecived;
    dateTime_t _timeStamp;

    void setCurrentTime(){return _timeStamp.getDateTime();}
};

struct messageBuffer_t{
    char _buffer[4800];    
};

#endif