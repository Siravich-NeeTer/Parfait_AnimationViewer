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
			LoadModel("Models/Alisa Mikhailovna (3D Model).fbx");
			CreateDepthResources();
			m_Framebuffers = std::make_unique<VulkanFramebuffer>(_vulkanContext, *m_SurfaceSwapchain, *m_RenderPass, std::vector<VkImageView>{m_DepthImageView});

			// Init Vertex, Index & Uniform Buffer
			m_VertexBuffer = std::make_unique<VulkanVertexBuffer<Vertex>>(_vulkanContext, *m_CommandPool, m_Vertices.data(), m_Vertices.size());
			m_IndexBuffer = std::make_unique<VulkanIndexBuffer>(_vulkanContext, *m_CommandPool, m_Indices.data(), m_Indices.size());
			m_UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				m_UniformBuffers[i] = std::make_unique<VulkanUniformBuffer<UniformBufferObject>>(_vulkanContext, *m_CommandPool);
				vkMapMemory(_vulkanContext.GetLogicalDevice(), m_UniformBuffers[i]->GetDeviceMemory(), 0, static_cast<VkDeviceSize>(sizeof(UniformBufferObject)), 0, &m_UniformBuffers[i]->GetMappedBuffer());
			}

			std::vector<VkDescriptorImageInfo> imageInfos;
			m_DebugTexture = std::make_unique<VulkanTexture>(_vulkanContext, *m_CommandPool);
			m_DebugTexture->LoadTexture("Models/null.png");

			m_Descriptor.get()->AddLayoutBinding({ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr });
			m_Descriptor.get()->AddLayoutBinding({ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 20, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr });
			m_Descriptor.get()->Init();
			// Write Uniform Buffer to Descriptor
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				m_Descriptor.get()->WriteUniformBuffer(0, m_UniformBuffers[i]->GetBuffer(), static_cast<VkDeviceSize>(sizeof(UniformBufferObject)), i);
				//m_Descriptor.get()->WriteImageBuffer(1, m_Textures[0].get()->GetImageView(), m_Textures[0].get()->GetSampler(), i);
				
				for (size_t j = 0; j < 20; j++)
				{
					VkDescriptorImageInfo imageInfo{};
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					if (j < m_Textures.size())
					{
						imageInfo.imageView = m_Textures[j]->GetImageView();
						imageInfo.sampler = m_Textures[j]->GetSampler();
					}
					else
					{
						imageInfo.imageView = m_DebugTexture->GetImageView();
						imageInfo.sampler = m_DebugTexture->GetSampler();
					}
					imageInfos.push_back(imageInfo);
				}
				m_Descriptor.get()->WriteImageArrayBuffer(1, imageInfos, i);

				m_Descriptor.get()->UpdateDescriptorSet();
			}

			m_GraphicsPipeline = std::make_unique<VulkanGraphicsPipeline>(_vulkanContext, *m_RenderPass, *m_Descriptor, std::vector<std::filesystem::path>{"Shaders/temp.vert", "Shaders/temp.frag"});

			glfwSetWindowUserPointer(_window, this);
			BindWindowEvents();

			CreateCommandBuffers(MAX_FRAMES_IN_FLIGHT);
			CreateSyncObject(MAX_FRAMES_IN_FLIGHT);
			CreateImGui();

			prepareOffscreen();
			offscrenPipeline = std::make_unique<VulkanGraphicsPipeline>(_vulkanContext, offscreenPass.renderPass, *m_Descriptor, std::vector<std::filesystem::path>{"Shaders/temp.vert", "Shaders/temp.frag"});
			ds = ImGui_ImplVulkan_AddTexture(offscreenPass.sampler, offscreenPass.color.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			m_Camera = Camera({ 0.0f, 0.0f, 5.0f });
		}
		VulkanWindowResources::~VulkanWindowResources()
		{
			vkDeviceWaitIdle(m_VkContextRef.GetLogicalDevice());

			destroyOffscreen();
			DestroyDepthResources();
			DestroySyncObject();

			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			vkDestroyDescriptorPool(m_VkContextRef.GetLogicalDevice(), m_ImGuiPool, nullptr);
		}

		void VulkanWindowResources::Update(float dt)
		{
			glfwPollEvents();
			if (Input::IsKeyBeginPressed(GLFW_MOUSE_BUTTON_RIGHT))
			{
				m_Camera.ResetMousePosition();
				isCameraMove = true;
				glfwSetInputMode(m_WindowRef, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			else if (Input::IsKeyEndPressed(GLFW_MOUSE_BUTTON_RIGHT))
			{
				isCameraMove = false;
				glfwSetInputMode(m_WindowRef, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}

			if(isCameraMove)
				m_Camera.ProcessMousesMovement();
			m_Camera.Input(dt);

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

			m_CommandBuffers[m_CurrentFrame]->Begin();
			/*
				First render pass: Offscreen rendering
			*/
			{
				VkClearValue clearValues[2];
				clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
				clearValues[1].depthStencil = { 1.0f, 0 };

				VkRenderPassBeginInfo renderPassBeginInfo{};
				renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassBeginInfo.renderPass = offscreenPass.renderPass;
				renderPassBeginInfo.framebuffer = offscreenPass.frameBuffer;
				renderPassBeginInfo.renderArea.extent.width = offscreenPass.width;
				renderPassBeginInfo.renderArea.extent.height = offscreenPass.height;
				renderPassBeginInfo.clearValueCount = 2;
				renderPassBeginInfo.pClearValues = clearValues;

				vkCmdBeginRenderPass(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = (float)offscreenPass.width;
				viewport.height = (float)offscreenPass.height;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), 0, 1, &viewport);

				VkRect2D scissor{};
				scissor.offset = { 0, 0 };
				scissor.extent = { (unsigned int)offscreenPass.width, (unsigned int)offscreenPass.height };
				vkCmdSetScissor(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), 0, 1, &scissor);

				VkBuffer vertexBuffers[] = { m_VertexBuffer.get()->GetBuffer() };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), 0, 1, vertexBuffers, offsets);
				vkCmdBindIndexBuffer(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), m_IndexBuffer.get()->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
				vkCmdBindDescriptorSets(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, offscrenPipeline.get()->GetPipelineLayout(), 0, 1, &m_Descriptor.get()->GetDescriptorSet(m_CurrentFrame), 0, NULL);
				vkCmdBindPipeline(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, offscrenPipeline.get()->GetPipeline());
				vkCmdDrawIndexed(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);

				vkCmdEndRenderPass(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer());
			}
			
			ImVec2 currentOffscreenSize;
			/*
			BeginRenderPass(*m_CommandBuffers[m_CurrentFrame], imageIndex);
			*/
			{
				VkRenderPassBeginInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = m_RenderPass->GetRenderPass();
				renderPassInfo.framebuffer = m_Framebuffers->GetFramebuffers()[imageIndex];
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = m_SurfaceSwapchain->GetExtent();

				std::array<VkClearValue, 2> clearValues{};
				clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
				clearValues[1].depthStencil = { 1.0f, 0 };

				renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
				renderPassInfo.pClearValues = clearValues.data();

				vkCmdBeginRenderPass(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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
				vkCmdBindIndexBuffer(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), m_IndexBuffer.get()->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
				vkCmdBindDescriptorSets(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline.get()->GetPipelineLayout(), 0, 1, &m_Descriptor.get()->GetDescriptorSet(m_CurrentFrame), 0, nullptr);
				vkCmdDrawIndexed(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer(), static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);

				// Render ImGui
				ImGui_ImplVulkan_NewFrame();
				ImGui_ImplGlfw_NewFrame();

				ImGui::NewFrame();
				
				ImGui::Begin("Viewport");

				ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
				ImGui::Image(ds, ImVec2{ (float)offscreenPass.width, (float)offscreenPass.height });
				currentOffscreenSize = ImGui::GetWindowSize();
				//std::cout << ImGui::GetWindowSize().x << " " << ImGui::GetWindowSize().y << "\n";

				ImGui::End();

				ImGui::ShowDemoWindow();

				ImGui::Render();
				ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer());

				vkCmdEndRenderPass(m_CommandBuffers[m_CurrentFrame]->GetCommandBuffer());
			}
			m_CommandBuffers[m_CurrentFrame]->End();
			/*
			EndRenderPass(*m_CommandBuffers[m_CurrentFrame]);
			*/

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

			if ((int)currentOffscreenSize.x != (int)offscreenSize.x ||
				(int)currentOffscreenSize.y != (int)offscreenSize.y)
			{
				offscreenSize = currentOffscreenSize;
				offscreenPass.width = offscreenSize.x;
				offscreenPass.height = offscreenSize.y;
				recreateOffscreen();
			}

			m_CurrentFrame = (m_CurrentFrame + 1) / MAX_FRAMES_IN_FLIGHT;
		}
		void VulkanWindowResources::UpdateUniform(uint32_t _currentFrame)
		{
			static auto startTime = std::chrono::high_resolution_clock::now();

			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

			UniformBufferObject ubo{};
			//ubo.model = glm::rotate(glm::mat4(1.0f), time * 0.25f * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
			//ubo.model = glm::rotate(glm::mat4(1.0f), time * 0.25f * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));
			ubo.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));
			ubo.view = m_Camera.GetViewMatrix();
			ubo.projection = glm::perspectiveRH_ZO(glm::radians(45.0f), m_SurfaceSwapchain.get()->GetExtent().width / (float)m_SurfaceSwapchain.get()->GetExtent().height, 0.1f, 100.0f);
			ubo.projection[1][1] *= -1;

			memcpy(m_UniformBuffers[_currentFrame]->GetMappedBuffer(), &ubo, sizeof(ubo));
		}

		void VulkanWindowResources::LoadModel(const std::filesystem::path& _path)
		{
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(_path.string(), 
				aiProcess_Triangulate | 
				aiProcess_FlipUVs | 
				aiProcess_JoinIdenticalVertices |
				aiProcess_PreTransformVertices |
				aiProcess_TransformUVCoords
			);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				throw std::runtime_error("ERROR::ASSIMP " + std::string(importer.GetErrorString()));
			}

			for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
				aiMaterial* material = scene->mMaterials[i];

				aiTextureType textureTypes[] = {
					aiTextureType_DIFFUSE
					// aiTextureType_SPECULAR,
					// aiTextureType_NORMALS,
					// aiTextureType_HEIGHT,
					// Add other texture types you may need
				};

				for (auto type : textureTypes) {
					if (material->GetTextureCount(type) > 0) {
						aiString texturePath;
						material->GetTexture(type, 0, &texturePath);
						std::string fullPath = texturePath.C_Str();
						// Load the texture into Vulkan

						std::cout << fullPath << "\n";

						m_Textures.push_back(std::make_unique<VulkanTexture>(m_VkContextRef, *m_CommandPool));
						m_Textures.back().get()->LoadTexture("Models/" + fullPath);
						//m_Textures[0].get()->LoadTexture(fullPath);
					}
				}
			}

			ProcessNode(scene->mRootNode, scene);
		}
		void VulkanWindowResources::ProcessNode(aiNode* node, const aiScene* scene)
		{
			// process each mesh located at the current node
			for (unsigned int i = 0; i < node->mNumMeshes; i++)
			{
				// the node object only contains indices to index the actual objects in the scene. 
				// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				ProcessMesh(mesh, scene, node->mTransformation);
			}
			// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
			for (unsigned int i = 0; i < node->mNumChildren; i++)
			{
				ProcessNode(node->mChildren[i], scene);
			}

		}
		void VulkanWindowResources::ProcessMesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4 _transform)
		{
			uint32_t startIdx = m_Vertices.size();
			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				Vertex vertex;
				vertex.pos = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);

				if (mesh->mTextureCoords[0]) // Check for texture coordinates
				{
					glm::vec2 vec;
					vec.x = mesh->mTextureCoords[0][i].x;
					vec.y = mesh->mTextureCoords[0][i].y;
					vertex.texCoord = vec;
					vertex.texIndex = mesh->mMaterialIndex;
				}
				else
				{
					vertex.texCoord = glm::vec2(0.0f, 0.0f);
				}

				m_Vertices.push_back(vertex);
			}
			for (unsigned int i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++)
					m_Indices.push_back(startIdx + face.mIndices[j]);
			}
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
		void VulkanWindowResources::CreateImGui()
		{
			//1: create descriptor pool for IMGUI
			// the size of the pool is very oversize, but it's copied from imgui demo itself.
			VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};

			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
			pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
			pool_info.pPoolSizes = pool_sizes;

			vkCreateDescriptorPool(m_VkContextRef.GetLogicalDevice(), &pool_info, nullptr, &m_ImGuiPool);

			// 2: initialize imgui library
			ImGui::CreateContext();
			
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
			ImGui::StyleColorsDark();

			ImGui_ImplGlfw_InitForVulkan(m_WindowRef, true);

			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = m_VkContextRef.GetInstance();
			init_info.PhysicalDevice = m_VkContextRef.GetPhysicalDevice();
			init_info.Device = m_VkContextRef.GetLogicalDevice();
			init_info.Queue = m_VkContextRef.GetGraphicsQueue();
			init_info.DescriptorPool = m_ImGuiPool;
			init_info.RenderPass = m_RenderPass->GetRenderPass();
			init_info.MinImageCount = 3;
			init_info.ImageCount = 3;
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

			ImGui_ImplVulkan_Init(&init_info);
			ImGui_ImplVulkan_CreateFontsTexture();
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

			glfwSetKeyCallback(m_WindowRef, Input::KeyCallBack);
			glfwSetCursorPosCallback(m_WindowRef, Input::CursorCallBack);
			glfwSetMouseButtonCallback(m_WindowRef, Input::MouseCallBack);
		}
		void VulkanWindowResources::FramebufferResizeCallback(GLFWwindow* window, int width, int height) 
		{
			VulkanWindowResources* app = reinterpret_cast<VulkanWindowResources*>(glfwGetWindowUserPointer(window));
			app->m_IsFramebufferResize = true;
		}
	}
}