#include <SPI.h>
#include <vector>
#include <GPRS.h>
#include <myRtc.h>
#include <dataTypes.h>
#include <auxilaryTest.h>
#include <myRTU.h>

#define BUFFER_SIZE 5000
#define TASK_CREATED SerialMon.printf("%s created\n", __func__);

String generateJsonRow(Measurements_t& data);
void listSerialize(const std::vector<String>& tempList, char* buffer);

ModbusMaster node;
// ######################################################################## TASKS #########################
QueueHandle_t measurementsQueue;
QueueHandle_t stringQueue;

TaskHandle_t requestTaskHandler = NULL;
TaskHandle_t sendTaskHandler = NULL;
TaskHandle_t serializeTaskHandler = NULL;
TaskHandle_t fAnemometerHandler = NULL;

void readTask(void *parameters)
{
  TASK_CREATED;

  while (true)
  {
    Measurements_t measurements;
    modbusLine::modbusRead(measurements._dataRecived, node);
    measurements._dataRecived.windCnt = anemometer::getWindCounter();
    measurements._timeStamp = RTC::getDateTime();

    // five attepts
    for (uint8_t cnt = 0; cnt < 5; cnt++)
    {
      if (xQueueSend(measurementsQueue, &measurements, pdMS_TO_TICKS(10)) == pdTRUE)
        break;
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  vTaskDelete(NULL);
}
void serializeTask(void *parameter)
{
  std::vector<String> tempList;
  tempList.reserve(30);
  TASK_CREATED;
  char buffer[BUFFER_SIZE];
  while(true)  {    
    Measurements_t measurements;
    if (xQueueReceive(measurementsQueue, &measurements, portMAX_DELAY) == pdTRUE){
      tempList.emplace_back(generateJsonRow(measurements));

      if (tempList.size() == 30){
        listSerialize(tempList, buffer);
        SerialMon.println("Serialize complete");
        if (xQueueSend(stringQueue, buffer, portMAX_DELAY) == pdTRUE){
          SerialMon.println("Serialize send");
          tempList.clear();
          strcpy(buffer, "");
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(400));
  }
  vTaskDelete(NULL);
}

void sendTask(void *parameters)
{
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  SIM800L *sim800l = new SIM800L((Stream *)&SerialAT, SIM800_RST_PIN, 15000, 15000);
  setupSim800Module(sim800l);
  TASK_CREATED;
  char buffer[BUFFER_SIZE];
  while (true)
  {  
    if(xQueuePeek(stringQueue, &buffer, portMAX_DELAY) == pdTRUE){
      bool success = false;
      for (int i = 0; i < 30; i++){
        success = postToHTTP(buffer, sim800l);
        if (success){
          xQueueReceive(stringQueue, &buffer, portMAX_DELAY);
          break;
        }
        else          vTaskDelay(pdMS_TO_TICKS(100));
      }
      if(!success) 
      {
        SerialMon.println("Failed to post after 30 retries");
        sim800l->reset();
        setupSim800Module(sim800l);
      }
    }
  }
  delete sim800l;
  vTaskDelete(NULL);
}

inline void createTasks()
{
  xTaskCreate(readTask, "readTask", 2 *8192, NULL, 3, &requestTaskHandler);
  xTaskCreate(serializeTask, "serializeTask", 2 *8192, NULL, 1, &serializeTaskHandler);
  xTaskCreate(sendTask, "sendTask", 2 *8192, NULL, 0, &sendTaskHandler);
}

// #######################################################################################################
void setup()
{
  SerialMon.begin(115200, SERIAL_8N1, RX, TX);
  
  SerialMon.println("Master..");

  RTC::setupRTC();
  SerialMon.println("Modbus setup"); 
  modbusLine::setupModbus(node); 
  SerialMon.println("Modbus started"); 
  
  anemometer::anemometer_Setup(&fAnemometerHandler);  
  SerialMon.println("anemometer started");
  SerialMon.println("Done");

  measurementsQueue = xQueueCreate(60, sizeof(Measurements_t));
  stringQueue = xQueueCreate(4, sizeof(messageBuffer_t));

  createTasks();

}

void loop()
{  
  vTaskDelete(NULL);
}

String generateJsonRow(Measurements_t& measuremets)
{
  String buffer;
  auto* timeNow = &measuremets._timeStamp;
  auto* data = &measuremets._dataRecived;
  buffer.reserve(200);

  StaticJsonDocument<192> doc;

  JsonObject dateTime = doc.createNestedObject("dateTime");
  dateTime["date"] = timeNow->mDate;
  dateTime["time"] = timeNow->mTime;

  JsonObject Anemometer = doc.createNestedObject("Anemometer");
  Anemometer["Speed"] =  data->windCnt;  
  Anemometer["Direction"] =  data->windDir;

  JsonObject Inclinometer = doc.createNestedObject("Inclinometer");
  Inclinometer["V1"] = data->inc_XVal;
  Inclinometer["V2"] = data->inc_YVal;

  serializeJson(doc, buffer);

  return buffer;
}
void listSerialize(const std::vector<String>& tempList, char* buffer)
{
  strcpy(buffer, "[");
  for(auto& vec : tempList)
  {
   strcat(buffer, vec.c_str());
   if(&vec != &tempList.back())   strcat(buffer, ","); // add coma only when its not last element
  }
  strcat(buffer, "]");
  //RAM_CHECK; 
}
