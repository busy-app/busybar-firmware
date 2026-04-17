local changeset = {}

local diag_config_enabled = slc.config("OPENTHREAD_CONFIG_DIAG_ENABLE") ~= nil and
      slc.config("OPENTHREAD_CONFIG_DIAG_ENABLE").value == "1"

if diag_config_enabled and not slc.is_provided("ot_diags") then
  table.insert(changeset, {
    ['component'] = 'ot_diags',
    ['action'] = 'add'
  })
end
return changeset
