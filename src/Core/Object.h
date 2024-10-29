#pragma once

#include <glm/glm.hpp>

namespace Parfait
{
	class Object
	{
		public:
			Object(uint32_t _id, const std::string& _objectName)
				: id(_id), name(_objectName)
			{
			}

			const glm::vec3 GetPosition() const { return position; }
			const glm::vec3 GetRotation() const { return rotation; }
			const glm::vec3 GetScale() const { return scale; }

			glm::mat4 GetModelMatrix() const
			{
				glm::mat4 ret(1.0f);

				ret = glm::translate(ret, position);
				ret *= glm::toMat4(glm::quat(glm::radians(rotation)));
				ret = glm::scale(ret, scale);

				return ret;
			}

			uint32_t id = std::numeric_limits<uint32_t>::max();
			std::string name;

			glm::vec3 position = glm::vec3(0.0f);
			glm::vec3 rotation = glm::vec3(0.0f);
			glm::vec3 scale = glm::vec3(1.0f);

		protected:
	};
}