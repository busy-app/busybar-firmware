local changeset = {}

local old_stack_task_priority = slc.config('SL_OPENTHREAD_RTOS_TASK_PRIORITY')
local old_stack_task_size = slc.config('SL_OPENTHREAD_OS_STACK_TASK_SIZE')

function getPriorityString(priority)
    pri = priority
    -- Check if we need to remove parenthesis
    if (string.find(priority,'%(') ~= nil) then
        -- If opening parenthesis is found proceed.
        logit("Removing the parenthesis")
        pri = string.sub(priority, string.find(priority,'%(') + 1, string.find(priority,'%)') - 1)
    end

    return pri
end

if (old_stack_task_priority ~= nil) then
  -- The project had a configuration option for SL_OPENTHREAD_RTOS_TASK_PRIORITY
  -- Remove prior setting for SL_OPENTHREAD_RTOS_TASK_PRIORITY

  table.insert(changeset, {
    ['option'] = 'SL_OPENTHREAD_RTOS_TASK_PRIORITY',
    ['action'] = 'remove'
  })
  -- Re-add setting for SL_OPENTHREAD_RTOS_STACK_TASK_PRIORITY (in bytes)
  val = getPriorityString(old_stack_task_priority.value)
  logit("Stack Priority : " .. val)
  logit("App Priority : " .. val - 1)

  table.insert(changeset, {
    ['option'] = 'SL_OPENTHREAD_RTOS_STACK_TASK_PRIORITY',
    ['value'] = val
  })
  table.insert(changeset, {
    ['option'] = 'SL_OPENTHREAD_RTOS_APP_TASK_PRIORITY',
    ['value'] = tostring(tonumber(val) - 1)
  })
end

if (old_stack_task_size ~= nil) then
    -- The project had a configuration option for SL_OPENTHREAD_OS_STACK_TASK_SIZE
    -- Remove prior setting for SL_OPENTHREAD_OS_STACK_TASK_SIZE
    table.insert(changeset, {
      ['option'] = 'SL_OPENTHREAD_OS_STACK_TASK_SIZE',
      ['action'] = 'remove'
    })
    -- Re-add setting for SL_OPENTHREAD_STACK_TASK_MEM_SIZE (in bytes)
    table.insert(changeset, {
      ['option'] = 'SL_OPENTHREAD_STACK_TASK_MEM_SIZE',
      ['value'] = tostring(tonumber(old_stack_task_size.value))
    })
  end

return changeset