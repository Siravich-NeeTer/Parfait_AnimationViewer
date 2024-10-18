#pragma once

#include <glm/glm.hpp>

namespace Parfait
{
	class Object
	{
		public:
			const glm::vec3 GetPosition() const { return position; }
			const glm::vec3 GetRotation() const { return rotation; }
			const glm::vec3 GetScale() const { return scale; }

			glm::vec3 position = glm::vec3(0.0f);
			glm::vec3 rotation = glm::vec3(0.0f);
			glm::vec3 scale = glm::vec3(1.0f);
		protected:
	};
}