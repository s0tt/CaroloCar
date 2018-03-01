#ifndef VIEWTRANSFORMER_H
#define VIEWTRANSFORMER_H

#include <opencv2/opencv.hpp>

class ViewTransformer
{
public:
   
    static const cv::Mat& toBirdview(const cv::Mat& matCarPerspective);
    static const cv::Mat& cutROI(const cv::Mat& matBirdview);

private:
    static const cv::Mat& getTransMat(const cv::Mat& matOrigPerspective);
	ViewTransformer();
};


#endif // VIEWTRANSFORMER_H
