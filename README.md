# Coffe operations

## Build project

```bash

# 1. Create and navigate into a build directory
mkdir build
cd build

# 2. Generate the build files (point to the parent directory)
# Note: If Qt isn't found, add -DCMAKE_PREFIX_PATH=/path/to/Qt/YOUR_VERSION/compiler
cmake ..

# 3. Compile the executable
cmake --build .

# 4. Execute the application
# On Linux/macOS:
./QtHelloWorld
# On Windows:
Debug\QtHelloWorld.exe
```
