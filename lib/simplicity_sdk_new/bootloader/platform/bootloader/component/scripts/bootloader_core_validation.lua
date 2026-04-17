--Validation script for Bootloader Image Parser with no encryption component

local has_parser_noenc = slc.is_provided("bootloader_image_parser_nonenc")
local has_bl_interface = slc.is_provided("bootloader_interface")

local btl_enforce_signed_upgrade = slc.config('BOOTLOADER_ENFORCE_SIGNED_UPGRADE')
local btl_enforce_encryption = slc.config('BOOTLOADER_ENFORCE_ENCRYPTED_UPGRADE')

local has_storage_single = slc.is_provided("bootloader_common_storage_single")
local has_storage        = slc.is_provided("bootloader_common_storage")
local is_storage_bl      = has_storage_single or has_storage

local btl_upgrade_watchdog        = slc.config('BOOTLOADER_UPGRADE_WATCHDOG')
local btl_upgrade_watchdog_period = slc.config('BOOTLOADER_UPGRADE_WATCHDOG_PERIOD')

if btl_enforce_encryption.value == "1" and has_parser_noenc then
    validation.error('Can not use parser without encryption support, since the bootloader is configured to enforce encrypted upgrade files',
	validation.target_for_defines({'BOOTLOADER_ENFORCE_ENCRYPTED_UPGRADE'}))
end

if has_bl_interface then
    validation.error('Cannot install Bootloader Application Interface component in a Gecko Bootloader project. Please un-install the Bootloader Application Interface component to avoid build errors',
	validation.target_for_project())
end

-- Watchdog setting only applies to STORAGE bootloaders
if (not is_storage_bl)
   and btl_upgrade_watchdog
   and btl_upgrade_watchdog.value == "1" then
    validation.error(
        'BOOTLOADER_UPGRADE_WATCHDOG is only supported on storage bootloaders (bootloader_common_storage_single or bootloader_common_storage). ' ..
        'Disable BOOTLOADER_UPGRADE_WATCHDOG or switch to a storage bootloader.',
        validation.target_for_defines({'BOOTLOADER_UPGRADE_WATCHDOG', 'BOOTLOADER_UPGRADE_WATCHDOG_PERIOD'})
    )
end