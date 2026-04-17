local changeset = {}

local old_config = slc.config('NVM3_DEFAULT_CACHE_SIZE')
local gp_sink_table_size_config = slc.config('SL_ZIGBEE_GP_SINK_TABLE_SIZE')
local gp_proxy_table_size_config = slc.config('SL_ZIGBEE_GP_PROXY_TABLE_SIZE')

if (old_config ~= nil) then
  if (gp_sink_table_size_config ~= nil) and (gp_proxy_table_size_config ~= nil) then
    local gp_sink_table_size_config_num = tonumber(gp_sink_table_size_config.value)
    local gp_proxy_table_size_config_num = tonumber(gp_proxy_table_size_config.value)
    if (gp_sink_table_size_config_num >= 25) and (gp_proxy_table_size_config_num >= 25) then
      -- The project had a configuration for NVM3 DEFAULT CACHE SIZE
      -- Remove prior setting for NVM3 DEFAULT CACHE SIZE
      table.insert(changeset, {
        ['option'] = 'NVM3_DEFAULT_CACHE_SIZE',
        ['action'] = 'remove'
      })
      -- Re-add setting for NVM3 DEFAULT CACHE SIZE (in bytes)
      table.insert(changeset, {
        ['option'] = 'NVM3_DEFAULT_CACHE_SIZE',
        ['value'] = tostring(1000)
      })
      table.insert(changeset, {
        ["status"] = "user_verification",
        ["description"] = "The NVM3_DEFAULT_CACHE_SIZE value has been changed to 1000 in order to accommodate the SL_ZIGBEE_GP_SINK_TABLE_SIZE and SL_ZIGBEE_GP_PROXY_TABLE_SIZE values. Please verify that this value fits the application needs and that it results in a functioning image."}
      )
    end
  end
end

return changeset