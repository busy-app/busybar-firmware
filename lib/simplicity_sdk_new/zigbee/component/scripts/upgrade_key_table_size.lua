local changeset = {}

local ncp_configuration = slc.is_provided("zigbee_ncp_configuration")

if ncp_configuration == true then
    local old_config_key_table_size = slc.config('SL_ZIGBEE_KEY_TABLE_SIZE')

    if (old_config_key_table_size ~= nil) then
    table.insert(changeset, {
        ['option'] = 'SL_ZIGBEE_KEY_TABLE_SIZE',
        ['action'] = 'remove'
    })
    end
end

return changeset