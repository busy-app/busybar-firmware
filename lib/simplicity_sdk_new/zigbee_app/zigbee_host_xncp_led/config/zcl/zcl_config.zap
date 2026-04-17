{
  "fileFormat": 2,
  "featureLevel": 103,
  "creator": "zap",
  "keyValuePairs": [
    {
      "key": "commandDiscovery",
      "value": "1"
    },
    {
      "key": "defaultResponsePolicy",
      "value": "always"
    },
    {
      "key": "manufacturerCodes",
      "value": "0x1049"
    }
  ],
  "package": [
    {
      "pathRelativity": "relativeToZap",
      "path": "../../../../../../../../../app/zcl/zcl-zap.json",
      "type": "zcl-properties",
      "category": "zigbee",
      "version": 1,
      "description": "Zigbee Silabs ZCL data"
    },
    {
      "pathRelativity": "relativeToZap",
      "path": "../../../../../gen-template/gen-templates.json",
      "type": "gen-templates-json",
      "category": "zigbee",
      "version": "zigbee-v0"
    }
  ],
  "endpointTypes": [
    {
      "id": 1,
      "name": "Primary",
      "deviceTypeRef": {
        "code": 0,
        "profileId": 260,
        "label": "SL-WWAH-door-lock",
        "name": "SL-WWAH-door-lock"
      },
      "deviceTypes": [
        {
          "code": 0,
          "profileId": 260,
          "label": "SL-WWAH-door-lock",
          "name": "SL-WWAH-door-lock"
        }
      ],
      "deviceVersions": [
        1
      ],
      "deviceIdentifiers": [
        0
      ],
      "deviceTypeName": "SL-WWAH-door-lock",
      "deviceTypeCode": 0,
      "deviceTypeProfileId": 260,
      "clusters": [
        {
          "name": "Basic",
          "code": 0,
          "mfgCode": null,
          "define": "BASIC_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            {
              "name": "ResetToFactoryDefaults",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ],
          "attributes": [
            {
              "name": "ZCL version",
              "code": 0,
              "mfgCode": null,
              "side": "server",
              "type": "int8u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x08",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "power source",
              "code": 7,
              "mfgCode": null,
              "side": "server",
              "type": "enum8",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x00",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "3",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Power Configuration",
          "code": 1,
          "mfgCode": null,
          "define": "POWER_CONFIG_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "2",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Identify",
          "code": 3,
          "mfgCode": null,
          "define": "IDENTIFY_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "Identify",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "IdentifyQueryResponse",
              "code": 0,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "IdentifyQuery",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ],
          "attributes": [
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "client",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "2",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Identify",
          "code": 3,
          "mfgCode": null,
          "define": "IDENTIFY_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            {
              "name": "Identify",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "IdentifyQueryResponse",
              "code": 0,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "IdentifyQuery",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ],
          "attributes": [
            {
              "name": "identify time",
              "code": 0,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x0000",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "2",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "On/off",
          "code": 6,
          "mfgCode": null,
          "define": "ON_OFF_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "Off",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "On",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "Toggle",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ],
          "attributes": [
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "client",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "2",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Alarms",
          "code": 9,
          "mfgCode": null,
          "define": "ALARM_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            {
              "name": "ResetAlarm",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "Alarm",
              "code": 0,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ResetAllAlarms",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ],
          "attributes": [
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x0001",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Time",
          "code": 10,
          "mfgCode": null,
          "define": "TIME_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            {
              "name": "time",
              "code": 0,
              "mfgCode": null,
              "side": "server",
              "type": "utc_time",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "time status",
              "code": 1,
              "mfgCode": null,
              "side": "server",
              "type": "bitmap8",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x00",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "2",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Over the Air Bootloading",
          "code": 25,
          "mfgCode": null,
          "define": "OTA_BOOTLOAD_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "ImageNotify",
              "code": 0,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "QueryNextImageRequest",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "QueryNextImageResponse",
              "code": 2,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ImageBlockRequest",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ImageBlockResponse",
              "code": 5,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "UpgradeEndRequest",
              "code": 6,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "UpgradeEndResponse",
              "code": 7,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ],
          "attributes": [
            {
              "name": "OTA Upgrade Server ID",
              "code": 0,
              "mfgCode": null,
              "side": "client",
              "type": "ieee_address",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0xffffffffffffffff",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "Offset (address) into the file",
              "code": 1,
              "mfgCode": null,
              "side": "client",
              "type": "int32u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0xffffffff",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "OTA Upgrade Status",
              "code": 6,
              "mfgCode": null,
              "side": "client",
              "type": "enum8",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x00",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "client",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "4",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Poll Control",
          "code": 32,
          "mfgCode": null,
          "define": "POLL_CONTROL_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            {
              "name": "CheckIn",
              "code": 0,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "CheckInResponse",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "FastPollStop",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ],
          "attributes": [
            {
              "name": "check-in interval",
              "code": 0,
              "mfgCode": null,
              "side": "server",
              "type": "int32u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x00003840",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "long poll interval",
              "code": 1,
              "mfgCode": null,
              "side": "server",
              "type": "int32u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x00000014",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "short poll interval",
              "code": 2,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x0002",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "fast poll timeout",
              "code": 3,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x0028",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "3",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Door Lock",
          "code": 257,
          "mfgCode": null,
          "define": "DOOR_LOCK_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            {
              "name": "LockDoor",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "LockDoorResponse",
              "code": 0,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "UnlockDoor",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "UnlockDoorResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ],
          "attributes": [
            {
              "name": "lock state",
              "code": 0,
              "mfgCode": null,
              "side": "server",
              "type": "enum8",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "",
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "lock type",
              "code": 1,
              "mfgCode": null,
              "side": "server",
              "type": "enum8",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "actuator enabled",
              "code": 2,
              "mfgCode": null,
              "side": "server",
              "type": "boolean",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "3",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Diagnostics",
          "code": 2821,
          "mfgCode": null,
          "define": "DIAGNOSTICS_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "3",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "SL Works With All Hubs",
          "code": 64599,
          "mfgCode": 4631,
          "define": "SL_WWAH_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            {
              "name": "EnableApsLinkKeyAuthorization",
              "code": 0,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ApsLinkKeyAuthorizationQueryResponse",
              "code": 0,
              "mfgCode": 4631,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "DisableApsLinkKeyAuthorization",
              "code": 1,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "PoweringOffNotification",
              "code": 1,
              "mfgCode": 4631,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ApsLinkKeyAuthorizationQuery",
              "code": 2,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "PoweringOnNotification",
              "code": 2,
              "mfgCode": 4631,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "RequestNewApsLinkKey",
              "code": 3,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ShortAddressChange",
              "code": 3,
              "mfgCode": 4631,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "EnableWwahAppEventRetryAlgorithm",
              "code": 4,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ApsAckEnablementQueryResponse",
              "code": 4,
              "mfgCode": 4631,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "DisableWwahAppEventRetryAlgorithm",
              "code": 5,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "PowerDescriptorChange",
              "code": 5,
              "mfgCode": 4631,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "RequestTime",
              "code": 6,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "NewDebugReportNotification",
              "code": 6,
              "mfgCode": 4631,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "EnableWwahRejoinAlgorithm",
              "code": 7,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "DebugReportQueryResponse",
              "code": 7,
              "mfgCode": 4631,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "DisableWwahRejoinAlgorithm",
              "code": 8,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "TrustCenterForClusterServerQueryResponse",
              "code": 8,
              "mfgCode": 4631,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SetIasZoneEnrollmentMethod",
              "code": 9,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "SurveyBeaconsResponse",
              "code": 9,
              "mfgCode": 4631,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ClearBindingTable",
              "code": 10,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "EnablePeriodicRouterCheckIns",
              "code": 11,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "DisablePeriodicRouterCheckIns",
              "code": 12,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "SetMacPollFailureWaitTime",
              "code": 13,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "SetPendingNetworkUpdate",
              "code": 14,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "RequireApsAcksOnUnicasts",
              "code": 15,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "RemoveApsAcksOnUnicastsRequirement",
              "code": 16,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ApsAckRequirementQuery",
              "code": 17,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "DebugReportQuery",
              "code": 18,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "SurveyBeacons",
              "code": 19,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "DisableOtaDowngrades",
              "code": 20,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "DisableMgmtLeaveWithoutRejoin",
              "code": 21,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "DisableTouchlinkInterpanMessageSupport",
              "code": 22,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "EnableWwahParentClassification",
              "code": 23,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "DisableWwahParentClassification",
              "code": 24,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "EnableTcSecurityOnNtwkKeyRotation",
              "code": 25,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "EnableWwahBadParentRecovery",
              "code": 26,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "DisableWwahBadParentRecovery",
              "code": 27,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "EnableConfigurationMode",
              "code": 28,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "DisableConfigurationMode",
              "code": 29,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "UseTrustCenterForClusterServer",
              "code": 30,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "TrustCenterForClusterServerQuery",
              "code": 31,
              "mfgCode": 4631,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "UseTrustCenterForClusterServerResponse",
              "code": 158,
              "mfgCode": 4631,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ],
          "attributes": [
            {
              "name": "disable ota downgrades",
              "code": 2,
              "mfgCode": 4631,
              "side": "server",
              "type": "boolean",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x00",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "mgmt leave without rejoin enabled",
              "code": 3,
              "mfgCode": 4631,
              "side": "server",
              "type": "boolean",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x01",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "network retry count",
              "code": 4,
              "mfgCode": 4631,
              "side": "server",
              "type": "int8u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0xFF",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "mac retry count",
              "code": 5,
              "mfgCode": 4631,
              "side": "server",
              "type": "int8u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0xFF",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "router checkin enabled",
              "code": 6,
              "mfgCode": 4631,
              "side": "server",
              "type": "boolean",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x00",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "touchlink interpan enabled",
              "code": 7,
              "mfgCode": 4631,
              "side": "server",
              "type": "boolean",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x00",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "wwah parent classification enabled",
              "code": 8,
              "mfgCode": 4631,
              "side": "server",
              "type": "boolean",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x00",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "wwah app event retry enabled",
              "code": 9,
              "mfgCode": 4631,
              "side": "server",
              "type": "boolean",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x01",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "wwah app event retry queue size",
              "code": 10,
              "mfgCode": 4631,
              "side": "server",
              "type": "int8u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x0A",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "wwah rejoin enabled",
              "code": 11,
              "mfgCode": 4631,
              "side": "server",
              "type": "boolean",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x00",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "mac poll failure wait time",
              "code": 12,
              "mfgCode": 4631,
              "side": "server",
              "type": "int8u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x03",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "configuration mode enabled",
              "code": 13,
              "mfgCode": 4631,
              "side": "server",
              "type": "boolean",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x01",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "current debug report id",
              "code": 14,
              "mfgCode": 4631,
              "side": "server",
              "type": "int8u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x00",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "tc security on ntwk key rotation enabled",
              "code": 15,
              "mfgCode": 4631,
              "side": "server",
              "type": "boolean",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x00",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "wwah bad parent recovery enabled",
              "code": 16,
              "mfgCode": 4631,
              "side": "server",
              "type": "boolean",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x00",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "pending network update channel",
              "code": 17,
              "mfgCode": 4631,
              "side": "server",
              "type": "int8u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0xFF",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "pending network update pan id",
              "code": 18,
              "mfgCode": 4631,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0xFFFF",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "ota max offline duration",
              "code": 19,
              "mfgCode": 4631,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x0000",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x0001",
              "reportable": 0,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            }
          ]
        }
      ]
    }
  ],
  "endpoints": [
    {
      "endpointTypeName": "Primary",
      "endpointTypeIndex": 0,
      "profileId": -1,
      "endpointId": 1,
      "networkId": 0,
      "parentEndpointIdentifier": null
    }
  ]
}