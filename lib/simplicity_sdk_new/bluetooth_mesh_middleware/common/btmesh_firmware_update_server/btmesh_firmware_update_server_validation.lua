-- Calculate maximum number of firmware update server rx message (segment) count
-- The Firmware Update Start and Firmware Update Firmware Metadata Check
-- messages contain metadata and these have variable length because of this.
-- The Firmware Update Start message is always longer in case of the same
-- metadata length because it has more fields.
-- Firmware Update Start message: Opcode (2)
--                                + Update TTL (1)
--                                + Update Timeout Base (2)
--                                + Update BLOB ID (8)
--                                + Update Firmware Image Index (1)
--                                + Incoming Firmware Metadata (Opt:1-255)
-- The max length of an unsegmented access layer message is 11 however the
-- mandatory part of Firmware Update Start message is 13 bytes without any
-- metadata so it never fits into one BT Mesh message only.
local function calc_fw_update_server_max_message_count(metadata_length)
    local TRANSMIC_MIN_SIZE = 4
    local FW_UPDATE_START_MIN_LENGTH = 13
    local fw_update_message_payload_size = FW_UPDATE_START_MIN_LENGTH + metadata_length
    return math.ceil((fw_update_message_payload_size + TRANSMIC_MIN_SIZE) / 12)
end

local metadata_length_name = "SL_BTMESH_FW_UPDATE_SERVER_METADATA_LENGTH_CFG_VAL"
local lpn_min_queue_length_name = "SL_BTMESH_LPN_MIN_QUEUE_LENGTH_CFG_VAL"
local lpn_component_selected = slc.is_selected("btmesh_lpn")

if lpn_component_selected then
    metadata_length = slc.config(metadata_length_name).number
    lpn_min_queue_length = slc.config(lpn_min_queue_length_name).number
    if lpn_min_queue_length < calc_fw_update_server_max_message_count(metadata_length) then
        local problem = "LPN min queue length and max metadata length config inconsistent"
        local description =
            string.format("The maximum firmware update server message count (%i) "
                          .. "derived from max metadata length (%i) shall fit "
                          .. "into LPN min queue length (%i)",
                          calc_fw_update_server_max_message_count(metadata_length),
                          metadata_length,
                          lpn_min_queue_length)
        validation.error(problem,
                         validation.target_for_defines({metadata_length_name,
                                                        lpn_min_queue_length_name}),
                         description,
                         nil)
    end
end
