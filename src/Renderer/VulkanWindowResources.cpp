#include "VulkanWindowResources.h"

namespace Parfait
{
	namespace Graphics 
	{
		VulkanWindowResources::VulkanWindowResources(const VulkanContext& _vulkanContext, GLFWwindow* _window)
			: m_VkContextRef(_vulkanContext), m_WindowRef(_window),
			m_SurfaceSwapchain(std::make_unique<VulkanSurfaceSwapchain>(_vulkanContext, *_window)),
			m_RenderPass(std::make_unique<VulkanRenderPass>(_vulkanContext, *m_SurfaceSwapchain)),
			m_Descriptor(std::make_unique<VulkanDescriptor>(_vulkanContext)),
			m_CommandPool(std::make_unique<VulkanCommandPool>(_vulkanContext))
		{
			CreateDepthResources();
			m_Framebuffers = std::make_unique<VulkanFramebuffer>(_vulkanContext, *m_SurfaceSwapchain, *m_RenderPass, std::vector<VkImageView>{m_DepthImageView});

			// Init Vertex, Index & Uniform Buffer
			m_VertexBuffer = std::make_unique<VulkanVertexBuffer<Vertex>>(_vulkanContext, *m_CommandPool, vertices.data(), vertices.size());
			m_IndexBuffer = std::make_unique<VulkanIndexBuffer>(_vulkanContext, *m_CommandPool, indices.data(), indices.size());
			m_UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				m_UniformBuffers[i] = std::make_unique<VulkanUniformBuffer<UniformBufferObject>>(_vulkanContext, *m_CommandPool);
				vkMapMemory(_vulkanContext.GetLogicalDevice(), m_UniformBuffers[i]->GetDeviceMemory(), 0, static_cast<VkDeviceSize>(sizeof(UniformBufferObject)), 0, &m_UniformBuffers[i]->GetMappedBuffer());
			}

			// Init Texture
			m_Textures.push_back(std::make_unique<VulkanTexture>(_vulkanContext, *m_CommandPool));
			m_Textures[0].get()->LoadTexture("Textures/texture.jpg");

			m_Descriptor.get()->AddLayoutBinding({ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr });
			m_Descriptor.get()->AddLayoutBinding({ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr });
			m_Descriptor.get()->Init();
			// Write Uniform Buffer to Descriptor
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				m_Descriptor.get()->WriteUniformBuffer(0, m_UniformBuffers[i]->GetBuffer(), static_cast<VkDeviceSize>(sizeof(UniformBufferObject)), i);
				m_Descriptor.get()->WriteImageBuffer(1, m_Textures[0].get()->GetImageView(), m_Textures[0].get()->GetSampler(), i);
				m_Descriptor.get()->UpdateDescriptorSet();
			}

			m_GraphicsPipeline = std::make_unique<VulkanGraphicsPipeline>(_vulkanContext, *m_RenderPass, *m_Descriptor, std::vector<std::filesystem::path>{"Shaders/temp.vert", "Shaders/temp.frag"});

			CreateCommandBuffers(MAX_FRAMES_IN_FLIGHT);
			CreateSyncObject(MAX_FRAMES_IN_FLIGHT);


			glfwSetWindowUserPointer(_window, this);
			BindWindowEvents();
		}
		VulkanWindowResources::~VulkanWindowResources()
		{
			vkDeviceWaitIdle(m_VkContextRef.GetLogicalDevice());

			DestroyDepthResources();
			DestroySyncObject();
		}

		void VulkanWindowResources::Update()
		{
			glfwPollEvents();
			Draw();

			if (glfwWindowShouldClose(m_WindowRef))
			{
				glfwDestroyWindow(m_WindowRef);  // Destroy the GLFW window safely
				//delete this;
			}
		}
		void VulkanWindowResources::Draw()
		{
			vkWaitForFences(m_VkContextRef.GetLogicalDevice(), 1, &m_InflightFence[m_CurrentFrame], VK_TRUE, UINT64_MAX);

			uint32_t imageIndex;
			VkResult result = vkAcquireNextImageKHR(m_VkContextRef.GetLogicalDevice(), m_SurfaceSwapchain.get()->GetSwapchain(), UINT64_MAX, m_PresentSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);
			if (result == VK_ERROR_OUT_OF_DATE_KHR)
			{
				m_IsFramebufferResize = false;
				RecreateSwapchain();
				return;
			}
			else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
			{
				throw std::runtime_error("failed to acquire swap chain image!");
			}

			UpdateUniform(m_CurrentFrame);

			vkResetFences(m_VkContextRef.GetLogicalDevice(), 1, &m_InflightFence[m_CurrentFrame]);

			vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), /*VkCommandBufferResetFlagBits*/ 0);
			BeginRenderPass(*m_CommandBuffers[m_CurrentFrame], imageIndex);
			{
				vkCmdBindPipeline(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline.get()->GetPipeline());

				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = (float)m_SurfaceSwapchain->GetExtent().width;
				viewport.height = (float)m_SurfaceSwapchain->GetExtent().height;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), 0, 1, &viewport);

				VkRect2D scissor{};
				scissor.offset = { 0, 0 };
				scissor.extent = m_SurfaceSwapchain->GetExtent();
				vkCmdSetScissor(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), 0, 1, &scissor);

				VkBuffer vertexBuffers[] = { m_VertexBuffer.get()->GetBuffer()};
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), 0, 1, vertexBuffers, offsets);
				vkCmdBindIndexBuffer(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), m_IndexBuffer.get()->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
				vkCmdBindDescriptorSets(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline.get()->GetPipelineLayout(), 0, 1, &m_Descriptor.get()->GetDescriptorSet(m_CurrentFrame), 0, nullptr);
				vkCmdDrawIndexed(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
			}
			EndRenderPass(*m_CommandBuffers[m_CurrentFrame]);

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore waitSemaphores[] = { m_PresentSemaphores[m_CurrentFrame] };
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer();

			VkSemaphore signalSemaphores[] = { m_RenderSemaphores[m_CurrentFrame] };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

			// TODO: Better Error Handler
			if (vkQueueSubmit(m_VkContextRef.GetGraphicsQueue(), 1, &submitInfo, m_InflightFence[m_CurrentFrame]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to submit draw command buffer!");
			}

			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			VkSwapchainKHR swapChains[] = { m_SurfaceSwapchain->GetSwapchain() };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;

			presentInfo.pImageIndices = &imageIndex;

			result = vkQueuePresentKHR(m_VkContextRef.GetPresentQueue(), &presentInfo);
			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_IsFramebufferResize)
			{
				RecreateSwapchain();
			}
			else if (result != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed to present swap chain image!");
			}

			m_CurrentFrame = (m_CurrentFrame + 1) / MAX_FRAMES_IN_FLIGHT;
		}
		void VulkanWindowResources::UpdateUniform(uint32_t _currentFrame)
		{
			static auto startTime = std::chrono::high_resolution_clock::now();

			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

			UniformBufferObject ubo{};
			ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.projection = glm::perspective(glm::radians(45.0f), m_SurfaceSwapchain.get()->GetExtent().width / (float)m_SurfaceSwapchain.get()->GetExtent().height, 0.1f, 10.0f);
			ubo.projection[1][1] *= -1;

			memcpy(m_UniformBuffers[_currentFrame]->GetMappedBuffer(), &ubo, sizeof(ubo));
		}

		void VulkanWindowResources::BeginRenderPass(const VulkanCommandBuffer& _VkCommandBuffer, uint32_t _imageIndex)
		{
			_VkCommandBuffer.Begin();

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_RenderPass->GetRenderPass();
			renderPassInfo.framebuffer = m_Framebuffers->GetFramebuffers()[_imageIndex];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_SurfaceSwapchain->GetExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
			clearValues[1].depthStencil = { 1.0f, 0 };

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(_VkCommandBuffer.GetCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		}
		void VulkanWindowResources::EndRenderPass(const VulkanCommandBuffer & _VkCommandBuffer)
		{
			vkCmdEndRenderPass(_VkCommandBuffer.GetCommandBuffer());
			_VkCommandBuffer.End();
		}

		void VulkanWindowResources::CreateCommandBuffers(uint32_t _size)
		{
			m_CommandBuffers.resize(_size);
			for (uint32_t i = 0; i < _size; i++)
			{
				m_CommandBuffers[i] = std::make_unique<VulkanCommandBuffer>(m_VkContextRef, *m_CommandPool);
			}
		}
		void VulkanWindowResources::CreateSyncObject(uint32_t _size)
		{
			m_PresentSemaphores.resize(_size);
			m_RenderSemaphores.resize(_size);
			m_InflightFence.resize(_size);

			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (size_t i = 0; i < _size; i++)
			{
				if (vkCreateSemaphore(m_VkContextRef.GetLogicalDevice(), &semaphoreInfo, nullptr, &m_PresentSemaphores[i]) != VK_SUCCESS ||
					vkCreateSemaphore(m_VkContextRef.GetLogicalDevice(), &semaphoreInfo, nullptr, &m_RenderSemaphores[i]) != VK_SUCCESS)
				{
					// TODO: Better Error Handler
					throw std::runtime_error("Failed to create semaphore");
				}

				if (vkCreateFence(m_VkContextRef.GetLogicalDevice(), &fenceInfo, nullptr, &m_InflightFence[i]) != VK_SUCCESS)
				{
					// TODO: Better Error Handler
					throw std::runtime_error("Failed to create fence");
				}
			}
		}
		void VulkanWindowResources::CreateDepthResources()
		{
			VkFormat depthFormat = FindDepthFormat(m_VkContextRef);

			CreateImage(m_VkContextRef,
				m_SurfaceSwapchain.get()->GetExtent().width, m_SurfaceSwapchain.get()->GetExtent().height,
				depthFormat,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_DepthImage, m_DepthImageMemory);
			m_DepthImageView = CreateImageView(m_VkContextRef, m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		}

		void VulkanWindowResources::RecreateSwapchain()
		{
			int width = 0, height = 0;
			glfwGetFramebufferSize(m_WindowRef, &width, &height);
			while (width == 0 || height == 0) 
			{
				glfwGetFramebufferSize(m_WindowRef, &width, &height);
				glfwWaitEvents();
			}

			vkDeviceWaitIdle(m_VkContextRef.GetLogicalDevice());

			m_SurfaceSwapchain.get()->RecreateSwapchainImageViews();

			// Re-Create Depth Resource
			DestroyDepthResources();
			CreateDepthResources();
			
			// Re-Bind depth attachment to Framebuffer
			m_Framebuffers.get()->RecreateFramebuffer({ m_DepthImageView });
		}

		void VulkanWindowResources::DestroySyncObject()
		{
			for (size_t i = 0; i < m_PresentSemaphores.size(); i++)
			{
				vkDestroySemaphore(m_VkContextRef.GetLogicalDevice(), m_PresentSemaphores[i], nullptr);
			}
			for (size_t i = 0; i < m_RenderSemaphores.size(); i++)
			{
				vkDestroySemaphore(m_VkContextRef.GetLogicalDevice(), m_RenderSemaphores[i], nullptr);
			}
			for (size_t i = 0; i < m_InflightFence.size(); i++)
			{
				vkDestroyFence(m_VkContextRef.GetLogicalDevice(), m_InflightFence[i], nullptr);
			}
		}
		void VulkanWindowResources::DestroyDepthResources()
		{
			vkDestroyImageView(m_VkContextRef.GetLogicalDevice(), m_DepthImageView, nullptr);
			vkDestroyImage(m_VkContextRef.GetLogicalDevice(), m_DepthImage, nullptr);
			vkFreeMemory(m_VkContextRef.GetLogicalDevice(), m_DepthImageMemory, nullptr);
		}

		void VulkanWindowResources::BindWindowEvents()
		{
			glfwSetFramebufferSizeCallback(m_WindowRef, FramebufferResizeCallback);
		}
		void VulkanWindowResources::FramebufferResizeCallback(GLFWwindow* window, int width, int height) 
		{
			VulkanWindowResources* app = reinterpret_cast<VulkanWindowResources*>(glfwGetWindowUserPointer(window));
			app->m_IsFramebufferResize = true;
		}
	}
}