local changeset = {}

local component_present = slc.is_selected('adc_ulp_component') or slc.is_selected('timer_ulp_component') or slc.is_selected('gpio_ulp_component') or  slc.is_selected('debug_ulp_component') or slc.is_selected('cts_ulp_component') or slc.is_selected('calender_ulp_component')or slc.is_selected('sysrtc_ulp_component') or slc.is_selected('bod_ulp_component') or slc.is_selected('user_files') or slc.is_selected('usart_ulp_component')or slc.is_selected('wdt_ulp_component') or slc.is_selected('i2c_ulp_component') or slc.is_selected('ssi_ulp_component') or slc.is_selected('i2s_ulp_component') or slc.is_selected('dma_ulp_component') or slc.is_selected('dac_ulp_component') or slc.is_selected('gpio_npss_component')

if component_present then
  table.insert(changeset, {
    ['status'] = 'automatic',
    ['description'] =  [['Warning: This upgrade will add the following component to the project which moves the ULP code to RAM. Please remove the  "PM PS2 Component" component to run the ULP code from flash:
    Component Path: Device > Si91x > MCU > Service > Power Manager > ULP Peripheral']]
  })
end
return changeset