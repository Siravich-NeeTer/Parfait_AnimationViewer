#pragma once

#include <GLFW/glfw3.h>
#include "Renderer/Renderer.h"

namespace Parfait
{
	class ParfaitEngine
	{
		public:
			ParfaitEngine(int _width, int _height);

			void Run();

			const Graphics::Renderer& GetRenderer() const { return *m_Renderer; }

		private:
			GLFWwindow* m_Window;
			std::unique_ptr <Graphics::Renderer> m_Renderer;

			void InitWindow(int _width, int _height);
	};
}