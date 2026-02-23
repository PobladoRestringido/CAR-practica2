#include <CAR-practica2/convolution.hpp>

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

Image Convolver::apply(const Image &img, const ConvolutionKernel &kernel)
{
    Image out(img.width, img.height, img.nChannels);

    for (int y = 1; y < img.height - 1; y++)
    {
        for (int x = 1; x < img.width - 1; x++)
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
    }

    return out;
}
