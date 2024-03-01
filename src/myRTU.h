#ifndef MYRTU_H
#define MYRTU_H

#include <ModbusMaster.h>
#include <driver/pcnt.h>
#include "dataTypes.h"

#define MAX485_DE       19
#define MAX485_RE_NEG   18
#define MAX485_RO       26
#define MAX485_DI       23

#define SerialRTU Serial
#define WindDir 0x02

namespace modbusLine
{
    void preTransmission()
    {
        digitalWrite(MAX485_RE_NEG, 1);
        digitalWrite(MAX485_DE, 1);
    }

    void postTransmission()
    {
        digitalWrite(MAX485_RE_NEG, 0);
        digitalWrite(MAX485_DE, 0);
    }

    void setupModbus(ModbusMaster& node)
    {
        pinMode(MAX485_RE_NEG, OUTPUT);
        pinMode(MAX485_DE, OUTPUT);
        // Init in receive mode
        digitalWrite(MAX485_RE_NEG, LOW);
        digitalWrite(MAX485_DE, LOW);

        node.preTransmission(preTransmission);
        node.postTransmission(postTransmission);

        delay(10);
    }

    void modbusRead(DataRecived &data, ModbusMaster &node)
    {      
        SerialRTU.begin(9600, SERIAL_8N1, MAX485_RO, MAX485_DI);
        node.begin(WindDir, SerialRTU);   

        bool result = false;
        for (uint8_t i = 0; i < 3; i++)
        {
            result = node.readHoldingRegisters(0x00, 1); // ony 0x00 is windDir
            if (result == node.ku8MBSuccess){
                data.windDir = node.getResponseBuffer(0x00);
                break;
            }
            else {                
                vTaskDelay(pdMS_TO_TICKS(1));
            }
        }  
        if(result != node.ku8MBSuccess) 
            SerialMon.println(String("Error modbus: ") + result);    

        SerialRTU.end();
    }
}

namespace anemometer
{
    int16_t* windCouter;
    int16_t getWindCounter() { return *windCouter;}
    
    void fAnemometer(void *pvParameters)
    {        
        int16_t WindCnt;
        windCouter = &WindCnt;
        for (;;)
        {
            pcnt_get_counter_value(PCNT_UNIT_0, windCouter);
            pcnt_counter_clear(PCNT_UNIT_0);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        vTaskDelete(NULL);
    }

    inline void anemometer_Setup(TaskHandle_t* fAnemometerHandler)
    {
        int16_t PCNT_H_LIM_VAL = 300;
        int16_t PCNT_L_LIM_VAL = 0;
        // Anemometer
        pcnt_config_t pcnt_config = {};
        pcnt_config.pulse_gpio_num = GPIO_NUM_14; // Set PCNT input signal and control GPIOs
        pcnt_config.ctrl_gpio_num = PCNT_PIN_NOT_USED;
        pcnt_config.channel = PCNT_CHANNEL_0;
        pcnt_config.unit = PCNT_UNIT_0;
        // What to do on the positive / negative edge of pulse input?
        pcnt_config.pos_mode = PCNT_COUNT_INC; // Count up on the positive edge
        pcnt_config.neg_mode = PCNT_COUNT_DIS; // Count down disable
        // What to do when control input is low or high?
        pcnt_config.lctrl_mode = PCNT_MODE_KEEP; // Keep the primary counter mode if low
        pcnt_config.hctrl_mode = PCNT_MODE_KEEP; // Keep the primary counter mode if high
        // Set the maximum and minimum limit values to watch
        pcnt_config.counter_h_lim = PCNT_H_LIM_VAL;
        pcnt_config.counter_l_lim = PCNT_L_LIM_VAL;
        pcnt_unit_config(&pcnt_config); // Initialize PCNT unit
        // default filter value for me was 16 even if no manual setting
        pcnt_filter_enable(PCNT_UNIT_0);
        pcnt_counter_pause(PCNT_UNIT_0);
        pcnt_counter_clear(PCNT_UNIT_0);
        pcnt_counter_resume(PCNT_UNIT_0); // start the show

        xTaskCreate(fAnemometer, "fAnemometer", 8192, NULL, 0, fAnemometerHandler);
    }
}

#endif