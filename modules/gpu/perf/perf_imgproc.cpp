#include "perf_precomp.hpp"

using namespace std;
using namespace testing;

namespace {

//////////////////////////////////////////////////////////////////////
// Remap

enum{HALF_SIZE=0, UPSIDE_DOWN, REFLECTION_X, REFLECTION_BOTH};
CV_ENUM(RemapMode, HALF_SIZE, UPSIDE_DOWN, REFLECTION_X, REFLECTION_BOTH);
#define ALL_REMAP_MODES ValuesIn(RemapMode::all())

void generateMap(cv::Mat& map_x, cv::Mat& map_y, int remapMode)
{
    for (int j = 0; j < map_x.rows; ++j)
    {
        for (int i = 0; i < map_x.cols; ++i)
        {
            switch (remapMode)
            {
            case HALF_SIZE:
                if (i > map_x.cols*0.25 && i < map_x.cols*0.75 && j > map_x.rows*0.25 && j < map_x.rows*0.75)
                {
                    map_x.at<float>(j,i) = 2 * (i - map_x.cols * 0.25) + 0.5;
                    map_y.at<float>(j,i) = 2 * (j - map_x.rows * 0.25) + 0.5;
                }
                else
                {
                    map_x.at<float>(j,i) = 0;
                    map_y.at<float>(j,i) = 0;
                }
                break;
            case UPSIDE_DOWN:
                map_x.at<float>(j,i) = i;
                map_y.at<float>(j,i) = map_x.rows - j;
                break;
            case REFLECTION_X:
                map_x.at<float>(j,i) = map_x.cols - i;
                map_y.at<float>(j,i) = j;
                break;
            case REFLECTION_BOTH:
                map_x.at<float>(j,i) = map_x.cols - i;
                map_y.at<float>(j,i) = map_x.rows - j;
                break;
            } // end of switch
        }
    }
}

DEF_PARAM_TEST(Sz_Depth_Cn_Inter_Border_Mode, cv::Size, MatDepth, int, Interpolation, BorderMode, RemapMode);

PERF_TEST_P(Sz_Depth_Cn_Inter_Border_Mode, ImgProc_Remap, Combine(
    GPU_TYPICAL_MAT_SIZES,
    Values(CV_8U, CV_16U, CV_32F),
    Values(1, 3, 4),
    Values(Interpolation(cv::INTER_NEAREST), Interpolation(cv::INTER_LINEAR), Interpolation(cv::INTER_CUBIC)),
    ALL_BORDER_MODES,
    ALL_REMAP_MODES))
{
    declare.time(20.0);

    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);
    int channels = GET_PARAM(2);
    int interpolation = GET_PARAM(3);
    int borderMode = GET_PARAM(4);
    int remapMode = GET_PARAM(5);

    int type = CV_MAKE_TYPE(depth, channels);

    cv::Mat src(size, type);
    fillRandom(src);

    cv::Mat xmap(size, CV_32FC1);
    cv::Mat ymap(size, CV_32FC1);

    generateMap(xmap, ymap, remapMode);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_xmap(xmap);
        cv::gpu::GpuMat d_ymap(ymap);
        cv::gpu::GpuMat d_dst;

        cv::gpu::remap(d_src, d_dst, d_xmap, d_ymap, interpolation, borderMode);

        TEST_CYCLE()
        {
            cv::gpu::remap(d_src, d_dst, d_xmap, d_ymap, interpolation, borderMode);
        }
    }
    else
    {
        cv::Mat dst;

        cv::remap(src, dst, xmap, ymap, interpolation, borderMode);

        TEST_CYCLE()
        {
            cv::remap(src, dst, xmap, ymap, interpolation, borderMode);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// Resize

DEF_PARAM_TEST(Sz_Depth_Cn_Inter_Scale, cv::Size, MatDepth, int, Interpolation, double);

PERF_TEST_P(Sz_Depth_Cn_Inter_Scale, ImgProc_Resize, Combine(
    GPU_TYPICAL_MAT_SIZES,
    Values(CV_8U, CV_16U, CV_32F),
    Values(1, 3, 4),
    ALL_INTERPOLATIONS,
    Values(0.5, 0.3, 2.0)))
{
    declare.time(20.0);

    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);
    int channels = GET_PARAM(2);
    int interpolation = GET_PARAM(3);
    double f = GET_PARAM(4);

    int type = CV_MAKE_TYPE(depth, channels);

    cv::Mat src(size, type);
    fillRandom(src);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_dst;

        cv::gpu::resize(d_src, d_dst, cv::Size(), f, f, interpolation);

        TEST_CYCLE()
        {
            cv::gpu::resize(d_src, d_dst, cv::Size(), f, f, interpolation);
        }
    }
    else
    {
        cv::Mat dst;

        cv::resize(src, dst, cv::Size(), f, f, interpolation);

        TEST_CYCLE()
        {
            cv::resize(src, dst, cv::Size(), f, f, interpolation);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// ResizeArea

DEF_PARAM_TEST(Sz_Depth_Cn_Scale, cv::Size, MatDepth, int, double);

PERF_TEST_P(Sz_Depth_Cn_Scale, ImgProc_ResizeArea, Combine(
    GPU_TYPICAL_MAT_SIZES,
    Values(CV_8U, CV_16U, CV_32F),
    Values(1, 3, 4),
    Values(0.2, 0.1, 0.05)))
{
    declare.time(1.0);

    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);
    int channels = GET_PARAM(2);
    int interpolation = cv::INTER_AREA;
    double f = GET_PARAM(3);

    int type = CV_MAKE_TYPE(depth, channels);

    cv::Mat src(size, type);
    fillRandom(src);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_dst;

        cv::gpu::resize(d_src, d_dst, cv::Size(), f, f, interpolation);

        TEST_CYCLE()
        {
            cv::gpu::resize(d_src, d_dst, cv::Size(), f, f, interpolation);
        }
    }
    else
    {
        cv::Mat dst;

        cv::resize(src, dst, cv::Size(), f, f, interpolation);

        TEST_CYCLE()
        {
            cv::resize(src, dst, cv::Size(), f, f, interpolation);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// WarpAffine

DEF_PARAM_TEST(Sz_Depth_Cn_Inter_Border, cv::Size, MatDepth, int, Interpolation, BorderMode);

PERF_TEST_P(Sz_Depth_Cn_Inter_Border, ImgProc_WarpAffine, Combine(
    GPU_TYPICAL_MAT_SIZES,
    Values(CV_8U, CV_16U, CV_32F),
    Values(1, 3, 4),
    Values(Interpolation(cv::INTER_NEAREST), Interpolation(cv::INTER_LINEAR), Interpolation(cv::INTER_CUBIC)),
    ALL_BORDER_MODES))
{
    declare.time(20.0);

    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);
    int channels = GET_PARAM(2);
    int interpolation = GET_PARAM(3);
    int borderMode = GET_PARAM(4);

    int type = CV_MAKE_TYPE(depth, channels);

    cv::Mat src(size, type);
    fillRandom(src);

    const double aplha = CV_PI / 4;
    double mat[2][3] = { {std::cos(aplha), -std::sin(aplha), src.cols / 2},
                         {std::sin(aplha),  std::cos(aplha), 0}};
    cv::Mat M(2, 3, CV_64F, (void*) mat);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_dst;

        cv::gpu::warpAffine(d_src, d_dst, M, size, interpolation, borderMode);

        TEST_CYCLE()
        {
            cv::gpu::warpAffine(d_src, d_dst, M, size, interpolation, borderMode);
        }
    }
    else
    {
        cv::Mat dst;

        cv::warpAffine(src, dst, M, size, interpolation, borderMode);

        TEST_CYCLE()
        {
            cv::warpAffine(src, dst, M, size, interpolation, borderMode);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// WarpPerspective

PERF_TEST_P(Sz_Depth_Cn_Inter_Border, ImgProc_WarpPerspective, Combine(
    GPU_TYPICAL_MAT_SIZES,
    Values(CV_8U, CV_16U, CV_32F),
    Values(1, 3, 4),
    Values(Interpolation(cv::INTER_NEAREST), Interpolation(cv::INTER_LINEAR), Interpolation(cv::INTER_CUBIC)),
    ALL_BORDER_MODES))
{
    declare.time(20.0);

    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);
    int channels = GET_PARAM(2);
    int interpolation = GET_PARAM(3);
    int borderMode = GET_PARAM(4);

    int type = CV_MAKE_TYPE(depth, channels);

    cv::Mat src(size, type);
    fillRandom(src);

    const double aplha = CV_PI / 4;
    double mat[3][3] = { {std::cos(aplha), -std::sin(aplha), src.cols / 2},
                         {std::sin(aplha),  std::cos(aplha), 0},
                         {0.0,              0.0,             1.0}};
    cv::Mat M(3, 3, CV_64F, (void*) mat);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_dst;

        cv::gpu::warpPerspective(d_src, d_dst, M, size, interpolation, borderMode);

        TEST_CYCLE()
        {
            cv::gpu::warpPerspective(d_src, d_dst, M, size, interpolation, borderMode);
        }
    }
    else
    {
        cv::Mat dst;

        cv::warpPerspective(src, dst, M, size, interpolation, borderMode);

        TEST_CYCLE()
        {
            cv::warpPerspective(src, dst, M, size, interpolation, borderMode);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// CopyMakeBorder

DEF_PARAM_TEST(Sz_Depth_Cn_Border, cv::Size, MatDepth, int, BorderMode);

PERF_TEST_P(Sz_Depth_Cn_Border, ImgProc_CopyMakeBorder, Combine(
    GPU_TYPICAL_MAT_SIZES,
    Values(CV_8U, CV_16U, CV_32F),
    Values(1, 3, 4),
    ALL_BORDER_MODES))
{
    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);
    int channels = GET_PARAM(2);
    int borderMode = GET_PARAM(3);

    int type = CV_MAKE_TYPE(depth, channels);

    cv::Mat src(size, type);
    fillRandom(src);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_dst;

        cv::gpu::copyMakeBorder(d_src, d_dst, 5, 5, 5, 5, borderMode);

        TEST_CYCLE()
        {
            cv::gpu::copyMakeBorder(d_src, d_dst, 5, 5, 5, 5, borderMode);
        }
    }
    else
    {
        cv::Mat dst;

        cv::copyMakeBorder(src, dst, 5, 5, 5, 5, borderMode);

        TEST_CYCLE()
        {
            cv::copyMakeBorder(src, dst, 5, 5, 5, 5, borderMode);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// Threshold

CV_ENUM(ThreshOp, cv::THRESH_BINARY, cv::THRESH_BINARY_INV, cv::THRESH_TRUNC, cv::THRESH_TOZERO, cv::THRESH_TOZERO_INV)
#define ALL_THRESH_OPS ValuesIn(ThreshOp::all())

DEF_PARAM_TEST(Sz_Depth_Op, cv::Size, MatDepth, ThreshOp);

PERF_TEST_P(Sz_Depth_Op, ImgProc_Threshold, Combine(
    GPU_TYPICAL_MAT_SIZES,
    Values(CV_8U, CV_16U, CV_32F, CV_64F),
    ALL_THRESH_OPS))
{
    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);
    int threshOp = GET_PARAM(2);

    cv::Mat src(size, depth);
    fillRandom(src);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_dst;

        cv::gpu::threshold(d_src, d_dst, 100.0, 255.0, threshOp);

        TEST_CYCLE()
        {
            cv::gpu::threshold(d_src, d_dst, 100.0, 255.0, threshOp);
        }
    }
    else
    {
        cv::Mat dst;

        cv::threshold(src, dst, 100.0, 255.0, threshOp);

        TEST_CYCLE()
        {
            cv::threshold(src, dst, 100.0, 255.0, threshOp);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// Integral

PERF_TEST_P(Sz, ImgProc_Integral, GPU_TYPICAL_MAT_SIZES)
{
    cv::Size size = GetParam();

    cv::Mat src(size, CV_8UC1);
    fillRandom(src);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_dst;
        cv::gpu::GpuMat d_buf;

        cv::gpu::integralBuffered(d_src, d_dst, d_buf);

        TEST_CYCLE()
        {
            cv::gpu::integralBuffered(d_src, d_dst, d_buf);
        }
    }
    else
    {
        cv::Mat dst;

        cv::integral(src, dst);

        TEST_CYCLE()
        {
            cv::integral(src, dst);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// IntegralSqr

PERF_TEST_P(Sz, ImgProc_IntegralSqr, GPU_TYPICAL_MAT_SIZES)
{
    cv::Size size = GetParam();

    cv::Mat src(size, CV_8UC1);
    fillRandom(src);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_dst;

        cv::gpu::sqrIntegral(d_src, d_dst);

        TEST_CYCLE()
        {
            cv::gpu::sqrIntegral(d_src, d_dst);
        }
    }
    else
    {
        FAIL();
    }
}

//////////////////////////////////////////////////////////////////////
// HistEvenC1

PERF_TEST_P(Sz_Depth, ImgProc_HistEvenC1, Combine(GPU_TYPICAL_MAT_SIZES, Values(CV_8U, CV_16U, CV_16S)))
{
    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);

    cv::Mat src(size, depth);
    fillRandom(src);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_hist;
        cv::gpu::GpuMat d_buf;

        cv::gpu::histEven(d_src, d_hist, d_buf, 30, 0, 180);

        TEST_CYCLE()
        {
            cv::gpu::histEven(d_src, d_hist, d_buf, 30, 0, 180);
        }
    }
    else
    {
        int hbins = 30;
        float hranges[] = {0.0f, 180.0f};
        int histSize[] = {hbins};
        const float* ranges[] = {hranges};
        int channels[] = {0};

        cv::Mat hist;

        cv::calcHist(&src, 1, channels, cv::Mat(), hist, 1, histSize, ranges);

        TEST_CYCLE()
        {
            cv::calcHist(&src, 1, channels, cv::Mat(), hist, 1, histSize, ranges);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// HistEvenC4

PERF_TEST_P(Sz_Depth, ImgProc_HistEvenC4, Combine(GPU_TYPICAL_MAT_SIZES, Values(CV_8U, CV_16U, CV_16S)))
{
    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);

    cv::Mat src(size, CV_MAKE_TYPE(depth, 4));
    fillRandom(src);

    int histSize[] = {30, 30, 30, 30};
    int lowerLevel[] = {0, 0, 0, 0};
    int upperLevel[] = {180, 180, 180, 180};

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_hist[4];
        cv::gpu::GpuMat d_buf;

        cv::gpu::histEven(d_src, d_hist, d_buf, histSize, lowerLevel, upperLevel);

        TEST_CYCLE()
        {
            cv::gpu::histEven(d_src, d_hist, d_buf, histSize, lowerLevel, upperLevel);
        }
    }
    else
    {
        FAIL();
    }
}

//////////////////////////////////////////////////////////////////////
// CalcHist

PERF_TEST_P(Sz, ImgProc_CalcHist, GPU_TYPICAL_MAT_SIZES)
{
    cv::Size size = GetParam();

    cv::Mat src(size, CV_8UC1);
    fillRandom(src);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_hist;
        cv::gpu::GpuMat d_buf;

        cv::gpu::calcHist(d_src, d_hist, d_buf);

        TEST_CYCLE()
        {
            cv::gpu::calcHist(d_src, d_hist, d_buf);
        }
    }
    else
    {
        FAIL();
    }
}

//////////////////////////////////////////////////////////////////////
// EqualizeHist

PERF_TEST_P(Sz, ImgProc_EqualizeHist, GPU_TYPICAL_MAT_SIZES)
{
    cv::Size size = GetParam();

    cv::Mat src(size, CV_8UC1);
    fillRandom(src);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_dst;
        cv::gpu::GpuMat d_hist;
        cv::gpu::GpuMat d_buf;

        cv::gpu::equalizeHist(d_src, d_dst, d_hist, d_buf);

        TEST_CYCLE()
        {
            cv::gpu::equalizeHist(d_src, d_dst, d_hist, d_buf);
        }
    }
    else
    {
        cv::Mat dst;

        cv::equalizeHist(src, dst);

        TEST_CYCLE()
        {
            cv::equalizeHist(src, dst);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// ColumnSum

PERF_TEST_P(Sz, ImgProc_ColumnSum, GPU_TYPICAL_MAT_SIZES)
{
    cv::Size size = GetParam();

    cv::Mat src(size, CV_32FC1);
    fillRandom(src);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_dst;

        cv::gpu::columnSum(d_src, d_dst);

        TEST_CYCLE()
        {
            cv::gpu::columnSum(d_src, d_dst);
        }
    }
    else
    {
        FAIL();
    }
}

//////////////////////////////////////////////////////////////////////
// Canny

DEF_PARAM_TEST(Image_AppertureSz_L2gradient, string, int, bool);

PERF_TEST_P(Image_AppertureSz_L2gradient, ImgProc_Canny, Combine(
    Values("perf/800x600.jpg", "perf/1280x1024.jpg", "perf/1680x1050.jpg"),
    Values(3, 5),
    Bool()))
{
    string fileName = GET_PARAM(0);
    int apperture_size = GET_PARAM(1);
    bool useL2gradient = GET_PARAM(2);

    cv::Mat image = readImage(fileName, cv::IMREAD_GRAYSCALE);
    ASSERT_FALSE(image.empty());

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_image(image);
        cv::gpu::GpuMat d_dst;
        cv::gpu::CannyBuf d_buf;

        cv::gpu::Canny(d_image, d_buf, d_dst, 50.0, 100.0, apperture_size, useL2gradient);

        TEST_CYCLE()
        {
            cv::gpu::Canny(d_image, d_buf, d_dst, 50.0, 100.0, apperture_size, useL2gradient);
        }
    }
    else
    {
        cv::Mat dst;

        cv::Canny(image, dst, 50.0, 100.0, apperture_size, useL2gradient);

        TEST_CYCLE()
        {
            cv::Canny(image, dst, 50.0, 100.0, apperture_size, useL2gradient);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// MeanShiftFiltering

DEF_PARAM_TEST_1(Image, string);

PERF_TEST_P(Image, ImgProc_MeanShiftFiltering, Values<string>("gpu/meanshift/cones.png"))
{
    declare.time(15.0);

    cv::Mat img = readImage(GetParam());
    ASSERT_FALSE(img.empty());

    cv::Mat rgba;
    cv::cvtColor(img, rgba, cv::COLOR_BGR2BGRA);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(rgba);
        cv::gpu::GpuMat d_dst;

        cv::gpu::meanShiftFiltering(d_src, d_dst, 50, 50);

        TEST_CYCLE()
        {
            cv::gpu::meanShiftFiltering(d_src, d_dst, 50, 50);
        }
    }
    else
    {
        cv::Mat dst;

        cv::pyrMeanShiftFiltering(img, dst, 50, 50);

        TEST_CYCLE()
        {
            cv::pyrMeanShiftFiltering(img, dst, 50, 50);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// MeanShiftProc

PERF_TEST_P(Image, ImgProc_MeanShiftProc, Values<string>("gpu/meanshift/cones.png"))
{
    declare.time(5.0);

    cv::Mat img = readImage(GetParam());
    ASSERT_FALSE(img.empty());

    cv::Mat rgba;
    cv::cvtColor(img, rgba, cv::COLOR_BGR2BGRA);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(rgba);
        cv::gpu::GpuMat d_dstr;
        cv::gpu::GpuMat d_dstsp;

        cv::gpu::meanShiftProc(d_src, d_dstr, d_dstsp, 50, 50);

        TEST_CYCLE()
        {
            cv::gpu::meanShiftProc(d_src, d_dstr, d_dstsp, 50, 50);
        }
    }
    else
    {
        FAIL();
    }
}

//////////////////////////////////////////////////////////////////////
// MeanShiftSegmentation

PERF_TEST_P(Image, ImgProc_MeanShiftSegmentation, Values<string>("gpu/meanshift/cones.png"))
{
    declare.time(5.0);

    cv::Mat img = readImage(GetParam());
    ASSERT_FALSE(img.empty());

    cv::Mat rgba;
    cv::cvtColor(img, rgba, cv::COLOR_BGR2BGRA);

    cv::Mat dst;

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(rgba);

        cv::gpu::meanShiftSegmentation(d_src, dst, 10, 10, 20);

        TEST_CYCLE()
        {
            cv::gpu::meanShiftSegmentation(d_src, dst, 10, 10, 20);
        }
    }
    else
    {
        FAIL();
    }
}

//////////////////////////////////////////////////////////////////////
// BlendLinear

PERF_TEST_P(Sz_Depth_Cn, ImgProc_BlendLinear, Combine(GPU_TYPICAL_MAT_SIZES, Values(CV_8U, CV_32F), Values(1, 3, 4)))
{
    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);
    int channels = GET_PARAM(2);

    int type = CV_MAKE_TYPE(depth, channels);

    cv::Mat img1(size, type);
    fillRandom(img1);

    cv::Mat img2(size, type);
    fillRandom(img2);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_img1(img1);
        cv::gpu::GpuMat d_img2(img2);
        cv::gpu::GpuMat d_weights1(size, CV_32FC1, cv::Scalar::all(0.5));
        cv::gpu::GpuMat d_weights2(size, CV_32FC1, cv::Scalar::all(0.5));
        cv::gpu::GpuMat d_dst;

        cv::gpu::blendLinear(d_img1, d_img2, d_weights1, d_weights2, d_dst);

        TEST_CYCLE()
        {
            cv::gpu::blendLinear(d_img1, d_img2, d_weights1, d_weights2, d_dst);
        }
    }
    else
    {
        FAIL();
    }
}

//////////////////////////////////////////////////////////////////////
// Convolve

DEF_PARAM_TEST(Sz_KernelSz_Ccorr, cv::Size, int, bool);

PERF_TEST_P(Sz_KernelSz_Ccorr, ImgProc_Convolve, Combine(GPU_TYPICAL_MAT_SIZES, Values(17, 27, 32, 64), Bool()))
{
    declare.time(10.0);

    cv::Size size = GET_PARAM(0);
    int templ_size = GET_PARAM(1);
    bool ccorr = GET_PARAM(2);

    cv::Mat image(size, CV_32FC1);
    image.setTo(1.0);

    cv::Mat templ(templ_size, templ_size, CV_32FC1);
    templ.setTo(1.0);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_image = cv::gpu::createContinuous(size, CV_32FC1);
        d_image.upload(image);

        cv::gpu::GpuMat d_templ = cv::gpu::createContinuous(templ_size, templ_size, CV_32FC1);
        d_templ.upload(templ);

        cv::gpu::GpuMat d_dst;
        cv::gpu::ConvolveBuf d_buf;

        cv::gpu::convolve(d_image, d_templ, d_dst, ccorr, d_buf);

        TEST_CYCLE()
        {
            cv::gpu::convolve(d_image, d_templ, d_dst, ccorr, d_buf);
        }
    }
    else
    {
        ASSERT_FALSE(ccorr);

        cv::Mat dst;

        cv::filter2D(image, dst, image.depth(), templ);

        TEST_CYCLE()
        {
            cv::filter2D(image, dst, image.depth(), templ);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// MatchTemplate8U

CV_ENUM(TemplateMethod, cv::TM_SQDIFF, cv::TM_SQDIFF_NORMED, cv::TM_CCORR, cv::TM_CCORR_NORMED, cv::TM_CCOEFF, cv::TM_CCOEFF_NORMED)
#define ALL_TEMPLATE_METHODS ValuesIn(TemplateMethod::all())

DEF_PARAM_TEST(Sz_TemplateSz_Cn_Method, cv::Size, cv::Size, int, TemplateMethod);

PERF_TEST_P(Sz_TemplateSz_Cn_Method, ImgProc_MatchTemplate8U, Combine(
    GPU_TYPICAL_MAT_SIZES,
    Values(cv::Size(5, 5), cv::Size(16, 16), cv::Size(30, 30)),
    Values(1, 3, 4),
    ALL_TEMPLATE_METHODS))
{
    cv::Size size = GET_PARAM(0);
    cv::Size templ_size = GET_PARAM(1);
    int cn = GET_PARAM(2);
    int method = GET_PARAM(3);

    cv::Mat image(size, CV_MAKE_TYPE(CV_8U, cn));
    fillRandom(image);

    cv::Mat templ(templ_size, CV_MAKE_TYPE(CV_8U, cn));
    fillRandom(templ);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_image(image);
        cv::gpu::GpuMat d_templ(templ);
        cv::gpu::GpuMat d_dst;

        cv::gpu::matchTemplate(d_image, d_templ, d_dst, method);

        TEST_CYCLE()
        {
            cv::gpu::matchTemplate(d_image, d_templ, d_dst, method);
        }
    }
    else
    {
        cv::Mat dst;

        cv::matchTemplate(image, templ, dst, method);

        TEST_CYCLE()
        {
            cv::matchTemplate(image, templ, dst, method);
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
// MatchTemplate32F

PERF_TEST_P(Sz_TemplateSz_Cn_Method, ImgProc_MatchTemplate32F, Combine(
    GPU_TYPICAL_MAT_SIZES,
    Values(cv::Size(5, 5), cv::Size(16, 16), cv::Size(30, 30)),
    Values(1, 3, 4),
    Values(TemplateMethod(cv::TM_SQDIFF), TemplateMethod(cv::TM_CCORR))))
{
    cv::Size size = GET_PARAM(0);
    cv::Size templ_size = GET_PARAM(1);
    int cn = GET_PARAM(2);
    int method = GET_PARAM(3);

    cv::Mat image(size, CV_MAKE_TYPE(CV_32F, cn));
    fillRandom(image);

    cv::Mat templ(templ_size, CV_MAKE_TYPE(CV_32F, cn));
    fillRandom(templ);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_image(image);
        cv::gpu::GpuMat d_templ(templ);
        cv::gpu::GpuMat d_dst;

        cv::gpu::matchTemplate(d_image, d_templ, d_dst, method);

        TEST_CYCLE()
        {
            cv::gpu::matchTemplate(d_image, d_templ, d_dst, method);
        }
    }
    else
    {
        cv::Mat dst;

        cv::matchTemplate(image, templ, dst, method);

        TEST_CYCLE()
        {
            cv::matchTemplate(image, templ, dst, method);
        }
    }
};

//////////////////////////////////////////////////////////////////////
// MulSpectrums

CV_FLAGS(DftFlags, 0, cv::DFT_INVERSE, cv::DFT_SCALE, cv::DFT_ROWS, cv::DFT_COMPLEX_OUTPUT, cv::DFT_REAL_OUTPUT)

DEF_PARAM_TEST(Sz_Flags, cv::Size, DftFlags);

PERF_TEST_P(Sz_Flags, ImgProc_MulSpectrums, Combine(
    GPU_TYPICAL_MAT_SIZES,
    Values(0, DftFlags(cv::DFT_ROWS))))
{
    cv::Size size = GET_PARAM(0);
    int flag = GET_PARAM(1);

    cv::Mat a(size, CV_32FC2);
    fillRandom(a, 0, 100);

    cv::Mat b(size, CV_32FC2);
    fillRandom(b, 0, 100);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_a(a);
        cv::gpu::GpuMat d_b(b);
        cv::gpu::GpuMat d_dst;

        cv::gpu::mulSpectrums(d_a, d_b, d_dst, flag);

        TEST_CYCLE()
        {
            cv::gpu::mulSpectrums(d_a, d_b, d_dst, flag);
        }
    }
    else
    {
        cv::Mat dst;

        cv::mulSpectrums(a, b, dst, flag);

        TEST_CYCLE()
        {
            cv::mulSpectrums(a, b, dst, flag);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// MulAndScaleSpectrums

PERF_TEST_P(Sz, ImgProc_MulAndScaleSpectrums, GPU_TYPICAL_MAT_SIZES)
{
    cv::Size size = GetParam();

    float scale = 1.f / size.area();

    cv::Mat src1(size, CV_32FC2);
    fillRandom(src1, 0, 100);

    cv::Mat src2(size, CV_32FC2);
    fillRandom(src2, 0, 100);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src1(src1);
        cv::gpu::GpuMat d_src2(src2);
        cv::gpu::GpuMat d_dst;

        cv::gpu::mulAndScaleSpectrums(d_src1, d_src2, d_dst, cv::DFT_ROWS, scale, false);

        TEST_CYCLE()
        {
            cv::gpu::mulAndScaleSpectrums(d_src1, d_src2, d_dst, cv::DFT_ROWS, scale, false);
        }
    }
    else
    {
        FAIL();
    }
}

//////////////////////////////////////////////////////////////////////
// Dft

PERF_TEST_P(Sz_Flags, ImgProc_Dft, Combine(
    GPU_TYPICAL_MAT_SIZES,
    Values(0, DftFlags(cv::DFT_ROWS), DftFlags(cv::DFT_INVERSE))))
{
    declare.time(10.0);

    cv::Size size = GET_PARAM(0);
    int flag = GET_PARAM(1);

    cv::Mat src(size, CV_32FC2);
    fillRandom(src, 0, 100);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_dst;

        cv::gpu::dft(d_src, d_dst, size, flag);

        TEST_CYCLE()
        {
            cv::gpu::dft(d_src, d_dst, size, flag);
        }
    }
    else
    {
        cv::Mat dst;

        cv::dft(src, dst, flag);

        TEST_CYCLE()
        {
            cv::dft(src, dst, flag);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// CornerHarris

DEF_PARAM_TEST(Image_Type_Border_BlockSz_ApertureSz, string, MatType, BorderMode, int, int);

PERF_TEST_P(Image_Type_Border_BlockSz_ApertureSz, ImgProc_CornerHarris, Combine(
    Values<string>("gpu/stereobm/aloe-L.png"),
    Values(CV_8UC1, CV_32FC1),
    Values(BorderMode(cv::BORDER_REFLECT101), BorderMode(cv::BORDER_REPLICATE), BorderMode(cv::BORDER_REFLECT)),
    Values(3, 5, 7),
    Values(0, 3, 5, 7)))
{
    string fileName = GET_PARAM(0);
    int type = GET_PARAM(1);
    int borderMode = GET_PARAM(2);
    int blockSize = GET_PARAM(3);
    int apertureSize = GET_PARAM(4);

    cv::Mat img = readImage(fileName, cv::IMREAD_GRAYSCALE);
    ASSERT_FALSE(img.empty());
    img.convertTo(img, type, type == CV_32F ? 1.0 / 255.0 : 1.0);

    double k = 0.5;

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_img(img);
        cv::gpu::GpuMat d_dst;
        cv::gpu::GpuMat d_Dx;
        cv::gpu::GpuMat d_Dy;
        cv::gpu::GpuMat d_buf;

        cv::gpu::cornerHarris(d_img, d_dst, d_Dx, d_Dy, d_buf, blockSize, apertureSize, k, borderMode);

        TEST_CYCLE()
        {
            cv::gpu::cornerHarris(d_img, d_dst, d_Dx, d_Dy, d_buf, blockSize, apertureSize, k, borderMode);
        }
    }
    else
    {
        cv::Mat dst;

        cv::cornerHarris(img, dst, blockSize, apertureSize, k, borderMode);

        TEST_CYCLE()
        {
            cv::cornerHarris(img, dst, blockSize, apertureSize, k, borderMode);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// CornerMinEigenVal

PERF_TEST_P(Image_Type_Border_BlockSz_ApertureSz, ImgProc_CornerMinEigenVal, Combine(
    Values<string>("gpu/stereobm/aloe-L.png"),
    Values(CV_8UC1, CV_32FC1),
    Values(BorderMode(cv::BORDER_REFLECT101), BorderMode(cv::BORDER_REPLICATE), BorderMode(cv::BORDER_REFLECT)),
    Values(3, 5, 7),
    Values(0, 3, 5, 7)))
{
    string fileName = GET_PARAM(0);
    int type = GET_PARAM(1);
    int borderMode = GET_PARAM(2);
    int blockSize = GET_PARAM(3);
    int apertureSize = GET_PARAM(4);

    cv::Mat img = readImage(fileName, cv::IMREAD_GRAYSCALE);
    ASSERT_FALSE(img.empty());

    img.convertTo(img, type, type == CV_32F ? 1.0 / 255.0 : 1.0);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_img(img);
        cv::gpu::GpuMat d_dst;
        cv::gpu::GpuMat d_Dx;
        cv::gpu::GpuMat d_Dy;
        cv::gpu::GpuMat d_buf;

        cv::gpu::cornerMinEigenVal(d_img, d_dst, d_Dx, d_Dy, d_buf, blockSize, apertureSize, borderMode);

        TEST_CYCLE()
        {
            cv::gpu::cornerMinEigenVal(d_img, d_dst, d_Dx, d_Dy, d_buf, blockSize, apertureSize, borderMode);
        }
    }
    else
    {
        cv::Mat dst;

        cv::cornerMinEigenVal(img, dst, blockSize, apertureSize, borderMode);

        TEST_CYCLE()
        {
            cv::cornerMinEigenVal(img, dst, blockSize, apertureSize, borderMode);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// BuildWarpPlaneMaps

PERF_TEST_P(Sz, ImgProc_BuildWarpPlaneMaps, GPU_TYPICAL_MAT_SIZES)
{
    cv::Size size = GetParam();

    cv::Mat K = cv::Mat::eye(3, 3, CV_32FC1);
    cv::Mat R = cv::Mat::ones(3, 3, CV_32FC1);
    cv::Mat T = cv::Mat::zeros(1, 3, CV_32F);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_map_x;
        cv::gpu::GpuMat d_map_y;

        cv::gpu::buildWarpPlaneMaps(size, cv::Rect(0, 0, size.width, size.height), K, R, T, 1.0, d_map_x, d_map_y);

        TEST_CYCLE()
        {
            cv::gpu::buildWarpPlaneMaps(size, cv::Rect(0, 0, size.width, size.height), K, R, T, 1.0, d_map_x, d_map_y);
        }
    }
    else
    {
        FAIL();
    }
}

//////////////////////////////////////////////////////////////////////
// BuildWarpCylindricalMaps

PERF_TEST_P(Sz, ImgProc_BuildWarpCylindricalMaps, GPU_TYPICAL_MAT_SIZES)
{
    cv::Size size = GetParam();

    cv::Mat K = cv::Mat::eye(3, 3, CV_32FC1);
    cv::Mat R = cv::Mat::ones(3, 3, CV_32FC1);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_map_x;
        cv::gpu::GpuMat d_map_y;

        cv::gpu::buildWarpCylindricalMaps(size, cv::Rect(0, 0, size.width, size.height), K, R, 1.0, d_map_x, d_map_y);

        TEST_CYCLE()
        {
            cv::gpu::buildWarpCylindricalMaps(size, cv::Rect(0, 0, size.width, size.height), K, R, 1.0, d_map_x, d_map_y);
        }
    }
    else
    {
        FAIL();
    }
}

//////////////////////////////////////////////////////////////////////
// BuildWarpSphericalMaps

PERF_TEST_P(Sz, ImgProc_BuildWarpSphericalMaps, GPU_TYPICAL_MAT_SIZES)
{
    cv::Size size = GetParam();

    cv::Mat K = cv::Mat::eye(3, 3, CV_32FC1);
    cv::Mat R = cv::Mat::ones(3, 3, CV_32FC1);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_map_x;
        cv::gpu::GpuMat d_map_y;

        cv::gpu::buildWarpSphericalMaps(size, cv::Rect(0, 0, size.width, size.height), K, R, 1.0, d_map_x, d_map_y);

        TEST_CYCLE()
        {
            cv::gpu::buildWarpSphericalMaps(size, cv::Rect(0, 0, size.width, size.height), K, R, 1.0, d_map_x, d_map_y);
        }
    }
    else
    {
        FAIL();
    }
}

//////////////////////////////////////////////////////////////////////
// Rotate

DEF_PARAM_TEST(Sz_Depth_Cn_Inter, cv::Size, MatDepth, int, Interpolation);

PERF_TEST_P(Sz_Depth_Cn_Inter, ImgProc_Rotate, Combine(
    GPU_TYPICAL_MAT_SIZES,
    Values(CV_8U, CV_16U, CV_32F),
    Values(1, 3, 4),
    Values(Interpolation(cv::INTER_NEAREST), Interpolation(cv::INTER_LINEAR), Interpolation(cv::INTER_CUBIC))))
{
    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);
    int channels = GET_PARAM(2);
    int interpolation = GET_PARAM(3);

    int type = CV_MAKE_TYPE(depth, channels);

    cv::Mat src(size, type);
    fillRandom(src);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_dst;

        cv::gpu::rotate(d_src, d_dst, size, 30.0, 0, 0, interpolation);

        TEST_CYCLE()
        {
            cv::gpu::rotate(d_src, d_dst, size, 30.0, 0, 0, interpolation);
        }
    }
    else
    {
        FAIL();
    }
}

//////////////////////////////////////////////////////////////////////
// PyrDown

PERF_TEST_P(Sz_Depth_Cn, ImgProc_PyrDown, Combine(
    GPU_TYPICAL_MAT_SIZES,
    Values(CV_8U, CV_16U, CV_32F),
    Values(1, 3, 4)))
{
    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);
    int channels = GET_PARAM(2);

    int type = CV_MAKE_TYPE(depth, channels);

    cv::Mat src(size, type);
    fillRandom(src);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_dst;

        cv::gpu::pyrDown(d_src, d_dst);

        TEST_CYCLE()
        {
            cv::gpu::pyrDown(d_src, d_dst);
        }
    }
    else
    {
        cv::Mat dst;

        cv::pyrDown(src, dst);

        TEST_CYCLE()
        {
            cv::pyrDown(src, dst);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// PyrUp

PERF_TEST_P(Sz_Depth_Cn, ImgProc_PyrUp, Combine(
    GPU_TYPICAL_MAT_SIZES,
    Values(CV_8U, CV_16U, CV_32F),
    Values(1, 3, 4)))
{
    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);
    int channels = GET_PARAM(2);

    int type = CV_MAKE_TYPE(depth, channels);

    cv::Mat src(size, type);
    fillRandom(src);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_dst;

        cv::gpu::pyrUp(d_src, d_dst);

        TEST_CYCLE()
        {
            cv::gpu::pyrUp(d_src, d_dst);
        }
    }
    else
    {
        cv::Mat dst;

        cv::pyrUp(src, dst);

        TEST_CYCLE()
        {
            cv::pyrUp(src, dst);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// CvtColor

DEF_PARAM_TEST(Sz_Depth_Code, cv::Size, MatDepth, CvtColorInfo);

PERF_TEST_P(Sz_Depth_Code, ImgProc_CvtColor, Combine(
    GPU_TYPICAL_MAT_SIZES,
    Values(CV_8U, CV_16U, CV_32F),
    Values(CvtColorInfo(4, 4, cv::COLOR_RGBA2BGRA),
           CvtColorInfo(4, 1, cv::COLOR_BGRA2GRAY),
           CvtColorInfo(1, 4, cv::COLOR_GRAY2BGRA),
           CvtColorInfo(3, 3, cv::COLOR_BGR2XYZ),
           CvtColorInfo(3, 3, cv::COLOR_XYZ2BGR),
           CvtColorInfo(3, 3, cv::COLOR_BGR2YCrCb),
           CvtColorInfo(3, 3, cv::COLOR_YCrCb2BGR),
           CvtColorInfo(3, 3, cv::COLOR_BGR2YUV),
           CvtColorInfo(3, 3, cv::COLOR_YUV2BGR),
           CvtColorInfo(3, 3, cv::COLOR_BGR2HSV),
           CvtColorInfo(3, 3, cv::COLOR_HSV2BGR),
           CvtColorInfo(3, 3, cv::COLOR_BGR2HLS),
           CvtColorInfo(3, 3, cv::COLOR_HLS2BGR),
           CvtColorInfo(3, 3, cv::COLOR_BGR2Lab),
           CvtColorInfo(3, 3, cv::COLOR_RGB2Lab),
           CvtColorInfo(3, 3, cv::COLOR_BGR2Luv),
           CvtColorInfo(3, 3, cv::COLOR_RGB2Luv),
           CvtColorInfo(3, 3, cv::COLOR_Lab2BGR),
           CvtColorInfo(3, 3, cv::COLOR_Lab2RGB),
           CvtColorInfo(3, 3, cv::COLOR_Luv2BGR),
           CvtColorInfo(3, 3, cv::COLOR_Luv2RGB),
           CvtColorInfo(1, 3, cv::COLOR_BayerBG2BGR),
           CvtColorInfo(1, 3, cv::COLOR_BayerGB2BGR),
           CvtColorInfo(1, 3, cv::COLOR_BayerRG2BGR),
           CvtColorInfo(1, 3, cv::COLOR_BayerGR2BGR),
           CvtColorInfo(4, 4, cv::COLOR_RGBA2mRGBA))))
{
    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);
    CvtColorInfo info = GET_PARAM(2);

    cv::Mat src(size, CV_MAKETYPE(depth, info.scn));
    fillRandom(src);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_dst;

        cv::gpu::cvtColor(d_src, d_dst, info.code, info.dcn);

        TEST_CYCLE()
        {
            cv::gpu::cvtColor(d_src, d_dst, info.code, info.dcn);
        }
    }
    else
    {
        cv::Mat dst;

        cv::cvtColor(src, dst, info.code, info.dcn);

        TEST_CYCLE()
        {
            cv::cvtColor(src, dst, info.code, info.dcn);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// SwapChannels

PERF_TEST_P(Sz, ImgProc_SwapChannels, GPU_TYPICAL_MAT_SIZES)
{
    cv::Size size = GetParam();

    cv::Mat src(size, CV_8UC4);
    fillRandom(src);

    const int dstOrder[] = {2, 1, 0, 3};

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);

        cv::gpu::swapChannels(d_src, dstOrder);

        TEST_CYCLE()
        {
            cv::gpu::swapChannels(d_src, dstOrder);
        }
    }
    else
    {
        FAIL();
    }
}

//////////////////////////////////////////////////////////////////////
// AlphaComp

CV_ENUM(AlphaOp, cv::gpu::ALPHA_OVER, cv::gpu::ALPHA_IN, cv::gpu::ALPHA_OUT, cv::gpu::ALPHA_ATOP, cv::gpu::ALPHA_XOR, cv::gpu::ALPHA_PLUS, cv::gpu::ALPHA_OVER_PREMUL, cv::gpu::ALPHA_IN_PREMUL, cv::gpu::ALPHA_OUT_PREMUL, cv::gpu::ALPHA_ATOP_PREMUL, cv::gpu::ALPHA_XOR_PREMUL, cv::gpu::ALPHA_PLUS_PREMUL, cv::gpu::ALPHA_PREMUL)
#define ALL_ALPHA_OPS ValuesIn(AlphaOp::all())

DEF_PARAM_TEST(Sz_Type_Op, cv::Size, MatType, AlphaOp);

PERF_TEST_P(Sz_Type_Op, ImgProc_AlphaComp, Combine(GPU_TYPICAL_MAT_SIZES, Values(CV_8UC4, CV_16UC4, CV_32SC4, CV_32FC4), ALL_ALPHA_OPS))
{
    cv::Size size = GET_PARAM(0);
    int type = GET_PARAM(1);
    int alpha_op = GET_PARAM(2);

    cv::Mat img1(size, type);
    fillRandom(img1);

    cv::Mat img2(size, type);
    fillRandom(img2);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_img1(img1);
        cv::gpu::GpuMat d_img2(img2);
        cv::gpu::GpuMat d_dst;

        cv::gpu::alphaComp(d_img1, d_img2, d_dst, alpha_op);

        TEST_CYCLE()
        {
            cv::gpu::alphaComp(d_img1, d_img2, d_dst, alpha_op);
        }
    }
    else
    {
        FAIL();
    }
}

//////////////////////////////////////////////////////////////////////
// ImagePyramidBuild

PERF_TEST_P(Sz_Depth_Cn, ImgProc_ImagePyramidBuild, Combine(GPU_TYPICAL_MAT_SIZES, Values(CV_8U, CV_16U, CV_32F), Values(1, 3, 4)))
{
    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);
    int channels = GET_PARAM(2);

    int type = CV_MAKE_TYPE(depth, channels);

    cv::Mat src(size, type);
    fillRandom(src);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);

        cv::gpu::ImagePyramid d_pyr;

        d_pyr.build(d_src, 5);

        TEST_CYCLE()
        {
            d_pyr.build(d_src, 5);
        }
    }
    else
    {
        FAIL();
    }
}

//////////////////////////////////////////////////////////////////////
// ImagePyramidGetLayer

PERF_TEST_P(Sz_Depth_Cn, ImgProc_ImagePyramidGetLayer, Combine(GPU_TYPICAL_MAT_SIZES, Values(CV_8U, CV_16U, CV_32F), Values(1, 3, 4)))
{
    cv::Size size = GET_PARAM(0);
    int depth = GET_PARAM(1);
    int channels = GET_PARAM(2);

    int type = CV_MAKE_TYPE(depth, channels);

    cv::Mat src(size, type);
    fillRandom(src);

    cv::Size dstSize(size.width / 2 + 10, size.height / 2 + 10);

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_dst;

        cv::gpu::ImagePyramid d_pyr(d_src, 3);

        d_pyr.getLayer(d_dst, dstSize);

        TEST_CYCLE()
        {
            d_pyr.getLayer(d_dst, dstSize);
        }
    }
    else
    {
        FAIL();
    }
}

//////////////////////////////////////////////////////////////////////
// HoughLines

DEF_PARAM_TEST(Sz_DoSort, cv::Size, bool);

PERF_TEST_P(Sz_DoSort, ImgProc_HoughLines, Combine(GPU_TYPICAL_MAT_SIZES, Bool()))
{
    declare.time(30.0);

    const cv::Size size = GET_PARAM(0);
    const bool doSort = GET_PARAM(1);

    const float rho = 1.0f;
    const float theta = CV_PI / 180.0f;
    const int threshold = 300;

    cv::RNG rng(123456789);

    cv::Mat src(size, CV_8UC1, cv::Scalar::all(0));

    const int numLines = rng.uniform(100, 300);
    for (int i = 0; i < numLines; ++i)
    {
        cv::Point p1(rng.uniform(0, src.cols), rng.uniform(0, src.rows));
        cv::Point p2(rng.uniform(0, src.cols), rng.uniform(0, src.rows));
        cv::line(src, p1, p2, cv::Scalar::all(255), 2);
    }

    if (runOnGpu)
    {
        cv::gpu::GpuMat d_src(src);
        cv::gpu::GpuMat d_lines;
        cv::gpu::GpuMat d_accum;
        cv::gpu::GpuMat d_buf;

        cv::gpu::HoughLines(d_src, d_lines, d_accum, d_buf, rho, theta, threshold, doSort);

        TEST_CYCLE()
        {
            cv::gpu::HoughLines(d_src, d_lines, d_accum, d_buf, rho, theta, threshold, doSort);
        }
    }
    else
    {
        std::vector<cv::Vec2f> lines;
        cv::HoughLines(src, lines, rho, theta, threshold);

        TEST_CYCLE()
        {
            cv::HoughLines(src, lines, rho, theta, threshold);
        }
    }
}

} // namespace
