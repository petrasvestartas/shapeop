# ReadMe

## install ninja

sudo apt-get install -y ninja-build


## ReBuild

```bash
rm -rf build && mkdir build && cd build && cmake -G Ninja .. && time ninja
```

## Incremental build with small changes

```bash
cd build && cmake -G Ninja .. && time ninja && ./cpp_example
```
