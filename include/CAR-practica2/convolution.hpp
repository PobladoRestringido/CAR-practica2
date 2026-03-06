#pragma once
#include "image.hpp"

/**
 * @brief Represents a fixed 3×3 convolution kernel.
 *
 * Stores the kernel weights in row‑major order and allows convenient
 * initialization using nested initializer lists.
 */
class ConvolutionKernel
{
public:
    float data[3][3];

    /**
     * @brief Constructs a 3×3 kernel from nested initializer lists.
     * @param init  Three rows of three floats each, in row‑major order.
     */
    ConvolutionKernel(std::initializer_list<std::initializer_list<float>> init);
};

/**
 * @brief Applies convolution filters to images.
 *
 * Provides static functions for performing 2D convolution on all channels
 * of an Image using a given 3×3 kernel.
 */

struct ConvolutionResult
{
    Image output;
    double elapsed_seconds;
};

class Convolver
{
public:
    /**
     * @brief Applies a 3×3 convolution kernel to an image.
     * @param img     Input image (read‑only).
     * @param kernel  Convolution kernel to apply.
     * @return A new Image containing the filtered result.
     */
    static Image apply_linear(const Image &img, const ConvolutionKernel &kernel);

    Image apply_simd(const Image &img, const ConvolutionKernel &kernel);

    /**
     * @brief Apply a convolution kernel to an image using either SIMD or scalar code.
     *
     * This function acts as a unified entry point for convolution. It selects
     * between the scalar implementation (`apply`) and the SIMD‑accelerated
     * implementation (`apply_simd`) based on the `use_simd` flag.
     *
     * @param img        Input image to be convolved.
     * @param kernel     3×3 convolution kernel.
     * @param use_simd   If true, the SIMD implementation is used; otherwise the
     *                   scalar fallback is used.
     *
     * @return A new Image containing the convolution result.
     *
     * @note The SIMD implementation requires that the image format and memory
     *       layout are compatible with the vectorized code. If in doubt, or when
     *       debugging, set `use_simd = false` to force the scalar path.
     */
    ConvolutionResult do_convolve(const Image &img,
                                  const ConvolutionKernel &kernel,
                                  bool use_simd);
};
