#include <CAR-practica2/image.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <CAR-practica2/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <CAR-practica2/stb_image_write.h>

Image::Image(int width, int height, int nChannels)
    : width(width), height(height), nChannels(nChannels),
      data(width * height * nChannels) {}

unsigned char Image::get(int x, int y, int channel) const
{
    return data[index(x, y, channel)];
}

void Image::set(int x, int y, int channel, float value)
{
    data[index(x, y, channel)] = std::clamp(value, 0.0f, 255.0f);
}

Image Image::load(const std::string &path)
{
    Image img;
    unsigned char *raw = stbi_load(path.c_str(), &img.width, &img.height, &img.nChannels, 0);

    if (!raw)
        throw std::runtime_error("Failed to load: " + path);

    img.data.assign(raw, raw + img.width * img.height * img.nChannels);
    stbi_image_free(raw);
    return img;
}

void Image::save_jpg(const std::string &path, int quality) const
{
    if (nChannels == 3)
    {
        stbi_write_jpg(path.c_str(), width, height, 3, data.data(), quality);
    }
    else if (nChannels == 4)
    {
        std::vector<unsigned char> rgb(width * height * 3);
        for (int i = 0, j = 0; i < width * height * 4; i += 4, j += 3)
        {
            rgb[j] = data[i];
            rgb[j + 1] = data[i + 1];
            rgb[j + 2] = data[i + 2];
        }
        stbi_write_jpg(path.c_str(), width, height, 3, rgb.data(), quality);
    }
    else
    {
        throw std::runtime_error("Unsupported channel count for JPG");
    }
}

int Image::index(int x, int y, int channel) const
{
    return (y * width + x) * nChannels + channel;
}
