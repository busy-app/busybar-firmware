local address_table_size = slc.config("SL_ZIGBEE_ADDRESS_TABLE_SIZE")
local af_plugin_address_table_size = slc.config("SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_SIZE")
local af_plugin_address_table_trust_center_cache_size = slc.config("SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_TRUST_CENTER_CACHE_SIZE")

if ( address_table_size ~= nil and af_plugin_address_table_size ~= nil and af_plugin_address_table_trust_center_cache_size ~= nil ) then
    if (tonumber(address_table_size.value) < tonumber(af_plugin_address_table_size.value) + tonumber(af_plugin_address_table_trust_center_cache_size.value)) then
        validation.error("Please ensure that SL_ZIGBEE_ADDRESS_TABLE_SIZE is greater than sum of (SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_SIZE + SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_TRUST_CENTER_CACHE_SIZE). ",
        validation.target_for_project(),
        nil,
        nil)
    end
end