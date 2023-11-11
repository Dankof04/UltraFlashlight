// by @Dankof04
// this is a test for me to try creating a new app for my own flipper zero

#include <furi.h>
#include <furi_hal_power.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <gui/elements.h>

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    FuriMutex* mutex;
    bool is_on;
} PluginState;

static void render_callback(Canvas* const canvas, void* ctx) {
    furi_assert(ctx);
    const PluginState* plugin_state = ctx;
    furi_mutex_acquire(plugin_state->mutex, FuriWaitForever);

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 64, 2, AlignCenter, AlignTop, "UltraFlashlight");

    canvas_set_font(canvas, FontSecondary);

    if(!plugin_state->is_on) {
        elements_multiline_text_aligned(
            canvas, 64, 28, AlignCenter, AlignTop, "Press OK button to turn ultraflashlight on");
    } else {
        elements_multiline_text_aligned(canvas, 64, 28, AlignCenter, AlignTop, "Ultraflashlight is on :) !");
        elements_multiline_text_aligned(
            canvas, 64, 40, AlignCenter, AlignTop, "Press OK button to turn off");
    }

    furi_mutex_release(plugin_state->mutex);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void flash_toggle(PluginState* const plugin_state) {
    furi_hal_gpio_write_a7(&gpio_ext_pa7, false);
    furi_hal_gpio_write_a6(&gpio_ext_pa6, false);
    furi_hal_gpio_write_a4(&gpio_ext_pa4, false);
    furi_hal_gpio_write_b3(&gpio_ext_pb3, false);
    furi_hal_gpio_write_b2(&gpio_ext_pb2, false);
    furi_hal_gpio_write_c3(&gpio_ext_pc3, false);
    furi_hal_gpio_write_c1(&gpio_ext_pc1, false);
    furi_hal_gpio_write_c0(&gpio_ext_pc0, false);
    furi_hal_gpio_init_a7(&gpio_ext_pa7, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init_a6(&gpio_ext_pa6, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init_a4(&gpio_ext_pa4, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init_b3(&gpio_ext_pb3, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init_b2(&gpio_ext_pb2, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init_c3(&gpio_ext_pc3, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init_c1(&gpio_ext_pc1, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init_c0(&gpio_ext_pc0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    if(plugin_state->is_on) {
        furi_hal_gpio_write_a7(&gpio_ext_pa7, false);
        furi_hal_gpio_write_a6(&gpio_ext_pa6, false);
        furi_hal_gpio_write_a4(&gpio_ext_pa4, false);
        furi_hal_gpio_write_b3(&gpio_ext_pb3, false);
        furi_hal_gpio_write_b2(&gpio_ext_pb2, false);
        furi_hal_gpio_write_c3(&gpio_ext_pc3, false);
        furi_hal_gpio_write_c1(&gpio_ext_pc1, false);
        furi_hal_gpio_write_c0(&gpio_ext_pc0, false);
        plugin_state->is_on = false;
    } else {
        furi_hal_gpio_write_a7(&gpio_ext_pa7, true);
        furi_hal_gpio_write_a6(&gpio_ext_pa6, true);
        furi_hal_gpio_write_a4(&gpio_ext_pa4, true);
        furi_hal_gpio_write_b3(&gpio_ext_pb3, true);
        furi_hal_gpio_write_b2(&gpio_ext_pb2, true);
        furi_hal_gpio_write_c3(&gpio_ext_pc3, true);
        furi_hal_gpio_write_c1(&gpio_ext_pc1, true);
        furi_hal_gpio_write_c0(&gpio_ext_pc0, true);
        plugin_state->is_on = true;
    }
}


int32_t ultraflashlight_app() {
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));

    PluginState* plugin_state = malloc(sizeof(PluginState));

    plugin_state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if(!plugin_state->mutex) {
        FURI_LOG_E("ultraflashlight", "cannot create mutex\r\n");
        furi_message_queue_free(event_queue);
        free(plugin_state);
        return 255;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, plugin_state);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    PluginEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);

        furi_mutex_acquire(plugin_state->mutex, FuriWaitForever);

        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    case InputKeyUp:
                    case InputKeyDown:
                    case InputKeyRight:
                    case InputKeyLeft:
                        break;
                    case InputKeyOk:
                        flash_toggle(plugin_state);
                        break;
                    case InputKeyBack:
                        processing = false;
                        break;
                    default:
                        break;
                    }
                }
            }
        }

        view_port_update(view_port);
        furi_mutex_release(plugin_state->mutex);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_mutex_free(plugin_state->mutex);

    return 0;
}
