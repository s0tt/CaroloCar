#include "ViewTransformer.h"
#include <iostream>

// Absolute path is path of build.sh!
const std::string CAM_CALIB_PATH = "calibration/";

ViewTransformer& ViewTransformer::getInstance(const cv::Size& size,  const float ROAD_PART_X, const float ROAD_PART_Y_LOW, const float ROAD_PART_Y_HIGH)
{
    static ViewTransformer instance; // Guaranteed to be destroyed.
                                     // Instantiated on first use.
    instance.init(size, ROAD_PART_X, ROAD_PART_Y_LOW, ROAD_PART_Y_HIGH);
    return instance;
}

void ViewTransformer::init(const cv::Size& size, const float ROAD_PART_X, const float ROAD_PART_Y_LOW, const float ROAD_PART_Y_HIGH) {
    this->size = size;
	double x_center = size.width /2;
	ROIpoints.push_back(cv::Point(x_center * (1-ROAD_PART_X), size.height* (1-ROAD_PART_Y_HIGH))); //top left
	ROIpoints.push_back(cv::Point(x_center + x_center * ROAD_PART_X, size.height* (1-ROAD_PART_Y_HIGH))); //top right
	ROIpoints.push_back(cv::Point(x_center + x_center * ROAD_PART_X, size.height* (1-ROAD_PART_Y_LOW))); //bottom right
	ROIpoints.push_back(cv::Point(x_center * (1-ROAD_PART_X), size.height* (1-ROAD_PART_Y_LOW))); //bottom left

    std::string camCalibFile = CAM_CALIB_PATH + std::to_string(size.width) + "x" + std::to_string(size.height) + ".yaml";
    cv::FileStorage opencvFile(camCalibFile, cv::FileStorage::READ);
    distortionMat = getDistortionMat(opencvFile);
    cameraMat = getCameraMat(opencvFile);
    //std::cout << "cameraMat = "<< std::endl << " "  << cameraMat.type() << std::endl << std::endl;
    cv::Mat cameraMatExtend;
    cv::Mat zeroCol = cv::Mat(3, 1, CV_64FC1, cv::Scalar(0));
    std::vector<cv::Mat> matrices = {cameraMat, zeroCol,};
    //std::cout << "Zeros = "<< std::endl << " "  << zeroCol.type() << std::endl << std::endl;
    cv::Mat matArray[] = {cameraMat, zeroCol,};
    //std::cout << "Array = "<< std::endl << " "  << matArray << std::endl << std::endl;
    cv::hconcat(matArray, 2, cameraMatExtend);
    //std::cout << cameraMatExtend << std::endl;
    transformationMat = cameraMatExtend * getTransMat(size);
}

const cv::Mat& ViewTransformer::undistort(const cv::Mat& matSrc) {
    static cv::Mat matDest;
    cv::undistort(matSrc, matDest, cameraMat, distortionMat);
    return matDest;
}

const cv::Mat& ViewTransformer::toBirdview(const cv::Mat& matCarPerspective)
{
    static cv::Mat MatBirdview;
    // Apply matrix transformation
    cv::warpPerspective(matCarPerspective,
                        MatBirdview,
                        transformationMat,
                        size,
                        cv::INTER_CUBIC | cv::WARP_INVERSE_MAP);

    return MatBirdview;
}

const std::vector<cv::Point2f>& ViewTransformer::toBirdview(const std::vector<cv::Point2f>& inputPoints)
{
    static std::vector<cv::Point2f> transformedPoints;
    // Apply matrix transformation
	cv::Mat inverseTransMat;
	cv::invert(transformationMat, inverseTransMat);
    cv::perspectiveTransform(inputPoints, transformedPoints, inverseTransMat);
    return transformedPoints;
}

const cv::Mat& ViewTransformer::cutROI(const cv::Mat& matOrig)
{
    const cv::Size origSize = matOrig.size();

	
     // return value
    static cv::Mat resizedMatCut;
	//std::cout << ROIpoints.at(0)<< "|" << ROIpoints.at(2) << std::endl;
    static cv::Mat matCut;
	matCut = matOrig(cv::Rect(ROIpoints.at(0), ROIpoints.at(2)));
	//TODO think if resize or return new size

	return matCut;
}

const std::vector<cv::Point2f>& ViewTransformer::getROIpoints()
{
	return ROIpoints;
}

const cv::Mat& ViewTransformer::cutTransformedROI(const cv::Mat& matOrig)
{
	std::vector<cv::Point2f> ROIpointsBirdview = toBirdview(ROIpoints);
	//std::cout << "ROIpointsBirdview" <<  ROIpointsBirdview << std::endl;
	cv::Point ROIpointArray[1][4];
		ROIpointArray[0][0] = ROIpointsBirdview.at(0);
		ROIpointArray[0][1] = ROIpointsBirdview.at(1);
		ROIpointArray[0][2] = ROIpointsBirdview.at(2);
		ROIpointArray[0][3] = ROIpointsBirdview.at(3);
		
		const cv::Point* ppt[1] = { ROIpointArray[0] };
		int npt[] = { 4 };
	
	
	cv::Mat mask(this->size,  matOrig.type(), cv::Scalar(0,0,0));
	cv::fillPoly(mask, ppt, npt, 1, cv::Scalar( 255, 255, 255 ),  8);
	
	
     // return value
    static cv::Mat matReturn;
	cv::Mat maskROI;
	if(matOrig.size != mask.size){
		maskROI = cutROI(mask);
	}else{
		maskROI = mask;
	}
	
	matOrig.copyTo(matReturn, maskROI);
    
	//TODO think if resize or return new size

	return matReturn;
}


const cv::Mat& ViewTransformer::getCameraMat(cv::FileStorage opencvFile)
{
	static cv::Mat cam_mat;
	opencvFile["camera_matrix"] >> cam_mat;
	opencvFile.release();
	//std::cout << "Cam Mat = "<< std::endl << " "  << cam_mat << std::endl << std::endl;
	return cam_mat;
}

const cv::Mat& ViewTransformer::getDistortionMat(cv::FileStorage opencvFile)
{
	static cv::Mat cam_distortion_mat;
	opencvFile["distortion_coefficients"] >> cam_distortion_mat;
	opencvFile.release();
	//std::cout << "Cam Distortion = "<< std::endl << " "  << cam_distortion_mat << std::endl << std::endl;
	return cam_distortion_mat;
}


const cv::Mat& ViewTransformer::getTransMat(const cv::Size& size)
{
    const double w = size.width;
    const double h = size.height;
    const double alpha = -1.046667; // for full size: -0.645444, & small -1.046667
    const double beta = 0.0;
    const double gamma = 0.0;
    const double distance = 510.0; // for full size:  70.0); & small 98.0
    const double f = 500.0;

    //----------------------------------------------------------------------------
    // Projection 2D -> 3D cv::Matrix
    const cv::Mat A1 = (cv::Mat_<double>(4, 3) <<
            1, 0, -w / 2,
            0, 1, -h / 2,
            0, 0, 0,
            0, 0, 1);

    // Rotation matrices around the X,Y,Z axis
    const cv::Mat RX = (cv::Mat_<double>(4, 4) <<
            1, 0, 0, 0,
            0, cos(alpha), -sin(alpha), 0,
            0, sin(alpha), cos(alpha), 0,
            0, 0, 0, 1);

    const cv::Mat RY = (cv::Mat_<double>(4, 4) <<
            cos(beta), 0, -sin(beta), 0,
            0, 1, 0, 0,
            sin(beta), 0, cos(beta), 0,
            0, 0, 0, 1);

    const cv::Mat RZ = (cv::Mat_<double>(4, 4) <<
            cos(gamma), -sin(gamma), 0, 0,
            sin(gamma), cos(gamma), 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1);

    // Composed rotation matrix with (RX,RY,RZ)
    const cv::Mat R = RX * RY * RZ;

    // Translation matrix on the Z axis change distance will change the height
    const cv::Mat T = (cv::Mat_<double>(4, 4) <<
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, distance,
            0, 0, 0, 1);

    // Camera Intrisecs matrix 3D -> 2D
    const cv::Mat A2 = (cv::Mat_<double>(3, 4) <<
            f, 0, w / 2, 0,
            0, f, h / 2, 0,
            0, 0, 1, 0);

    static cv::Mat transMat = (T * (R * A1));

    return transMat;
}
