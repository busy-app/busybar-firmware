/***************************************************************************/
/**
 * @file clock_manager_test_main.c
 * @brief Clock Manager Unity Test Functions
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "sl_si91x_clock_manager.h"
#include "sl_gpio_board.h"
#include "sl_driver_gpio.h"
#include "sl_si91x_driver_gpio.h"
#include "rsi_debug.h"
#include "unity.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#if (defined(SL_SI91X_MCU_CLK_OUT_EN) && (SL_SI91X_MCU_CLK_OUT_EN == 1))
#define MCU_CLK_OUT_GPIO_OUTPUT_HIGH        1           // GPIO output high value
#define MCU_CLK_OUT_GPIO_OUTPUT_LOW         0           // GPIO output low value
#define MCU_CLK_OUT_DIV_FACTOR_MAX          0x3F        // Maximum division factor for MCU clock out
#define SL_SI91X_MCU_CLK_OUT_GPIO_DIRECTION GPIO_OUTPUT // Direction output
#endif
/*******************************************************************************
 ***************************  Global Variables   *******************************
 ******************************************************************************/

/*******************************************************************************
 ************************  Static Function Prototypes   ************************
 ******************************************************************************/

/*******************************************************************************
 ***************************  Local Variables   *******************************
 ******************************************************************************/
//M4_SOC_CLK_SRC_SEL_T clk_source = M4_SOC_CLK_SRC_SEL_T::M4_SOC_CLK_SRC_SEL_T_PLL;
//uint32_t pll_freq               = 100; // Example PLL frequency in MHz
//uint32_t pll_ref_clk            = 50;  // Example PLL reference clock frequency in MHz
//PLL_TYPE_T pll_type             = PLL_TYPE_T::PLL_TYPE_T_M4PLL;

/*******************************************************************************
 ************************  Test Function Prototypes  ****************************
 ******************************************************************************/
void test_clock_manager_init(void);
void test_clock_manager_m4_set_core_clk(void);
void test_clock_manager_set_pll_freq(void);
void test_clock_manager_m4_get_core_clk_src_freq(void);
void test_clock_manager_get_pll_freq(void);
void test_clock_manager_control_pll(void);
#if (defined(SL_SI91X_MCU_CLK_OUT_EN) && (SL_SI91X_MCU_CLK_OUT_EN == 1))
void test_clock_manager_mcu_clk_out(void);
#endif
void test_delay_ms_with_gpio_pulse(void);
/******************************************************************************
 * Main function in which all the test cases are tested using unity framework
 ******************************************************************************/
int app_init()
{
  UnityBeginGroup("Clock Manager");
  RUN_TEST(test_clock_manager_init, __LINE__);
  RUN_TEST(test_clock_manager_m4_set_core_clk, __LINE__);
  RUN_TEST(test_clock_manager_set_pll_freq, __LINE__);
  RUN_TEST(test_clock_manager_m4_get_core_clk_src_freq, __LINE__);
  RUN_TEST(test_clock_manager_get_pll_freq, __LINE__);
  RUN_TEST(test_clock_manager_control_pll, __LINE__);
#if (defined(SL_SI91X_MCU_CLK_OUT_EN) && (SL_SI91X_MCU_CLK_OUT_EN == 1))
  RUN_TEST(test_clock_manager_mcu_clk_out, __LINE__);
#endif
  RUN_TEST(test_delay_ms_with_gpio_pulse, __LINE__);

  UnityEnd();
  UnityPrintf("END");
  while (1) {
  }
}

/*******************************************************************************
 * Function to test initialization of Clock Manager
 ******************************************************************************/
void test_clock_manager_init(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Clock Manager Init \n");
  sl_status_t status;

  UnityPrintf("Testing initialized clock manager \n");
  status = sl_si91x_clock_manager_init();
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, Clock Manager initialized successfully \n");

  UnityPrintf("Clock Manager Init test completed \n");
}

/*******************************************************************************
 * Function to test configure the M4 core clock source
 ******************************************************************************/
void test_clock_manager_m4_set_core_clk(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Clock Manager set M4 core clock \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_clock_manager_m4_set_core_clk(6, PLL_REF_CLK_VAL_XTAL);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Status of API is correct, testing invalid parameter successfully \n");

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_clock_manager_m4_set_core_clk(M4_ULPREFCLK, MAX_PLL_FREQUENCY + 1);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Status of API is correct, testing invalid parameter successfully \n");

  UnityPrintf("Testing correct parameter \n");
  status = sl_si91x_clock_manager_m4_set_core_clk(M4_ULPREFCLK, PLL_REF_CLK_VAL_XTAL);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, Clock Manager set M4 core clock successfully \n");

  UnityPrintf("Testing correct parameter \n");
  status = sl_si91x_clock_manager_m4_set_core_clk(M4_SOCPLLCLK, MAX_PLL_FREQUENCY);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, Clock Manager set M4 core clock successfully \n");

  UnityPrintf("Testing correct parameter \n");
  status = sl_si91x_clock_manager_m4_set_core_clk(M4_INTFPLLCLK, MAX_PLL_FREQUENCY);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, Clock Manager set M4 core clock successfully \n");

  // Setting back to default state.
  status = sl_si91x_clock_manager_m4_set_core_clk(M4_ULPREFCLK, PLL_REF_CLK_VAL_XTAL);

  UnityPrintf("Clock Manager set M4 core clock test completed \n");
}

/*******************************************************************************
 * Function to test set pll frequency.
 ******************************************************************************/
void test_clock_manager_set_pll_freq(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Clock Manager set pll frequency \n");
  sl_status_t status;
  uint32_t pll_ref_clk = XTAL_CLK_FREQ;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_clock_manager_set_pll_freq(I2S_PLL, MAX_PLL_FREQUENCY + 1, pll_ref_clk);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Status of API is correct, testing invalid parameter successfully \n");

  UnityPrintf("Testing correct parameter \n");
  status = sl_si91x_clock_manager_set_pll_freq(I2S_PLL, MAX_PLL_FREQUENCY, pll_ref_clk);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, Clock Manager set M4 core clock successfully \n");

  UnityPrintf("Testing correct parameter \n");
  status = sl_si91x_clock_manager_set_pll_freq(SOC_PLL, MAX_PLL_FREQUENCY, pll_ref_clk);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, Clock Manager set M4 core clock successfully \n");

  UnityPrintf("Testing correct parameter \n");
  status = sl_si91x_clock_manager_set_pll_freq(INTF_PLL, MAX_PLL_FREQUENCY, pll_ref_clk);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, Clock Manager set pll frequency successfully \n");

  UnityPrintf("Clock Manager set pll frequency test completed \n");
}

/*******************************************************************************
 * Function to test get core clock.
 ******************************************************************************/
void test_clock_manager_m4_get_core_clk_src_freq(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Clock Manager get core clock \n");
  sl_si91x_m4_soc_clk_src_sel_t get_core_clk;
  uint32_t get_core_clk_freq;

  get_core_clk = sl_si91x_clock_manager_m4_get_core_clk_src_freq(&get_core_clk_freq);
  UnityPrintf("Core clock Frequency = %ld \n", get_core_clk_freq);
  UnityPrintf("Core clock = %ld \n", get_core_clk);
  UnityPrintf("Status of API is correct, Clock Manager get core clock successfully \n");

  UnityPrintf("Clock Manager get core clock test completed \n");
}

/*******************************************************************************
 * Function to test get pll frequency.
 ******************************************************************************/
void test_clock_manager_get_pll_freq(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Clock Manager get pll frequency \n");
  uint32_t get_pll_freq;

  get_pll_freq = sl_si91x_clock_manager_get_pll_freq(INTF_PLL);
  UnityPrintf("PLL Frequency = %ld \n", get_pll_freq);
  UnityPrintf("Status of API is correct, Clock Manager get pll frequency successfully \n");

  UnityPrintf("Clock Manager get pll frequency test completed \n");
}

/*******************************************************************************
 * Function to test Controls the selected PLL clock.
 ******************************************************************************/
void test_clock_manager_control_pll(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Clock Manager control PLL \n");
  sl_status_t status;
  boolean_t enable = 1;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_clock_manager_control_pll(3, enable);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with correct parameter \n");
  status = sl_si91x_clock_manager_control_pll(I2S_PLL, enable);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, Clock Manager control PLL successfully \n");

  UnityPrintf("Testing with correct parameter \n");
  status = sl_si91x_clock_manager_control_pll(SOC_PLL, enable);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, Clock Manager control PLL successfully \n");

  UnityPrintf("Testing with correct parameter \n");
  status = sl_si91x_clock_manager_control_pll(INTF_PLL, enable);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, Clock Manager control PLL successfully \n");

  UnityPrintf("Clock Manager control PLL test completed \n");
}
#if (defined(SL_SI91X_MCU_CLK_OUT_EN) && (SL_SI91X_MCU_CLK_OUT_EN == 1))
/*******************************************************************************
 * Function to test MCU Clock Output Configuration
 ******************************************************************************/

void test_clock_manager_mcu_clk_out(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing sl_si91x_clock_manager_mcu_clk_out API with GPIO coverage \n");

  sl_status_t status;
  sl_si91x_gpio_pin_config_t gpio_config;

  // ---------- Invalid GPIO ----------
  UnityPrintf("Testing with invalid GPIO pin \n");
  gpio_config.port_pin.pin = 99; // Unsupported pin
  status = sl_si91x_clock_manager_mcu_clk_out(gpio_config, SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_RC_32MHZ, 0);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Status of API is correct,,Tested with invalid parameter successfully \n");

  // ---------- Valid GPIOs ----------
  int valid_pins[] = { SL_SI91X_GPIO_11_PIN, SL_SI91X_GPIO_12_PIN, SL_SI91X_GPIO_15_PIN };

  for (int i = 0; i < 3; i++) {
    gpio_config.port_pin.pin  = valid_pins[i];
    gpio_config.direction     = 0;
    gpio_config.port_pin.port = 0;
    //
    UnityPrintf("Testing with GPIO pin %d \n", gpio_config.port_pin.pin);

    // Invalid division factor
    status = sl_si91x_clock_manager_mcu_clk_out(gpio_config,
                                                SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_RC_32MHZ,
                                                MCU_CLK_OUT_DIV_FACTOR_MAX + 1);
    TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);

    UnityPrintf("Status of API is correct,,Tested with invalid division factor successfully \n");

    // Gated clock source
    status = sl_si91x_clock_manager_mcu_clk_out(gpio_config, SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_GATED, 0);
    TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);

    UnityPrintf("Status of API is correct,Tested with SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_GATED source successfully \n");

    // RC 32MHz
    status = sl_si91x_clock_manager_mcu_clk_out(gpio_config, SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_RC_32MHZ, 2);
    TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
    UnityPrintf(
      "Status of API is correct,Tested with SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_RC_32MHZ source successfully \n");

    // XTAL
    status = sl_si91x_clock_manager_mcu_clk_out(gpio_config, SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_XTAL, 2);
    TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
    UnityPrintf(
      "Status of API is correct,Tested with SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_XTAL clk source successfully \n");

    // RC 32KHz
    status = sl_si91x_clock_manager_mcu_clk_out(gpio_config, SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_RC_32KHZ, 2);
    TEST_ASSERT(status == SL_STATUS_OK || status == SL_STATUS_NOT_INITIALIZED);
    UnityPrintf(
      "Status of API is correct,Tested with SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_RC_32KHZ clk source successfully \n");

    // INTF PLL
    status = sl_si91x_clock_manager_mcu_clk_out(gpio_config, SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_INTF_PLL, 2);
    TEST_ASSERT(status == SL_STATUS_OK || status == SL_STATUS_NOT_INITIALIZED);
    UnityPrintf(
      "Status of API is correct,Tested with SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_INTF_PLL source successfully \n");

    // SOC PLL
    status = sl_si91x_clock_manager_mcu_clk_out(gpio_config, SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_SOC_PLL, 2);
    TEST_ASSERT(status == SL_STATUS_OK || status == SL_STATUS_NOT_INITIALIZED);
    UnityPrintf("Status of API is correct,Tested with SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_SOC_PLL source successfully \n");

    // I2S PLL
    status = sl_si91x_clock_manager_mcu_clk_out(gpio_config, SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_I2S_PLL, 2);
    TEST_ASSERT(status == SL_STATUS_OK || status == SL_STATUS_NOT_INITIALIZED);
    UnityPrintf("Status of API is correct,Tested with SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_I2S_PLL successfully \n");

    // Invalid clock source (default case)
    status = sl_si91x_clock_manager_mcu_clk_out(gpio_config, (sl_clock_manager_mcu_clk_out_sel_t)99, 2);
    TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  }

  UnityPrintf("MCU Clock Output Configuration tests with GPIO and all sources coverage completed \n");

  UnityPrintf("MCU Clock Output Configuration tests completed successfully\n");
}

#endif
/*******************************************************************************
 * Function to test delay function using gpio pulse
 ******************************************************************************/
void test_delay_ms_with_gpio_pulse(void)
{
  UnityPrintf("\n");
  UnityPrintf("\nTesting Delay_ms() with GPIO pulse\n");
  sl_si91x_gpio_pin_config_t gpio_pin_config = { { SL_SI91X_GPIO_12_PORT, SL_SI91X_GPIO_12_PIN }, GPIO_OUTPUT };
  sl_status_t status                         = sl_gpio_set_configuration(gpio_pin_config);
  if (status != SL_STATUS_OK) {
    UnityPrintf("gpio configuration fail\n");
  }
  status = sl_gpio_driver_toggle_pin(&gpio_pin_config.port_pin);

  sl_si91x_delay_ms(100); //check the pulse width should be ~100ms
  status = sl_gpio_driver_toggle_pin(&gpio_pin_config.port_pin);

  UnityPrintf("Testing Delay_ms() with GPIO pulse\n");
}
