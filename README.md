# Gravity Cube
### A Physics simulation running on an esp32 that displays on a cube of led panels

![alt text](https://github.com/Oachristensen/Gravity-Cube/blob/master/Showcase.gif)

### Hardware
The simulation runs on an Esp32s3 that gets data from an ICM-20948 IMU and sends it through a level shifter to 8 ws2812b led panels in series
![alt text](https://github.com/Oachristensen/Gravity-Cube/blob/master/Led_Cube_block_diagram.png)
#### More detail here
[PCB schematic]

### Software
The program is written in C and compiled using the Espressif build system and the Espressif led_strip library

[Buildguide]

