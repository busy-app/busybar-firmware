from busylib import BusyBar, types

bb = BusyBar(token="tpmHnmYvrGLiiFuUBzTmVfWRC_99edo39adokJxxoE0")
# bb = BusyBar("10.46.21.147")

print(bb.get_version().api_semver)

# print(bb.get_wifi_status())
# print(bb.connect_wifi(types.ConnectRequestConfig(ssid="b1-iot", password="MakeIoTGreatAgain1337", security=types.WifiSecurityMethod.WPA2, ip_config=types.WifiIpConfig(ip_method=types.WifiIpMethod.DHCP))))
# print(bb.get_wifi_status())
# print(bb.disconnect_wifi())

# print(bb.ble_enable())
# print(bb.ble_disable())
