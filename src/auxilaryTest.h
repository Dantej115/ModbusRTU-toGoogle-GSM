#include <myRtc.h>
#include <Arduino.h>
// ############################## TESTING CASE #################
#define RAM_CHECK  ; RAM_Check(__func__, uxTaskGetStackHighWaterMark(NULL))
void RAM_Check(const char* name, size_t bytes)
{  
  Serial.printf("Task \"%s\" RAM Usage: %u bytes\n", name, bytes);
  size_t freeHeapSize = esp_get_free_heap_size();
  Serial.printf("Free Heap Size: %u bytes\n", freeHeapSize);
  Serial.println();
}

bool FakePost(const char* buffer)
{
  vTaskDelay(pdMS_TO_TICKS(2000));
  static bool result = true;
  result = !result;
  return result;
}

dateTime_t fakeTimeStamp()
{
  static int cnt = 0;
  dateTime_t temp;  
  snprintf(temp.time, 25, "%d:%d:%d", cnt++, cnt + 1, cnt + 2);  
  snprintf(temp.date, 25, "%d:%d:%d", cnt * 2 , cnt * 2 + 1, cnt * 10);
  return temp;
}