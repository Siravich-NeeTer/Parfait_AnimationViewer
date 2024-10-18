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
#include "Math/Quaternion.h"
#include "Math/VQS.h"

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

            const Math::VQS& GetLocalTransform() const { return m_LocalTransform; }
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

			Math::VQS m_LocalTransform;
			std::string m_Name;
			int m_ID;

			float GetScaleFactor(float _lastTimeStamp, float _nextTimeStamp, float _animationTime);

			glm::vec3 InterpolatePosition(float _animationTime);
            Math::Quaternion InterpolateRotation(float _animationTime);
            glm::vec3 InterpolateScaling(float _animationTime);
	};
}