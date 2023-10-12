#include <SPI.h>
#include <dataTypes.h>
#include <vector>
//#include <GPRS.h>
#include <esp_now.h>
#include <WiFi.h>
#include<deque>

#define RAM_CHECK  ; RAM_Check(__func__, uxTaskGetStackHighWaterMark(NULL))
#define TASK_CREATED Serial.printf("%s created\n", __func__);

void RAM_Check(const char* name, size_t bytes)
{  
  Serial.printf("Task \"%s\" RAM Usage: %u bytes\n", name, bytes);
  size_t freeHeapSize = esp_get_free_heap_size();
  Serial.printf("Free Heap Size: %u bytes\n", freeHeapSize);
  Serial.println();
}
// ############################## ESP NOW #################
uint8_t slaveAddress[] = {0x30, 0xC6, 0xF7, 0x04, 0x64, 0x2C};
esp_now_peer_info_t peerInfo;

typedef struct request_t {
 const char request[4] = "GET";
}RequestMsg;

void registerPeer()
{
  memcpy(peerInfo.peer_addr, slaveAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

    // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;    
  }
}

dateTime_t fakeTimeStamp()
{
  static int cnt = 0;
  dateTime_t temp;  
  snprintf(temp.time, 25, "%d:%d:%d", cnt++, cnt + 1, cnt + 2);  
  snprintf(temp.date, 25, "%d:%d:%d", cnt * 2 , cnt * 2 + 1, cnt * 10);
  return temp;
}

QueueHandle_t measurementsQueue;
QueueHandle_t stringQueue;
// ########################################################

uint64_t getEspTimeinMs(){return esp_timer_get_time() / 1000;};

String generateJsonRow(Measurements_t& data);

TaskHandle_t requestTaskHandler = NULL;
TaskHandle_t sendTaskHandler = NULL;
TaskHandle_t serializeTaskHandler = NULL;

void listSerialize(const std::vector<String>& tempList, char* buffer)
{
  strcpy(buffer, "[");
  for(auto& vec : tempList)
  {
   strcat(buffer, vec.c_str());
   if(&vec != &tempList.back())   strcat(buffer, ","); // add coma only when its not last element
  }
  strcat(buffer, "]");
  RAM_CHECK; 
}

// ######################################################################## TASK #########################

esp_err_t sendRequest(const uint8_t* adress)
{
  const RequestMsg request;
  return esp_now_send(adress, (uint8_t *)&request, sizeof(request));
}

void requestTask(void *parameters)
{
   TASK_CREATED;
  // loop
  while (true)
  {
    esp_err_t result = ESP_FAIL;
    uint8_t counter = 0;
    while (result != ESP_OK)    {      
      result = sendRequest(slaveAddress);
      counter++;
      if(counter > 5) break;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  vTaskDelete(NULL);
}

void onDataReceived(const uint8_t *mac, const uint8_t *data, int len)
{
  Measurements_t measurements;  
  memcpy(&measurements._dataRecived, data, sizeof(data));
  measurements._timeStamp = fakeTimeStamp();

  // five attepts
  for (uint8_t cnt = 0; cnt < 5; cnt++)  {
    if(xQueueSend(measurementsQueue, &measurements, pdMS_TO_TICKS(50)) == pdTRUE) break;
  }
}
void serializeTask(void *parameter)
{
  std::vector<String> tempList;
  tempList.reserve(30);
  TASK_CREATED;
  char buffer[5000];
  while(true)  {    
    Measurements_t measurements;
    if (xQueueReceive(measurementsQueue, &measurements, portMAX_DELAY) == pdTRUE){
      tempList.emplace_back(generateJsonRow(measurements));

      if (tempList.size() == 30){
        listSerialize(tempList, buffer);
        if (xQueueSend(stringQueue, buffer, portMAX_DELAY) == pdTRUE){
          RAM_CHECK;
          tempList.clear();
          strcpy(buffer, "");
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(400));
  }
  vTaskDelete(NULL);
}

// void sendTask(void *parameters)
// {
//   SIM800L *sim800l = new SIM800L((Stream *)&SerialAT, SIM800_RST_PIN, 15000, 15000);
//   setupSim800Module(sim800l);
//   globalTimer = getEspTimeinMs();
//   Serial.println("SEND TASK CREATED");

//   while (true)
//   {  
//     String buffer;
//     if(xQueuePeek(stringQueue, &buffer, portMAX_DELAY) == pdTRUE){
//       bool success = false;
//       for (int i = 0; i < 30; i++){
//         success = postToHTTP(buffer.c_str(), sim800l);
//         if (success){
//           xQueueReceive(stringQueue, &buffer, portMAX_DELAY);
//           break;
//         }
//         else          vTaskDelay(pdMS_TO_TICKS(100));
//       }
//       if(!success) Serial.println("Failed to post after 30 retries");
//     }
//   }
//   delete sim800l;
//   vTaskDelete(NULL);
// }

bool FakePost(const char* buffer)
{
  vTaskDelay(pdMS_TO_TICKS(2000));
  static bool result = true;
  result = !result;
  return result;
}
void sendTask(void *parameters)
{
  TASK_CREATED;
  char buffer[5000];
  while (true)
  {  
   if(xQueuePeek(stringQueue, buffer, pdMS_TO_TICKS(10))){
      bool success = false;      
        while (!success){
         success = FakePost(buffer);
         if (success){         
            xQueueReceive(stringQueue, buffer, pdMS_TO_TICKS(10));
            RAM_CHECK;     
            strcpy(buffer, "");
            break;
         }
         vTaskDelay(pdMS_TO_TICKS(100));
        }
      }
      vTaskDelay(pdMS_TO_TICKS(1000));  
   }
  Serial.println("SEND TASK DELETED");
  vTaskDelete(NULL);
}
inline void createTasks()
{
  xTaskCreate(requestTask, "requestTask", 2048, NULL, 0, &requestTaskHandler);
  xTaskCreate(serializeTask, "serializeTask", 8192, NULL, 0, &serializeTaskHandler);
  xTaskCreate(sendTask, "sendTask", 8192, NULL, 0, &sendTaskHandler);
}

// #######################################################################################################
void setup()
{
  Serial.begin(115200);
  //SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("Master..");
  //myRtc::setupRtc();

  measurementsQueue = xQueueCreate(60, sizeof(Measurements_t));
  stringQueue = xQueueCreate(4, sizeof(messageBuffer_t));

  esp_now_register_recv_cb(onDataReceived);
  registerPeer();
  createTasks();

}

//=============
inline bool readTaskIsValid() { return requestTaskHandler != NULL;}
inline bool sendTaskIsValid() { return sendTaskHandler != NULL;}

void loop()
{  
  // if there are no send in 2 min, reset tasks
}

String generateJsonRow(Measurements_t& measuremets)
{
  String buffer;
  auto* timeNow = &measuremets._timeStamp;
  auto* data = &measuremets._dataRecived;
  buffer.reserve(200);

  StaticJsonDocument<192> doc;

  JsonObject dateTime = doc.createNestedObject("dateTime");
  dateTime["date"] = timeNow->date;
  dateTime["time"] = timeNow->time;

  JsonObject Anemometer = doc.createNestedObject("Anemometer");
  Anemometer["Speed"] =  data->windCnt;  
  Anemometer["Direction"] =  data->windDir;

  JsonObject Inclinometer = doc.createNestedObject("Inclinometer");
  Inclinometer["V1"] = data->inc_XVal;
  Inclinometer["V2"] = data->inc_YVal;

  serializeJson(doc, buffer);

  return buffer;
}
