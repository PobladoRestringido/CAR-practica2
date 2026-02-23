# How to compile and run code

To compile the code, open a terminal, go to the `build` folder and execute the
following:

`cmake --build .`

Once compiled, move up to the project's root and execute:

`./build/CAR-practica2`

It's important that you **move up** to the root folder before executing, because of
relative paths when opening the dataset images.

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
