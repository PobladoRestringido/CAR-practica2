#include <CAR-practica2/convolution.hpp>
#include <immintrin.h>
#include <iostream>
#include <chrono>

ConvolutionKernel::ConvolutionKernel(
    std::initializer_list<std::initializer_list<float>> init)
{
    int y = 0;
    for (auto &row : init)
    {
        int x = 0;
        for (auto &v : row)
        {
            data[y][x++] = v;
        }
        y++;
    }
}

void do_scalar_pixel(int x, int y, const Image &img, Image &out, const ConvolutionKernel &kernel)
{
    float acc[4] = {0, 0, 0, 0};

    for (int ky = -1; ky <= 1; ky++)
    {
        for (int kx = -1; kx <= 1; kx++)
        {
            int px = x + kx;
            int py = y + ky;
            float weight = kernel.data[ky + 1][kx + 1];

            for (int c = 0; c < img.nChannels; c++)
            {
                acc[c] += img.get(px, py, c) * weight;
            }
        }
    }

    for (int c = 0; c < img.nChannels; c++)
    {
        out.set(x, y, c, acc[c]);
    }
}

ConvolutionResult Convolver::do_convolve(const Image &img,
                                         const ConvolutionKernel &kernel,
                                         bool use_simd)
{
    using clock = std::chrono::high_resolution_clock;
    auto start = clock::now();

    Image result = use_simd
                       ? apply_simd(img, kernel)
                       : apply_linear(img, kernel);

    auto end = clock::now();
    std::chrono::duration<double> elapsed = end - start;

    return ConvolutionResult{std::move(result), elapsed.count()};
}

Image Convolver::apply_linear(const Image &img, const ConvolutionKernel &kernel)
{
    Image out(img.width, img.height, img.nChannels);

    for (int y = 1; y < img.height - 1; y++)
    {
        for (int x = 1; x < img.width - 1; x++)
        {

            do_scalar_pixel(x, y, img, out, kernel);
        }
    }

    return out;
}

/**
 * apply_simd(img, kernel)
 *
 * Applies a 3×3 convolution to an RGB image using SSE SIMD intrinsics.
 * Processes 4 pixels at a time by packing their R/G/B values into __m128 vectors.
 *
 * Main steps:
 * 1. Iterate over the image interior (skip borders).
 * 2. For each row, process pixels in chunks of 4 using SIMD.
 * 3. For each 3×3 kernel position:
 *      - Load 4 R values into a vector (__m128)
 *      - Load 4 G values into a vector
 *      - Load 4 B values into a vector
 *      - Multiply each vector by the kernel weight
 *      - Accumulate into running sums
 * 4. Convert the final float sums to integers.
 * 5. Clamp to [0,255] and store back into the output image.
 *
 * Intrinsics used:
 * - _mm_setzero_ps      : create a zero vector
 * - _mm_set1_ps         : broadcast a scalar float to all 4 lanes
 * - _mm_set_ps          : build a vector from 4 floats
 * - _mm_mul_ps          : element-wise multiply
 * - _mm_add_ps          : element-wise add
 * - _mm_cvttps_epi32    : convert float→int (truncate)
 * - _mm_store_si128     : store 4 ints to memory
 */
Image Convolver::apply_simd(const Image &img, const ConvolutionKernel &kernel)
{
    const int nChannels = img.nChannels;
    /*
    Stride is simply the number of bytes you must skip to move from the start of one
    image row to the start of the next. It’s used so the code can correctly jump through
    the 1‑D memory buffer as if it were a 2‑D image.
    */
    const int stride = img.width * nChannels;
    Image out = Image(img.width, img.height, nChannels);

    // ITERATE OVER IMAGE'S PIXELS
    for (int imageY = 1; imageY < img.height - 1; imageY++)
    {
        int imageX = 1;
        for (; imageX < img.width - 1 - 3; imageX += 4)
        {
            __m128 sumR = _mm_setzero_ps();
            __m128 sumG = _mm_setzero_ps();
            __m128 sumB = _mm_setzero_ps();

            // ITERATE OVER KERNEL
            for (int kernelY = -1; kernelY <= 1; kernelY++)
            {
                for (int kernelX = -1; kernelX <= 1; kernelX++)
                {
                    float currentKernelWeight = kernel.data[kernelY + 1][kernelX + 1];
                    __m128 vectorizedWeight = _mm_set1_ps(currentKernelWeight);

                    const unsigned char *ptr =
                        img.data.data() + ((imageY + kernelY) * stride + (imageX + kernelX) * nChannels);

                    float r0 = ptr[0];
                    float g0 = ptr[1];
                    float b0 = ptr[2];

                    float r1 = ptr[3];
                    float g1 = ptr[4];
                    float b1 = ptr[5];

                    float r2 = ptr[6];
                    float g2 = ptr[7];
                    float b2 = ptr[8];

                    float r3 = ptr[9];
                    float g3 = ptr[10];
                    float b3 = ptr[11];

                    __m128 R = _mm_set_ps(r3, r2, r1, r0);
                    __m128 G = _mm_set_ps(g3, g2, g1, g0);
                    __m128 B = _mm_set_ps(b3, b2, b1, b0);

                    sumR = _mm_add_ps(sumR, _mm_mul_ps(R, vectorizedWeight));
                    sumG = _mm_add_ps(sumG, _mm_mul_ps(G, vectorizedWeight));
                    sumB = _mm_add_ps(sumB, _mm_mul_ps(B, vectorizedWeight));
                }
            }

            __m128i r32 = _mm_cvttps_epi32(sumR);
            __m128i g32 = _mm_cvttps_epi32(sumG);
            __m128i b32 = _mm_cvttps_epi32(sumB);

            alignas(16) int r[4], g[4], b[4];
            _mm_store_si128((__m128i *)r, r32);
            _mm_store_si128((__m128i *)g, g32);
            _mm_store_si128((__m128i *)b, b32);

            uint8_t *outputPtr = out.data.data() + (imageY * stride + imageX * nChannels);

            for (int i = 0; i < 4; i++)
            {
                outputPtr[i * 3 + 0] = std::clamp(r[i], 0, 255);
                outputPtr[i * 3 + 1] = std::clamp(g[i], 0, 255);
                outputPtr[i * 3 + 2] = std::clamp(b[i], 0, 255);
            }
        }

        // TAIL LOOP
        for (; imageX < img.width - 1; imageX++)
            do_scalar_pixel(imageX, imageY, img, out, kernel);
    }

    return out;
}
