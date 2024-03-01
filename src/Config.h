#ifndef CONFIG_H
#define CONFIG_H

// Your GPRS credentials (leave empty, if not needed)
const char apn[] = "internet";      // APN (example: internet.vodafone.pt) use https://wiki.apnchanger.org
const char gprsUser[] = ""; // GPRS User
const char gprsPass[] = ""; // GPRS Password

// SIM card PIN (leave empty, if not defined)
const char simPIN[] = "";

// Server details
const char host[] = "script.google.com";
const int httpsPort = 443;

const char URL[] = "https://script.google.com/macros/s/AKfycbwKfR_3T9TXgYx1LeMZBh9GqtRyfpmr1doqwyx-euHSev9jc3AszeuXjqISOZwYiKE2Xw/exec";
//const char URL[] = "https://script.google.com/macros/s/AKfycbywjVaM4sKBQE6aNriiW22Cex8QdWup510h_YLwrYFS2mwzuXVVQ5ZHiooxhHZKOVWn8g/exec"; // data test
const char CONTENT_TYPE[] = "application/json";


// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial1
// Set serial for AT commands (to SIM800 module)
 #define SerialAT Serial2

#define MODEM_TX 17
#define MODEM_RX 16

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800   // Modem is SIM800
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb


// void manualSend(char *HeapBuffer)
// {
//   char internalBuffer [2000] = "";
//   SerialAT.println("AT+HTTPPARA=\"CID\",1");
//   SerialAT.println("AT+HTTPPARA=\"URL\",\"https://script.google.com/macros/s/AKfycbywjVaM4sKBQE6aNriiW22Cex8QdWup510h_YLwrYFS2mwzuXVVQ5ZHiooxhHZKOVWn8g/exec\""); //Server address
//   SerialAT.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
//   sprintf(internalBuffer, "AT+HTTPDATA=%d,%d", strlen(HeapBuffer), POST_TIMEOUT);
//   SerialAT.println(internalBuffer);
//   SerialAT.println(HeapBuffer);
//   SerialAT.println("AT+HTTPACTION=1");
//   SerialAT.println("AT+HTTPTERM");
//   Serial.println("Manual SEND");
// }
#endif