# Xen Music Sync
A tiny tool (**30KB~**) that can sync your main Music library to any removable disk/phone.

It will walk you through a couple questions until both directories are synced.

# Building
### First of all don't forget to modify the [main.cpp](https://github.com/Xen-E/xms/blob/main/main.cpp#L28) file and add your paths/filter.
### CMake
After downloading/cloning the repo get inside the folder and:
```bash
mkdir build && cd build
```
After we created the directory and got in, Now configure CMake:
```bash
cmake -DSMALL=ON ..
```
Notice the **-DSMALL** option, it will produce a very tiny and fast executable. You can skip it if you want. Now finally let's build our project:
```bash
make -j4
```
the **-j4** is optional, It will speed up the building process by creating more jobs.

### Compiler
If you don't like CMake then pick any compiler of your choice, Like GCC for example and then:
```bash
g++ -s -Os -std=c++17 main.cpp -o sync
```
Notice the **-std=c++17**, It's very important because our project use [Filesystem](https://en.cppreference.com/w/cpp/filesystem). 

# Usage
Nothing special, just type the name of the binary; Example...I named mine **sync.exe** and i put it in **system32** directory which will allow me to access it quickly from the **cmd** by just using:
1. <kbd>Super</kbd> + <kbd>R</kbd>
2. type **cmd**
3. hit <kbd>Enter</kbd>
4. type **sync**
5. hit <kbd>Enter</kbd>

That's it! It will check for any missing files and sync them.

## License
GNU Lesser General Public License version 3 (LGPLv3).
