#include "esp_log.h"


#define FN_TAG "sim_functions"

#define X_SIZE 8
#define Y_SIZE 8

#define MAX_LEDS X_SIZE*Y_SIZE

#define NUM_SIM 32

typedef struct Pixel
{
 int x;
 int y;
 bool value;
};



static int index_from_cords(int x, int y) {
    return(x+(y*(Y_SIZE)));
}

static bool can_move(int x,int y, struct Pixel pixel_array[]) {
    if (pixel_array[index_from_cords(x, y)].value == true) {
        return false;
    }
    else {
        return true;
    }
}

static int move_pixel(int old_index, int move_x, int move_y, struct Pixel pixel_array[]) {
    int new_x = (pixel_array[old_index].x + move_x);
    int new_y = (pixel_array[old_index].y + move_y);
    int new_index = index_from_cords(new_x , new_y);
    pixel_array[new_index].value = true;
    pixel_array[old_index].value = false;
    ESP_LOGI(FN_TAG, "moved pixel %d to %d", old_index, new_index);
    return new_index;
}


static void configure_pixels(struct Pixel pixel_array[]) {
    for (int y = 0; y < Y_SIZE; y++) {
         for (int x = 0; x < X_SIZE; x++) {
            pixel_array[index_from_cords(x, y)].x = x;
            pixel_array[index_from_cords(x,y)].y = y;
            pixel_array[index_from_cords(x,y)].value = false;

        }
    }
}
static bool array_contains(int num, int array[NUM_SIM]) {
    for (int i = 0; i < NUM_SIM; i++ ) {
        if (array[i] == num) {
            return true;
        }
    }
    return false;
}

// A lot of the logic here is jank but I am leaving it so it can be changed easier later
static void run_sim(struct Pixel pixel_array[]) {
    // I dont like counter here but it should work
    int counter = 0;
    int selected_indexes[NUM_SIM] = {-1};
    bool left = false;
    bool right = false;

    
    for (int i = 0; i < MAX_LEDS; i++) {
        switch (esp_random()%2) {
            case 0:
                left = true;
                break;
            case 1:
                right = true;
                break;
            }

        int new_index = -1;
        if (array_contains(i, selected_indexes)) {
            continue;
        }
        if (pixel_array[i].value == true) {
            if (pixel_array[i].y != 7) {
                if (can_move(pixel_array[i].x ,pixel_array[i].y +1, pixel_array)) {
                    new_index = move_pixel(i, 0, 1, pixel_array);
                }

            }
        }
        if (right) {
            if (pixel_array[i].value == true) {
                if (pixel_array[i].x != 7) {
                    if (can_move(pixel_array[i].x +1 ,pixel_array[i].y, pixel_array)) {
                        new_index = move_pixel(i, 1, 0, pixel_array);
                    }
                }   
            }
            if (pixel_array[i].value == true) {
                if (pixel_array[i].x != 0) {
                    if (can_move(pixel_array[i].x -1 ,pixel_array[i].y, pixel_array)) {
                        new_index = move_pixel(i, -1, 0, pixel_array);
                    }
                }
            }
        }
        if (left) {
            if (pixel_array[i].value == true) {
                if (pixel_array[i].x != 0) {
                    if (can_move(pixel_array[i].x -1 ,pixel_array[i].y, pixel_array)) {
                        new_index = move_pixel(i, -1, 0, pixel_array);
                    }
                }
            }
            if (pixel_array[i].value == true) {
                if (pixel_array[i].x != 7) {
                    if (can_move(pixel_array[i].x +1 ,pixel_array[i].y, pixel_array)) {
                        new_index = move_pixel(i, 1, 0, pixel_array);
                    }
                }   
            }
        }  
        
        if (new_index != -1) {
            selected_indexes[counter] = new_index;
            counter++;
        }
    }
    ESP_LOGI(FN_TAG, "new_op");
}

