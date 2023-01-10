# BSATK

node.js bindings for bsatk, a simplistic library for parsing bsa files used in games based on the gamebryo engine.

# Compiling

## Windows

zlib is currently included as compiled artifacts.

## Linux

Libraries must be installed prior to building.

- Arch  
```sh
sudo pacman -Syu zlib lz4
```

- Debian  
```sh
sudo apt install zlib1g-dev liblz4-dev
```

- Fedora  
```sh
sudo dnf install zlib-devel lz4-devel
```
