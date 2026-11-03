# et_heartbeat

A non-blocking "lub-dub" heartbeat LED for bare-metal STM32 firmware.

The LED beats twice, then rests, like a real heart. If the main loop stops, the heartbeat stops. This makes the LED a simple health indicator for your program.

Part of the `et` (embedded toolkit) project.

## Behavior

One cycle takes 1000 ms (about 60 beats per minute):

| Phase | LED  | Duration |
|-------|------|----------|
| 0     | on   | 80 ms    |
| 1     | off  | 100 ms   |
| 2     | on   | 80 ms    |
| 3     | off  | 740 ms   |

The pattern is fixed in `et_heartbeat_init()`.

## Why it exists

A heartbeat LED answers one question: **is the main loop still running?**

| Heartbeat      | Meaning                          |
|----------------|----------------------------------|
| Beats normally | The main loop runs               |
| Frozen         | The main loop is blocked or hung |

The function that updates the LED is called from the main loop, not from an interrupt. If an interrupt drove the LED, it would keep beating while the main loop is stuck, and it would give you false information.

## Requirements

- STM32 with the HAL driver (tested on STM32F103)
- A GPIO pin configured as a push-pull output (for example with STM32CubeMX)
- A working `HAL_GetTick()` (SysTick, 1 ms tick)
- `main.h`, which CubeMX generates, because the header includes it to get the HAL types

## Add it to your project

1. Copy `et_heartbeat.h` to `Core/Inc` and `et_heartbeat.c` to `Core/Src`.
2. If you use CMake, add the source file to `target_sources` in the top-level `CMakeLists.txt`:

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Core/Src/et_heartbeat.c
)
```

## Usage

```c
#include "et_heartbeat.h"

static et_heartbeat_ctx_t heartbeat;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    /* LED on PA2, lights when the pin is high (active-high). */
    if (et_heartbeat_init(&heartbeat, GPIOA, GPIO_PIN_2, false) != 0) {
        Error_Handler();
    }

    while (1) {
        et_beat_heart(&heartbeat);   /* call on every pass of the loop */
        /* ... other work. Do not block. ... */
    }
}
```

With CubeMX, write your code only between the `USER CODE BEGIN` and `USER CODE END` markers. Code outside the markers is deleted when you regenerate the project.

## API

### `int et_heartbeat_init(et_heartbeat_ctx_t *ctx, GPIO_TypeDef *port, uint16_t pin, bool is_active_low)`

Prepares the context and turns the LED on (start of phase 0).

| Parameter       | Description                                                   |
|-----------------|---------------------------------------------------------------|
| `ctx`           | Context to initialize. Must not be NULL.                      |
| `port`          | GPIO port of the LED. Must not be NULL.                       |
| `pin`           | GPIO pin mask, for example `GPIO_PIN_5`. Must not be zero.    |
| `is_active_low` | `true` if the LED lights when the pin is low.                 |

Returns `0` on success and `-1` if a parameter is not valid. On failure, the context and the LED do not change.

Call it one time, after `MX_GPIO_Init()` and before `et_beat_heart()`.

### `int et_beat_heart(et_heartbeat_ctx_t *ctx)`

Checks the time. If the current phase is over, it moves to the next phase and updates the LED. Otherwise it returns immediately.

Returns `0` on success and `-1` if `ctx` is NULL.

- Call it on every pass of the main loop.
- It does not block.
- Do not call it from an interrupt. Do not share one context between the main loop and an interrupt.

## Design notes

- **Non-blocking.** The function reads the clock, compares, and returns. It never waits.
- **Wrap-safe timing.** It uses `(uint32_t)(now - last) >= duration`. The result stays correct when the 32-bit millisecond counter wraps (about every 49.7 days).
- **State from the phase index.** Even phases are lit and odd phases are dark. The module stores no separate "LED is on" flag, so the LED state and the phase cannot disagree.
- **Polarity in one place.** The `is_active_low` setting is applied in one helper function, so the rest of the code uses logical on and off.
- **No dynamic memory, no globals.** The caller owns the context. You can run several heartbeats, one context each.
- **Resync on each phase change.** The start time of each phase is the time of the check, so the module does not "catch up" after a long delay in the main loop.

## Timing accuracy

Each phase lasts at least its nominal time. A phase can last longer by up to one pass of the main loop, because the module notices the end of a phase only when the function is called. A slow main loop makes the pattern slower.

## Limitations

- The pattern is fixed (80/100/80/740 ms). There is no run-time configuration yet.
- The header includes `main.h`, so the module depends on a CubeMX-generated project and cannot compile on a PC.
- One LED per context. The GPIO pin is not configured by the module.
- No unit tests yet.

## Roadmap

- [ ] Configurable pattern (array of phase durations)
- [ ] Hardware abstraction through function pointers (time source and LED write), so the logic compiles on a PC
- [ ] Host unit tests (phase order, timing edges, counter wraparound)
- [ ] Error-code LED module that works with this heartbeat

## Hardware note

Check your board's schematic for the LED polarity. If the beats look inverted (lit during the pauses), change `is_active_low`.