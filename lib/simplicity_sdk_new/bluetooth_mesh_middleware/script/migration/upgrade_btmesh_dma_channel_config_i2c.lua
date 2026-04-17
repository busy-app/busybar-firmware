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
      -- If i2c_core is provided, increment DMA channel count by 2 because the
      -- new i2c driver supports asynchronous transmission and reception using
      -- separate DMA channels.
      if slc.is_provided("i2c_core") then
        new_dma_count = new_dma_count + 2
        table.insert(changeset, {
          ['option'] = 'EMDRV_DMADRV_DMA_CH_COUNT',
          ['value'] = tostring(new_dma_count)
        })
      end
    end
  end
end

return changeset
