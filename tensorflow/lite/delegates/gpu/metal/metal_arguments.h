/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#ifndef TENSORFLOW_LITE_DELEGATES_GPU_METAL_METAL_ARGUMENTS_H_
#define TENSORFLOW_LITE_DELEGATES_GPU_METAL_METAL_ARGUMENTS_H_

#import <Metal/Metal.h>

#include <map>
#include <string>
#include <vector>

#include "tensorflow/lite/delegates/gpu/common/status.h"
#include "tensorflow/lite/delegates/gpu/common/task/arguments.h"
#include "tensorflow/lite/delegates/gpu/common/task/gpu_object_desc.h"
#include "tensorflow/lite/delegates/gpu/metal/gpu_object.h"
#include "tensorflow/lite/delegates/gpu/metal/metal_device.h"

namespace tflite {
namespace gpu {
namespace metal {

class MetalArguments : public ArgumentsBinder {
 public:
  MetalArguments() = default;

  absl::Status Init(const std::map<std::string, std::string>& linkables,
                    MetalDevice* device, Arguments* args, std::string* code);

  // Move only
  MetalArguments(MetalArguments&& args) = default;
  MetalArguments& operator=(MetalArguments&& args) = default;
  MetalArguments(const MetalArguments&) = delete;
  MetalArguments& operator=(const MetalArguments&) = delete;

  absl::Status SetInt(const std::string& name, int value) override;
  absl::Status SetFloat(const std::string& name, float value) override;
  absl::Status SetHalf(const std::string& name, half value) override;
  absl::Status SetObjectRef(const std::string& name, const GPUObject& object);

  void Encode(id<MTLComputeCommandEncoder> encoder, int buffer_offset) const;

 private:
  // creates structure with layout:
  // struct uniforms_buffer {
  //   int val_0;
  //   int val_1;
  //   float val_2;
  //   int dummy;  // for alignment
  // };
  std::string ScalarArgumentsToStructWithScalarFields(Arguments* args,
                                                      std::string* code);

  // creates structure with layout:
  // struct uniforms_buffer {
  //   int4 val_0_val_1_dummy_dummy;
  //   float4 val_2_dummy_dummy_dummy;
  // };
  std::string ScalarArgumentsToStructWithVec4Fields(Arguments* args,
                                                    std::string* code);

  absl::Status AllocateObjects(const Arguments& args, id<MTLDevice> device);
  absl::Status AddObjectArgs(Arguments* args);

  void AddGPUResources(const std::string& name, const GPUResources& resources,
                       Arguments* args);

  std::string GetListOfArgs(int buffer_offset);

  absl::Status SetGPUResources(const std::string& name,
                               const GPUResourcesWithValue& resources);

  void AddBuffer(const std::string& name, const GPUBufferDescriptor& desc);

  absl::Status SetBuffer(const std::string& name, id<MTLBuffer> handle);

  absl::Status SetObjectsResources(const Arguments& args);

  absl::Status ResolveSelectorsPass(
      const GpuInfo& gpu_info, const Arguments& args,
      const std::map<std::string, std::string>& linkables, std::string* code);

  absl::Status ResolveSelector(
      const GpuInfo& gpu_info, const Arguments& args,
      const std::map<std::string, std::string>& linkables,
      const std::string& object_name, const std::string& selector,
      const std::vector<std::string>& function_args,
      const std::vector<std::string>& template_args, std::string* result);

  void ResolveObjectNames(const std::string& object_name,
                          const std::vector<std::string>& member_names,
                          std::string* code);

  void ResolveArgsPass(std::string* code);

  static constexpr char kArgsPrefix[] = "args.";
  struct IntValue {
    int value;

    // many arguments generated automatically and not used
    // to reduce amount of data transferred we adding this optimization
    bool active = false;

    // offset to shared storage.
    uint32_t bytes_offset = -1;
  };
  std::map<std::string, IntValue> int_values_;

  struct FloatValue {
    float value;

    // many arguments generated automatically and not used
    // to reduce amount of data transferred we adding this optimization
    bool active = false;

    // offset to shared storage.
    uint32_t bytes_offset = -1;
  };
  std::map<std::string, FloatValue> float_values_;
  std::vector<uint8_t> const_data_;

  struct MetalBufferDescriptor {
    GPUBufferDescriptor desc;
    id<MTLBuffer> handle;
  };
  std::map<std::string, MetalBufferDescriptor> buffers_;

  std::map<std::string, GPUObjectDescriptorPtr> object_refs_;
  std::vector<GPUObjectPtr> objects_;
};

}  // namespace metal
}  // namespace gpu
}  // namespace tflite

#endif  // TENSORFLOW_LITE_DELEGATES_GPU_METAL_METAL_ARGUMENTS_H_
