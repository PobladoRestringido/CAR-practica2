#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <vector>
#include <filesystem>
#include <algorithm>

using namespace std;

#define OUTPUT_DIRECTORY "./output/"

struct Imagen
{
    int w, h, c;
    unsigned char *data;
};

vector<string> obtener_rutas_imagenes(const string &carpeta)
{
    vector<string> archivos;

    for (const auto &entry : filesystem::directory_iterator(carpeta))
    {
        if (!entry.is_regular_file())
            continue;

        string ext = entry.path().extension().string();
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
            ext == ".bmp" || ext == ".tga")
        {
            archivos.push_back(entry.path().string());
        }
    }
    return archivos;
}

/**
 * @brief Computes the linear index of a pixel channel in a flat image buffer.
 *
 * Converts 2D pixel coordinates (x, y) and a channel index into the
 * corresponding 1D offset within a row‑major, tightly packed image array.
 * The layout is assumed to be:
 *
 *     [R G B R G B R G B ...]   for each row
 *
 * @param x         X‑coordinate of the pixel (0 ≤ x < width).
 * @param y         Y‑coordinate of the pixel (0 ≤ y < height).
 * @param c         Channel index (0 ≤ c < channels).
 * @param width     Image width in pixels.
 * @param channels  Number of channels per pixel (e.g., 3 for RGB, 4 for RGBA).
 *
 * @return The linear index into the image buffer for the requested channel.
 */
inline int pixel_index(int x, int y, int c, int width, int channels)
{
    return (y * width + x) * channels + c;
}

inline unsigned char get_channel(const Imagen &img, int x, int y, int c)
{
    return img.data[pixel_index(x, y, c, img.w, img.c)];
}

inline void set_channel(Imagen &img, int x, int y, int c, float value)
{
    img.data[pixel_index(x, y, c, img.w, img.c)] =
        std::clamp(value, 0.0f, 255.0f);
}
/**
 * @brief Applies a 3×3 convolution kernel to a single pixel.
 *
 * Computes the convolution result for the pixel at (x, y) by multiplying each
 * neighbor in its 3×3 neighborhood with the corresponding kernel weight and
 * accumulating the result per channel. The computed value is written into the
 * output image at the same coordinates.
 *
 * @param input     Source image (read‑only).
 * @param output    Destination image where the filtered pixel is written.
 * @param x         X‑coordinate of the pixel being processed.
 * @param y         Y‑coordinate of the pixel being processed.
 * @param kernel    3×3 convolution kernel (row‑major).
 */
void convolve_pixel(const Imagen &input,
                    Imagen &output,
                    int x, int y,
                    const float kernel[3][3])
{
    // Accumulator for each channel (supports up to 4 channels: RGBA)
    float channel_sum[4] = {0, 0, 0, 0};

    // Iterate over the 3×3 neighborhood around (x, y)
    for (int kernel_y = -1; kernel_y <= 1; kernel_y++)
    {
        for (int kernel_x = -1; kernel_x <= 1; kernel_x++)
        {
            int neighbor_x = x + kernel_x;
            int neighbor_y = y + kernel_y;

            float weight = kernel[kernel_y + 1][kernel_x + 1];

            // Accumulate weighted contribution for each channel
            for (int c = 0; c < input.c; c++)
            {
                channel_sum[c] += get_channel(input, neighbor_x, neighbor_y, c) * weight;
            }
        }
    }

    // Write the final clamped result into the output image
    for (int c = 0; c < input.c; c++)
    {
        set_channel(output, x, y, c, channel_sum[c]);
    }
}

/**
 * @brief Applies a 3×3 convolution kernel to an image.
 *
 * This function performs a standard 2D convolution over all channels of the
 * input image, producing a new image with the same dimensions and channel count.
 * Border pixels (the outermost 1‑pixel frame) are left unmodified.
 *
 * @param img     Input image (read‑only).
 * @param kernel  3×3 convolution kernel. Must not be null.
 *
 * @return A new Imagen containing the filtered result. The caller is
 *         responsible for freeing out.data with delete[].
 */
Imagen apply_convolution(const Imagen &img, const float kernel[3][3])
{
    Imagen out;
    out.w = img.w;
    out.h = img.h;
    out.c = img.c;
    out.data = new unsigned char[img.w * img.h * img.c];

    for (int y = 1; y < img.h - 1; y++)
    {
        for (int x = 1; x < img.w - 1; x++)
        {

            float acc[4] = {0, 0, 0, 0};

            for (int ky = -1; ky <= 1; ky++)
            {
                for (int kx = -1; kx <= 1; kx++)
                {

                    int px = x + kx;
                    int py = y + ky;
                    int idx = (py * img.w + px) * img.c;

                    for (int c = 0; c < img.c; c++)
                    {
                        acc[c] += img.data[idx + c] * kernel[ky + 1][kx + 1];
                    }
                }
            }

            int out_idx = (y * img.w + x) * img.c;
            for (int c = 0; c < img.c; c++)
            {
                out.data[out_idx + c] = std::clamp(acc[c], 0.0f, 255.0f);
            }
        }
    }

    return out;
}

int main()
{

    // Ensure output directory exists
    filesystem::create_directories(OUTPUT_DIRECTORY);

    vector<string> paths = obtener_rutas_imagenes("./LostCat-PS/LostCat-PS/pet/");

    for (const string &path : paths)
    {

        Imagen img;
        img.data = stbi_load(path.c_str(), &img.w, &img.h, &img.c, 0); // <-- do NOT force channels

        if (!img.data)
        {
            cerr << "No se pudo cargar: " << path << endl;
            continue;
        }

        cout << "Imagen: " << path
             << " w: " << img.w
             << " h: " << img.h
             << " c: " << img.c << endl;

        // Build output path
        string filename = path.substr(path.find_last_of("/\\") + 1);
        string new_path = OUTPUT_DIRECTORY + filename;

        Imagen output_img;
        output_img.w = img.w;
        output_img.h = img.h;
        output_img.c = img.c;
        output_img.data = new unsigned char[img.w * img.h * img.c];

        float kernel[3][3] = {
            {-1, -1, -1},
            {-1, 8, -1},
            {-1, -1, -1}};

        for (int y = 1; y < img.h - 1; y++)
        {
            for (int x = 1; x < img.w - 1; x++)
            {

                float acc[3] = {0, 0, 0};

                for (int ky = -1; ky <= 1; ky++)
                {
                    for (int kx = -1; kx <= 1; kx++)
                    {

                        int px = x + kx;
                        int py = y + ky;

                        int idx = (py * img.w + px) * img.c;

                        for (int c = 0; c < img.c; c++)
                        {
                            acc[c] += img.data[idx + c] * kernel[ky + 1][kx + 1];
                        }
                    }
                }

                int out_idx = (y * img.w + x) * img.c;
                for (int c = 0; c < img.c; c++)
                {
                    output_img.data[out_idx + c] = std::clamp(acc[c], 0.0f, 255.0f);
                }
            }
        }

        // Write JPG (only works with 3 channels)
        int success = 0;

        if (output_img.c == 3)
        {
            success = stbi_write_jpg(new_path.c_str(), output_img.w, output_img.h, 3, output_img.data, 90);
        }
        else if (output_img.c == 4)
        {
            // Convert RGBA → RGB
            vector<unsigned char> rgb(output_img.w * output_img.h * 3);
            for (int i = 0, j = 0; i < output_img.w * output_img.h * 4; i += 4, j += 3)
            {
                rgb[j] = output_img.data[i];
                rgb[j + 1] = output_img.data[i + 1];
                rgb[j + 2] = output_img.data[i + 2];
            }
            success = stbi_write_jpg(new_path.c_str(), output_img.w, output_img.h, 3, rgb.data(), 90);
        }
        else
        {
            cerr << "Formato no soportado (c=" << output_img.c << "): " << path << endl;
        }

        if (!success)
        {
            cerr << "Error: no se ha podido escribir el archivo " << new_path << endl;
        }

        stbi_image_free(img.data);
    }

    return 0;
}
