# ESP32 Load Cell Calibration

This code is used to calibrate a HX711 load cell, and to get the scale factor required for calibration. This is very important to read accurate weights from the load cell, as every load cell can have a different scale factor, and the scale factor for one load cell can change over time.

## To use
1. Wire up your HX711 to your board, with DOUT to GPIO 15 and SCK to GPIO 4
2. Flash your board with this code
3. When prompted in serial monitor, add a known weight to your load cell
4. Wait for the reading to come through, and remove
5. Repeat this a few times, and take an average value
6. Divide this value by your known weight
7. Use your calculated value as your scale factor.