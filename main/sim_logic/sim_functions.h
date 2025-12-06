
// header def
#ifndef SIM_FUNCTIONS_H
#define SIM_FUNCTIONS_H

#include "esp_log.h"
#include <stdbool.h>
#include "config.h"

#define FN_TAG "sim_functions"

#define MAX_PIXELS X_SIZE * Y_SIZE * Z_SIZE


struct Pixel {
    int x;
    int y;
    int z;
    bool value;
};

struct MoveParams {
    float x_down;
    float x_right;
    float x_left;
    float y_down;
    float y_right;
    float y_left;
    float z_down;
    float z_left;
    float z_right;
};
typedef struct my_vector {
    float x;
    float y;
    float z;
    float magnitude;
};
extern void fill_array_with_int(int array[NUM_SIM], int num);

extern int index_from_cords(int x, int y, int z);

// Takes in a pair of x_y values and a velocity, recursively checks down the velocity list to see if any spaces are free to move
// if a velocity is found it is returned, else returns -1 for a failure
extern float can_move(float x, float y, float z, struct Pixel pixel_array[], int velocity, int index);

// Takes a pair of x_y values and an index, sets the old indexes value to false, changes the new index to true, and returns it
extern int move_pixel(int old_index, int new_x, int new_y, int new_z, struct Pixel pixel_array[]);

extern void configure_pixels(struct Pixel pixel_array[]);

extern bool array_contains(int num, int array[NUM_SIM]);

// sets all of move_params to proper values based on unit vector
extern struct MoveParams set_move_params(struct my_vector unit_vector, float velocity);

// runs a bunch of logic on the pixel_array
extern void run_sim(struct Pixel pixel_array[], struct my_vector unit_vector);

#endif