#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>
#include <stdarg.h>

#include "ViewTransformer.h"
#ifdef __linux__ 
#include "Serial.h"
#endif

int checkKey();
std::vector<float> drawLines(std::vector<cv::Vec2f> s_lines, cv::Mat& color);
inline double gaussKernel(std::vector<float> angles);
float movingAverage(std::vector<float> angles);
std::vector<cv::Point2f>& getROIpoints(const cv::Size& size, const float ROAD_PART_Y, const float ROAD_PART_X_LOW, const float ROAD_PART_X_HIGH);

const std::string winHough = "Fahrspurerkennung";
const std::string winOrig = "Original";
const std::string winEdge = "Kantenerkennung";
const std::string winUndist = "Undistort";

constexpr float MAX_ANGLE = 50;
constexpr float ROAD_PART_Y = 0.9; //0.9
constexpr float ROAD_PART_X_LOW = 0.28; //0.25
constexpr float ROAD_PART_X_HIGH = 0.6; //0.55
constexpr float_t ANGLE_INFLUENCE = 0.05;

// TODO create different configs
#ifdef __linux__ 
std::string videoPath = "/home/nvidia/CaroloCup/vision/rec/minute3.mp4";
#elif _WIN32
std::string videoPath = "C:/Users/tmonn/Documents/Studienarbeit/Git/vision/rec/minute3.mp4";
#endif

const cv::Size imgSize(1920, 1080);
float angleOld = 0;
int min_threshold_hough = 0;
int max_trackbar = 150;

int main(int argc, char** argv)
{
	if(argc > 1) {
		videoPath = argv[1];
		std::cout << "Video path set to: " << videoPath << std::endl;
	}

#ifdef __linux__ 
	int serial_port = Serial::init_serial();
	if(serial_port == -1) {
		std::cout << "Failed setting up serial port" << std::endl;
		//return 0;
	}
#endif

	ViewTransformer viewTransformer = ViewTransformer::getInstance(imgSize);
	std::vector<cv::Point2f>& ROIpoints = getROIpoints(imgSize, ROAD_PART_Y, ROAD_PART_X_LOW, ROAD_PART_X_HIGH);
	std::cout << "Press ESC to exit, p for pause" << std::endl;
	cv::VideoCapture cap(videoPath);


	// Set camera resolution and frame rate
	cap.set(CV_CAP_PROP_FRAME_WIDTH,imgSize.width);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT,imgSize.height);
	cap.set(CV_CAP_PROP_FPS, 30);

		
	
	// Create windows
	int s_trackbar = 20;
	std::string labelMinThreshHough = "Thresh";
	cv::namedWindow(winEdge, cv::WINDOW_AUTOSIZE);
	cv::namedWindow(winHough, cv::WINDOW_AUTOSIZE);
	cv::namedWindow(winOrig, cv::WINDOW_AUTOSIZE);
	cv::namedWindow(winUndist, cv::WINDOW_AUTOSIZE);
	cv::createTrackbar(labelMinThreshHough, winHough, &s_trackbar, max_trackbar);

	// Read all images from video
	cv::Mat matSrc;
	while (cap.read(matSrc))
	{
		// Compress to smaller size
		//cv::resize(matSrc, matSrc, imgSize);
		

		
		//Undistort image
		cv::Mat matUndist = matSrc;//viewTransformer.undistort(matSrc);
		

		// Transform to birdview
		cv::Mat matBirdview = viewTransformer.toBirdview(matUndist);
	
		// Cut ROI
		cv::Mat matCut;
		matCut = viewTransformer.cutROI(matBirdview, viewTransformer.toBirdview(ROIpoints));
	

		
		// Convert to greyscale
		cv::Mat matSrcGray;
		cv::cvtColor(matCut, matSrcGray, cv::COLOR_RGB2GRAY);

		// Blurr
		cv::Mat matBlurred;
		cv::GaussianBlur(matSrcGray, matBlurred, cv::Size(3, 3), 0, 0, cv::BORDER_DEFAULT);

		// Find threshold
		cv::Mat matBinarized, matTmp;
		double otsu_thresh_val = cv::threshold(matBlurred, matTmp, 0, 255,
			CV_THRESH_BINARY | CV_THRESH_OTSU);
		cv::threshold(matBlurred, matBinarized, 200, std::numeric_limits<uint8_t>::max(),
			cv::THRESH_TOZERO);
		
		
		//Detects all edges
		cv::Mat matEdgesFull;
		cv::Canny(matBinarized, matEdgesFull, otsu_thresh_val * 0.75, otsu_thresh_val);
		
		//Mask detected edges with ROI
		cv::Mat matEdges = viewTransformer.maskEdges(matEdgesFull, viewTransformer.toBirdview(ROIpoints));
	
		
		// Hough Transformation
		std::vector<cv::Vec2f> s_lines;
		cv::HoughLines(matEdges, s_lines, 1, CV_PI / 180, min_threshold_hough + s_trackbar, 0, 0);;
		//cv::Mat matColor = viewTransformer.cutROI(matBirdview);
		
		// Extract angle
		std::vector<float> angles = drawLines(s_lines, matCut);
		float angle = gaussKernel(angles); //movingAverage(angles);
		std::cout << "Angle gliding: " << round(angle) << std::endl;

		// Transmit
#ifdef __linux__ 
		int serial_written = Serial::write_float(serial_port, angle);
		//std::cout << "Written: " << serial_written << std::endl;
#endif


		// Merge picture with part of detected lines
		cv::Mat matFinal= matBirdview.clone();
			
		
		// Show pictures
		cv::resize(matFinal, matFinal, cv::Size(640,640.0/matFinal.size().width*matFinal.size().height));
		imshow(winHough, matFinal);
		cv::resize(matEdges, matEdges, cv::Size(640,640.0/matEdges.size().width*matEdges.size().height));
		imshow(winEdge, matEdges);
		cv::resize(matBirdview, matBirdview, cv::Size(640,640.0/matBirdview.size().width*matBirdview.size().height));
		imshow(winOrig, matBirdview);
		cv::resize(matSrc, matSrc, cv::Size(640,640.0/matUndist.size().width*matUndist.size().height));
		imshow(winUndist, matSrc);

		// Check for Esc or Pause
		if (checkKey() == -1) {
			return 0;
		}
	}
	return 0;
}

inline double gauss(double sigma, double x) {
    double expVal = -1 * (pow(x, 2) / pow(2 * sigma, 2));
    double divider = sqrt(2 * CV_PI * pow(sigma, 2));
    return (1 / divider) * exp(expVal);
}

inline double gaussKernel(std::vector<float> angles) {
    std::vector<double> weight;
	angles.push_back(angleOld);

    for(int i = 0; i < angles.size(); i++) {
		weight.push_back(gauss(3, angles[i]-angleOld));
		//std::cout << "Angle: " << angles[i]-angleOld << " | weight: " << weight[i] << std::endl;
    }
	
	double sum = 0;
	for(int i = 0; i < weight.size(); i++) {
		sum += weight[i];
    }
	
	for(int i = 0; i < weight.size(); i++) {
		weight[i] = weight[i] / sum;
    }
	
	double angle = 0;
	for(int i = 0; i < weight.size(); i++) {
		angle += weight[i] * angles[i];
    }
	angleOld = angle;
    return angle;
}


std::vector<cv::Point2f>& getROIpoints(const cv::Size& size, const float ROAD_PART_Y, const float ROAD_PART_X_LOW, const float ROAD_PART_X_HIGH){

	static std::vector<cv::Point2f> ROIpoints;
	double y_center = imgSize.width /2;
	
	//calculate point coordinates for ROI rectangle
	ROIpoints.push_back(cv::Point(y_center * (1-ROAD_PART_Y), size.height* (1-ROAD_PART_X_HIGH))); //top left
	ROIpoints.push_back(cv::Point(y_center + y_center * ROAD_PART_Y, size.height* (1-ROAD_PART_X_HIGH))); //top right
	ROIpoints.push_back(cv::Point(y_center + y_center * ROAD_PART_Y, size.height* (1-ROAD_PART_X_LOW))); //bottom right
	ROIpoints.push_back(cv::Point(y_center * (1-ROAD_PART_Y), size.height* (1-ROAD_PART_X_LOW))); //bottom left
	
	
	return ROIpoints;
}

std::vector<float> drawLines(std::vector<cv::Vec2f> s_lines, cv::Mat& color) {
	std::vector<float> angles;
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
		angles.push_back(degreeAngle);
		//std::cout << "(" << ") Angle: " << std::to_string(degreeAngle) << " | Theta: " << std::to_string(theta) << std::endl;

		double cos_t = cos(theta);
		double sin_t = sin(theta);

		double x0 = rho * cos_t;
		double y0 = rho * sin_t;
		double alpha = 1000;

		cv::Point pt1(cvRound(x0 + alpha * (-sin_t)), cvRound(y0 + alpha * cos_t));
		cv::Point pt2(cvRound(x0 - alpha * (-sin_t)), cvRound(y0 - alpha * cos_t));
		cv::line(color, pt1, pt2, cv::Scalar(0, 0, 255), 1, CV_AA);
	}
	// Draw oldAngle in green
	cv::line(color, cv::Point(color.size().width / 2, color.size().height), 
	         cv::Point(color.size().width / 2 + tan(angleOld / 180 * CV_PI) * 100, 
			 color.size().height - 100), cv::Scalar(0, 255, 0), 1, CV_AA);

	return angles;
}

float movingAverage(std::vector<float> angles) {
	if (angles.size() == 0)
		return angleOld;
	// TODO weight by delta to gliding 
	float angleAvg;
	float angleSum = 0;
	for (auto const& angle : angles) {
		angleSum += angle;
	}
	angleAvg = angleSum / angles.size();
	float angleNew = angleOld * (1 - ANGLE_INFLUENCE) + angleAvg * ANGLE_INFLUENCE;
	//std::cout << "Angle avg: " << std::to_string(angleAvg) << std::endl;
	angleOld = angleNew;
	return angleNew;
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