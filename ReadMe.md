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
- https://shapeop.org/ShapeOpDoc.0.1.0/class_shape_op_1_1_edge_strain_constraint.html

## Currently implemented examples
Unary force
![image](https://github.com/user-attachments/assets/ce5a9fc8-f736-4f4a-9b2e-fabfe35f2bbc)

Cable Net
![image](https://github.com/user-attachments/assets/5d171b1d-4ff0-4450-98cf-3047fef1c1bc)

Normal Force
![image](https://github.com/user-attachments/assets/b8ab6bbd-18fb-4590-bdda-558787f391a9)


Normal Force from Mesh
![image](https://github.com/user-attachments/assets/7fd23085-e84b-41f2-ad6d-df6cd1f11394)
