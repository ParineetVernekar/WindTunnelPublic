# Condition Thing

## About
This code is designed for the ConditionThing on an ESP32. It has implementation for a DHT11 sensor and a MPXV7002DP Pitot tube sensor. 

## Pins
- GPIO 13 -> DHT11 data
- GPIO 34 -> MPXV7002DP data

## Tasks
This code uses FreeRTOS tasks, and has 4 defined tasks
1. PushTask -> this task pushes data from the queue, into AWS IoT
2. GetTask -> this task gets data from the respective sensors and adds it to the queue
3. TimerTask -> this task runs every second and increments the timestamp
4. OTATask -> this task doesn't run during a data run, but allows you to upload new firmware remotely