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
    float k[3][3];

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
class Convolver
{
public:
    /**
     * @brief Applies a 3×3 convolution kernel to an image.
     * @param img     Input image (read‑only).
     * @param kernel  Convolution kernel to apply.
     * @return A new Image containing the filtered result.
     */
    static Image apply(const Image &img, const ConvolutionKernel &kernel);
};
