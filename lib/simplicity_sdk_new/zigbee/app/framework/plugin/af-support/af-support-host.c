#include "af-support-host.h"
#include "command-prototypes.h"

sl_zigbee_af_status_t sli_zigbee_af_support_read_attribute(sl_zigbee_af_attribute_search_record_t *attRecord,
                                                           sl_zigbee_af_attribute_type_t *dataType,
                                                           uint8_t *dataPtr,
                                                           uint16_t readLength)
{
  // call ezsp command to read attribute
  return sl_zigbee_ezsp_read_attribute(attRecord->endpoint, attRecord->clusterId, attRecord->attributeId,
                                       attRecord->clusterMask, attRecord->manufacturerCode, dataType, (uint8_t*)(&readLength), dataPtr);
}

sl_zigbee_af_status_t sli_zigbee_af_support_write_attribute(sl_zigbee_af_attribute_search_record_t *attRecord,
                                                            sl_zigbee_af_attribute_type_t dataType,
                                                            uint8_t *dataPtr,
                                                            bool overrideReadOnlyAndDataType,
                                                            bool justTest)
{
  uint8_t dataLength = 0;
  if (sl_zigbee_af_is_string_attribute_type(dataType)) {
    dataLength = sl_zigbee_af_string_length(dataPtr);
  } else if (sl_zigbee_af_is_long_string_attribute_type(dataType)) {
    dataLength = sl_zigbee_af_long_string_length(dataPtr);
  } else {
    dataLength = sl_zigbee_af_get_data_size(dataType);
  }
  return sl_zigbee_ezsp_write_attribute(attRecord->endpoint, attRecord->clusterId, attRecord->attributeId,
                                        attRecord->clusterMask, attRecord->manufacturerCode, overrideReadOnlyAndDataType, justTest, dataType, dataLength, dataPtr);
}
