#Script to capture an amount of images with the package streamer

echo "Enter the number of pictures to take:"
read num_pics

for i in $(eval echo {1..$num_pics})
do
	sudo streamer -c /dev/video1 -s 1920x1080 -f jpeg -o $i.jpeg
	echo "Picture $i.jpg taken successfully \n Press any key to continue..."
	read empty
	
done
