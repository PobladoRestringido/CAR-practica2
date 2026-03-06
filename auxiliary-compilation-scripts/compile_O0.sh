g++ -O0 -c src/main.cpp -Iinclude -o main.o
g++ -O0 -c src/image.cpp -Iinclude -o image.o
g++ -O0 -c src/convolution.cpp -Iinclude -o convolution.o
g++ main.o image.o convolution.o -o main_O0