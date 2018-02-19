#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>
#include <conio.h>           // may have to modify this line if not using Windows
#include <stdarg.h>

#include "ViewTransformer.h"

cv::Mat readImg(std::string path);
cv::Mat canny(cv::Mat imgOriginal);
void show(int n, cv::Mat img...);
int video(std::string path);
int checkKey();
void drawLines(std::vector<cv::Vec2f> s_lines, cv::Mat& color);

int min_threshold_hough = 20;
int max_trackbar = 150;
const std::string winHough = "Fahrspurerkennung";
const std::string winOrig = "Original";
const std::string winEdge = "Kantenerkennung";
constexpr int32_t MAX_ANGLE = 30;
constexpr float_t ROAD_PART = 0.3;
const std::string videoPath = "C:/Users/tmonn/Documents/Studienarbeit/OpenCV/minute3.mp4";
cv::Size imgSize(640, 360);

int main(int argc, char** argv)
{

	std::cout << "Press ESC to exit, p for pause" << std::endl;
	cv::VideoCapture cap;

	if (!cap.open(videoPath))
	{
		std::cout << "Could not open video" << std::endl;
		return 0;
	}

	// Create windows
	int s_trackbar = 30;
	std::string labelMinThreshHough = "Thresh:" + std::to_string(min_threshold_hough + s_trackbar);
	cv::namedWindow(winEdge, cv::WINDOW_AUTOSIZE);
	cv::namedWindow(winHough, cv::WINDOW_AUTOSIZE);
	cv::namedWindow(winOrig, cv::WINDOW_AUTOSIZE);
	cv::createTrackbar(labelMinThreshHough, winHough, &s_trackbar, max_trackbar);

	// Read all images from video
	cv::Mat matSrc;
	while (cap.read(matSrc))
	{
		// Convert to greyscale
		cv::Mat matSrcGray;
		cv::cvtColor(matSrc, matSrcGray, cv::COLOR_RGB2GRAY);

		// Compress to smaller size
		cv::Mat matResized;
		cv::resize(matSrcGray, matResized, imgSize);

		// Blurr
		cv::Mat matBlurred;
		cv::GaussianBlur(matResized, matBlurred, cv::Size(3, 3), 0, 0, cv::BORDER_DEFAULT);

		// Find threshold
		cv::Mat matBinarized, matTmp;
		double otsu_thresh_val = cv::threshold(matBlurred, matTmp, 0, 255,
			CV_THRESH_BINARY | CV_THRESH_OTSU);
		cv::threshold(matBlurred, matBinarized, 200, std::numeric_limits<uint8_t>::max(),
			cv::THRESH_TOZERO);

		// Edge detection
		cv::Mat matEdges;
		cv::Canny(matBinarized, matEdges, otsu_thresh_val * 0.75, otsu_thresh_val);

		// Transform to birdview
		cv::Mat matBirdview = ViewTransformer::toBirdview(matEdges);

		// Cut ROI
		cv::Mat matCut = ViewTransformer::cutROI(matBirdview);
		cv::Mat matRoad(matCut(cv::Rect(0, matCut.size().height * (1 - ROAD_PART), matCut.size().width, matCut.size().height * ROAD_PART)));

		// Hough Transformation
		std::vector<cv::Vec2f> s_lines;
		cv::HoughLines(matRoad, s_lines, 1, CV_PI / 180, min_threshold_hough + s_trackbar, 0, 0);
		cv::Mat matColor;
		cv::cvtColor(matRoad, matColor, cv::COLOR_GRAY2BGR);
		drawLines(s_lines, matColor);

		// Merge picture with part of detected lines
		cv::Mat matFinal(matCut(cv::Rect(0, 0, matCut.size().width, matCut.size().height * (1 - ROAD_PART))));
		cv::cvtColor(matFinal, matFinal, cv::COLOR_GRAY2BGR);
		matFinal.push_back(matColor);

		// Show pictures
		imshow(winHough, matFinal);
		imshow(winEdge, matEdges);
		imshow(winOrig, matBirdview);

		// Check for Esc or Pause
		if (checkKey() == -1)
			return 0;
	}
	return 0;
}

void drawLines(std::vector<cv::Vec2f> s_lines, cv::Mat& color) {

	cv::Mat lines(imgSize, CV_8UC3);
	for (size_t i = 0; i < s_lines.size(); i++)
	{
		float rho = s_lines[i][0];
		float theta = s_lines[i][1];
		float degreeAngle = (theta * 180 / CV_PI);
		degreeAngle = (degreeAngle>90.0) ? (degreeAngle - 180) : degreeAngle;

		// if the angle is very flat(close to horizontal) 
		if (std::abs(degreeAngle) > MAX_ANGLE)
		{
			continue;
		}
		//std::cout << "(" << ") Angle: " << std::to_string(degreeAngle) << " " << std::to_string(theta) << std::endl;

		double cos_t = cos(theta);
		double sin_t = sin(theta);

		double x0 = rho * cos_t;
		double y0 = rho * sin_t;
		double alpha = 1000;

		cv::Point pt1(cvRound(x0 + alpha * (-sin_t)), cvRound(y0 + alpha * cos_t));
		cv::Point pt2(cvRound(x0 - alpha * (-sin_t)), cvRound(y0 - alpha * cos_t));
		cv::line(color, pt1, pt2, cv::Scalar(0, 0, 200), 3, CV_AA);
	}
}


int checkKey() {

	char cPressedKey;
	bool loop = false;
	do {
		cPressedKey = cv::waitKey(10);
		if (cPressedKey == 27) // Check for Esc Key
		{
			return -1;
		}
		else if ((cPressedKey == ' ') || (cPressedKey == 'p'))
		{
			std::cout << "Code paused, press 'p' or [space] again to resume" << std::endl;
			loop = !loop;
		}
	} while (loop);
	
	return 0;
}


/*
int main(int argc, char** argv)
{
	String imgPath = "C:/Users/tmonn/Documents/Studienarbeit/OpenCV/1.png";

	Mat imgOriginal = readImg(imgPath);
	if(imgOriginal.empty())
		return -1;

	Mat imgCanny = canny(imgOriginal);

	ViewTransformer vt = new ViewTransformer();
	cv::resize(imgCanny, imgCanny, Size(640,360));
	Mat imgTrans = vt.to_birdview(imgCanny);

	// cut ROI with mostly "street"
	//Mat imgCut = vt.cut_ROI_from_birdview(imgTrans, Size(640, 360));


	show(2, imgOriginal, imgTrans);

	return 0;
}


Mat readImg(String path) {
	// Read the image file
	Mat imgOriginal = imread(path);          // open image
	if (imgOriginal.empty()) // Check for failure
	{
		cout << "Could not open or find the image" << endl;
	}
	return imgOriginal;
}

int video(String path) {

	VideoCapture cam(path);
	cam.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	cam.set(CV_CAP_PROP_FRAME_HEIGHT, 480);


	Mat edges;
	namedWindow("edges", CV_WINDOW_NORMAL);

	while (1) {
		Mat frame;
		cam >> frame;
		cvtColor(frame, edges, COLOR_BGR2GRAY);
		GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
		Canny(edges, edges, 50, 175, 3);
		imshow("edges", edges);
		if (waitKey(30) >= 0) break;
	}


	return 0;
}

Mat canny(Mat imgOriginal) {
	Mat imgGrayscale;       // grayscale of input image
	Mat imgBlurred;         // intermediate blured image
	Mat imgCanny;           // Canny edge image

	cvtColor(imgOriginal, imgGrayscale, CV_BGR2GRAY);       // convert to grayscale

	GaussianBlur(imgGrayscale,          // input image
		imgBlurred,                     // output image
		Size(5,5),                     // smoothing window width and height in pixels
		3);                           // sigma value, determines how much the image will be blurred
	resize(imgBlurred, imgBlurred, Size(640, 480));

	Canny(imgBlurred,           // input image
		imgCanny,                   // output image
		150,                         // low threshold
		230);                       // high threshold

									// declare windows
									// note: you can use CV_WINDOW_NORMAL which allows resizing the window
	//show(2, imgBlurred, imgCanny);
	return imgCanny;
}

void show(int n, Mat img...) {
	va_list valist;
	va_start(valist, n);

	String* win = new String[n];

	for (int i = 0; i < n; i++)
	{
		win[i] = "IMG " + std::to_string(i);
	}
	for (int i = 0; i < n; i++)
	{
		namedWindow(win[i], CV_WINDOW_NORMAL);   // or CV_WINDOW_AUTOSIZE for a fixed size window matching the resolution of the image
		imshow(win[i], va_arg(valist, Mat));     // show windows
	}
	va_end(valist);
	waitKey(0);                 // hold windows open until user presses a key
	for (int i = 0; i < n; i++)
	{
		destroyWindow(win[i]); //destroy the created window
	}
}
*/