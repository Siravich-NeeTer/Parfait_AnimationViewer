#pragma once

#include <glm/glm.hpp>

namespace Parfait
{
	class Object
	{
		public:

		protected:
			glm::vec3 position = glm::vec3(0.0f);
			glm::vec3 rotation = glm::vec3(0.0f);
			glm::vec3 scale = glm::vec3(0.01f);
	};
}