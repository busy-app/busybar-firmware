local changeset = {}

local zigbee_stack_configuration = slc.is_provided('zigbee_pro_stack_support')

if zigbee_stack_configuration == true then
  local packet_buffer_count = slc.config('EMBER_PACKET_BUFFER_COUNT')
  if (packet_buffer_count ~= nil) then
    local packet_buffer_num = tonumber(packet_buffer_count.value)
    -- remove the old 'packet_buffer_count' configuration
    table.insert(changeset, {
      ['option'] = 'EMBER_PACKET_BUFFER_COUNT',
      ['action'] = 'remove'
    })
    local new_config = nil
    -- the new values are done by finding a nearby value to packet_buffer_count * 36
    if (packet_buffer_num <= 28) then
      new_config = 'SL_ZIGBEE_TINY_PACKET_BUFFER_HEAP'
    elseif (packet_buffer_num <= 56) then
      new_config = 'SL_ZIGBEE_SMALL_PACKET_BUFFER_HEAP'
    elseif (packet_buffer_num <= 113) then
      new_config = 'SL_ZIGBEE_MEDIUM_PACKET_BUFFER_HEAP'
    else
      new_config = 'SL_ZIGBEE_LARGE_PACKET_BUFFER_HEAP'
    end
    -- choose the 'nearest' option from the new config
    table.insert(changeset, {
      ['option'] = 'SL_ZIGBEE_PACKET_BUFFER_HEAP_SIZE',
      ['value'] = new_config
    })
  end
end

return changeset