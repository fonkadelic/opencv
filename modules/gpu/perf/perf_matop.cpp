#include "perf_precomp.hpp"

using namespace std;
using namespace testing;

namespace {

//////////////////////////////////////////////////////////////////////
// SetTo

PERF_TEST_P(Sz_Depth_Cn, MatOp_SetTo, Combine(GPU_TYPICAL_MAT_SIZES, Values(CV_8U, CV_16U, CV_32F, CV_64F), Values(1, 3, 4)))
{
    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);
    int channels = GET_PARAM(2);

    int type = CV_MAKE_TYPE(depth, channels);

    cv::Scalar val(1, 2, 3, 4);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(size, type);

        d_src.setTo(val);

        TEST_CYCLE()
        {
            d_src.setTo(val);
        }
    }
    else
    {
        cv::Mat src(size, type);

        src.setTo(val);

        TEST_CYCLE()
        {
            src.setTo(val);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// SetToMasked

PERF_TEST_P(Sz_Depth_Cn, MatOp_SetToMasked, Combine(GPU_TYPICAL_MAT_SIZES, Values(CV_8U, CV_16U, CV_32F, CV_64F), Values(1, 3, 4)))
{
    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);
    int channels = GET_PARAM(2);

    int type = CV_MAKE_TYPE(depth, channels);

    cv::Mat src(size, type);
    fillRandom(src);

    cv::Mat mask(size, CV_8UC1);
    fillRandom(mask, 0, 2);

    cv::Scalar val(1, 2, 3, 4);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_mask(mask);

        d_src.setTo(val, d_mask);

        TEST_CYCLE()
        {
            d_src.setTo(val, d_mask);
        }
    }
    else
    {
        src.setTo(val, mask);

        TEST_CYCLE()
        {
            src.setTo(val, mask);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// CopyToMasked

PERF_TEST_P(Sz_Depth_Cn, MatOp_CopyToMasked, Combine(GPU_TYPICAL_MAT_SIZES, Values(CV_8U, CV_16U, CV_32F, CV_64F), Values(1, 3, 4)))
{
    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);
    int channels = GET_PARAM(2);

    int type = CV_MAKE_TYPE(depth, channels);

    cv::Mat src(size, type);
    fillRandom(src);

    cv::Mat mask(size, CV_8UC1);
    fillRandom(mask, 0, 2);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_mask(mask);
        cv::gpu::GpuMat d_dst;

        d_src.copyTo(d_dst, d_mask);

        TEST_CYCLE()
        {
            d_src.copyTo(d_dst, d_mask);
        }
    }
    else
    {
        cv::Mat dst;

        src.copyTo(dst, mask);

        TEST_CYCLE()
        {
            src.copyTo(dst, mask);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// ConvertTo

DEF_PARAM_TEST(Sz_2Depth, cv::Size, MatDepth, MatDepth);

PERF_TEST_P(Sz_2Depth, MatOp_ConvertTo, Combine(GPU_TYPICAL_MAT_SIZES, Values(CV_8U, CV_16U, CV_32F, CV_64F), Values(CV_8U, CV_16U, CV_32F, CV_64F)))
{
    cv::Size size = GET_PARAM(0);
    int depth1 = GET_PARAM(1);
    int depth2 = GET_PARAM(2);

    cv::Mat src(size, depth1);
    fillRandom(src);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_dst;

        d_src.convertTo(d_dst, depth2, 0.5, 1.0);

        TEST_CYCLE()
        {
            d_src.convertTo(d_dst, depth2, 0.5, 1.0);
        }
    }
    else
    {
        cv::Mat dst;

        src.convertTo(dst, depth2, 0.5, 1.0);

        TEST_CYCLE()
        {
            src.convertTo(dst, depth2, 0.5, 1.0);
        }
    }
}

} // namespace
