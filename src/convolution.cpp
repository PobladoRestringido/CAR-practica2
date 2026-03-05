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
            k[y][x++] = v;
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
            float weight = kernel.k[ky + 1][kx + 1];

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
    std::cout << "Convolver:: You're using LINEAR convolutions";

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

Image Convolver::apply_simd(const Image &img, const ConvolutionKernel &kernel)
{
    std::cout << "Convolver:: You're using SIMD convolutions";

    const int C = img.nChannels; // = 3
    const int stride = img.width * C;

    Image out(img.width, img.height, C);

    for (int y = 1; y < img.height - 1; y++)
    {
        int x = 1;

        for (; x < img.width - 1 - 3; x += 4)
        {
            __m128 sumR = _mm_setzero_ps();
            __m128 sumG = _mm_setzero_ps();
            __m128 sumB = _mm_setzero_ps();

            for (int ky = -1; ky <= 1; ky++)
            {
                for (int kx = -1; kx <= 1; kx++)
                {
                    float w = kernel.k[ky + 1][kx + 1];
                    __m128 weight = _mm_set1_ps(w);

                    const unsigned char *ptr =
                        img.data.data() + ((y + ky) * stride + (x + kx) * C);

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

                    sumR = _mm_add_ps(sumR, _mm_mul_ps(R, weight));
                    sumG = _mm_add_ps(sumG, _mm_mul_ps(G, weight));
                    sumB = _mm_add_ps(sumB, _mm_mul_ps(B, weight));
                }
            }

            __m128i r32 = _mm_cvttps_epi32(sumR);
            __m128i g32 = _mm_cvttps_epi32(sumG);
            __m128i b32 = _mm_cvttps_epi32(sumB);

            alignas(16) int r[4], g[4], b[4];
            _mm_store_si128((__m128i *)r, r32);
            _mm_store_si128((__m128i *)g, g32);
            _mm_store_si128((__m128i *)b, b32);

            uint8_t *dst = out.data.data() + (y * stride + x * C);

            for (int i = 0; i < 4; i++)
            {
                dst[i * 3 + 0] = std::clamp(r[i], 0, 255);
                dst[i * 3 + 1] = std::clamp(g[i], 0, 255);
                dst[i * 3 + 2] = std::clamp(b[i], 0, 255);
            }
        }

        for (; x < img.width - 1; x++)
            do_scalar_pixel(x, y, img, out, kernel);
    }

    return out;
}
