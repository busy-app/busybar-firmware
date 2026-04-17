local changeset = {}

local rtos = slc.is_provided("cmsis_rtos2")

if rtos == true then
    local cli_task_stack_size_configuration = slc.config('SL_CLI_EXAMPLE_TASK_STACK_SIZE')
    if cli_task_stack_size_configuration ~= nil then
        local cli_task_stack_size_value = tonumber(cli_task_stack_size_configuration.value)
        if (cli_task_stack_size_value < 600) then
            validation.error("To prevent stack overflow when using an RTOS, ensure the CLI task stack size is configured to at least 600 words.",
            validation.target_for_defines({"SL_CLI_EXAMPLE_TASK_STACK_SIZE"}),
            nil,
            nil)
        end
    end
end

return changeset