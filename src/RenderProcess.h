#pragma once

#include <glm/mat4x4.hpp>

#include <vulkan/vulkan.h>

#include "UniformBufferBuilder.h"

class Buffer;

class RenderProcess final
{
public:
  RenderProcess(VkDevice device,
                VkPhysicalDevice physicalDevice,
                VkCommandPool commandPool,
                VkDescriptorPool descriptorPool,
                VkDescriptorSetLayout descriptorSetLayout);
  ~RenderProcess();

  #pragma pack(push, 1)
  struct UniformBufferDataScene final
  {
    glm::mat4 world;
    glm::mat4 viewProjection[2]; // View projection matrices, 0 = left eye, 1 = right eye
  } uniformBufferDataScene;

  struct UniformBufferDataAnimation final
  {
    float time;                  // For animation
  } uniformBufferDataAnimation;
  #pragma pack(pop)

  bool isValid() const;
  VkCommandBuffer getCommandBuffer() const;
  VkSemaphore getDrawableSemaphore() const;
  VkSemaphore getPresentableSemaphore() const;
  VkFence getBusyFence() const;
  VkDescriptorSet getDescriptorSet() const;

  bool updateUniformBufferData() const;

private:
  bool valid = true;

  VkDevice device = nullptr;
  VkCommandBuffer commandBuffer = nullptr;
  VkSemaphore drawableSemaphore = nullptr, presentableSemaphore = nullptr;
  VkFence busyFence = nullptr;
  Buffer* uniformBuffer = nullptr;
  VkDescriptorSet descriptorSet = nullptr;
  mutable UniformBufferBuilder uniformBufferBuilder;
};