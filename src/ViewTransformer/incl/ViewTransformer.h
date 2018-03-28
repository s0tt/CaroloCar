#ifndef VIEWTRANSFORMER_H
#define VIEWTRANSFORMER_H

#include <opencv2/opencv.hpp>

class ViewTransformer
{

public:

    // Methods
    const cv::Mat& toBirdview(const cv::Mat& matCarPerspective);
    const cv::Mat& cutROI(const cv::Mat& matBirdview);
    const cv::Mat& undistort(const cv::Mat& matSrc);
	const std::vector<cv::Point2f>& ViewTransformer::toBirdview(const std::vector<cv::Point2f>& inputPoints);
	const cv::Mat& ViewTransformer::cutTransformedROI(const cv::Mat& matOrig);
	const std::vector<cv::Point2f>& ViewTransformer::getROIpoints();
	
    // Singleton
    static ViewTransformer& getInstance(const cv::Size& size,  const float ROAD_PART_X, const float ROAD_PART_Y_LOW, const float ROAD_PART_Y_HIGH);
    //ViewTransformer(ViewTransformer const&) = delete;
    //void operator=(ViewTransformer const&)  = delete;

private:
    // Methods
    static const cv::Mat& getTransMat(const cv::Size& size);
    static const cv::Mat& getDistortionMat(cv::FileStorage opencvFile);
    static const cv::Mat& getCameraMat(cv::FileStorage opencvFile);

    // Singleton methods
    void init(const cv::Size& size, const float ROAD_PART_X, const float ROAD_PART_Y_LOW, const float ROAD_PART_Y_HIGH);
    ViewTransformer() {}

    // Attributes
    cv::Mat transformationMat;
    cv::Mat cameraMat;
    cv::Mat distortionMat;
    cv::Size size;
	std::vector<cv::Point2f> ROIpoints;

};


#endif // VIEWTRANSFORMER_H
