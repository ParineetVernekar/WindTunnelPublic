# BASE CODE FOR ALL THINGS

## To use for a new sensor
- go to main.ino file
- setup device specific code by
1. edit the device specific code before the WebServer is setup
2. start the new sensor and run initialisation in setup()
3. edit queue.h and queue.cpp -> e.g for the ConditionThing -> change QueueData structure to have value_humidity, value_temperature, etc
4. edit the main.ino publishData() function to create a JSON document with the same elements as QueueData
5. edit the GetDataCode() function in order to get data and push it into the dataQueue
