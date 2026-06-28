#ifndef CONFIG_H
#define CONFIG_H

//LED Color def (if on battery do not go too high as to draw too much current)
#define R 10
#define G 10
#define B 10

//Matrix size def (probably won't ever change)
#define X_SIZE 8
#define Y_SIZE 8
#define Z_SIZE 8

//Pin config def
#define DATA_GPIO 37
#define MAX_LEDS (Y_SIZE*X_SIZE*6)

// Delay between sim cycles (gets flickery below 25)
#define DELAY 20
#define SHAKE_FLASH_DELAY 100

//Debug settings (output debug data to serial monitor)
#define MAIN_DEBUG false
#define PANEL_DEBUG false
#define SIM_DEBUG false
// way too much info when at full speed, slow down DELAY if you want to test
#define SIM_DEBUG_DETAILED false

//Amount of simulated pixels
#define NUM_SIM 200

//Toggles flashing on shake
#define SHAKE_ENABLE true
// The range for when shake flashing starts, 32767 = 2g, around 35000 is a good range that allows tilting and movement without triggering shake
#define SHAKE_CUTOFF 35000
#endif  