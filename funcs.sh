
function wait_for_device() {
    local device_ip=$1
    while true; do
        ping -c 1 -W 1 $device_ip
        if [[ $? -eq 0 ]]; then
            break
        fi
        sleep 1
    done
}
