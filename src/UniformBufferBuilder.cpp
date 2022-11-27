#include "UniformBufferBuilder.h"


UniformBufferBuilder::UniformBufferBuilder(const VkPhysicalDevice targetPhysicalDevice)
  : deviceMinAlign(deviceMinAlign)
{
}

void UniformBufferBuilder::add(const size_t blockSize)
{
  VkDeviceSize sz = static_cast<VkDeviceSize>(blockSize);
  sizes.push_back(sz);
  alignedStarts.push_back(nextAlignedStart);

  if (deviceMinAlign > 0)
  {
    sz = (sz + deviceMinAlign - 1) & ~(deviceMinAlign - 1);
  }
  nextAlignedStart += sz;
}
  
// Returns the total size of the buffer
VkDeviceSize UniformBufferBuilder::toAllock() const
{
  return nextAlignedStart;
}

VkDeviceSize UniformBufferBuilder::offset(const size_t blockIndex) const
{
  return alignedStarts[blockIndex];
}

VkDeviceSize UniformBufferBuilder::size(const size_t blockIndex) const
{
  return sizes[blockIndex];
}


void UniformBufferBuilder::copy(void* uniformBuffer, const void* blockData, const size_t blockIndex)
{
  char* uniformBlitTo = static_cast<char*>(uniformBuffer) + alignedStarts[blockIndex];
  std::memcpy(uniformBlitTo, blockData, static_cast<size_t>(sizes[blockIndex]));
}