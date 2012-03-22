#include "LinearFilter.h"

LinearFilter::LinearFilter(QImage* image, int kernelRadio, int* kernelPtr,
                           ImagePolicy::BorderPolicy policy) :
    Filter(image, kernelRadio, policy),
    kernelPtrInt(kernelPtr),
    kernelPtrDouble(0)
{
}

LinearFilter::LinearFilter(QImage *image, int kernelRadio, double *kernelPtr,
                           ImagePolicy::BorderPolicy policy) :
    Filter(image, kernelRadio, policy),
    kernelPtrInt(0),
    kernelPtrDouble(kernelPtr)
{
}

LinearFilter::~LinearFilter()
{
}

uchar LinearFilter::doFiltering(int x, int y, ColorOffset offset)
{
    if (kernelPtrInt) {
        return doFilteringInt(x, y, offset);
    } else if (kernelPtrDouble) {
        return doFilteringDouble(x, y, offset);
    } else {
        return 0;
    }
}

uchar LinearFilter::doFilteringInt(int x, int y, ColorOffset offset)
{
    // convolution
    int sum = 0;
    for (int i = -kernelRadio; i <= kernelRadio; ++i) {
        for (int j = -kernelRadio; j <= kernelRadio; ++j) {
            sum += getBorderedValue(i, j, offset) *
                    kernelPtrInt[get2DIndex(kernelRadio, i, j)];
        }
    }
    if (sum <= 0) {
        return 0;
    } else if (sum > MAX_BIT_VALUE) {
        return MAX_BIT_VALUE;
    } else {
        return (uchar)sum;
    }
}

uchar LinearFilter::doFilteringDouble(int x, int y, ColorOffset offset)
{
    // convolution
    double sum = 0;
    for (int i = -kernelRadio; i <= kernelRadio; ++i) {
        for (int j = -kernelRadio; j <= kernelRadio; ++j) {
            sum += getBorderedValue(i, j, offset) *
                    kernelPtrDouble[get2DIndex(kernelRadio, i, j)];
        }
    }
    if (sum <= 0) {
        return 0;
    } else if (sum > MAX_BIT_VALUE) {
        return MAX_BIT_VALUE;
    } else {
        return (uchar)sum;
    }
}