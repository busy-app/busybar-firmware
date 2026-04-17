local changeset = {}
-- Migration script for btmesh DMA channel configuration
if slc.is_provided("dmadrv") then
  local old_dma_count_config = slc.config('EMDRV_DMADRV_DMA_CH_COUNT')
  if old_dma_count_config ~= nil then
    local old_dma_count = tonumber(old_dma_count_config.value)
    -- If the old config is present and is not the maximum value, then increment
    -- the DMA channel count only in this case to avoid excess memory usage.
    -- The default maximum value is 8 channels which shall be sufficient for
    -- most use cases.
    if old_dma_count < 8 then
      local new_dma_count = old_dma_count
      -- If iostream_uart_common is provided, then increment DMA channel count
      -- by 1 because iostream_uart_common requires a DMA channel for
      -- transmission as well.
      -- Note: iostream_uart_common used a single DMA channel (for RX) in the past.
      if slc.is_provided("iostream_uart_common") then
        new_dma_count = new_dma_count + 1
        table.insert(changeset, {
          ['option'] = 'EMDRV_DMADRV_DMA_CH_COUNT',
          ['value'] = tostring(new_dma_count)
        })
      end
    end
  end
end

return changeset
