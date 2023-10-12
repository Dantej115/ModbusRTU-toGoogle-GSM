#ifndef GPRS_H
#define GPRS_H

// im using h file as inline code to have clear project in main
#include <Sim800l.h>
#include <Config.h>

#define MODEM_TX 17
#define MODEM_RX 16
#define SIM800_RST_PIN 25
#define POST_TIMEOUT 30000
#define READ_TIMEOUT 33000

inline void setupSim800Module(SIM800L *sim800l)
{
  // Wait until the module is ready to accept AT commands
  while (!sim800l->isReady())
  {
    Serial.println(F("Problem to initialize AT command, retry in 1 sec"));
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
  Serial.println(F("Setup Complete!"));

  // Wait for the GSM signal
  uint8_t signal = sim800l->getSignal();
  Serial.println("Getting signal...");
  while (signal <= 0)
  {
    vTaskDelay(50 / portTICK_PERIOD_MS);
    signal = sim800l->getSignal();
  }
  Serial.print(F("Signal OK (strenght: "));
  Serial.print(signal);
  Serial.println(F(")"));

  // Wait for operator network registration (national or roaming network)
  Serial.println("Getting network...");
  NetworkRegistration network = sim800l->getRegistrationStatus();
  while (network != REGISTERED_HOME && network != REGISTERED_ROAMING)
  {
    vTaskDelay(10 / portTICK_PERIOD_MS);
    network = sim800l->getRegistrationStatus();
  }
  Serial.println(F("Network registration OK"));

  // Setup APN for GPRS configuration
  bool success = sim800l->setupGPRS(apn);
  Serial.println("Enabling GPRS");
  while (!success)
  {
    success = sim800l->setupGPRS(apn);
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  Serial.println(F("GPRS config OK"));

  Serial.println("Connecting to gprs...");
  while (sim800l->connectGPRS())
  {
    Serial.println(".");
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  Serial.print(F("GPRS connected with IP "));
  Serial.println(sim800l->getIP());
  
}

inline bool postToHTTP(const char *HeapBuffer, SIM800L *sim800l)
{  
  Serial.println("START POST");
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
        Serial.println("RESET");
        sim800l->reset();
        setupSim800Module(sim800l);
        return false;
      }
    }    

    Serial.print(F("HTTP POST error "));
    Serial.println(rc);

    if (rc == 408)
    {
      for (auto i = 0; i < 10; i++)
      {
        SerialAT.println("AT+HTTPTERM");
        while(SerialAT.available())    SerialMon.println(SerialAT.read());
        vTaskDelay(10 / portTICK_PERIOD_MS);
        SerialAT.println("AT+HTTPINIT");
        while(SerialAT.available())    SerialMon.println(SerialAT.read());
      }
    }

    for(auto i = 0; i < 10; i++) 
    {
      SerialAT.println("AT+HTTPTERM");
      while(SerialAT.available())    SerialMon.println(SerialAT.read());
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    // timeout needs to 1s delay after try again
    return false;
  }
  // Success, output the data received on the serial
  Serial.println(F("HTTP POST successful"));
  erroCounter = 0;
  SerialAT.println("AT+HTTPTERM");
  return true;
}

#endif