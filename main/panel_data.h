#define N 1
#define E 2
#define S 3
#define W 4
#define UP 5
#define DOWN 6

#define PANEL_TAG "panel_data"

// Cube has z direction with South being +, y direction with Up being +, and x with East being +

typedef struct led_panel {
    int panel_num;         // 0-5
    int panel_orientation; // 0 90, 180, 270
    int panel_direction;   // N E S W UP DOWN
};

// EDIT THIS ONCE THE CUBE IS BUILT
struct led_panel panel_array[6] = {
    {0, 0, N},
    {1, 0, E},
    {2, 0, S},
    {3, 0, W},
    {4, 0, UP},
    {5, 0, DOWN}};

// lookup table for matrix position to panel direction
uint8_t cord_to_panel_lookup[8][8][8];

// builds panel lookup data
void init_panel_lookup() {
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            for (int z = 0; z < 8; z++) {
                uint8_t mask = 0; // Start with no panels

                // Set bit flags for the panels this pixel touches
                if (x == 7)
                    mask |= 0b000001; // EAST panel
                if (x == 0)
                    mask |= 0b000010; // WEST panel
                if (y == 0)           // TODO: MIGHT HAVE TO INVERT Y
                    mask |= 0b000100; // DOWN panel
                if (y == 7)
                    mask |= 0b001000; // UP panel
                if (z == 7)
                    mask |= 0b010000; // S panel
                if (z == 0)
                    mask |= 0b100000; // N panel

                cord_to_panel_lookup[x][y][z] = mask;
            }
        }
    }
}
// Takes an x y z value and returns the panel data from cord_to_panel_lookup
uint8_t get_panels(int x, int y, int z) {
    return cord_to_panel_lookup[x][y][z];
}

void draw_panels(int x, int y, int z) {
    uint8_t panels = get_panels(x, y, z);
    if (panels & 0b000001)
        draw_panels(E, y, z);
    if (panels & 0b000010)
        draw_panels(W, y, z);
    if (panels & 0b000100)
        draw_panels(DOWN, x, y); // TODO: MAY HAVE TO INVERT THESE ALSO
    if (panels & 0b001000)
        draw_panels(UP, x, y);
    if (panels & 0b010000)
        draw_panels(S, x, z);
    if (panels & 0b100000)
        draw_panels(N, x, z);
}
//
void draw_on_panels(int direction, int x, int y) {
    int selected_panel = -1;

    int new_x;
    int new_y;
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
    int led_index = (0 + 64 * panel_array[selected_panel].panel_num) + (x + y * Y_SIZE);
    led_strip_set_pixel(led_strip, led_index, 0, 0, 0);
}
