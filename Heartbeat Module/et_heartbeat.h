#ifndef ET_HEARTBEAT_H
#define ET_HEARTBEAT_H

#include <stdint.h>
#include <stdbool.h>
#include "main.h" 


typedef struct et_heartbeat_ctx_t{

    GPIO_TypeDef *GPIO_port; // The port on the chip.
    uint16_t port_pin; // The pin on the specific port.

    int sequence_index;
    uint16_t heartbeat_sequence[4];
    
    // uint32_t is used as HAL_GetTick() returns uint32_t.
    uint32_t last_toggle_ms; // Records the timestamp of last toggle.

    bool LED_is_on; // Records current state of LED.

    // User is expected to provide LED_active_low according to hardware.
    bool LED_active_low; // Records whether LED should turn on at RESET

} et_heartbeat_ctx_t;

/**
 * @brief  Initializes a heartbeat context and turns the LED on.
 *
 * @param  heartbeat_ctx  Pointer to the heartbeat context. Must not be NULL.
 * @param  port           GPIO port of the LED. Must not be NULL.
 * @param  pin            GPIO pin mask of the LED, for example GPIO_PIN_5. Must not be zero.
 * @param  on_ms          Minimum time that the LED stays on, in milliseconds. Must be more than zero.
 * @param  off_ms         Minimum time that the LED stays off, in milliseconds. Must be more than zero.
 * @param  is_active_low  Set to true if the LED comes on when the pin is low.
 *                        Set to false if the LED comes on when the pin is high.
 *
 * @return 0 if the initialization is successful.
 * @return -1 if a parameter is not valid.
 *
 * @pre    The GPIO pin is configured as an output (for example by MX_GPIO_Init).
 * @post   On success, the LED is on and the context is ready for use.
 * @post   On failure, the context and the LED do not change.
 *
 * @note   Call this function one time, before you call et_beat_heart.
 */
int et_heartbeat_init(et_heartbeat_ctx_t *heartbeat_ctx, GPIO_TypeDef *port, uint16_t pin, bool is_active_low);

/**
 * @brief  Updates the heartbeat LED. Changes the LED state when the time of the current phase ends.
 *
 * @param  heartbeat_ctx  Pointer to the heartbeat context. Must not be NULL.
 *                        The context must be initialized with et_heartbeat_init.
 *
 * @return 0 if the function is successful.
 * @return -1 if heartbeat_ctx is NULL.
 *
 * @note   Call this function repeatedly from the main loop. Call it as often as possible.
 * @note   This function does not block. Do not use blocking delays in the main loop.
 *         A long delay in the main loop causes the LED phases to be longer than set.
 * @note   If the main loop stops, the LED stops. This is correct. It shows that the main loop does not run.
 *
 * @warning Do not call this function from an interrupt.
 *          Do not use the same context from the main loop and from an interrupt.
 */
int et_beat_heart(et_heartbeat_ctx_t *heartbeat_ctx);


#endif /* ET_HEARTBEAT_H */