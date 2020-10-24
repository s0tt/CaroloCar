sudo sed -i 's/op_mode=2/op_mode=0/g' /etc/modprobe.d/bcmdhd.conf
sudo shutdown -r now
#changes OP Mode to 0
#Allows WiFi connection from Jetson
