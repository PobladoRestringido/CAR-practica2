# How to Get the Dataset

1. Visit the Kaggle dataset page:  
   https://www.kaggle.com/datasets/anonymousds2025/pet-cats-and-stray-cats-2025

2. Download the `.zip` file.

3. Extract its contents into the **root directory** of the project.

After extraction, your project structure should look like this:

```
CAR-practica2
├── build
├── include
├── LostCat-PS
└── LostCat-PSC
```

# How to compile and run code

First get the dataset from Kaggle (see instructions above).

Afterward, simply open a terminal window and execute the following (**make sure you're at the
project's root!**):

`./run_full_suite.sh`

# Class diagram

```mermaid
classDiagram

    class Image {
        +int width
        +int height
        +int nChannels
        +vector<unsigned char> data
        +Image()
        +Image(int width, int height, int channels)
        +unsigned char get(int x, int y, int channel)
        +void set(int x, int y, int channel, float value)
        +static Image load(string path)
        +void save_jpg(string path, int quality)
        -int index(int x, int y, int channel)
    }

    class ConvolutionKernel {
        +float data[3][3]
        +ConvolutionKernel(initializer_list<initializer_list<float>> init)
    }

    class ConvolutionResult {
        +Image output
        +double elapsed_seconds
    }

    class Convolver {
        +static Image apply_linear(const Image& img, const ConvolutionKernel& kernel)
        +Image apply_simd(const Image& img, const ConvolutionKernel& kernel)
        +ConvolutionResult do_convolve(const Image& img, const ConvolutionKernel& kernel, bool use_simd)
    }

    %% Relationships
    Convolver --> Image : uses
    Convolver --> ConvolutionKernel : uses
    Convolver --> ConvolutionResult : returns
    ConvolutionResult --> Image : contains
```

## Flags (unnecessary; simply follow instructions above)

You can pass the following flags when running `compile_and_run.sh`:

- `--simd` — use SIMD‑accelerated convolution
- `--nosimd` — use the scalar (non‑SIMD) convolution
