# ReadMe

## install ninja

sudo apt-get install -y ninja-build


## ReBuild

```bash
rm -rf build && mkdir build && cd build && cmake -G Ninja .. && time ninja
```

## Incremental build with small changes

```bash
cd build && cmake -G Ninja .. && time ninja && ./example
```


## ShapeOp

ShapeOp is a C++ library for solving shape optimization problems.
It is a nice library, but there are zero examples publicly available.

Few repositories to reverse engineer the code from wrappers:

- https://github.com/AndersDeleuran/ShapeOpGHPython
- https://github.com/BlockResearchGroup/compas_meshpattern
- https://github.com/MeshGeometry/IogramSource/tree/master/ThirdParty/ShapeOp