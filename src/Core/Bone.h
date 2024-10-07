#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <string>
#include <vector>

#include "Math/AssimpGLMHelpers.h"
#include "Math/MathUtility.h"

#include "Renderer/Utilities/VulkanUtilities.h"

namespace Parfait
{
	struct KeyPosition
	{
		glm::vec3 position;
		float timeStamp;
	};
	struct KeyRotation
	{
		Math::Quaternion orientation;
		float timeStamp;
	};
	struct KeyScale
	{
		glm::vec3 scale;
		float timeStamp;
	};

	class Bone
	{
		public:
            Bone(const std::string& _name, int _ID, const aiNodeAnim* _channel);

            const glm::mat4& GetLocalTransform() const { return m_LocalTransform; }
            const std::string& GetBoneName() const { return m_Name; }
            const int& GetBoneID() const { return m_ID; }

            void Update(float _animationTime);

            int GetPositionIndex(float _animationTime) const;
            int GetRotationIndex(float _animationTime) const;
            int GetScaleIndex(float _animationTime) const;

		private:
			std::vector<KeyPosition> m_Positions;
			std::vector<KeyRotation> m_Rotations;
			std::vector<KeyScale> m_Scales;
			int m_NumPositions;
			int m_NumRotations;
			int m_NumScalings;

			glm::mat4 m_LocalTransform;
			std::string m_Name;
			int m_ID;

			float GetScaleFactor(float _lastTimeStamp, float _nextTimeStamp, float _animationTime);

			glm::mat4 InterpolatePosition(float _animationTime);
            glm::mat4 InterpolateRotation(float _animationTime);
            glm::mat4 InterpolateScaling(float _animationTime);
	};
}