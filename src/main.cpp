#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include <CAR-practica2/image.hpp>
#include <CAR-practica2/convolution.hpp>
#include <chrono>

namespace fs = std::filesystem;

std::vector<std::string> obtener_rutas_imagenes(const std::string &carpeta)
{

    std::vector<std::string> archivos;

    for (const auto &entry : fs::directory_iterator(carpeta))
    {
        if (!entry.is_regular_file())
            continue;

        std::string ext = entry.path().extension().string();
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
            ext == ".bmp" || ext == ".tga")
        {
            archivos.push_back(entry.path().string());
        }
    }
    return archivos;
}

int main(int argc, char **argv)
{
    using clock = std::chrono::high_resolution_clock;
    auto start = clock::now();
    int MAX_N_IMAGES = 50;
    double elapsed_convolution_time = 0;

    bool use_simd = true; // default

    if (argc > 1)
    {
        std::string simd_flag = argv[1];
        if (simd_flag == "0" || simd_flag == "--nosimd")
            use_simd = false;
        else if (simd_flag == "1" || simd_flag == "--simd")
            use_simd = true;
    }

    fs::create_directories("output/");

    std::vector<std::string> paths = obtener_rutas_imagenes("./LostCat-PS/LostCat-PS/pet/");

    if (paths.size() > MAX_N_IMAGES)
    {
        paths.resize(MAX_N_IMAGES);
    }

    ConvolutionKernel edge_kernel = {
        {-1, -1, -1},
        {-1, 8, -1},
        {-1, -1, -1}};

    for (const auto &path : paths)
    {
        try
        {
            Image img;
            img.load(path);
            Convolver convolver;
            ConvolutionResult res = convolver.do_convolve(img, edge_kernel, use_simd);
            elapsed_convolution_time += res.elapsed_seconds;

            std::string filename = path.substr(path.find_last_of("/\\") + 1);
            res.output.save_jpg("output/" + filename);

            std::cout << "Processed: " << filename << "\n";
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }

    auto end = clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Total execution time: " << elapsed.count() << " seconds\n";
    std::cout << "Total convolution time: " << elapsed_convolution_time << " seconds\n";

    return 0;
}
