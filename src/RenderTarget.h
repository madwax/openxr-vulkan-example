#pragma once

#include <vulkan/vulkan.h>

class RenderTarget final
{
public:
  RenderTarget(VkDevice device, VkImage image, VkExtent2D size, VkFormat format, VkRenderPass renderPass);

  void destroy() const; // Only call when construction succeeded

  bool isValid() const;
  VkImage getImage() const;
  VkFramebuffer getFramebuffer() const;

private:
  bool valid = true;

  VkDevice device = nullptr;
  VkImage image = nullptr;
  VkImageView imageView = nullptr;
  VkFramebuffer framebuffer = nullptr;
};