#pragma once

#include <glm/mat4x4.hpp>

#include <vulkan/vulkan.h>

#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <vector>

class Context;
class RenderTarget;

class Headset final
{
public:
  Headset(const Context* context);
  ~Headset();

  enum class BeginFrameResult
  {
    Error,       // An error occurred
    RenderFully, // Render this frame normally
    SkipRender,  // Skip rendering the frame but end it
    SkipFully    // Skip processing this frame entirely without ending it
  };
  BeginFrameResult beginFrame(uint32_t& swapchainImageIndex);
  void endFrame() const;

  bool isValid() const;
  bool isExitRequested() const;
  VkRenderPass getRenderPass() const;
  size_t getEyeCount() const;
  VkExtent2D getEyeResolution(size_t eyeIndex) const;
  glm::mat4 getEyeViewMatrix(size_t eyeIndex) const;
  glm::mat4 getEyeProjectionMatrix(size_t eyeIndex) const;
  RenderTarget* getRenderTarget(size_t swapchainImageIndex) const;

private:
  bool valid = true;
  bool exitRequested = false;

  const Context* context = nullptr;

  size_t eyeCount = 0u;
  std::vector<glm::mat4> eyeViewMatrices;
  std::vector<glm::mat4> eyeProjectionMatrices;

  XrSession session = nullptr;
  XrSessionState sessionState = XR_SESSION_STATE_UNKNOWN;
  XrSpace space = nullptr;
  XrFrameState frameState = {};
  XrViewState viewState = {};

  std::vector<XrViewConfigurationView> eyeImageInfos;
  std::vector<XrView> eyePoses;
  std::vector<XrCompositionLayerProjectionView> eyeRenderInfos;

  XrSwapchain swapchain = nullptr;
  std::vector<RenderTarget*> swapchainRenderTargets;

  VkRenderPass renderPass = nullptr;

  // Depth buffer
  VkImage depthImage = nullptr;
  VkDeviceMemory depthMemory = nullptr;
  VkImageView depthImageView = nullptr;

  bool beginSession() const;
  bool endSession() const;
};