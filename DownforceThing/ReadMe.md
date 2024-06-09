# Downforce Thing

## About
This code is designed for the DownforceThing on an ESP32. It has implementation for a HX711 load cell amplifier.

## Pins
- GPIO 15 -> HX711 DOUT 
- GPIO 4 -> HX711 SCK

## Tasks
This code uses FreeRTOS tasks, and has 4 defined tasks
1. PushTask -> this task pushes data from the queue, into AWS IoT
2. GetTask -> this task gets data from the respective sensors and adds it to the queue
3. TimerTask -> this task runs every second and increments the timestamp
4. OTATask -> this task doesn't run during a data run, but allows you to upload new firmware remotely