#ifndef VIEWTRANSFORMER_H
#define VIEWTRANSFORMER_H

#include <opencv2/opencv.hpp>

class ViewTransformer
{

public:

    // Methods
    const cv::Mat& toBirdview(const cv::Mat& matCarPerspective);
    const cv::Mat& cutROI(const cv::Mat& matBirdview, const std::vector<cv::Point2f>& ROIpoints);
    const cv::Mat& undistort(const cv::Mat& matSrc);
	const std::vector<cv::Point2f>& ViewTransformer::toBirdview(const std::vector<cv::Point2f>& inputPoints);
	const cv::Mat& ViewTransformer::maskEdges(const cv::Mat& matOrig, const std::vector<cv::Point2f>& ROIpoints);
	
    // Singleton
    static ViewTransformer& getInstance(const cv::Size& size);
    //ViewTransformer(ViewTransformer const&) = delete;
    //void operator=(ViewTransformer const&)  = delete;

private:
    // Methods
    static const cv::Mat& getTransMat(const cv::Size& size);
    static const cv::Mat& getDistortionMat(cv::FileStorage opencvFile);
    static const cv::Mat& getCameraMat(cv::FileStorage opencvFile);

    // Singleton methods
    void init(const cv::Size& size);
    ViewTransformer() {}

    // Attributes
    cv::Mat transformationMat;
    cv::Mat cameraMat;
    cv::Mat distortionMat;
    cv::Size size;

};


#endif // VIEWTRANSFORMER_H
