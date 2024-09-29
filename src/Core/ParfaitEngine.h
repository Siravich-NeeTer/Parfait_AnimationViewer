#pragma once

#include <GLFW/glfw3.h>
#include "Renderer/VulkanWindowResources.h"

namespace Parfait
{
	class ParfaitEngine
	{
		public:
			ParfaitEngine();

			void MakeWindow(int _width, int _height, const char* _title);
			void Run();

		private:
			std::unique_ptr <Graphics::VulkanContext> m_VkContext;
			std::vector<std::unique_ptr<Graphics::VulkanWindowResources>> m_WindowResources;
			std::vector<GLFWwindow*> m_Windows;
	};
}