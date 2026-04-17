local changeset = {}

if not slc.is_provided("device_series_2") and not slc.is_provided("device_series_3") then
    table.insert(changeset, {
        ["status"] = "impossible",
        ["description"] = "From 2024.6.0 and above, Series 0 and Series 1 devices are no longer supported"}
    )
end

return changeset