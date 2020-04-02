g++ -O4 -c UpdateBoid.cpp -L./ -I./
g++ -fopenmp Boids.o UpdateBoid.o -L./ -I./ -lGL -lglut -lGLU -lm -o Boids
