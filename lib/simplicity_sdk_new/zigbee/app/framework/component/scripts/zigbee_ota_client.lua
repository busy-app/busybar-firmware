-- This validation script checks following:
-- The OTA Client with signature verify support must be enabled if SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_SIGNATURE_VERIFICATION_SUPPORT sets to 1
local verification_support = slc.config("SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_SIGNATURE_VERIFICATION_SUPPORT").value == "1"
local ota_client_signature_verify_support = slc.is_provided("zigbee_ota_client_signature_verify_support")
if verification_support then
    if ota_client_signature_verify_support == false then
        validation.error("The \"OTA client with signature verify support\" component must be installed first when desiring to use OTA signature verification in the OTA Bootload Cluster Client component.",
                         validation.target_for_defines({"SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_SIGNATURE_VERIFICATION_SUPPORT",
                                                        "SL_CATALOG_ZIGBEE_OTA_CLIENT_SIGNATURE_VERIFY_SUPPORT_PRESENT"}),
                                                       nil,
                                                       nil
                        )
    end
end
