// main.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <chrono>
#include <thread>

#include <holoscan/holoscan.hpp>
#include <holoscan/operators/format_converter/format_converter.hpp>
#include <gxf/std/tensor.hpp>

using namespace holoscan;

class SyntheticSourceOp : public Operator {
 public:
  HOLOSCAN_OPERATOR_FORWARD_ARGS(SyntheticSourceOp)

  SyntheticSourceOp() = default;

  void setup(OperatorSpec &spec) override {
    // output port named "out"
    spec.output<holoscan::gxf::Entity>("out");
    spec.param(allocator_, "allocator", "Memory allocator", "Memory allocator for tensor");
    spec.param(count_, "count", "Frame count", "Number of frames to emit", 1);
  }

  void compute(InputContext&, OutputContext& output, ExecutionContext& context) override {
    // Check if we should still emit frames
    if (frame_count_ >= count_.get()) {
      return; // Stop emitting
    }

    // build a tiny synthetic HxWxC uint8 image (H=48, W=64, C=3)
    const int H = 48;
    const int W = 64;
    const int C = 3;

    // Create a simple contiguous buffer (uint8)
    std::vector<uint8_t> buf(H * W * C);
    for (size_t i = 0; i < buf.size(); ++i) buf[i] = static_cast<uint8_t>(i & 0xFF);

    // Create a GXF entity with tensor
    auto out_message = nvidia::gxf::Entity::New(context.context());
    if (!out_message) {
      throw std::runtime_error("Failed to allocate output message");
    }

    auto gxf_tensor = out_message.value().add<nvidia::gxf::Tensor>();
    if (!gxf_tensor) {
      throw std::runtime_error("Failed to allocate tensor");
    }

    // Configure tensor shape and type
    nvidia::gxf::Shape shape{H, W, C};
    auto primitive_type = nvidia::gxf::PrimitiveType::kUnsigned8;
    auto element_size = sizeof(uint8_t);
    auto storage_type = nvidia::gxf::MemoryStorageType::kHost;

    // Get GXF allocator handle from Holoscan allocator
    auto maybe_gxf_allocator = nvidia::gxf::Handle<nvidia::gxf::Allocator>::Create(context.context(),
        allocator_.get()->gxf_cid());
    if (!maybe_gxf_allocator) {
      throw std::runtime_error("Failed to get GXF allocator handle");
    }
    
    gxf_tensor.value()->reshape<uint8_t>(shape, storage_type, maybe_gxf_allocator.value());
    
    // Copy data to tensor
    std::memcpy(gxf_tensor.value()->pointer(), buf.data(), buf.size());

    // emit
    auto result = holoscan::gxf::Entity(std::move(out_message.value()));
    output.emit(result, "out");
  }

 private:
  Parameter<std::shared_ptr<Allocator>> allocator_;
};


class SaverOp : public Operator {
 public:
  HOLOSCAN_OPERATOR_FORWARD_ARGS(SaverOp)

  SaverOp() = default;

  void setup(OperatorSpec &spec) override {
    spec.input<holoscan::gxf::Entity>("in");
    spec.param(out_path_, "out_path", "Output file path", "Path to save metadata", std::string("/tmp/fc_metadata.txt"));
  }

  void compute(InputContext &context, OutputContext&, ExecutionContext&) override {
    auto maybe_message = context.receive<holoscan::gxf::Entity>("in");
    if (!maybe_message) {
      HOLOSCAN_LOG_ERROR("SaverOp: no message received");
      return;
    }

    auto message = maybe_message.value();
    
    // Try to get the tensor from the message
    auto maybe_tensor = message.get<holoscan::Tensor>();
    if (!maybe_tensor) {
      HOLOSCAN_LOG_ERROR("SaverOp: no tensor found in message");
      return;
    }

    auto tensor = maybe_tensor;

    // Get tensor metadata
    auto shape = tensor->shape();
    auto ndim = tensor->ndim();
    auto dtype = tensor->dtype();
    
    // Convert dtype to string representation
    std::string dtype_str;
    if (dtype.code == kDLUInt && dtype.bits == 8) {
      dtype_str = "uint8";
    } else if (dtype.code == kDLFloat && dtype.bits == 32) {
      dtype_str = "float32";
    } else {
      dtype_str = "unknown";
    }

    // write a simple metadata file
    std::string out_path = out_path_.get();

    std::ofstream ofs(out_path);
    ofs << "dtype=" << dtype_str << " layout=hwc shape=";
    for (int64_t i = 0; i < ndim; ++i) {
      if (i) ofs << ",";
      ofs << shape[i];
    }
    ofs << std::endl;
    ofs.close();

    // short sleep to ensure file flushed if test immediately reads it
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
  }

 private:
  Parameter<std::string> out_path_;
};


int main(int argc, char **argv) {
  Application app;

  // Create allocator
  auto allocator = app.make_resource<UnboundedAllocator>("allocator");

  // instantiate operators
  auto src = app.make_operator<SyntheticSourceOp>("src", Arg("allocator") = allocator);
  auto fmt = app.make_operator<ops::FormatConverterOp>("fmt",
      Arg("out_dtype") = std::string("float32"),
      Arg("in_dtype") = std::string("uint8"),
      Arg("pool") = allocator
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
