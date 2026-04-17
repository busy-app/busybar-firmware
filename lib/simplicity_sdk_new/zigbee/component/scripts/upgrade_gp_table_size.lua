local changeset = {}

local ncp_configuration = slc.is_provided("zigbee_ncp_configuration")

if ncp_configuration == true then
    local old_config_gp_proxy = slc.config('SL_ZIGBEE_GP_PROXY_TABLE_SIZE')
    local old_config_gp_sink = slc.config('SL_ZIGBEE_GP_SINK_TABLE_SIZE')

    if (old_config_gp_proxy ~= nil) then
    table.insert(changeset, {
        ['option'] = 'SL_ZIGBEE_GP_PROXY_TABLE_SIZE',
        ['action'] = 'remove'
    })
    end
    if (old_config_gp_sink ~= nil) then
        table.insert(changeset, {
        ['option'] = 'SL_ZIGBEE_GP_SINK_TABLE_SIZE',
        ['action'] = 'remove'
        })
    end
end

return changeset