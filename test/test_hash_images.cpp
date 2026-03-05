#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <openssl/sha.h> // or any SHA256 implementation you prefer

#include "convolution.hpp" // your Convolver, Image, Kernel

// Compute SHA256 of a byte buffer
std::string sha256(const std::vector<unsigned char> &data)
{
    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256(data.data(), data.size(), hash);

    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return oss.str();
}

int main()
{
    // Load your test image
    Image img = Image::load("test.png");

    // Example 3×3 kernel
    ConvolutionKernel kernel({{0.f, -1.f, 0.f},
                              {-1.f, 5.f, -1.f},
                              {0.f, -1.f, 0.f}});

    Convolver conv;

    // Run both versions
    ConvolutionResult out_linear = conv.do_convolve(img, kernel, 0);
    ConvolutionResult out_simd = conv.do_convolve(img, kernel, 1);

    // Hash both outputs
    std::string h_linear = sha256(out_linear.output.data);
    std::string h_simd = sha256(out_simd.output.data);

    std::cout << "Linear SHA256: " << h_linear << "\n";
    std::cout << "SIMD   SHA256: " << h_simd << "\n";

    if (h_linear == h_simd)
    {
        std::cout << "Images are IDENTICAL.\n";
        return 0;
    }

    std::cout << "Images DIFFER.\n";

    /*
    // Optional: find first differing pixel
    for (size_t i = 0; i < out_linear.data.size(); i++)
    {
        if (out_linear.data[i] != out_simd.data[i])
        {
            int pixel = i / img.nChannels;
            int channel = i % img.nChannels;
            int x = pixel % img.width;
            int y = pixel / img.width;

            std::cout << "First difference at (x=" << x
                      << ", y=" << y
                      << ", c=" << channel << "): "
                      << "linear=" << (int)out_linear.data[i]
                      << ", simd=" << (int)out_simd.data[i]
                      << "\n";
            break;
        }
    }
    */

    return 0;
}
