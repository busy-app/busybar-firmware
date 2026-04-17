local changeset = {}

if slc.is_provided("interrupt_manager") then
  local s2_interrupts_in_ram = slc.config('SL_INTERRUPT_MANAGER_S2_INTERRUPTS_IN_RAM')

  if (s2_interrupts_in_ram ~= nil) then
    table.insert(changeset, {
      ['option'] = 'SL_INTERRUPT_MANAGER_S2_INTERRUPTS_IN_RAM',
      ['action'] = 'remove'
    })
  end
end

return changeset