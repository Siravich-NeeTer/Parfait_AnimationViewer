#include "Bone.h"

namespace Parfait
{
    Bone::Bone(const std::string& _name, int _ID, const aiNodeAnim* _channel)
        :
        m_Name(_name),
        m_ID(_ID),
        m_LocalTransform(1.0f)
    {
        m_NumPositions = _channel->mNumPositionKeys;

        for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
        {
            aiVector3D aiPosition = _channel->mPositionKeys[positionIndex].mValue;
            float timeStamp = _channel->mPositionKeys[positionIndex].mTime;
            KeyPosition data;
            data.position = AssimpGLMHelpers::GetGLMVec(aiPosition);
            data.timeStamp = timeStamp;
            m_Positions.push_back(data);
        }

        m_NumRotations = _channel->mNumRotationKeys;
        for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
        {
            aiQuaternion aiOrientation = _channel->mRotationKeys[rotationIndex].mValue;
            float timeStamp = _channel->mRotationKeys[rotationIndex].mTime;
            KeyRotation data;
            data.orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
            data.timeStamp = timeStamp;
            m_Rotations.push_back(data);
        }

        m_NumScalings = _channel->mNumScalingKeys;
        for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
        {
            aiVector3D scale = _channel->mScalingKeys[keyIndex].mValue;
            float timeStamp = _channel->mScalingKeys[keyIndex].mTime;
            KeyScale data;
            data.scale = AssimpGLMHelpers::GetGLMVec(scale);
            data.timeStamp = timeStamp;
            m_Scales.push_back(data);
        }
    }


    void Bone::Update(float _animationTime)
    {
        glm::mat4 translation = InterpolatePosition(_animationTime);
        glm::mat4 rotation = InterpolateRotation(_animationTime);
        glm::mat4 scale = InterpolateScaling(_animationTime);
        m_LocalTransform = translation * rotation * scale;
    }

    int Bone::GetPositionIndex(float _animationTime) const 
    {
        for (int index = 0; index < m_NumPositions - 1; ++index)
        {
            if (_animationTime < m_Positions[index + 1].timeStamp)
                return index;
        }
        assert(0);
    }
    int Bone::GetRotationIndex(float _animationTime) const 
    {
        for (int index = 0; index < m_NumRotations - 1; ++index)
        {
            if (_animationTime < m_Rotations[index + 1].timeStamp)
                return index;
        }
        assert(0);
    }
    int Bone::GetScaleIndex(float _animationTime) const 
    {
        for (int index = 0; index < m_NumScalings - 1; ++index)
        {
            if (_animationTime < m_Scales[index + 1].timeStamp)
                return index;
        }
        assert(0);
    }

    float Bone::GetScaleFactor(float _lastTimeStamp, float _nextTimeStamp, float _animationTime)
    {
        float scaleFactor = 0.0f;
        float midWayLength = _animationTime - _lastTimeStamp;
        float framesDiff = _nextTimeStamp - _lastTimeStamp;
        scaleFactor = midWayLength / framesDiff;
        return scaleFactor;
    }

    glm::mat4 Bone::InterpolatePosition(float _animationTime)
    {
        if (1 == m_NumPositions)
            return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

        int p0Index = GetPositionIndex(_animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp,
            m_Positions[p1Index].timeStamp, _animationTime);
        glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position,
            m_Positions[p1Index].position, scaleFactor);
        return glm::translate(glm::mat4(1.0f), finalPosition);
    }
    glm::mat4 Bone::InterpolateRotation(float _animationTime)
    {
        if (1 == m_NumRotations)
        {
            auto rotation = glm::normalize(m_Rotations[0].orientation);
            return glm::toMat4(rotation);
        }

        int p0Index = GetRotationIndex(_animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp,
            m_Rotations[p1Index].timeStamp, _animationTime);
        glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation,
            m_Rotations[p1Index].orientation, scaleFactor);
        finalRotation = glm::normalize(finalRotation);
        return glm::toMat4(finalRotation);
    }
    glm::mat4 Bone::InterpolateScaling(float _animationTime)
    {
        if (1 == m_NumScalings)
            return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

        int p0Index = GetScaleIndex(_animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp,
            m_Scales[p1Index].timeStamp, _animationTime);
        glm::vec3 finalScale = glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale
            , scaleFactor);
        return glm::scale(glm::mat4(1.0f), finalScale);
    }
}