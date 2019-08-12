#include <application.h>

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;

// LCD instance
bc_gfx_t *gfx;

// Led strip
static uint32_t _bc_module_power_led_strip_dma_buffer[LED_STRIP_COUNT * LED_STRIP_TYPE * 2];
const bc_led_strip_buffer_t led_strip_buffer =
{
    .type = LED_STRIP_TYPE,
    .count = LED_STRIP_COUNT,
    .buffer = _bc_module_power_led_strip_dma_buffer
};

void bc_change_lcd_text(uint64_t *id, const char *topic, void *value, void *param);

static const bc_radio_sub_t subs[] = 
{
    {"lcd/-/text/set", BC_RADIO_SUB_PT_STRING, bc_change_lcd_text, (void *) NULL}
};

static struct
{
    enum
    {
        LED_STRIP_SHOW_COLOR = 0,
        LED_STRIP_SHOW_COMPOUND = 1,
        LED_STRIP_SHOW_EFFECT = 2,

    } show;
    bc_led_strip_t self;
    uint32_t color;
    struct
    {
        uint8_t data[BC_RADIO_NODE_MAX_COMPOUND_BUFFER_SIZE];
        int length;
    } compound;

    bc_scheduler_task_id_t update_task_id;

} led_strip = { .show = LED_STRIP_SHOW_COLOR, .color = 0 };

void bc_change_lcd_text(uint64_t *id, const char *topic, void *value, void *param)
{
    bc_gfx_clear(gfx);
    bc_gfx_draw_string(gfx, 20, 20, (char *)value, true);
    bc_gfx_update(gfx);
}

void bc_radio_node_on_led_strip_color_set(uint64_t *id, uint32_t *color)
{
    (void) id;

    bc_led_strip_effect_stop(&led_strip.self);

    bc_log_debug("%x", *color);

    led_strip.color = *color;

    led_strip.show = LED_STRIP_SHOW_COLOR;

    bc_led_strip_fill(&led_strip.self, led_strip.color);

    bc_scheduler_plan_now(led_strip.update_task_id);
}

void bc_radio_node_on_led_strip_brightness_set(uint64_t *id, uint8_t *brightness)
{
    (void) id;

    bc_led_strip_set_brightness(&led_strip.self, *brightness);

    bc_led_strip_fill(&led_strip.self, led_strip.color);

    bc_scheduler_plan_now(led_strip.update_task_id);
}

void led_strip_update_task(void *param)
{
    (void) param;

    if (!bc_led_strip_is_ready(&led_strip.self))
    {
        bc_scheduler_plan_current_now();

        return;
    }

    bc_led_strip_write(&led_strip.self);

    bc_scheduler_plan_current_relative(250);
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_strip_effect_theater_chase_rainbow(&led_strip.self, 20);
    }
    else if(event == BC_BUTTON_EVENT_RELEASE)
    {
        bc_radio_pub_push_button(0);
    }
}

void application_init(void)
{
    srand(time());

    // Initialize logging
    bc_log_init(BC_LOG_LEVEL_DUMP, BC_LOG_TIMESTAMP_ABS);

    // Initialize Button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_OFF);

    // Initalize Radio 
    bc_radio_init(BC_RADIO_MODE_NODE_LISTENING);
    bc_radio_pairing_request("color-game", VERSION);
    bc_radio_set_subs((bc_radio_sub_t *) subs, sizeof(subs)/sizeof(bc_radio_sub_t));


    // Initialize LCD
    bc_module_lcd_init();
    gfx = bc_module_lcd_get_gfx();

    // Initialize power module
    bc_module_power_init(); 
    bc_led_strip_init(&led_strip.self, bc_module_power_get_led_strip_driver(), &led_strip_buffer);
    led_strip.update_task_id = bc_scheduler_register(led_strip_update_task, NULL, 0);

    bc_led_strip_set_brightness(&led_strip.self, 50);

    bc_led_strip_effect_theater_chase_rainbow(&led_strip.self, 20);
}