-- cs_initiator validation script for checking initiator configuration validity.
local modify_msg = "Modify cs_initiator_config.h!"

-- max connection count must align with the BT max connection count
local bt_max_conn =
  slc.config('SL_BT_CONFIG_MAX_CONNECTIONS').number
local cs_max_conn =
  slc.config('CS_INITIATOR_MAX_CONNECTIONS').number

if bt_max_conn ~= nil and cs_max_conn ~= nil then
  local err_msg = "Invalid number of maximum initiator connections!"
  if bt_max_conn < cs_max_conn then
    validation.error(err_msg,
    validation.target_for_defines({'CS_INITIATOR_MAX_CONNECTIONS'}),
    [[Number of maximum initiator connections (]] .. cs_max_conn .. [[) is greater
    than the number of allowed Bluetooth LE connections (]] .. bt_max_conn .. [[)! ]] .. modify_msg,
    nil)
  end
  if not (cs_max_conn >= 1 and cs_max_conn <= bt_max_conn) then
    validation.error(err_msg,
    validation.target_for_defines({'CS_INITIATOR_MAX_CONNECTIONS'}),
    [[Number of maximum initiator connections (]] .. cs_max_conn .. [[) is out of range!
    Valid range is 1 to 4! ]] .. modify_msg,
    nil)
  end
end


-- Default min & max connection event length
local cs_min_ce_len =
  slc.config('CS_INITIATOR_DEFAULT_MIN_CE_LENGTH').number
local cs_max_ce_len =
  slc.config('CS_INITIATOR_DEFAULT_MAX_CE_LENGTH').number
if cs_min_ce_len ~= nil and cs_max_ce_len ~= nil then
  if cs_min_ce_len > cs_max_ce_len then
    validation.error(
    "Invalid default minimum and maximum connection event length values!",
    validation.target_for_defines({'CS_INITIATOR_DEFAULT_MIN_CE_LENGTH', 'CS_INITIATOR_DEFAULT_MAX_CE_LENGTH'}),
    [[Default minimum connection event length (]] .. cs_min_ce_len .. [[) is greater
    than the default maximum connection event length is (]] .. cs_max_ce_len .. [[)! ]] .. modify_msg,
    nil)
  end
end


-- CS procedure execution
local proc_cnt =
  slc.config('CS_INITIATOR_DEFAULT_MAX_PROCEDURE_COUNT').number
if proc_cnt ~= nil and proc_cnt ~= 0 and proc_cnt ~= 1 then
  validation.error(
  "Invalid CS procedure count!",
  validation.target_for_defines({'CS_INITIATOR_DEFAULT_MAX_PROCEDURE_COUNT'}),
  [[']] .. proc_cnt .. [[' is invalid value! ]] .. modify_msg,
  nil)
end

-- Antenna offset
local antenna_offset = slc.config('CS_INITIATOR_ANTENNA_OFFSET').number
if antenna_offset ~= nil and antenna_offset ~= 0 and antenna_offset ~= 1 then
  validation.error(
  "Invalid antenna offset value!",
  validation.target_for_defines({'CS_INITIATOR_ANTENNA_OFFSET'}),
  [[']] .. antenna_offset .. [[' is invalid value! ]] .. modify_msg,
  nil)
end


-- CS main mode RTT and CS algo mode real-time fast
-- combination is not supported!
local cs_main_mode = slc.config('CS_INITIATOR_DEFAULT_CS_MAIN_MODE').value
local cs_algo_mode = slc.config('CS_INITIATOR_DEFAULT_ALGO_MODE').value
err_msg = "Object tracking mode is incompatible with"

if cs_main_mode ~= nil and cs_algo_mode ~= nil then
  if cs_main_mode == 'sl_bt_cs_mode_rtt' and cs_algo_mode == 'SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST' then
    validation.error(
    err_msg .. " CS main mode!",
    validation.target_for_defines({'CS_INITIATOR_DEFAULT_CS_ALGO_MODE'}),
    [[Object tracking mode (]] .. cs_algo_mode .. [[) is incompatible with
    CS main mode (]] .. cs_main_mode .. [[)! ]] .. modify_msg,
    nil)
  end
end

-- CS main mode RTT and CS sub mode RTT combination is not supported!
local cs_main_mode = slc.config('CS_INITIATOR_DEFAULT_CS_MAIN_MODE').value
local cs_sub_mode = slc.config('CS_INITIATOR_DEFAULT_ALGO_MODE').value
err_msg = "CS main mode is incompatible with"

if cs_main_mode ~= nil and cs_sub_mode ~= nil then
  if cs_main_mode == 'sl_bt_cs_mode_rtt' and cs_sub_mode == 'sl_bt_cs_mode_rtt' then
    validation.error(
    err_msg .. " CS sub mode!",
    validation.target_for_defines({'CS_INITIATOR_DEFAULT_CS_ALGO_MODE'}),
    [[CS main mode (]] .. cs_main_mode .. [[) is incompatible with
    CS sub mode (]] .. cs_sub_mode .. [[)! ]] .. modify_msg,
    nil)
  end
end
