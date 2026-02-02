#!/bin/bash

set -e

NODE_ID=1234
PIN_CODE=20202021

chip-tool pairing onnetwork $NODE_ID $PIN_CODE
