# Format Converter Sample

This application demonstrates the usage of the [`holoscan::ops::FormatConverterOp`](https://docs.nvidia.com/clara-holoscan/sdk-user-guide/sdk_operators.html#formatconverterop) in a C++ Holohub application.

The pipeline generates a synthetic RGB image tensor, converts it from `uint8` HWC format to `float32` HWC format, and writes the output tensor’s metadata (dtype, shape, layout) to a file for verification.

This serves as a minimal reference for integrating and testing Holoscan operators in C++.

---

### Requirements

- [Holoscan SDK](https://developer.nvidia.com/holoscan-sdk) version 3.3 or higher
- NVIDIA GPU with CUDA support
- Recommended: Run inside the official Holoscan NGC container:  
  `nvcr.io/nvidia/clara-holoscan/holoscan:<version>-dgpu`

No special capture cards or additional hardware are required.

---

### Data

This application does not require external data. A synthetic tensor is generated internally by the `SyntheticSourceOp`.

---

### Build Instructions

Please refer to the top-level Holohub `README.md` for build instructions.  In summary:

```bash
mkdir build && cd build
cmake .. -DENABLE_TESTS=ON
make -j
```
