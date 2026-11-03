#include "et_heartbeat.h"

#define ET_LED_TURN_ON 1
#define ET_LED_TURN_OFF 0

static void write_led(et_heartbeat_ctx_t *heartbeat_ctx, GPIO_TypeDef *port, uint16_t pin, bool turn_on) {
    if (heartbeat_ctx->LED_active_low) {
        if (turn_on) HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
        else HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
    }
    else if (!heartbeat_ctx->LED_active_low) {
        if (turn_on) HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
    }
}

int et_heartbeat_init(et_heartbeat_ctx_t *heartbeat_ctx, GPIO_TypeDef *port, uint16_t pin, bool is_active_low) {

    if (heartbeat_ctx == NULL) {
        return -1;
    }

    if (port == NULL) {
        return -1;
    }

    if (pin == 0) {
        return -1;
    }

    if (on_ms == 0 || off_ms == 0) {
        return -1;
    }

    heartbeat_ctx->GPIO_port = port;
    heartbeat_ctx->port_pin = pin;

    heartbeat_ctx->sequence_index = 0;
    heartbeat_ctx->heartbeat_sequence[0] = 80;
    heartbeat_ctx->heartbeat_sequence[1] = 100;
    heartbeat_ctx->heartbeat_sequence[2] = 80;
    heartbeat_ctx->heartbeat_sequence[3] = 740;

    heartbeat_ctx->last_toggle_ms = HAL_GetTick();

    if (is_active_low != 0 && is_active_low != 1) {
        return -1;
    }
    heartbeat_ctx->LED_active_low = is_active_low;

    // Turn the LED on.
    write_led(heartbeat_ctx, port, pin, ET_LED_TURN_ON);

    heartbeat_ctx->LED_is_on = 1;

    return 0;

}


int et_beat_heart(et_heartbeat_ctx_t *heartbeat_ctx) {

    if (heartbeat_ctx == NULL) {
        return -1;
    }

    uint32_t current_time = HAL_GetTick();

    if ((current_time - heartbeat_ctx->last_toggle_ms) >= heartbeat_ctx->heartbeat_sequence[heartbeat_ctx->sequence_index]) {

        if (heartbeat_ctx->sequence_index == 0 || heartbeat_ctx->sequence_index == 2) write_led(heartbeat_ctx, heartbeat_ctx->GPIO_port, heartbeat_ctx->port_pin, ET_LED_TURN_OFF);
        else write_led(heartbeat_ctx, heartbeat_ctx->GPIO_port, heartbeat_ctx->port_pin, ET_LED_TURN_ON);
    
        heartbeat_ctx->last_toggle_ms = current_time;
        heartbeat_ctx->sequence_index = (heartbeat_ctx->sequence_index + 1) % 4;

    }

    return 0;

}