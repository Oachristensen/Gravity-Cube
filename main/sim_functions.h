#include "esp_log.h"

#define FN_TAG "sim_functions"

#define X_SIZE 8
#define Y_SIZE 8

#define MAX_LEDS X_SIZE *Y_SIZE

#define NUM_SIM 32

typedef struct Pixel {
    int x;
    int y;
    bool value;
};

typedef struct MoveParams {
    float x_down;
    float x_right;
    float x_left;
    float y_down;
    float y_right;
    float y_left;
};

static void fill_array_with_int(int array[NUM_SIM], int num) {
    for (int i = 0; i < NUM_SIM; i++) {
        array[i] = num;
    }
}

static int index_from_cords(int x, int y) {
    return (x + (y * (Y_SIZE)));
}

static bool can_move(int x, int y, struct Pixel pixel_array[]) {
    if (pixel_array[index_from_cords(x, y)].value == true) {
        return false;
    } else {
        return true;
    }
}

static int move_pixel(int old_index, int x_mult, int y_mult, struct Pixel pixel_array[]) {
    int new_x = (pixel_array[old_index].x + x_mult);
    int new_y = (pixel_array[old_index].y + y_mult);
    int new_index = index_from_cords(new_x, new_y);
    pixel_array[new_index].value = true;
    pixel_array[old_index].value = false;
    ESP_LOGI(FN_TAG, "moved pixel %d to %d", old_index, new_index);
    return new_index;
}

static void configure_pixels(struct Pixel pixel_array[]) {
    for (int y = 0; y < Y_SIZE; y++) {
        for (int x = 0; x < X_SIZE; x++) {
            pixel_array[index_from_cords(x, y)].x = x;
            pixel_array[index_from_cords(x, y)].y = y;
            pixel_array[index_from_cords(x, y)].value = false;
        }
    }
}
static bool array_contains(int num, int array[NUM_SIM]) {
    for (int i = 0; i < NUM_SIM; i++) {
        if (array[i] == num) {
            return true;
        }
    }
    return false;
}

// sets all of move_params to proper values
static struct MoveParams set_move_params(int angle) {
    // effectively a 5x5 unit square where each value is rounded, will have bugs because of moving 2x need to be fixed
    struct MoveParams move_params = {
        .x_down = round(2* sin(angle + -90)),
        .x_left = round(2* sin(angle + 180)),
        .x_right = round(2* sin(angle)),
        .y_down = round(2*cos(angle - 90)),
        .y_left = round(2* cos(angle + 180)),
        .y_right = round(2* cos(angle)),
};
return move_params;
}


// A lot of the logic here is jank but I am leaving it so it can be changed easier later
static void run_sim(struct Pixel pixel_array[], int angle) {
    // I dont like counter here but it should work
    int counter = 0;
    int selected_indexes[NUM_SIM];
    fill_array_with_int(selected_indexes, -1);
    bool left = false;
    bool right = false;

    struct MoveParams move_params = set_move_params(angle);

    for (int i = 0; i < MAX_LEDS; i++) {
        // decides whether to move left or right if both are availible
        switch (esp_random() % 2) {
        case 0:
            left = true;
            break;
        case 1:
            right = true;
            break;
        }


        int new_index = -1;
        if (array_contains(i, selected_indexes)) {
            ESP_LOGI(FN_TAG, "pixel at %d skipped", i);
            continue;
        }
        if (pixel_array[i].value == true) {
            if (pixel_array[i].y != 7) {
                if (can_move(pixel_array[i].x + move_params.x_down, pixel_array[i].y + move_params.y_down, pixel_array)) {
                    ESP_LOGI(FN_TAG, "pixel at %d passed y check", i);
                    new_index = move_pixel(i, move_params.x_down, move_params.y_down, pixel_array);
                }
            }
        }
        if (right) {
            if (pixel_array[i].value == true) {
                if (pixel_array[i].x != 7) {
                    ESP_LOGI(FN_TAG, "pixel at %d passed right check", i);

                    if (can_move(pixel_array[i].x + move_params.x_right, pixel_array[i].y + move_params.y_right, pixel_array)) {
                        new_index = move_pixel(i, 1, 0, pixel_array);
                    }
                }
            }
            if (pixel_array[i].value == true) {
                if (pixel_array[i].x != 0) {
                    if (can_move(pixel_array[i].x + move_params.x_left, pixel_array[i].y + move_params.y_left, pixel_array)) {
                        new_index = move_pixel(i, -1, 0, pixel_array);
                    }
                }
            }
        }
        if (left) {
            if (pixel_array[i].value == true) {
                if (pixel_array[i].x != 0) {
                    if (can_move(pixel_array[i].x + move_params.x_left, pixel_array[i].y + move_params.y_left, pixel_array)) {
                        new_index = move_pixel(i, -1, 0, pixel_array);
                    }
                }
            }
            if (pixel_array[i].value == true) {
                if (pixel_array[i].x != 7) {
                    if (can_move(pixel_array[i].x + move_params.x_right, pixel_array[i].y + move_params.y_right, pixel_array)) {
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
