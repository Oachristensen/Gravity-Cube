
// header def
#ifndef PANEL_DATA_H
#define PANEL_DATA_H

#define N 1
#define E 2
#define S 3
#define W 4
#define UP 5
#define DOWN 6

#define PANEL_TAG "panel_data"
#include "config.h"
#include "led_strip.h"
#include <stdint.h>

#define PANEL_DEBUG false

// Cube has z direction with up being +, y direction with North being +, and x with East being +

typedef struct led_panel {
    int panel_num;         // 0-5
    int panel_orientation; // 0 90, 180, 270
    int panel_direction;   // N E S W UP DOWN
    bool inverted_x;       // true false
    bool inverted_y;       // true false
};

// lookup table for matrix position to panel direction
extern uint8_t cord_to_panel_lookup[8][8][8];

// builds panel lookup data
void init_panel_lookup();

// Takes an x y z value and returns the panel data from cord_to_panel_lookup
uint8_t get_panels(int x, int y, int z);

// TODO clean up logic
//  Takes a set of cordinates and a direction for a pixel and converts it to a panel index
void draw_on_panels(int direction, int x, int y, led_strip_handle_t led_strip);

// Reads lookup table for directions before drawing on respective panel
void draw_panels(int x, int y, int z, led_strip_handle_t led_strip);

#endif