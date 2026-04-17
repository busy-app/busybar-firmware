# Migrating from WiSeConnect™ SDK v3.1.4 to v3.2.0

## Table of Contents

- [Overview](#overview)
- [Migration Steps](#migration-steps)
  - [Update Files](#update-files)
  - [Update Macros](#update-macros)

## Overview

This is a guide for updating an existing application using the WiSeConnect™ SDK v3.1.4 to a v3.2.0 application.

There are few naming and file changes in v3.2.0 as compared to v3.1.4, mostly in order to standardize the names and improve the overall usage experience of the application programming interface (API). Migration requires the names everywhere to be updated in the existing code of an application.

## Migration Steps

In order to convert a WiSeConnect SDK v3.1.4 application to a v3.2.0 application,

1. Open your existing application project in Simplicity Studio.

2. In each source file of the project, replace the v3.1.4 names or interfaces with v3.2.0 names or interfaces. 

Refer to the tables in each of the sections that follow which map the v3.1.4 API elements to v3.2.0. In some instances, the differences between v3.1.4 and v3.2.0 are highlighted in **bold** text.
- [Update Files](#update-files)
- [Update Macros](#update-macros)

### Update Files

| **v3.1.4** | **v3.2.0** |
|----------------|----------------|
| sl_si91x_pwm_init_led0_config.h | removed sl_si91x_pwm_init_led0_config.h |

**Note:-**
If issue is seen with existing projects which are using led0 instance with PWM component then follow the below steps
1. Copy the source file from the older versions sl_si91x_pwm_init_led0_config.h to components/board/silabs/config/common_config/sl_si91x_pwm_init_led0_config.h
2. Update the board config component brd4338a_config.slcc for brd4338a  and brd4342a_config.slcc for brd4342a with the below content
```
  - override:
      component: "%extension-wiseconnect3_sdk%pwm_instance"
      file_id: pwm_config
      instance: led0
    path: common_config/sl_si91x_pwm_init_led0_config.h
```

### Update Macros

| **v3.1.4** | **v3.2.0** |
|----------------|----------------|
| RTE_USART0_FRAC_DIV_EN | RTE_USART0_FRAC_DIV_SEL |
| RTE_UART1_FRAC_DIV_EN | RTE_UART1_FRAC_DIV_SEL |