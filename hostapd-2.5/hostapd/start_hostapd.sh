#!/bin/bash

sudo ifconfig wlan1 down
sudo ifconfig wlan1 10.10.0.1/24 up

sleep 1

sudo service isc-dhcp-server restart
sudo ./hostapd ./hostapd.conf

# disable interface after hostapd terminated
sudo ifconfig wlan1 down
