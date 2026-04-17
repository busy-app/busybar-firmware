-- The 'bluetooth_feature_connection' component requires the correct
-- role-specific components to be selected. If the application has selected the
-- 'bluetooth_feature_connection' but not the role-specific components, then
-- add relevant role components to the project.
local changeset = {}
if slc.is_selected('bluetooth_feature_connection') and
    not slc.is_selected('bluetooth_feature_connection_role_peripheral') and
    not slc.is_selected('bluetooth_feature_connection_role_central') then
    -- The application includes the connection component but not the
    -- role-specific components. Add connection role components for those roles
    -- that the selection of other components can support.
    if slc.is_selected('bluetooth_feature_advertiser') or
       slc.is_selected('bluetooth_feature_pawr_sync') then
        -- Application has components that can create peripheral connections.
        -- Add the peripheral role component.
        table.insert(changeset, {
            ['description'] = 'The application has components that can create peripheral connections, so connection role component for peripheral role is added.',
            ['component'] = 'bluetooth_feature_connection_role_peripheral',
            ['action'] = 'add',
        })
    end
    if slc.is_selected('bluetooth_feature_extended_scanner') or
       slc.is_selected('bluetooth_feature_legacy_scanner') or
       slc.is_selected('bluetooth_feature_scanner') or
       slc.is_selected('bluetooth_feature_pawr_advertiser') then
        -- Application has components that can create central connections. Add
        -- the central role component.
        table.insert(changeset, {
            ['description'] = 'The application has components that can create central connections, so connection role component for central role is added.',
            ['component'] = 'bluetooth_feature_connection_role_central',
            ['action'] = 'add',
        })
    end
end

return changeset
