local changeset = {}

if slc.is_provided("interrupt_manager") then
  local old_config = slc.config('SL_INTERRUPT_MANAGER_S2_INTERRUPTS_IN_RAM')

  if (old_config.value == '1') then
    table.insert(changeset, {
      ['component'] = 'interrupt_manager_vector_table_in_ram',
      ['action'] = 'add'
    })
  end
end

return changeset