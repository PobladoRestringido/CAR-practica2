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
