# Fan Thing 8266

## About
This code is designed for the FanThing on an ESP8266, specifically a Wemos R1 board. It has implementation for a MOSFET PWM signal to be sent. It receives a FAN_CHANGE message from AWS IoT, and changes the fan speed to the value required.

## Pins
- GPIO D6 -> MOSFET Fan Output

## Tasks
This code uses FreeRTOS tasks, and has 4 defined tasks
1. PushTask -> this task pushes data from the queue, into AWS IoT
2. TimerTask -> this task runs every second and increments the timestamp
3. OTATask -> this task doesn't run during a data run, but allows you to upload new firmware remotely