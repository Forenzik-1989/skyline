// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <gpu.h>

namespace skyline::gpu::interconnect::node {
    /**
     * @brief A generic node for simply executing a function
     */
    template<typename FunctionSignature = void(vk::raii::CommandBuffer &, const std::shared_ptr<FenceCycle> &, GPU &)>
    struct FunctionNodeBase {
        std::function<FunctionSignature> function;

        FunctionNodeBase(std::function<FunctionSignature> &&function) : function(function) {}

        template<class... Args>
        void operator()(Args &&... args) {
            function(std::forward<Args>(args)...);
        }
    };

    using FunctionNode = FunctionNodeBase<>;

    /**
     * @brief Creates and begins a VkRenderPass alongside managing all resources bound to it and to the subpasses inside it
     */
    struct RenderPassNode {
      private:
        /**
         * @brief Storage for all resources in the VkRenderPass that have their lifetimes bond to the completion fence
         */
        struct Storage : public FenceCycleDependency {
            vk::raii::Device *device{};
            vk::Framebuffer framebuffer{};
            vk::RenderPass renderPass{};

            ~Storage();
        };

        std::shared_ptr<Storage> storage;

        std::vector<vk::ImageView> attachments;
        std::vector<vk::AttachmentDescription> attachmentDescriptions;

        std::vector<vk::AttachmentReference> attachmentReferences;
        std::vector<std::vector<u32>> preserveAttachmentReferences; //!< Any attachment that must be preserved to be utilized by a future subpass, these are stored per-subpass to ensure contiguity

        constexpr static uintptr_t NoDepthStencil{std::numeric_limits<uintptr_t>::max()}; //!< A sentinel value to denote the lack of a depth stencil attachment in a VkSubpassDescription

        /**
         * @brief Rebases a pointer containing an offset relative to the beginning of a container
         */
        template<typename Container, typename T>
        constexpr T *RebasePointer(const Container &container, const T *offset) {
            return reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(container.data()) + reinterpret_cast<uintptr_t>(offset));
        }

      public:
        std::vector<vk::SubpassDescription> subpassDescriptions;
        std::vector<vk::SubpassDependency> subpassDependencies;

        vk::Rect2D renderArea;
        std::vector<vk::ClearValue> clearValues;

        RenderPassNode(vk::Rect2D renderArea);

        /**
         * @note Any preservation of attachments from previous subpasses is automatically handled by this
         * @return The index of the attachment in the render pass which can be utilized with VkAttachmentReference
         */
        u32 AddAttachment(TextureView *view);

        /**
         * @brief Creates a subpass with the attachments bound in the specified order
         */
        void AddSubpass(span<TextureView *> inputAttachments, span<TextureView *> colorAttachments, TextureView *depthStencilAttachment);

        /**
         * @brief Clears a color attachment in the current subpass with VK_ATTACHMENT_LOAD_OP_CLEAR
         * @param colorAttachment The index of the attachment in the attachments bound to the current subpass
         * @return If the attachment could be cleared or not due to conflicts with other operations
         * @note We require a subpass to be attached during this as the clear will not take place unless it's referenced by a subpass
         */
        bool ClearColorAttachment(u32 colorAttachment, const vk::ClearColorValue &value);

        /**
         * @brief Clears the depth/stencil attachment in the current subpass with VK_ATTACHMENT_LOAD_OP_CLEAR
         * @return If the attachment could be cleared or not due to conflicts with other operations
         * @note We require a subpass to be attached during this as the clear will not take place unless it's referenced by a subpass
         */
        bool ClearDepthStencilAttachment(const vk::ClearDepthStencilValue &value);

        vk::RenderPass operator()(vk::raii::CommandBuffer &commandBuffer, const std::shared_ptr<FenceCycle> &cycle, GPU &gpu);
    };

    /**
     * @brief A node which progresses to the next subpass during a render pass
     */
    struct NextSubpassNode {
        void operator()(vk::raii::CommandBuffer &commandBuffer, const std::shared_ptr<FenceCycle> &cycle, GPU &gpu) {
            commandBuffer.nextSubpass(vk::SubpassContents::eInline);
        }
    };

    using SubpassFunctionNode = FunctionNodeBase<void(vk::raii::CommandBuffer &, const std::shared_ptr<FenceCycle> &, GPU &, vk::RenderPass, u32)>;

    /**
     * @brief A FunctionNode which progresses to the next subpass prior to calling the function
     */
    struct NextSubpassFunctionNode : private SubpassFunctionNode {
        using SubpassFunctionNode::SubpassFunctionNode;

        void operator()(vk::raii::CommandBuffer &commandBuffer, const std::shared_ptr<FenceCycle> &cycle, GPU &gpu, vk::RenderPass renderPass, u32 subpassIndex) {
            commandBuffer.nextSubpass(vk::SubpassContents::eInline);
            SubpassFunctionNode::operator()(commandBuffer, cycle, gpu, renderPass, subpassIndex);
        }
    };

    /**
     * @brief Ends a VkRenderPass that would be created prior with RenderPassNode
     */
    struct RenderPassEndNode {
        void operator()(vk::raii::CommandBuffer &commandBuffer, const std::shared_ptr<FenceCycle> &cycle, GPU &gpu) {
            commandBuffer.endRenderPass();
        }
    };

    using NodeVariant = std::variant<FunctionNode, RenderPassNode, NextSubpassNode, SubpassFunctionNode, NextSubpassFunctionNode, RenderPassEndNode>; //!< A variant encompassing all command nodes types
}
