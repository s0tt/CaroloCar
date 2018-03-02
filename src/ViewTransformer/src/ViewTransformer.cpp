#include "ViewTransformer.h"
#include <iostream>

// Absolute path is path of build.sh!
const std::string camera_calib_path = "calibration/camera.yaml";

const cv::Mat& ViewTransformer::toBirdview(const cv::Mat& matCarPerspective)
{
    cv::Mat cameraMatExtend;
    cv::Mat cameraMat = getCameraMat();
    //std::cout << "cameraMat = "<< std::endl << " "  << cameraMat.type() << std::endl << std::endl;
    cv::Mat zeroCol = cv::Mat(3, 1, CV_64FC1, cv::Scalar(0));
    std::vector<cv::Mat> matrices = {cameraMat, zeroCol,};
    //std::cout << "Zeros = "<< std::endl << " "  << zeroCol.type() << std::endl << std::endl;
    cv::Mat matArray[] = {cameraMat, zeroCol,};
    //std::cout << "Array = "<< std::endl << " "  << matArray << std::endl << std::endl;
    cv::hconcat(matArray, 2, cameraMatExtend);
    //std::cout << cameraMatExtend << std::endl;
    cv::Mat transformationMat = cameraMatExtend * getTransMat(matCarPerspective);
    static cv::Mat MatBirdview;

    // Apply matrix transformation
    cv::warpPerspective(matCarPerspective,
                        MatBirdview,
                        transformationMat,
                        matCarPerspective.size(),
                        cv::INTER_CUBIC | cv::WARP_INVERSE_MAP);

    return MatBirdview;
}

const cv::Mat& ViewTransformer::cutROI(const cv::Mat& matOrig)
{
	const cv::Size origSize = matOrig.size();
    // cut this amount in percent away on each side
    const float iXCutFactor = 0.3;
    // start all the way from the top until this value (in %)
    const float iYCutFactor = 0.5; // TODO: finaly check for lower boundery
    const int iXTopLeft = static_cast<int>(origSize.width*iXCutFactor);
    const int iYTopLeft = static_cast<int>(origSize.height*0.075);
    const int iXBottomRight = static_cast<int>(origSize.width-(2*iXTopLeft));
    const int iYBottomRight = static_cast<int>(origSize.height-(iYCutFactor*origSize.height));

     // return value
    static cv::Mat resizedMatCut;

    cv::Mat matCut = matOrig(cv::Rect(iXTopLeft,iYTopLeft,iXBottomRight, iYBottomRight));
    cv::resize(matCut, resizedMatCut, origSize);

    return resizedMatCut;
}

const cv::Mat& ViewTransformer::getCameraMat()
{
	cv::FileStorage opencv_file(camera_calib_path, cv::FileStorage::READ);
	static cv::Mat cam_mat;
	opencv_file["camera_matrix"] >> cam_mat;
	opencv_file.release();
	// TODO don't call every time
	//std::cout << "Cam Mat = "<< std::endl << " "  << cam_mat << std::endl << std::endl;
	return cam_mat;
}

const cv::Mat& ViewTransformer::getDistortionMat()
{
	cv::FileStorage opencv_file(camera_calib_path, cv::FileStorage::READ);
	static cv::Mat cam_distortion_mat;
	opencv_file["distortion_coefficients"] >> cam_distortion_mat;
	opencv_file.release();
	//std::cout << "Cam Distortion = "<< std::endl << " "  << cam_distortion_mat << std::endl << std::endl;
	return cam_distortion_mat;
}


const cv::Mat& ViewTransformer::getTransMat(const cv::Mat& matOrigPerspective)
{
    const double w = matOrigPerspective.size().width;
    const double h = matOrigPerspective.size().height;
    const double alpha = -1.046667; // for full size: -0.645444, & small -1.046667
    const double beta = 0.0;
    const double gamma = 0.0;
    const double distance = 80.0; // for full size:  70.0); & small 98.0
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
