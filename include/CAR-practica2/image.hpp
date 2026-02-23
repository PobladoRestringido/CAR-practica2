#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>

class Image
{
public:
    int width = 0, height = 0, nChannels = 0;
    std::vector<unsigned char> data;

    Image() = default;
    /**
     * @brief Constructs an empty image with the given dimensions.
     * @param width   Image width in pixels.
     * @param height  Image height in pixels.
     * @param channels Number of color channels per pixel.
     */
    Image(int width, int height, int channels);

    /**
     * @brief Returns the value of a specific pixel channel.
     * @param x       X‑coordinate of the pixel.
     * @param y       Y‑coordinate of the pixel.
     * @param channel Channel index (0 ≤ channel < c).
     * @return The channel value as an unsigned byte.
     */
    unsigned char get(int x, int y, int channel) const;

    /**
     * @brief Sets the value of a specific pixel channel.
     * @param x       X‑coordinate of the pixel.
     * @param y       Y‑coordinate of the pixel.
     * @param channel Channel index.
     * @param value   New value, clamped to the range [0, 255].
     */
    void set(int x, int y, int channel, float value);

    /**
     * @brief Loads an image from disk using stb_image.
     * @param path Filesystem path to the image file.
     * @return A fully initialized Image object.
     * @throws std::runtime_error if loading fails.
     */
    static Image load(const std::string &path);

    /**
     * @brief Saves the image as a JPEG file.
     * @param path    Output file path.
     * @param quality JPEG quality (1–100), default is 90.
     * @throws std::runtime_error if saving fails or format unsupported.
     */
    void save_jpg(const std::string &path, int quality = 90) const;

private:
    /**
     * @brief Computes the linear index of a pixel channel in the buffer.
     * @param x       X‑coordinate of the pixel.
     * @param y       Y‑coordinate of the pixel.
     * @param channel Channel index.
     * @return The corresponding 1D index into the data vector.
     */
    int index(int x, int y, int channel) const;
};
