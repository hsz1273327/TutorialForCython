cython -3 -+  emb.pyx

g++ -I /Users/mac/micromamba/envs/py3.10/include/python3.10 -I /usr/local/include -L/Users/mac/micromamba/envs/py3.10/lib -lpython3.10 -o cythonembeddingc -std=c++20 main.cpp emb.cpp