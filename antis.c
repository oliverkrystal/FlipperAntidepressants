#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include "antis_icons.h"
#include "dolphin_state.h"
#include "saved_struct.h"
#include <power/power_service/power.h>

const char* funnyText[] =
{"Stop poking my brain",
 "You're a terrible owner",
 "I'll remember this!",
 "This really isnt ok",
 "Just feed me RFID cards!",
 "Forget to charge me too?"};

int funnyTextIndex = 0;

DolphinState* stateLocal = 0;
char strButthurt[16];
char strXp[16];
int btnIndex = 0;

static void draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);

    //graphics
    canvas_clear(canvas);
    canvas_draw_frame(canvas, 0, 0, 128, 64);
    canvas_draw_icon(canvas, 3, 12, &I_passport_bad1_46x49);
    if(btnIndex == 0){
        canvas_draw_icon(canvas, 110, 15, &I_ButtonLeftSmall_3x5);
    }
    else if(btnIndex == 1){
        canvas_draw_icon(canvas, 110, 25, &I_ButtonLeftSmall_3x5);
    }

    //strings
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 3, 9, funnyText[funnyTextIndex]);
    snprintf(strButthurt, 16, "Set anger @ %lu", stateLocal->data.butthurt);
    snprintf(strXp, 16, "XP: %lu", stateLocal->data.icounter);
    canvas_draw_str(canvas, 51, 21, strButthurt);
    canvas_draw_str(canvas, 51, 31, strXp);

    //save button
    if(btnIndex == 2){
        canvas_draw_rbox(canvas, 51, 46, 74, 15, 3);
        canvas_invert_color(canvas);
        canvas_draw_str_aligned(canvas, 88, 53, AlignCenter, AlignCenter, "Save & Reboot");
        canvas_invert_color(canvas);
        canvas_draw_rframe(canvas, 51, 46, 74, 15, 3);
    }
    else{
        canvas_invert_color(canvas);
        canvas_draw_rbox(canvas, 51, 46, 74, 15, 3);
        canvas_invert_color(canvas);
        canvas_draw_str_aligned(canvas, 88, 53, AlignCenter, AlignCenter, "Save & Reboot");
        canvas_draw_rframe(canvas, 51, 46, 74, 15, 3);
    }
}

static void input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t bigsad_app(void* p) {
    UNUSED(p);

    InputEvent event;
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, NULL);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    funnyTextIndex = rand() % 5;

    stateLocal = malloc(sizeof(DolphinState));

    bool running = true;
    bool success = saved_struct_load("/int/.dolphin.state", &stateLocal->data, sizeof(DolphinStoreData), 0xD0, 0x01);
    if(!success){
        running = false;
    }

    while(running) {
        furi_check(furi_message_queue_get(event_queue, &event, FuriWaitForever) == FuriStatusOk);
        if(event.type == InputTypePress){
            if(event.key == InputKeyOk && btnIndex == 2) {
                bool result = saved_struct_save("/int/.dolphin.state", &stateLocal->data, sizeof(DolphinStoreData), 0xD0, 0x01);
                if(result){
                    furi_delay_ms(100);
                    furi_hal_power_reset();
                    running = false;
                    return 0;
                }
            }
            if(event.key == InputKeyUp) {
                if(btnIndex == 0){ btnIndex = 2; }
                else{ btnIndex--; }
            }
            if(event.key == InputKeyDown) {
                if(btnIndex == 2){ btnIndex = 0; }
                else{ btnIndex++; }
            }
            if(event.key == InputKeyRight){
                if(btnIndex == 0 && stateLocal->data.butthurt < 14){ stateLocal->data.butthurt++; }
                else if(btnIndex == 1){ stateLocal->data.icounter += 10; }
            }
            if(event.key == InputKeyLeft){
                if(btnIndex == 0 && stateLocal->data.butthurt > 0){ stateLocal->data.butthurt--; }
                else if(btnIndex == 1){
                    if(stateLocal->data.icounter < 10){ stateLocal->data.icounter = 0; }
                    else{ stateLocal->data.icounter -= 10; }
                }
            }
            if(event.key == InputKeyBack) {
                running = false;
            }
        }
    }

    furi_message_queue_free(event_queue);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);

    return 0;
}
