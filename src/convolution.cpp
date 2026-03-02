#include <CAR-practica2/convolution.hpp>
#include <immintrin.h>

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

Image Convolver::apply(const Image &img, const ConvolutionKernel &kernel)
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

Image Convolver::apply_simd(const Image &img, const ConvolutionKernel &kernel)
{
    std::size_t stride = img.width * img.nChannels;
    Image out = Image(img.width, img.height, img.nChannels);

    for (int y = 1; y < img.height - 1; y++)
    {
        int x = 1;

        // SIMD loop: process 4 pixels per iteration
        for (; x < img.width - 1 - 3; x += 4)
        {
            __m128 sum_vec = _mm_setzero_ps();

            for (int ky = -1; ky <= 1; ky++)
            {
                for (int kx = -1; kx <= 1; kx++)
                {
                    float w = kernel.k[ky + 1][kx + 1];
                    __m128 weight = _mm_set1_ps(w);

                    const unsigned char *ptr = img.data.data() + ((y + ky) * stride + (x + kx) * img.nChannels);

                    __m128i bytes = _mm_loadu_si32(ptr);
                    __m128i ints = _mm_cvtepu8_epi32(bytes);
                    __m128 pix = _mm_cvtepi32_ps(ints);

                    sum_vec = _mm_add_ps(sum_vec, _mm_mul_ps(pix, weight));
                }
            }

            __m128i i32 = _mm_cvttps_epi32(sum_vec);
            __m128i i16 = _mm_packs_epi32(i32, i32);
            __m128i i8 = _mm_packus_epi16(i16, i16);

            _mm_storeu_si32(out.data.data() + (y * stride + x * img.nChannels), i8);
        }

        // Tail loop: leftover 1–3 pixels
        for (; x < img.width - 1; x++)
            do_scalar_pixel(x, y, img, out, kernel);
    }

    return out;
}
