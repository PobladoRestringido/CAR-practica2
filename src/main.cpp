#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include <CAR-practica2/image.hpp>
#include <CAR-practica2/convolution.hpp>
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

int main()
{

    fs::create_directories("output/");

    std::vector<std::string> paths = obtener_rutas_imagenes("./LostCat-PS/LostCat-PS/pet/");

    ConvolutionKernel edge_kernel = {
        {-1, -1, -1},
        {-1, 8, -1},
        {-1, -1, -1}};

    for (const auto &path : paths)
    {
        try
        {
            Image img = Image::load(path);
            Image filtered = Convolver::apply(img, edge_kernel);

            std::string filename = path.substr(path.find_last_of("/\\") + 1);
            filtered.save_jpg("output/" + filename);

            std::cout << "Processed: " << filename << "\n";
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }

    return 0;
}
