-- This validation script checks following:
-- On an SoC Trust Center, either select the Security Link Keys and configure
-- link key table size greater than 0, or enable hashed link key option.
local is_host = slc.is_provided("zigbee_ezsp")
local is_trust_center = slc.config("SLI_ZIGBEE_PRIMARY_NETWORK_DEVICE_TYPE").value == "SLI_ZIGBEE_NETWORK_DEVICE_TYPE_COORDINATOR_OR_ROUTER"
local security_link_keys_enabled = slc.is_provided("zigbee_security_link_keys")
local link_key_table_size = "0"
local hashed_link_key_enabled = slc.config("SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_SECURITY_ALLOW_TC_USING_HASHED_LINK_KEY").value == "1"

if security_link_keys_enabled then
    link_key_table_size = slc.config("SL_ZIGBEE_KEY_TABLE_SIZE").value
end

if not is_host and is_trust_center then
    if not hashed_link_key_enabled then
        if not security_link_keys_enabled then
            validation.error("The Network Creator Security component now requires the Security Link Keys component. " ..
            "Please enable Security Link Keys. "..
            "Hashed link keys may be used instead, but use this as a last resort. Hashed link keys will be removed in a future release. "..
            "Using hashed link keys can be done via a configuration within the Network Creator Security component.",
            validation.target_for_project(),
            nil,
            nil)
        elseif link_key_table_size == "0" then
            validation.error("Invalid Link Key Table size.",
            validation.target_for_defines({"SL_ZIGBEE_KEY_TABLE_SIZE"}),
            nil,
            nil)
        end
    end
end