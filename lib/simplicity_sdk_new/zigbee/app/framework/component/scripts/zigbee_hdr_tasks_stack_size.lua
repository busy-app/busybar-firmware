local changeset = {}

local rtos = slc.is_provided("cmsis_rtos2")
local series_3 = slc.is_provided("device_series_3")

if rtos == true and series_3 == true then
    local cli_task_stack_size_configuration = slc.config('SL_CLI_EXAMPLE_TASK_STACK_SIZE')
    local zb_stack_task_stack_size_configuration = slc.config('SL_ZIGBEE_STACK_RTOS_TASK_STACK_SIZE')
    local zb_app_task_stack_size_configuration = slc.config('SL_ZIGBEE_APP_FRAMEWORK_RTOS_TASK_STACK_SIZE')
    if cli_task_stack_size_configuration ~= nil and zb_stack_task_stack_size_configuration ~= nil and zb_app_task_stack_size_configuration ~= nil then
        local cli_task_stack_size_value = tonumber(cli_task_stack_size_configuration.value)
        local zb_stack_task_stack_size_value = tonumber(zb_stack_task_stack_size_configuration.value)
        local zb_app_task_stack_size_value = tonumber(zb_app_task_stack_size_configuration.value)
        if (cli_task_stack_size_value < 1500) then
            validation.warning(("To prevent stack overflow when using HDR PHY with maximum paylod, ensure the CLI task stack size is configured to at least 1500 words."),
            validation.target_for_defines({"SL_CLI_EXAMPLE_TASK_STACK_SIZE"}),
            nil,
            nil)
        end

        if (zb_stack_task_stack_size_value < 5800) then
            validation.warning(("To prevent stack overflow when using HDR PHY with maximum paylod, ensure the stack task stack size is configured to at least 5800 words."),
            validation.target_for_defines({"SL_ZIGBEE_STACK_RTOS_TASK_STACK_SIZE"}),
            nil,
            nil)
        end

        if (zb_app_task_stack_size_value < 6000) then
            validation.warning(("To prevent stack overflow when using HDR PHY with maximum paylod, ensure the APP framework task stack size is configured to at least 6000 words."),
            validation.target_for_defines({"SL_ZIGBEE_APP_FRAMEWORK_RTOS_TASK_STACK_SIZE"}),
            nil,
            nil)
        end
    end
end

return changeset