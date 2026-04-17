local changeset = {}

if slc.is_provided('ot_platform_abstraction_core') then
  -- Replace sl_system with sl_system_compatibility on baremetal applications
  if not slc.is_provided('kernel') then
    table.insert(changeset, {
      ['component'] = 'sl_system',
      ['action'] = 'remove',
    })
    table.insert(changeset, {
      ['component'] = 'sl_system_compatibility',
      ['action'] = 'add',
    })
  else
    -- Some components were pulling sl_system directly so some project 
    -- had sl_system even without adding it to their .slcp. 
    -- These components were updated to not pull sl_system anymore.
    -- This is a workaround to add sl_system back to the .slcp of the application.
    table.insert(changeset, {
      ['component'] = 'sl_system',
      ['action'] = 'add',
    })
  end
end

return changeset