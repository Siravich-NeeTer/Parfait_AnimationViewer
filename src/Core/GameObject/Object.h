#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Parfait
{
	class Object
	{
		public:
			Object() = default;
			Object(uint32_t _id, const std::string& _objectName)
				: id(_id), name(_objectName)
			{
			}
			virtual ~Object()
			{

			}

			uint32_t id = std::numeric_limits<uint32_t>::max();
			std::string name;

			glm::vec3 position = glm::vec3(0.0f);
			glm::vec3 rotation = glm::vec3(0.0f);
			glm::vec3 scale = glm::vec3(1.0f);

			virtual void Update(float _dt)
			{

			}

			const glm::vec3& GetPosition() const { return position; }
			const glm::vec3& GetRotation() const { return rotation; }
			const glm::vec3& GetScale() const { return scale; }

			glm::mat4 GetModelMatrix() const
			{
				glm::mat4 model(1.0f);

				// Compute with parent's model matrix
				Object* curParent = parent;
				while (curParent != nullptr)
				{
					model = model * curParent->GetModelMatrix();
					curParent = curParent->parent;
				}

				model = glm::translate(model, position);
				model *= glm::toMat4(glm::quat(glm::radians(rotation)));
				model = glm::scale(model, scale);

				return model;
			}

			const Object* GetParent() const { return parent; }
			const std::vector<Object*>& GetChildren() const { return children; }

			void SetParent(Object* _parent)
			{
				this->parent = _parent;
				_parent->children.push_back(this);
			}

		protected:
			Object* parent = nullptr;
			std::vector<Object*> children;
	};
}