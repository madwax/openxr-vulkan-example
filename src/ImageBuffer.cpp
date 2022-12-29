#include "ImageBuffer.h"

#include "Util.h"

#include <sstream>

ImageBuffer::ImageBuffer(const Context* context,
                         VkExtent2D size,
                         VkFormat format,
                         VkImageUsageFlagBits usage,
                         VkSampleCountFlagBits samples,
                         VkImageAspectFlags aspect,
                         size_t layerCount)
: context(context)
{
  const VkDevice device = context->getVkDevice();

  // Create an image
  VkImageCreateInfo imageCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
  imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
  imageCreateInfo.extent.width = size.width;
  imageCreateInfo.extent.height = size.height;
  imageCreateInfo.extent.depth = 1u;
  imageCreateInfo.mipLevels = 1u;
  imageCreateInfo.arrayLayers = static_cast<uint32_t>(layerCount);
  imageCreateInfo.format = format;
  imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageCreateInfo.usage = usage;
  imageCreateInfo.samples = samples;
  imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  if (vkCreateImage(device, &imageCreateInfo, nullptr, &image) != VK_SUCCESS)
  {
    util::error(Error::GenericVulkan);
    valid = false;
    return;
  }

  VkMemoryRequirements memoryRequirements;
  vkGetImageMemoryRequirements(device, image, &memoryRequirements);

  VkPhysicalDeviceMemoryProperties supportedMemoryProperties;
  vkGetPhysicalDeviceMemoryProperties(context->getVkPhysicalDevice(), &supportedMemoryProperties);

  const VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  const VkMemoryPropertyFlags typeFilter = memoryRequirements.memoryTypeBits;
  uint32_t suitableMemoryTypeIndex = 0u;
  bool memoryTypeFound = false;
  for (uint32_t memoryTypeIndex = 0u; memoryTypeIndex < supportedMemoryProperties.memoryTypeCount; ++memoryTypeIndex)
  {
    const VkMemoryPropertyFlags propertyFlags = supportedMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;
    if (typeFilter & (1 << memoryTypeIndex) && (propertyFlags & memoryProperties) == memoryProperties)
    {
      suitableMemoryTypeIndex = memoryTypeIndex;
      memoryTypeFound = true;
      break;
    }
  }

  if (!memoryTypeFound)
  {
    util::error(Error::FeatureNotSupported, "Suitable image buffer memory type");
    valid = false;
    return;
  }

  VkMemoryAllocateInfo memoryAllocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
  memoryAllocateInfo.allocationSize = memoryRequirements.size;
  memoryAllocateInfo.memoryTypeIndex = suitableMemoryTypeIndex;
  if (vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &memory) != VK_SUCCESS)
  {
    std::stringstream s;
    s << memoryRequirements.size << " bytes for image buffer";
    util::error(Error::OutOfMemory, s.str());
    valid = false;
    return;
  }

  if (vkBindImageMemory(device, image, memory, 0u) != VK_SUCCESS)
  {
    util::error(Error::GenericVulkan);
    valid = false;
    return;
  }

  // Create an image view
  VkImageViewCreateInfo imageViewCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
  imageViewCreateInfo.image = image;
  imageViewCreateInfo.format = format;
  imageViewCreateInfo.viewType = (layerCount == 1u ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY);
  imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                     VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
  imageViewCreateInfo.subresourceRange.layerCount = static_cast<uint32_t>(layerCount);
  imageViewCreateInfo.subresourceRange.aspectMask = aspect;
  imageViewCreateInfo.subresourceRange.baseArrayLayer = 0u;
  imageViewCreateInfo.subresourceRange.baseMipLevel = 0u;
  imageViewCreateInfo.subresourceRange.levelCount = 1u;
  if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView) != VK_SUCCESS)
  {
    util::error(Error::GenericVulkan);
    valid = false;
    return;
  }
}

ImageBuffer::~ImageBuffer()
{
  const VkDevice device = context->getVkDevice();
  if (device)
  {
    if (imageView)
    {
      vkDestroyImageView(device, imageView, nullptr);
    }

    if (memory)
    {
      vkFreeMemory(device, memory, nullptr);
    }

    if (image)
    {
      vkDestroyImage(device, image, nullptr);
    }
  }
}

bool ImageBuffer::isValid() const
{
  return valid;
}

VkImageView ImageBuffer::getImageView() const
{
  return imageView;
}
