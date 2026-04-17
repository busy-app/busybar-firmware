local changeset = {}

-- If the common_token_manager is present, always add token_manager
if slc.is_provided('common_token_manager') then
    table.insert(changeset, {
        ['component'] = 'token_manager',
        ['action'] = 'add'
    })
end

return changeset
