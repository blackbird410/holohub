// main.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <chrono>
#include <thread>

#include <holoscan/holoscan.hpp>
#include <holoscan/operators/format_converter/format_converter.hpp>

using namespace holoscan;

class SyntheticSourceOp : public Operator {
 public:
  HOLOSCAN_OPERATOR_FORWARD_ARGS(SyntheticSourceOp)

  SyntheticSourceOp() = default;

  void setup(OperatorSpec &spec) override {
    // output port named "out"
    spec.output<std::shared_ptr<holoscan::Tensor>>("out");
  }

  void compute(InputContext&, OutputContext& output, ExecutionContext&) override {
    // build a tiny synthetic HxWxC uint8 image (H=48, W=64, C=3)
    const int H = 48;
    const int W = 64;
    const int C = 3;

    // Create a simple contiguous buffer (uint8)
    std::vector<uint8_t> buf(H * W * C);
    for (size_t i = 0; i < buf.size(); ++i) buf[i] = static_cast<uint8_t>(i & 0xFF);

    // Create a holoscan::Tensor backed by CPU memory.
    // NOTE: API name Tensor(...) may vary by SDK; this pattern is common.
    TensorSpec t_spec;
    t_spec.dtype = "uint8";
    t_spec.shape = {H, W, C};        // HWC layout
    t_spec.layout = "hwc";
    t_spec.pool = "default_pool";    // use default pool (adjust if needed)

    auto tensor = std::make_shared<Tensor>(buf.data(), buf.size() * sizeof(uint8_t), t_spec);

    // emit
    output.emit(tensor, "out");
  }
};


class SaverOp : public Operator {
 public:
  HOLOSCAN_OPERATOR_FORWARD_ARGS(SaverOp)

  SaverOp() = default;

  void setup(OperatorSpec &spec) override {
    spec.input<std::shared_ptr<holoscan::Tensor>>("in");
    spec.param<std::string>("out_path", "/tmp/fc_metadata.txt");
  }

  void compute(InputContext &context, OutputContext&, ExecutionContext&) override {
    auto maybe_tensor = context.receive<std::shared_ptr<Tensor>>("in");
    if (!maybe_tensor) {
      GXF_LOG_ERROR("SaverOp: no tensor received");
      return;
    }

    auto tensor = maybe_tensor.value();

    // Inspect metadata: dtype, shape, layout
    std::string dtype = tensor->spec().dtype;
    std::vector<int64_t> shape = tensor->spec().shape;
    std::string layout = tensor->spec().layout;

    // write a simple metadata file
    std::string out_path;
    this->param("out_path").get(out_path);

    std::ofstream ofs(out_path);
    ofs << "dtype=" << dtype << " layout=" << layout << " shape=";
    for (size_t i = 0; i < shape.size(); ++i) {
      if (i) ofs << ",";
      ofs << shape[i];
    }
    ofs << std::endl;
    ofs.close();

    // short sleep to ensure file flushed if test immediately reads it
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
  }
};


int main(int argc, char **argv) {
  Application app;

  // instantiate operators
  auto src = app.make_operator<SyntheticSourceOp>("src");
  auto fmt = app.make_operator<operators::FormatConverterOp>("fmt",
      // parameters - adapt parameter names if your SDK uses different names
      // request float32 and hwc layout (HWC)
      std::map<std::string, std::string>{
        {"dst_type", "float32"},
        {"layout", "hwc"}
      }
  );
  auto saver = app.make_operator<SaverOp>("saver");

  // connect
  app.add_flow(src, fmt);
  app.add_flow(fmt, saver);

  // run for a short time - source emits one frame and the app should exit
  app.run();

  std::cout << "format_converter_app finished\n";
  return 0;
}
