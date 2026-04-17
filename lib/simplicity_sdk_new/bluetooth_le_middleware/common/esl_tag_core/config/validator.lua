-- validation script for proper component configuration
local config_minval = slc.config("ESL_TAG_ADVERTISING_INTERVAL_MIN")
local config_maxval = slc.config("ESL_TAG_ADVERTISING_INTERVAL_MAX")
local config_intermittent_adv_enabled = slc.config("ESL_TAG_INTERMITTENT_ADVERTISING")
local config_secondary_adv_delay = slc.config("ESL_TAG_SECONDARY_ADVERTISING_DELAY")
local config_scheduler_queue_size = slc.config("APP_SCHEDULER_MAX_QUEUE_SIZE")
local config_scheduler_data_size = slc.config("APP_SCHEDULER_MAX_DATA_SIZE")
local config_queue_data_size = slc.config("CIRCULAR_QUEUE_LEN_MAX")
local config_displays = slc.config("ESL_TAG_MAX_DISPLAYS")
local config_leds = slc.config("ESL_TAG_MAX_LEDS")
local const_min_scheduler_queue = 1
local const_min_queue_data_size = 48
local const_min_data_size = 18
local sync_scanner_present = slc.is_provided("bluetooth_feature_sync_scanner")
local config_scan_enabled = slc.config("ESL_TAG_SYNC_SCAN_ENABLE")

if config_displays ~= nil then
  const_min_scheduler_queue = const_min_scheduler_queue + config_displays.number
end

if config_leds ~= nil then
  const_min_scheduler_queue = const_min_scheduler_queue + config_leds.number
end

if const_min_scheduler_queue < 2 then
  const_min_scheduler_queue = 2
end

if config_scheduler_queue_size ~= nil then
  if config_scheduler_queue_size.number < const_min_scheduler_queue then
    validation.error(
      "The scheduler queue min size (" .. config_scheduler_queue_size.value .. ") shall be greater than or equal to " .. tostring(const_min_scheduler_queue), 
      validation.target_for_defines({"APP_SCHEDULER_MAX_QUEUE_SIZE", "ESL_TAG_MAX_DISPLAYS", "ESL_TAG_MAX_LEDS"}),
      "Please set the APP_SCHEDULER_MAX_QUEUE_SIZE to at least " .. tostring(const_min_scheduler_queue) .. "!",
      nil)
  end
end

if config_scheduler_data_size ~= nil then
  if config_scheduler_data_size.number < const_min_data_size then
    validation.error(
      "The scheduler min data size (" .. config_scheduler_data_size.value .. ") shall be greater than or equal to " .. tostring(const_min_data_size), 
      validation.target_for_defines({"APP_SCHEDULER_MAX_DATA_SIZE"}),
      "Please set the APP_SCHEDULER_MAX_DATA_SIZE to at least " .. tostring(const_min_data_size) .. "!",
      nil)
  end
end

if config_scan_enabled ~= nil then
  if config_scan_enabled.number ~= 0 then
    if not sync_scanner_present then
      validation.error(
        "Sync-by-scan is enabled, but will not work unless bluetooth_feature_sync_scanner component is added to the project.", 
        validation.target_for_defines({"ESL_TAG_SYNC_SCAN_ENABLE, ESL_TAG_SCAN_TIMEOUT_SEC"}),
        "Please add the required 'Bluetooth LE Controller: Synchronization to periodic advertising trains by scanning' feature to the project before proceeding!",
        nil)
    end
  else
    if sync_scanner_present then
      validation.warning(
        "Sync-by-scan is disabled while the bluetooth_feature_sync_scanner component is still present in the project.", 
        validation.target_for_defines({"ESL_TAG_SYNC_SCAN_ENABLE"}),
        "You may want to consider removing the 'Bluetooth LE Controller: Synchronization to periodic advertising trains by scanning' feature to reduce code size before proceeding!",
        nil)
    end
  end
end

if config_minval ~= nil and config_maxval ~= nil then
  if config_minval.number > config_maxval.number then
    validation.error(
      "The minimum value " .. config_minval.value .. " shall be less than or equal to the maximum value: " .. config_maxval.value, 
      validation.target_for_defines({"ESL_TAG_ADVERTISING_INTERVAL_MIN", "ESL_TAG_ADVERTISING_INTERVAL_MAX"}),
      "Please set these values properly!",
      nil)
  end
  if config_intermittent_adv_enabled ~= nil and config_intermittent_adv_enabled.number ~= 0 and config_secondary_adv_delay ~= nil then
    if config_secondary_adv_delay.number >= config_minval.number then
      validation.error(
        "The secondary advertising delay value " .. config_secondary_adv_delay.number .. " shall be less than the minimum adverting interval value: " .. config_minval.value, 
        validation.target_for_defines({"ESL_TAG_ADVERTISING_INTERVAL_MIN", "ESL_TAG_SECONDARY_ADVERTISING_DELAY"}),
        "Please set these values properly!",
        nil)
    end
  end
end

if config_queue_data_size ~= nil then
  if config_queue_data_size.number < const_min_queue_data_size then
    validation.error(
      "The circular queue max data size (" .. config_queue_data_size.value .. ") shall be greater than or equal to " .. tostring(const_min_queue_data_size), 
      validation.target_for_defines({"CIRCULAR_QUEUE_LEN_MAX"}),
      "Please set the CIRCULAR_QUEUE_LEN_MAX to at least " .. tostring(const_min_queue_data_size) .. "!",
      nil)
  end
end
