#include "RenderProcess.h"
#include "Buffer.h"
#include "Util.h"

#include <array>

RenderProcess::RenderProcess(VkDevice device,
                             VkPhysicalDevice physicalDevice,
                             VkCommandPool commandPool,
                             VkDescriptorPool descriptorPool,
                             VkDescriptorSetLayout descriptorSetLayout)
: uniformBufferBuilder(physicalDevice)
, device(device)
{
  // Initialize the uniform buffer data
  uniformBufferDataScene.world = glm::mat4(1.0f);
  uniformBufferDataScene.viewProjection[0] = glm::mat4(1.0f);
  uniformBufferDataScene.viewProjection[1] = glm::mat4(1.0f);
  uniformBufferDataAnimation.time = 0.0f;

  // Allocate a command buffer
  VkCommandBufferAllocateInfo commandBufferAllocateInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
  commandBufferAllocateInfo.commandPool = commandPool;
  commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandBufferAllocateInfo.commandBufferCount = 1u;

  if (vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer) != VK_SUCCESS)
  {
    util::error(Error::GenericVulkan);
    valid = false;
    return;
  }

  // Create semaphores
  VkSemaphoreCreateInfo semaphoreCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
  if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &drawableSemaphore) != VK_SUCCESS)
  {
    util::error(Error::GenericVulkan);
    valid = false;
    return;
  }

  if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &presentableSemaphore) != VK_SUCCESS)
  {
    util::error(Error::GenericVulkan);
    valid = false;
    return;
  }

  // Create a memory fence
  VkFenceCreateInfo fenceCreateInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
  fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Make sure the fence starts off signaled
  if (vkCreateFence(device, &fenceCreateInfo, nullptr, &busyFence) != VK_SUCCESS)
  {
    util::error(Error::GenericVulkan);
    valid = false;
    return;
  }

  uniformBufferBuilder.add(sizeof(UniformBufferDataScene));
  uniformBufferBuilder.add(sizeof(UniformBufferDataAnimation));

  uniformBuffer =
    new Buffer(device, physicalDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBufferBuilder.toAllock());
  if (!uniformBuffer->isValid())
  {
    valid = false;
    return;
  }

  // Allocate a descriptor set
  VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
  descriptorSetAllocateInfo.descriptorPool = descriptorPool;
  descriptorSetAllocateInfo.descriptorSetCount = 1u;
  descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
  const VkResult result = vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet);
  if (result != VK_SUCCESS)
  {
    util::error(Error::GenericVulkan);
    valid = false;
    return;
  }

  // Associate the descriptor set with the uniform buffer
  std::array<VkDescriptorBufferInfo, 2u> descriptorBufferInfos;

  descriptorBufferInfos.at(0u).buffer = uniformBuffer->getVkBuffer();
  descriptorBufferInfos.at(0u).offset = uniformBufferBuilder.offset(0);
  descriptorBufferInfos.at(0u).range = uniformBufferBuilder.size(0);

  descriptorBufferInfos.at(1u).buffer = uniformBuffer->getVkBuffer();
  descriptorBufferInfos.at(1u).offset = uniformBufferBuilder.offset(1);
  descriptorBufferInfos.at(1u).range = uniformBufferBuilder.size(1);

  std::array<VkWriteDescriptorSet, 2u> writeDescriptorSets{};

  writeDescriptorSets.at(0u).sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeDescriptorSets.at(0u).pNext = nullptr;
  writeDescriptorSets.at(0u).dstSet = descriptorSet;
  writeDescriptorSets.at(0u).dstBinding = 0u;
  writeDescriptorSets.at(0u).dstArrayElement = 0u;
  writeDescriptorSets.at(0u).descriptorCount = 1u;
  writeDescriptorSets.at(0u).descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  writeDescriptorSets.at(0u).pBufferInfo = &descriptorBufferInfos.at(0u);
  writeDescriptorSets.at(0u).pImageInfo = nullptr;
  writeDescriptorSets.at(0u).pTexelBufferView = nullptr;

  writeDescriptorSets.at(1u).sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeDescriptorSets.at(1u).pNext = nullptr;
  writeDescriptorSets.at(1u).dstSet = descriptorSet;
  writeDescriptorSets.at(1u).dstBinding = 1u;
  writeDescriptorSets.at(1u).dstArrayElement = 0u;
  writeDescriptorSets.at(1u).descriptorCount = 1u;
  writeDescriptorSets.at(1u).descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  writeDescriptorSets.at(1u).pBufferInfo = &descriptorBufferInfos.at(1u);
  writeDescriptorSets.at(1u).pImageInfo = nullptr;
  writeDescriptorSets.at(1u).pTexelBufferView = nullptr;

  vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0u,
                         nullptr);
}

RenderProcess::~RenderProcess()
{
  delete uniformBuffer;

  vkDestroyFence(device, busyFence, nullptr);
  vkDestroySemaphore(device, presentableSemaphore, nullptr);
  vkDestroySemaphore(device, drawableSemaphore, nullptr);
}

bool RenderProcess::isValid() const
{
  return valid;
}

VkCommandBuffer RenderProcess::getCommandBuffer() const
{
  return commandBuffer;
}

VkSemaphore RenderProcess::getDrawableSemaphore() const
{
  return drawableSemaphore;
}

VkSemaphore RenderProcess::getPresentableSemaphore() const
{
  return presentableSemaphore;
}

VkFence RenderProcess::getBusyFence() const
{
  return busyFence;
}

VkDescriptorSet RenderProcess::getDescriptorSet() const
{
  return descriptorSet;
}

bool RenderProcess::updateUniformBufferData() const
{
  void* data = uniformBuffer->map();
  if (!data)
  {
    return false;
  }

  uniformBufferBuilder.copy(data, &uniformBufferDataScene, 0);
  uniformBufferBuilder.copy(data, &uniformBufferDataAnimation, 1);

  uniformBuffer->unmap();

  return true;
}