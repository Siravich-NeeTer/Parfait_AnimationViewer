#include "ParfaitEngine.h"

namespace Parfait
{
	ParfaitEngine::ParfaitEngine()
	{
		if (glfwInit() == GLFW_FALSE)
		{
			throw std::runtime_error("GLFW Init Error!\n");
		}
		// GLFW_NO_API for specifying not to create OpenGL context
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		m_VkContext = std::make_unique<Graphics::VulkanContext>();
	}

	void ParfaitEngine::MakeWindow(int _width, int _height, const char* _title)
	{
		// 4th parameter : Optionally specify a monitor to open the window
		// 5th parameter : Only for OpenGL
		GLFWwindow* window = glfwCreateWindow(_width, _height, _title, nullptr, nullptr);
		m_Windows.emplace_back(window);
		m_WindowResources.emplace_back(std::make_unique<Graphics::VulkanWindowResources>(*m_VkContext, window));
	}
	void ParfaitEngine::Run() 
	{
		float prevTime = 0.0f;
		while (!m_Windows.empty())
		{
			for (size_t i = 0; i < m_WindowResources.size(); i++)
			{
				float currentTime = glfwGetTime();

				m_WindowResources[i].get()->Update(currentTime - prevTime);

				Input::EndFrame();

				if (glfwWindowShouldClose(m_Windows[i]))
				{
					m_WindowResources[i].reset();
					m_WindowResources.erase(m_WindowResources.begin() + i);
					m_Windows.erase(m_Windows.begin() + i);
					continue;
				}

				prevTime = currentTime;
			}
		}
	}
}