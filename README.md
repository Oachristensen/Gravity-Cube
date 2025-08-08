# Gravity Cube
### A Physics simulation running on an esp32 that displays on a cube of led panels

[example video]

### Hardware
The simulation runs on an Esp32s3 that gets data from an ICM-20948 IMU and sends it through a level shifter to 8 ws2812b led panels in series
![[Led_Cube_block_diagram.png]]
#### More detail here
[PCB schematic]

### Software
The program is written in C and compiled using the Espressif build system
[Buildguide]

