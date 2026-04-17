
component_table = {
    iostream_rtt = 'SL_IOSTREAM_TYPE_RTT',
    iostream_usart = 'SL_IOSTREAM_TYPE_UART',
    iostream_eusart = 'SL_IOSTREAM_TYPE_UART',
    iostream_vuart = 'SL_IOSTREAM_TYPE_VUART'
}

-- Get possible component names for a stream-type
function get_acceptable_components(t, type)
    local result_str = ""
    local cnt = 1
    for key,value in pairs(t) do
        if value == type then
            cnt = cnt + 1
            if cnt > 2 then
                result_str = result_str .. ", " .. key
            else
                result_str = result_str .. key
            end
        end
    end
    return result_str
end

-- Get the selected components based on the stream-type
function get_selected_components_for_type(project, stream_type)
    local arr = {}
    for name, type in pairs(component_table) do
        if (type == stream_type) then
            -- Assumption: the feature that we are interested in is provided by a component with the same name.
            if project.is_provided(name) then
                arr[name] = project.component(name)                
            end
        end
    end
    return arr
end

function len(table)
    local i = 0
    for key,value in pairs(table) do
        i = i + 1
    end
    return i
end

-- Calculate instance count
function get_instance_count_for_components(components, instance_name_set)
    local found = 0
    local ins = tostring(instance_name_set.value):gsub('"','') -- remove "
    for component_name, component in pairs(components) do
        if component.is_instantiable then
            for instance_name,v in pairs(component.instances) do
                if ins == instance_name then
                    found = found + 1
                end
            end
        end
    end
    return found
end

-- Validation script for checking IO stream configuration.
local stream_type = slc.config('SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_STREAM_TYPE')
if stream_type ~= nil then
    local components = get_selected_components_for_type(slc, stream_type.value)
    local component_count = len(components)
    
    if (component_count == 0) then
        local cstring = get_acceptable_components(component_table, stream_type.value)
        validation.error("IO Stream is not found by type!",
                        validation.target_for_defines({'SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_STREAM_TYPE'}),
                        "Selected type " .. stream_type.value:gsub("SL_IOSTREAM_TYPE_", "") ..  " is not present. Chose a different type or add one of the components to the project: " .. cstring,
                        nil)
    else
        stream_name = slc.config('SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_STREAM_INSTANCE')
        local instance_fit = get_instance_count_for_components(components, stream_name)

        if component_count > 1 then
            if instance_fit == 0 then
                validation.error("Multiple IO Stream components found by type but instance is not found by name!",
                                validation.target_for_defines({'SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_STREAM_INSTANCE'}),
                                "Multiple IO Stream components found by type " .. stream_type.value:gsub("SL_IOSTREAM_TYPE_", "") ..  " but no instance found with name '" .. tostring(stream_name.value):gsub('"','') .. "' to resolve the ambiguity. " ..
                                "Modify the instance name configuration or select a different stream type!",
                                nil)
            elseif instance_fit > 1 then
                validation.error("Multiple IO Stream components found by type and multiple instances found by name!",
                                validation.target_for_defines({'SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_STREAM_INSTANCE'}),
                                "Multiple IO Stream components found by type " .. stream_type.value:gsub("SL_IOSTREAM_TYPE_", "") ..  " and multiple instances found with name '" .. tostring(stream_name.value):gsub('"','') .. "'. " ..
                                "Ambiguity could not be resolved! Modify the instance name, remove one of them from the project or select a different stream type!",
                                nil)
            end
        else
            local iter = pairs(components)
            local component_name, component = iter(components)
            if (component.is_instantiable) and (instance_fit < 1) then
                    validation.error("IO Stream found by type but instance is not found by name!",
                    validation.target_for_defines({'SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_STREAM_INSTANCE'}),
                    "IO Stream component found by type " .. stream_type.value:gsub("SL_IOSTREAM_TYPE_", "") ..  " but no instance found with name '" .. tostring(stream_name.value):gsub('"','') .. "'. " ..
                    "Modify the instance name or select another stream type!",
                    nil)            
            end
        end
    end
end