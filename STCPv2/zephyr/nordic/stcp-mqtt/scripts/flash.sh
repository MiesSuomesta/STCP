#!/bin/bash

cd /home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0 && (
	west flash -d ../stcp-mqtt/build-nrf9151
)
