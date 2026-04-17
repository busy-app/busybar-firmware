-- validation script for proper component configuration
local config_cmd_buf_size = slc.config("SL_NCP_CMD_BUF_SIZE") -- SL_BT_NCP_TRANSPORT_CONFIG_RX_BUF_SIZE must be at least twice as large
local config_evt_buf_size = slc.config("SL_NCP_EVT_BUF_SIZE") -- must not be bigger than SL_BT_NCP_TRANSPORT_CONFIG_TX_BUF_SIZE
local config_rx_buf_size = slc.config("SL_BT_NCP_TRANSPORT_CONFIG_RX_BUF_SIZE") -- SL_NCP_CMD_BUF_SIZE must be less than or equal to the half of this value
local config_tx_buf_size = slc.config("SL_BT_NCP_TRANSPORT_CONFIG_TX_BUF_SIZE") -- SL_NCP_EVT_BUF_SIZE should be less or equal to this
local config_psa_key_slots = slc.config("SL_PSA_KEY_USER_SLOT_COUNT") -- if secure NCP is used, this must not be 0!
local config_ncp_sec_present = slc.is_provided("ncp_sec") -- if present, the default low limit for RX/TX is insufficient: need to add the further overhead

local const_min_buffer_size = 260 -- This is the current default value as of writing (09/05/2024), but may change in the future - see configuration header of the bt_ncp_transport components for actual limit
local const_bgapi_header_size = 4 -- defined by SL_BGAPI_MSG_HEADER_LEN in sl_bgapi.h
local const_buffer_spare_space = (const_min_buffer_size - const_bgapi_header_size - 251) -- This simply assumes BGAPI message payload up to 251 bytes as limited by MTU size, currently (but the BGAPI message could hold 2047 bytes of data by specification, otherwise)
local const_sec_overhead = 9 -- 40 bits of Init Vector and 32bit of MAC for AES_CCM

-- Determine the actual minimum buffer size value
local min_buffer_size = const_min_buffer_size - const_buffer_spare_space

-- Check if secure ncp overhead is needed, and also if the PSA user key slot count is set properly
if config_ncp_sec_present ~= nil and config_ncp_sec_present then
  min_buffer_size = min_buffer_size + const_sec_overhead -- add sec ncp overhead
  if config_psa_key_slots ~= nil then
    if config_psa_key_slots.number < 1 then
      validation.error(
        "The PSA key slots count must not be zero if secure NCP is used!",
        validation.target_for_defines({"SL_PSA_KEY_USER_SLOT_COUNT"}),
        "Please set the SL_PSA_KEY_USER_SLOT_COUNT to at least 1!",
        nil
      )
    end
  end
end

if config_cmd_buf_size ~= nil and config_rx_buf_size ~= nil then
  local min_rx_buf_size = 2 * config_cmd_buf_size.number
  if config_rx_buf_size.number < min_rx_buf_size then
    validation.error(
      "The buffer size of the bt_ncp_transport receiver (" ..
        config_rx_buf_size.value ..
          ") must be greater than or equal to twice the ncp command buffer (" ..
            config_cmd_buf_size.value .. ")!",
      validation.target_for_defines({"SL_BT_NCP_TRANSPORT_CONFIG_RX_BUF_SIZE", "SL_NCP_CMD_BUF_SIZE"}),
      "Please set the SL_BT_NCP_TRANSPORT_CONFIG_RX_BUF_SIZE to at least " .. tostring(min_rx_buf_size) .. "!",
      nil
    )
  end
  if config_cmd_buf_size.number < min_buffer_size then
    validation.error(
      "The NCP command buffer size (" ..
        config_cmd_buf_size.value .. ") must be greater than or equal to " .. tostring(min_buffer_size) .. "!",
      validation.target_for_defines({"SL_NCP_CMD_BUF_SIZE"}),
      "Please set the SL_NCP_CMD_BUF_SIZE accordingly!",
      nil
    )
  end
end

if config_evt_buf_size ~= nil and config_tx_buf_size ~= nil then
  local min_tx_buf_size = config_evt_buf_size.number
  if config_tx_buf_size.number < min_tx_buf_size then
    validation.error(
      "The buffer size of the bt_ncp_transport transmitter (" ..
        config_tx_buf_size.value ..
          ") must be greater than or equal to the ncp event buffer (" .. config_evt_buf_size.value .. ")!",
      validation.target_for_defines({"SL_BT_NCP_TRANSPORT_CONFIG_TX_BUF_SIZE", "SL_NCP_EVT_BUF_SIZE"}),
      "Please set the SL_BT_NCP_TRANSPORT_CONFIG_TX_BUF_SIZE to at least " .. tostring(min_tx_buf_size) .. "!",
      nil
    )
  end
  if config_evt_buf_size.number < min_buffer_size then
    validation.error(
      "The NCP event buffer size (" ..
        config_evt_buf_size.value .. ") must be greater than or equal to " .. tostring(min_buffer_size) .. "!",
      validation.target_for_defines({"SL_NCP_EVT_BUF_SIZE"}),
      "Please set the SL_NCP_EVT_BUF_SIZE accordingly!",
      nil
    )
  end
end
