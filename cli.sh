#!/usr/bin/env bash

# DEVICE_IP=10.12.34.1
DEVICE_IP=10.0.4.20
DEVICE_IP_REF=10.0.4.22

if [[ $1 != "" ]]; then
    DEVICE_IP=$1
fi

if [[ $DEVICE_IP == "ref" ]]; then
    DEVICE_IP=$DEVICE_IP_REF
fi

. ./funcs.sh
wait_for_device $DEVICE_IP

set -x -e

TELNET_CFG="$HOME/.telnetrc"
# if not exist create it
if [[ ! -f $TELNET_CFG ]]; then
    touch $TELNET_CFG
fi
if [[ $(cat $TELNET_CFG | md5sum) != "a7406582213a3d53fd4811d05588e754  -" ]]; then
    cp -v $TELNET_CFG $TELNET_CFG.bak
    echo "DEFAULT" > $TELNET_CFG
    echo "  mode character" >> $TELNET_CFG
    echo "  set binary" >> $TELNET_CFG
fi

# nc -c 10.12.34.1 23
telnet $DEVICE_IP



