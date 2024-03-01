/*************************************************
Renderer Class:
1. SwapChain
2. cmd buffers' life cycle
3. draw a frame

We only have one render in an application
*************************************************/
#pragma once

#include "lve_window.hpp"
#include "lve_device.hpp"
#include "lve_swap_chain.hpp"

// std
#include <memory>
#include <cassert>

namespace Vk {

    class LveRenderer
    {
    public:
        LveRenderer(LveWindow& window, LveDevice& device);
        ~LveRenderer();

        LveRenderer(const LveRenderer&) = delete;
        LveRenderer& operator=(const LveRenderer&) = delete;

        [[nodiscard("neglect vkRenderPass")]]
        VkRenderPass getSwapChainRenderPass() const { return lveSwapChain->getRenderPass(); }
        
        [[nodiscard("neglect aspect ratio")]]
        float getAspectRatio() const { return lveSwapChain->extentAspectRatio(); }

        [[nodiscard("neglect isFrameInProgress")]]
        bool isFrameInProgress() const { return isFrameStarted; }
        
        VkCommandBuffer getCurrentCommandBuffer() 
        {
            assert(isFrameStarted && "cannot get cmd buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const 
        {
            assert(isFrameStarted && "cannot get frame index when frame is not in progress");
            return currentFrameIndex;
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        LveWindow& lveWindow;
        LveDevice& lveDevice;
        std::unique_ptr<LveSwapChain> lveSwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex{0};
        bool isFrameStarted{false};
    };

}