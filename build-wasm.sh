#!/bin/bash

# Ensure emscripten environment is activated
if [ -z "$EMSDK" ]; then
    echo "Error: Emscripten environment not detected. Please source emsdk_env.sh first."
    exit 1
fi

# Clean previous builds
make distclean || true

autoreconf -fiv

# Configure with minimal features for WASM build
emconfigure ./configure \
    --disable-shared \
    --enable-static \
    CXXFLAGS="-O3"

# Build the library
emmake make

# Create the WASM module
# emcc -O3 \
#     src/lib/.libs/libfst.a \
#     -s WASM=1 \
#     -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
#     -s EXPORTED_FUNCTIONS='["_malloc", "_free"]' \
#     -s ALLOW_MEMORY_GROWTH=1 \
#     -o ../fst.js
