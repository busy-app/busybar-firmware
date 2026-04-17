--[[
    This script validates that the max circular queue length is greater than or equal to
    the max receive buffer count.
--]]
local max_circular_queue_size_config = slc.config('CIRCULAR_QUEUE_LEN_MAX')
local max_rx_buffer_count_config = slc.config('SL_OPENTHREAD_RADIO_RX_BUFFER_COUNT')


if max_circular_queue_size_config == nil or max_rx_buffer_count_config == nil then
    if max_circular_queue_size_config == nil then
        validation.error("CIRCULAR_QUEUE_LEN_MAX is undefined.",
                         validation.target_for_defines({"CIRCULAR_QUEUE_LEN_MAX"}),
                         "Define CIRCULAR_QUEUE_LEN_MAX equal to or greater than SL_OPENTHREAD_RADIO_RX_BUFFER_COUNT.",
                         nil
        )
    end
    if max_rx_buffer_count_config == nil then
        validation.error("SL_OPENTHREAD_RADIO_RX_BUFFER_COUNT is undefined.",
                         validation.target_for_defines({"SL_OPENTHREAD_RADIO_RX_BUFFER_COUNT"}),
                         "Configure SL_OPENTHREAD_RADIO_RX_BUFFER_COUNT to desired value, recommended value is 16.",
                         nil
        )
    end
elseif tonumber(max_circular_queue_size_config.value) < tonumber(max_rx_buffer_count_config.value) then
    validation.error("The max circular queue length is too small.",
                     validation.target_for_defines({"CIRCULAR_QUEUE_LEN_MAX", "SL_OPENTHREAD_RADIO_RX_BUFFER_COUNT"}),
                     "The max circular queue length must be greater than or equal to the max RX buffer count.",
                     nil
    )
end
