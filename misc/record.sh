echo "Input recording time in minutes:"
read total_minutes
DATE_WITH_TIME=$(date +%Y%m%d_%H%M%S)
sudo mkdir "/home/nvidia/recordings/REC_$DATE_WITH_TIME"
for i in $(eval echo {1..$total_minutes})


	do
		echo "Recording minute $i..."
		sudo streamer -q -c /dev/video1 -s 1920x1080 -f jpeg -t 1800 -r 30 -j 75 -w 0 -o /home/nvidia/recordings/REC_$DATE_WITH_TIME/minute$i.avi

	done
