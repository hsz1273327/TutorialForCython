# cython --3 -+ helloc/**/*.pyx

# g++ -I /Users/mac/micromamba/envs/py3.10/include/python3.10 -o ./helloc/universal/funcs.o -c ./helloc/universal/funcs.cpp
# g++ -I /Users/mac/micromamba/envs/py3.10/include/python3.10 -o ./helloc/helloc.o -c ./helloc/helloc.cpp
# g++ -I /Users/mac/micromamba/envs/py3.10/include/python3.10 -o ./main.o -c ./main.cpp

# g++ -L/Users/mac/micromamba/envs/py3.10/lib -lpython3.10 main.o ./helloc/helloc.o ./helloc/universal/funcs.o -o hellocexec

cythonize -i --3 helloc/**/*.pyx

rename helloc/helloc.h helloc/helloc.helloc.h

g++ -I /Users/mac/micromamba/envs/py3.10/include/python3.10 -L/Users/mac/micromamba/envs/py3.10/lib -lpython3.10 main.cpp ./helloc/helloc.cpp ./helloc/universal/funcs.cpp -o hellocexec


cython --3 -+ src/**/*.pyx
g++ -I /Users/mac/micromamba/envs/py3.10/include/python3.10 -o ./src/helloc.o -c ./src/helloc.cpp
g++ -I /Users/mac/micromamba/envs/py3.10/include/python3.10 -o ./src/main.o -c ./src/main.cpp
g++ -L/Users/mac/micromamba/envs/py3.10/lib -lpython3.10 ./src/main.o ./src/helloc.o -o hellocexec