g++ -c -O3 ParticleFilters.c -no-pie
g++  *.o -O3 -g -lGL -lGLU -lglut -o ParticleFilters -no-pie
