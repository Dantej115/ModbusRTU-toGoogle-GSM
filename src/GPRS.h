#ifndef GPRS_H
#define GPRS_H

#include <Sim800l.h>
#include <Config.h>


#define SIM800_RST_PIN 22
#define POST_TIMEOUT 30000
#define READ_TIMEOUT 33000

void setupSim800Module(SIM800L *sim800l)
{
  // Wait until the module is ready to accept AT commands
  while (!sim800l->isReady())
  {
    SerialMon.println(F("Problem to initialize AT command, retry in 1 sec"));
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
  SerialMon.println(F("Setup Complete!"));

  // Wait for the GSM signal
  uint8_t signal = sim800l->getSignal();
  SerialMon.println("Getting signal...");
  while (signal <= 0)
  {
    vTaskDelay(50 / portTICK_PERIOD_MS);
    signal = sim800l->getSignal();
  }
  SerialMon.print(F("Signal OK (strenght: "));
  SerialMon.print(signal);
  SerialMon.println(F(")"));

  // Wait for operator network registration (national or roaming network)
  SerialMon.println("Getting network...");
  NetworkRegistration network = sim800l->getRegistrationStatus();
  while (network != REGISTERED_HOME && network != REGISTERED_ROAMING)
  {
    vTaskDelay(10 / portTICK_PERIOD_MS);
    network = sim800l->getRegistrationStatus();
  }
  SerialMon.println(F("Network registration OK"));

  // Setup APN for GPRS configuration
  bool success = sim800l->setupGPRS(apn);
  SerialMon.println("Enabling GPRS");
  while (!success)
  {
    success = sim800l->setupGPRS(apn);
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  SerialMon.println(F("GPRS config OK"));

  SerialMon.println("Connecting to gprs...");
  while (sim800l->connectGPRS())
  {
    SerialMon.println(".");
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  SerialMon.print(F("GPRS connected with IP "));
  SerialMon.println(sim800l->getIP());  
  
}

inline bool postToHTTP(const char *HeapBuffer, SIM800L *sim800l)
{  
  SerialMon.println("START POST");
  static uint8_t erroCounter = 0;
  uint16_t rc = sim800l->doPost(URL, CONTENT_TYPE, HeapBuffer, POST_TIMEOUT, READ_TIMEOUT);
  
  if (rc != 200 && rc != 705)
  {
    if (rc > 700)
    {
      // on errors 700+ something weird happens
      erroCounter++;
      if (erroCounter > 10)
      {
        SerialMon.println("RESET");
        sim800l->reset();
        setupSim800Module(sim800l);
        return false;
      }
    }    

    SerialMon.print(F("HTTP POST error "));
    SerialMon.println(rc);

    if (rc == 408)
    {
      for (auto i = 0; i < 10; i++)
      {
        SerialAT.println("AT+HTTPTERM");
        //while(SerialAT.available())    SerialMon.println(SerialAT.read());
        vTaskDelay(10 / portTICK_PERIOD_MS);
        SerialAT.println("AT+HTTPINIT");
        //while(SerialAT.available())    SerialMon.println(SerialAT.read());
      }
    }

    for(auto i = 0; i < 10; i++) 
    {
      SerialAT.println("AT+HTTPTERM");
      //while(SerialAT.available())    SerialMon.println(SerialAT.read());
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    // timeout needs to 1s delay after try again
    return false;
  }
  // Success, output the data received on the serial
  SerialMon.println(F("HTTP POST successful"));
  erroCounter = 0;
  SerialAT.println("AT+HTTPTERM");
  return true;
}

#endif