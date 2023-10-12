#ifndef DATATYPES_H
#define DATATYPES_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <myRtc.h>

struct dataRecived
{
    int16_t windDir;
    int16_t windCnt;
    int16_t inc_XVal;
    int16_t inc_YVal;

    dataRecived()
    {
        windDir = INT16_MIN;
        windCnt = INT16_MIN;
        inc_XVal = INT16_MIN;
        inc_YVal = INT16_MIN;   
    }
    bool dataCheck()
    {
        if(windDir == INT16_MIN &&
        windCnt == INT16_MIN &&
        inc_XVal == INT16_MIN &&
        inc_YVal == INT16_MIN )         return false;
        // if everything good return check ok
        return true;        
    }
};

struct Measurements_t{
    dataRecived _dataRecived;
    dateTime_t _timeStamp;

    void setCurrentTime(){return _timeStamp.getDateTime();}
};

struct messageBuffer_t{
    char _buffer[4800];    
};

#endif