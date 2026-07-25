#!/bin/bash
set -x
LOCALVERSION="$(date +"-%Y.%m.%d_%H-%M-%S-stcp")" bash build-rpi-package.sh && (
	ssh pi@192.168.1.199 sudo reboot
)

sleep 4;
until ping -c 1 192.168.1.199
do
	echo -n "Not up at ";
	date
	sleep 1
done

bash build-and-deploy.sh


