--[[
    This validation script checks following:
    Only one type of architecture is allowed to be included in Zigbee applications
--]]
local i386_enabled   = slc.is_provided("zigbee_i386")
local arm32_enabled  = slc.is_provided("zigbee_arm32")
local x86_64_enabled = slc.is_provided("zigbee_x86_64")
local arm64_enabled  = slc.is_provided("zigbee_arm64")
local aarch64_cortex_a72_gcc_12_3_0_musl_enabled  = slc.is_provided("zigbee_aarch64_cortex_a72_gcc_12_3_0_musl")
local aarch64_ndk_r25c_enabled  = slc.is_provided("zigbee_aarch64_android_ndk_r25c")
local aarch64_tizen_enabled  = slc.is_provided("zigbee_aarch64_tizen-0.1-13.1_gcc-9.2-softfp")
local arm7l_tizen_enabled  = slc.is_provided("zigbee_armv7l_tizen-0.1-13.1_gcc-9.2-softfp")

local function isMoreThanOneArch(...)
    local args = {...}
    local arch_num = 0
    for index, value in ipairs(args) do
        if value == true then
            arch_num = arch_num + 1
            if arch_num > 1 then
                return true
            end
        end
    end
    return false
 end

if (isMoreThanOneArch(i386_enabled, arm32_enabled, x86_64_enabled, arm64_enabled, aarch64_cortex_a72_gcc_12_3_0_musl_enabled, 
                      aarch64_ndk_r25c_enabled, aarch64_tizen_enabled, arm7l_tizen_enabled )) then
    validation.error("Multiple architectures are not supported.",
                     validation.target_for_project(),
                     "Remove the \"Recommended Linux architecture\" component first.",
                     nil)
end