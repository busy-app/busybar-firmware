#ifndef SL_HAL_GPIO_INIT_EXP_3_CONFIG_H
#define SL_HAL_GPIO_INIT_EXP_3_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h>Pin settings

// <o SL_HAL_GPIO_INIT_EXP_3_MODE> Pin mode
// <SL_GPIO_MODE_DISABLED=> Disabled
// <SL_GPIO_MODE_INPUT=> Input
// <SL_GPIO_MODE_INPUT_PULL=> Input with pull-up/down
// <SL_GPIO_MODE_INPUT_PULL_FILTER=> Input with pull-up/down and filter
// <SL_GPIO_MODE_PUSH_PULL=> Push-pull output
// <SL_GPIO_MODE_PUSH_PULL_ALTERNATE=> Push-pull output (alternate)
// <SL_GPIO_MODE_WIRED_OR=> Open-source output
// <SL_GPIO_MODE_WIRED_OR_PULL_DOWN=> Open-source output with pull-down
// <SL_GPIO_MODE_WIRED_AND=> Open-drain output
// <SL_GPIO_MODE_WIRED_AND_FILTER=> Open-drain output with filter
// <SL_GPIO_MODE_WIRED_AND_PULL_UP=> Open-drain output with pull-up
// <SL_GPIO_MODE_WIRED_AND_PULL_UP_FILTER=> Open-drain output with pull-up and filter
// <SL_GPIO_MODE_WIRED_AND_ALTERNATE=> Open-drain output (alternate)
// <SL_GPIO_MODE_WIRED_AND_ALTERNATE_FILTER=> Open-drain output with filter (alternate)
// <SL_GPIO_MODE_WIRED_AND_ALTERNATE_PULL_UP=> Open-drain output with pull-up (alternate)
// <SL_GPIO_MODE_WIRED_AND_ALTERNATE_PULL_UP_FILTER=> Open-drain output with pull-up and filter (alternate)
// <i> Default: SL_GPIO_MODE_PUSH_PULL
#define SL_HAL_GPIO_INIT_EXP_3_MODE        SL_GPIO_MODE_PUSH_PULL

// <o SL_HAL_GPIO_INIT_EXP_3_DOUT> DOUT <0-1>
// <i> In push-pull mode: The drive direction for the pin
// <i> In input mode: Pull-up (1) or pull-down (0)
// <i> In open-source mode: Set to 0 for the idle state
// <i> In open-drain mode: Set to 1 for the idle state
// <i> Default: 0
#define SL_HAL_GPIO_INIT_EXP_3_DOUT        0

// </h> end pin settings

// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>

// <gpio> SL_HAL_GPIO_INIT_EXP_3
// $[GPIO_SL_HAL_GPIO_INIT_EXP_3]
#define SL_HAL_GPIO_INIT_EXP_3_PORT              SL_GPIO_PORT_A
#define SL_HAL_GPIO_INIT_EXP_3_PIN               8

// [GPIO_SL_HAL_GPIO_INIT_EXP_3]$

// <<< sl:end pin_tool >>>

#endif // SL_HAL_GPIO_INIT_EXP_3_CONFIG_H
