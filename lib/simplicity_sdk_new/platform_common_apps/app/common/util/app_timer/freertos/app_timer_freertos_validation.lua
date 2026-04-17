-- app_timer validation
local cfg_value = slc.config("configUSE_TIMERS")
local enable_static = slc.config('configSUPPORT_STATIC_ALLOCATION')
local enable_dynamic = slc.config('configSUPPORT_DYNAMIC_ALLOCATION')

if cfg_value == nil or autonumber_common.autonumber(cfg_value.value) == 0 then
        validation.error("Kernel configUSE_TIMERS config needs to be enabled",
        validation.target_for_defines({"configUSE_TIMERS"}),
        "Please enable configUSE_TIMERS",
        nil)
end

if (enable_static == nil or autonumber_common.autonumber(enable_static.value) == 0) and (enable_dynamic == nil or autonumber_common.autonumber(enable_dynamic.value) == 0) then
        validation.error("Kernel configSUPPORT_STATIC_ALLOCATION or configSUPPORT_DYNAMIC_ALLOCATION config needs to be enabled in FreeRTOS config (FreeRTOSConfig.h)",
        validation.target_for_defines({"configSUPPORT_STATIC_ALLOCATION", "configSUPPORT_DYNAMIC_ALLOCATION"}),
        "Please enable configSUPPORT_STATIC_ALLOCATION or configSUPPORT_DYNAMIC_ALLOCATION",
        nil)
end

