local changeset = {}

if slc.is_selected("zw_core") then
  local rail_pm_init = slc.config('SL_RAIL_UTIL_RAIL_POWER_MANAGER_INIT')
  if rail_pm_init ~= nil then
    table.insert(changeset, {
      ['option'] = 'SL_RAIL_UTIL_RAIL_POWER_MANAGER_INIT',
      ['value'] = '1',
      ['description'] = 'Enable RAIL Power Manager initialization (default configuration)',
      ['status'] = 'automatic'
    })
  end
end

return changeset