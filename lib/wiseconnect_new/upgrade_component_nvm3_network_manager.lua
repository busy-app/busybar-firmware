local changeset = {}

-- In wiseconnect 3.4.2, the NVM3 example projects did not explicitly include
-- network_manager. When upgrading
-- directly to 4.0.0, this can surface as a missing dependency.
--
-- Run only for 3.4.2->4.0.0 NVM3 migration (network_manager missing).
-- Skip for 4.0.0->4.0.1 (network_manager already present in slcp).
-- Exclude PSA apps entirely (they manage their own components).
local nvm3_common_flash = slc.is_provided("sl_si91x_common_flash_nvm3")
local network_manager_missing = (not slc.is_selected("network_manager")) and (not slc.is_provided("network_manager"))
local is_psa_app = slc.is_selected("psa_crypto") or slc.is_selected("psa_driver")

if nvm3_common_flash and network_manager_missing and (not is_psa_app) then
  table.insert(changeset, {
    ['component'] = 'network_manager',
    ['action'] = 'add'
  })
  -- Add NVM3 variants (required for NVM3 common-flash). Do NOT add basic_*;
  -- basic_* and nvm3_* are mutually exclusive.
  if (not slc.is_selected("nvm3_network_config_manager")) and (not slc.is_provided("nvm3_network_config_manager")) then
    table.insert(changeset, {
      ['component'] = 'nvm3_network_config_manager',
      ['action'] = 'add'
    })
  end
  if (not slc.is_selected("nvm3_credential_manager")) and (not slc.is_provided("nvm3_credential_manager")) then
    table.insert(changeset, {
      ['component'] = 'nvm3_credential_manager',
      ['action'] = 'add'
    })
  end
  -- Remove basic variants when present so they do not conflict with nvm3_*.
  if slc.is_selected("basic_network_config_manager") then
    table.insert(changeset, {
      ['component'] = 'basic_network_config_manager',
      ['action'] = 'remove'
    })
  end
  if slc.is_selected("basic_credential_manager") then
    table.insert(changeset, {
      ['component'] = 'basic_credential_manager',
      ['action'] = 'remove'
    })
  end
end

return changeset
 
