#pragma once

#include <vulkan/vulkan.h>

#include <vector>

/** Used to help to build a big uniform buffer with all the different blocks all 
* correctly aligned.
* This just keeps track of the how things should be layedout and not used to build stuff up.
*/
class UniformBufferBuilder final
{
public:
  UniformBufferBuilder() = delete;
  UniformBufferBuilder(const VkPhysicalDevice targetPhysicalDevice);
  ~UniformBufferBuilder() = default;

  void add(size_t blockSize);
  
  // Returns the total size of the buffer
  VkDeviceSize toAllock() const;

  // Returns the aligned offset to the start of block Indexed
  VkDeviceSize offset(size_t blockIndex) const;
  // Returns the size of the block without any aligned padding
  VkDeviceSize size(size_t blockIndex) const;

  // Used to update a block
  // The unifromBuffer points to the base of the uniform buffer, all off setting is done internal
  void copy(void* uniformBuffer, const void* blockData, size_t blockIndex);

private:
  const VkDeviceSize deviceMinAlign = 0;
  VkDeviceSize nextAlignedStart = 0;

  std::vector<VkDeviceSize> alignedStarts;
  std::vector<VkDeviceSize> sizes;
};

