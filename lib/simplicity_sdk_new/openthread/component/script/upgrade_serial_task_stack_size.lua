-- Ensure SL_OPENTHREAD_SERIAL_TASK_MEM_SIZE is at least a minimum recommended value
-- in RTOS-enabled applications.

local changeset = {}

-- Minimum recommended stack size in bytes for the OpenThread RTOS serial task.
local MIN_SERIAL_TASK_STACK_SIZE = 3840

local serial_task_config = slc.config('SL_OPENTHREAD_SERIAL_TASK_MEM_SIZE')
local current_value = nil

if serial_task_config ~= nil and serial_task_config.value ~= nil then
  current_value = tonumber(serial_task_config.value:match("%d+"))
end

if current_value ~= nil and current_value < MIN_SERIAL_TASK_STACK_SIZE then
  table.insert(changeset, {
    ['option'] = 'SL_OPENTHREAD_SERIAL_TASK_MEM_SIZE',
    ['value'] = tostring(MIN_SERIAL_TASK_STACK_SIZE)
  })
end

return changeset
