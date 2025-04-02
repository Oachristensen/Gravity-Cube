#define N 1
#define E 2
#define S 3
#define W 4
#define UP 5
#define DOWN 6


#define PANEL_TAG "panel_data"

#define PANEL_DEBUG false

// Cube has z direction with up being +, y direction with North being +, and x with East being +

typedef struct led_panel {
    int panel_num;         // 0-5
    int panel_orientation; // 0 90, 180, 270
    int panel_direction;   // N E S W UP DOWN
    bool inverted_x; // true false
    bool inverted_y; // true false

};

// EDIT THIS ONCE THE CUBE IS BUILT
struct led_panel panel_array[6] = {
    {0, 0, W, false, false}, //good
    {1, 0, S, true, false}, //good
    {2, 0, N, false, false}, //good
    {3, 90, E, true, false}, //good
    {4, 90, UP, false, true}, //good
    {5, 270, DOWN, false, false}}; //good

// lookup table for matrix position to panel direction
uint8_t cord_to_panel_lookup[8][8][8];

// builds panel lookup data
void init_panel_lookup() {
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            for (int z = 0; z < 8; z++) {
                uint8_t mask = 0; // Start with no panels

                // Set bit flags for the panels this pixel touches
                if (x == 0)
                    mask |= 0b000001; // EAST panel
                if (x == 7)
                    mask |= 0b000010; // WEST panel
                if (y == 7)        
                    mask |= 0b000100; // SOUTH panel
                if (y == 0)
                    mask |= 0b001000; // NORTH panel
                if (z == 0)
                    mask |= 0b010000; // UP panel
                if (z == 7)
                    mask |= 0b100000; // DOWN panel

                cord_to_panel_lookup[x][y][z] = mask;
            }
        }
    }
}
// Takes an x y z value and returns the panel data from cord_to_panel_lookup
uint8_t get_panels(int x, int y, int z) {
    return cord_to_panel_lookup[x][y][z];
}

//TODO clean up logic
// Takes a set of cordinates and a direction for a pixel and converts it to a panel index
void draw_on_panels(int direction, int x, int y, led_strip_handle_t led_strip) {
    int selected_panel = -1;

    int new_x = x;
    int new_y = y;
    for (int i = 0; i < 6; i++) {
        if (panel_array[i].panel_direction == direction) {
            selected_panel = i;
            break;
        }
    }
    switch (panel_array[selected_panel].panel_orientation) {
    case -1:
        ESP_LOGI(PANEL_TAG, "panel not correctly selected");
        break;
    // inverts or switches cordinates based on panel orientation ex: x= 7-x flips x horizontally
    case 0:
        new_x = x;
        new_y = y;
        break;
    case 90:
        new_x = 7 - y;
        new_y = x;
        break;
    case 180:
        new_x = 7 - x;
        new_y = 7 - y;
        break;
    case 270:
        new_x = y;
        new_y = 7 - x;
        break;
    default:
        //! This should never happen
        ESP_LOGI(PANEL_TAG, "Invalid orientation entered, running as 0");
        new_x = x;
        new_y = y;
        break;
    }
    if (PANEL_DEBUG) {
        ESP_LOGI("paneldata", "old x: %d, old y: %d, Direction: %d", x, y, direction);
        ESP_LOGI("paneldata", "X: %d, Y: %d, Panel_num: %d", new_x, new_y, panel_array[selected_panel].panel_num);
    }
    if (panel_array[selected_panel].inverted_x) {
        new_x = 7-new_x;
    }
    if (panel_array[selected_panel].inverted_y) {
        new_y = 7-new_y;
    }
    int panel_num = panel_array[selected_panel].panel_num;
    int led_index = (0 + (64 * panel_num)) + (new_x + new_y * Y_SIZE);
    if (PANEL_DEBUG) {
        ESP_LOGI("paneldata", "led_index %d", led_index);
    }
    led_strip_set_pixel(led_strip, led_index, R, G, B);
}
//Reads lookup table for directions before drawing on respective panel
void draw_panels(int x, int y, int z, led_strip_handle_t led_strip) {
    uint8_t panels = get_panels(x, y, z);
    if (panels & 0b000001)
        draw_on_panels(E, y, z, led_strip);
    if (panels & 0b000010)
        draw_on_panels(W, y, z, led_strip);
    if (panels & 0b000100)
        draw_on_panels(S, x, z, led_strip);
    if (panels & 0b001000)
        draw_on_panels(N, x, z, led_strip);
    if (panels & 0b010000)
        draw_on_panels(UP, x, y, led_strip);
    if (panels & 0b100000)
        draw_on_panels(DOWN, x, y, led_strip);
}
//
