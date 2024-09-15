#include "ParfaitEngine.h"

namespace Parfait
{
	ParfaitEngine::ParfaitEngine(int _width, int _height)
	{
		InitWindow(_width, _height);

		m_Renderer = std::make_unique<Graphics::Renderer>(m_Window);
	}
	void ParfaitEngine::Run() 
	{
		while (!glfwWindowShouldClose(m_Window))
		{
			glfwPollEvents();
			m_Renderer->Draw();
		}

	}

	void ParfaitEngine::InitWindow(int _width, int _height)
	{
		if (glfwInit() == GLFW_FALSE)
		{
			throw std::runtime_error("GLFW Init Error!\n");
		}

		// GLFW_NO_API for specifying not to create OpenGL context
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// 4th parameter : Optionally specify a monitor to open the window
		// 5th parameter : Only for OpenGL
		m_Window = glfwCreateWindow(_width, _height, "ParfaitEngine", nullptr, nullptr);
	}
}