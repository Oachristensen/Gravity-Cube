#include "esp_log.h"

#define FN_TAG "sim_functions"
#define SIM_DEBUG false
#define X_SIZE 8
#define Y_SIZE 8
#define Z_SIZE 8

#define MAX_LEDS X_SIZE *Y_SIZE *Z_SIZE

#define NUM_SIM 46

typedef struct Pixel {
    int x;
    int y;
    int z;
    bool value;
};

typedef struct MoveParams {
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

static void fill_array_with_int(int array[NUM_SIM], int num) {
    for (int i = 0; i < NUM_SIM; i++) {
        array[i] = num;
    }
}

// This doesnt work anymore
static int index_from_cords(int x, int y, int z) {
    int index = (x + (y * (Y_SIZE) + (z * X_SIZE * Y_SIZE)));
    return index;
}

// Takes in a pair of x_y values and a velocity, recursively checks down the velocity list to see if any spaces are free to move
// if a velocity is found it is returned, else returns -1 for a failure
static float can_move(float x, float y, float z, struct Pixel pixel_array[], int velocity, int index) {

    float static_x = pixel_array[index].x;
    float static_y = pixel_array[index].y;
    float static_z = pixel_array[index].z;
    if (SIM_DEBUG) {
        ESP_LOGI(FN_TAG, "static_x = %f", static_x);
        ESP_LOGI(FN_TAG, "static_y = %f", static_y);
        ESP_LOGI(FN_TAG, "static_z = %f", static_z);
    }
    for (float cur_vel = velocity; cur_vel > 0; cur_vel--) {

        float new_x = static_x + round(x * (cur_vel / velocity));
        float new_y = static_y + round(y * (cur_vel / velocity));
        float new_z = static_z + round(z * (cur_vel / velocity));

        // checking if new values are in the bounds of the pixel_array
        if (new_x > 7 || new_x < 0 || new_y > 7 || new_y < 0 || new_z < 0 || new_z > 7) {
            continue;
        }

        else if (pixel_array[index_from_cords(new_x, new_y, new_z)].value == false) {
            if (SIM_DEBUG) {
                ESP_LOGI(FN_TAG, "Pixel at %d passed check", index_from_cords(new_x, new_y, new_z));
            }
            return cur_vel;
        }
    }
    return -1;
}

// Takes a pair of x_y values and an index, sets the old indexes value to false, changes the new index to true, and returns it
static int move_pixel(int old_index, int new_x, int new_y, int new_z, struct Pixel pixel_array[]) {
    int new_index = index_from_cords(new_x, new_y, new_z);
    pixel_array[new_index].value = true;
    pixel_array[old_index].value = false;
    if (SIM_DEBUG) {
        ESP_LOGI(FN_TAG, "moved pixel %d to %d, using values X: %d Y: %d", old_index, new_index, new_x, new_y);
    }
    return new_index;
}

static void configure_pixels(struct Pixel pixel_array[]) {
    for (int z = 0; z < Z_SIZE; z++) {
        for (int y = 0; y < Y_SIZE; y++) {
            for (int x = 0; x < X_SIZE; x++) {
                pixel_array[index_from_cords(x, y, z)].x = x;
                pixel_array[index_from_cords(x, y, z)].y = y;
                pixel_array[index_from_cords(x, y, z)].z = z;
                pixel_array[index_from_cords(x, y, z)].value = false;
                if (SIM_DEBUG) {
                    ESP_LOGI(FN_TAG, "%d", index_from_cords(x, y, z));
                }
            }
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
static struct MoveParams set_move_params(float theta, float phi, float velocity) {
    // effectively a 5x5 unit square where each value is rounded
    struct MoveParams move_params = {
        .x_down = velocity * cos(theta) * cos(phi),
        .x_right = velocity * sin(theta) * cos(phi),
        .y_down = velocity * cos(theta) * sin(phi),
        .y_right = velocity * sin(theta) * sin(phi),
        .z_down = velocity * sin(phi),
        .z_left = velocity * cos(theta + M_PI_2) * cos(phi),
        .z_right = velocity * cos(theta - M_PI_2) * cos(phi),
    };
    return move_params;
}

// runs a bunch of logic on the pixel_array
static void run_sim(struct Pixel pixel_array[], float theta, float phi) {

    // eventually I will change this based on a real value
    float velocity = 2;

    // keeps track of pixels already moved so that things cant move twice in a cycle
    int counter = 0;
    int selected_indexes[NUM_SIM];

    fill_array_with_int(selected_indexes, -1);

    struct MoveParams move_params = set_move_params(theta, phi, velocity);
    if (SIM_DEBUG) {
        ESP_LOGI(FN_TAG, "x_left: %f", move_params.x_left);
        ESP_LOGI(FN_TAG, "x_right: %f", move_params.x_right);
        ESP_LOGI(FN_TAG, "x_down: %f", move_params.x_down);
        ESP_LOGI(FN_TAG, "y_left: %f", move_params.y_left);
        ESP_LOGI(FN_TAG, "y_right: %f", move_params.y_right);
        ESP_LOGI(FN_TAG, "y_down: %f", move_params.y_down);
    }
    for (int i = 0; i < MAX_LEDS; i++) {

        bool left = false;
        bool right = false;
        int new_index = -1;

        if (array_contains(i, selected_indexes)) {
            if (SIM_DEBUG) {
                ESP_LOGI(FN_TAG, "pixel at %d skipped", i);
            }
            continue;
        }
        // Checks for active pixel, moves on if not
        if (pixel_array[i].value == true) {
            if (SIM_DEBUG) {
                ESP_LOGI(FN_TAG, "pixel at %d selectd", i);
            }
            // decides whether to move left or right if both are availible
            switch (esp_random() % 2) {
            case 0:
                left = true;
                if (SIM_DEBUG) {
                    ESP_LOGI(FN_TAG, "selected left");
                }
                break;
            case 1:
                right = true;
                if (SIM_DEBUG) {
                    ESP_LOGI(FN_TAG, "selected right");
                }
                break;
            }

            // down
            float set_velocity_down = can_move(move_params.x_down, move_params.y_down, move_params.z_down, pixel_array, velocity, i);
            if (SIM_DEBUG) {
                ESP_LOGI(FN_TAG, "down_velocity = %f", set_velocity_down);
            }
            if (set_velocity_down != -1) {
                int new_x = pixel_array[i].x + round(move_params.x_down * (set_velocity_down / velocity));
                int new_y = pixel_array[i].y + round(move_params.y_down * (set_velocity_down / velocity));
                int new_z = pixel_array[i].z + round(move_params.z_down * (set_velocity_down / velocity));

                new_index = move_pixel(i, new_x, new_y, new_z, pixel_array);
                selected_indexes[counter] = new_index;
                counter++;
                continue;
            }
            // right first then left
            if (right) {
                float set_velocity_right = can_move(move_params.x_right, move_params.y_right, move_params.z_right, pixel_array, velocity, i);
                if (SIM_DEBUG) {
                    ESP_LOGI(FN_TAG, "right_velocity = %f", set_velocity_right);
                }
                if (set_velocity_right != -1) {
                    int new_x = pixel_array[i].x + round(move_params.x_right * (set_velocity_right / velocity));
                    int new_y = pixel_array[i].y + round(move_params.y_right * (set_velocity_right / velocity));
                    int new_z = pixel_array[i].z + round(move_params.z_right * (set_velocity_right / velocity));

                    new_index = move_pixel(i, new_x, new_y, new_z, pixel_array);
                    selected_indexes[counter] = new_index;
                    counter++;
                    continue;
                }
            } else {
                float set_velocity_left = can_move(move_params.x_left, move_params.y_left, move_params.z_left, pixel_array, velocity, i);
                if (SIM_DEBUG) {
                    ESP_LOGI(FN_TAG, "left_velocity = %f", set_velocity_left);
                }
                if (set_velocity_left != -1) {
                    int new_x = pixel_array[i].x + round(move_params.x_left * (set_velocity_left / velocity));
                    int new_y = pixel_array[i].y + round(move_params.y_left * (set_velocity_left / velocity));
                    int new_z = pixel_array[i].z + round(move_params.z_left * (set_velocity_left / velocity));

                    new_index = move_pixel(i, new_x, new_y, new_z, pixel_array);
                    selected_indexes[counter] = new_index;
                    counter++;
                    continue;
                }
            }
            // left first then right
            if (left) {
                float set_velocity_left = can_move(move_params.x_left, move_params.y_left, move_params.z_left, pixel_array, velocity, i);
                if (SIM_DEBUG) {
                    ESP_LOGI(FN_TAG, "left_velocity = %f", set_velocity_left);
                }

                if (set_velocity_left != -1) {
                    int new_x = pixel_array[i].x + round(move_params.x_left * (set_velocity_left / velocity));
                    int new_y = pixel_array[i].y + round(move_params.y_left * (set_velocity_left / velocity));
                    int new_z = pixel_array[i].z + round(move_params.z_left * (set_velocity_left / velocity));

                    new_index = move_pixel(i, new_x, new_y, new_z, pixel_array);
                    selected_indexes[counter] = new_index;
                    counter++;
                    continue;
                }
            } else {
                float set_velocity_right = can_move(move_params.x_right, move_params.y_right, move_params.z_right, pixel_array, velocity, i);
                if (SIM_DEBUG) {
                    ESP_LOGI(FN_TAG, "right_velocity = %f", set_velocity_right);
                }

                if (set_velocity_right != -1) {
                    int new_x = pixel_array[i].x + round(move_params.x_right * (set_velocity_right / velocity));
                    int new_y = pixel_array[i].y + round(move_params.y_right * (set_velocity_right / velocity));
                    int new_z = pixel_array[i].z + round(move_params.z_right * (set_velocity_right / velocity));

                    new_index = move_pixel(i, new_x, new_y, new_z, pixel_array);
                    selected_indexes[counter] = new_index;
                    counter++;
                    continue;
                }
            }
        }
        if (SIM_DEBUG) {
            ESP_LOGI(FN_TAG, "new_op");
        }
    }
}
