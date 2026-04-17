local changeset = {}

local function map_old_to_new_flow_control(old_value)
  local mapping = {
    ['eusartHwFlowControlNone'] = 'SL_IOSTREAM_EUSART_UART_FLOW_CTRL_NONE',
    ['eusartHwFlowControlCts'] = 'SL_IOSTREAM_EUSART_UART_FLOW_CTRL_CTS',
    ['eusartHwFlowControlRts'] = 'SL_IOSTREAM_EUSART_UART_FLOW_CTRL_RTS',
    ['eusartHwFlowControlCtsAndRts'] = 'SL_IOSTREAM_EUSART_UART_FLOW_CTRL_CTS_RTS'
  }
  return mapping[old_value] or 'SL_IOSTREAM_EUSART_UART_FLOW_CTRL_NONE' -- Default to NONE if no match
end

local eusarts = slc.component('iostream_eusart')
for k, v in pairs(eusarts.instances) do
  local instance_config_fc_type = 'SL_IOSTREAM_EUSART_' .. string.upper(k) .. '_FLOW_CONTROL_TYPE'
  local old_flow_control = slc.config(instance_config_fc_type)
  
  if (old_flow_control ~= nil) then
    -- The project had a configuration option for SL_IOSTREAM_EUSART_{INSTANCE}_FLOW_CONTROL_TYPE
    -- Remove prior setting for SL_IOSTREAM_EUSART_{INSTANCE}_FLOW_CONTROL_TYPE 
    table.insert(changeset, {
      ['option'] = instance_config_fc_type,
      ['action'] = 'remove',
      ['instance'] = k
    })

    -- Map old value to new value
    local new_flow_control_value = map_old_to_new_flow_control(old_flow_control.value)
    logit("Updating Flow Control for instance " .. string.upper(k) .. ": " .. new_flow_control_value)

    table.insert(changeset, {
      ['option'] = instance_config_fc_type,
      ['value'] = new_flow_control_value,
      ['instance'] = k
    })
  end
end

return changeset