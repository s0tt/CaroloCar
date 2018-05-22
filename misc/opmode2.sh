sudo sed -i 's/op_mode=0/op_mode=2/g' /etc/modprobe.d/bcmdhd.conf
sudo shutdown -h now
#changes OP Mode to 2
#allows WiFi Hotspot mode from jetson
