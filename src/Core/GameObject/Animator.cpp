#include "Animator.h"
#include <iostream>
#include <stdlib.h>
#include <queue>
#include <utility>

#include <glm/gtx/string_cast.hpp>

namespace Parfait
{
    glm::quat FromToRotation(const glm::vec3& from, const glm::vec3& to)
    {
        // Normalize the input vectors to ensure they are unit vectors
        glm::vec3 fromNormalized = glm::normalize(from);
        glm::vec3 toNormalized = glm::normalize(to);

        // Compute the cross product (axis of rotation)
        glm::vec3 axis = glm::cross(fromNormalized, toNormalized);

        // If the cross product is close to zero, the vectors are parallel
        if (glm::length(axis) < 1e-6f) {
            // If vectors are parallel, we return identity quaternion
            // (no rotation needed) or 180 degree rotation along any perpendicular axis
            if (glm::dot(fromNormalized, toNormalized) > 0.0f) {
                return glm::quat(1, 0, 0, 0); // No rotation
            }
            else {
                // Vectors are exactly opposite, return 180 degree rotation around an arbitrary axis
                return glm::quat(glm::pi<float>(), glm::vec3(1, 0, 0)); // 180 degree rotation along x-axis
            }
        }

        // Normalize the axis of rotation
        axis = glm::normalize(axis);

        // Compute the angle between the two vectors (in radians)
        float angle = acos(glm::dot(fromNormalized, toNormalized));

        // Create a quaternion from the axis and angle
        glm::quat rotation = glm::angleAxis(angle, axis);

        return rotation;
    }

    Animator::Animator(Model* model)
    {
        m_CurrentAnimationTime = 0.0f;
        m_CurrentNextAnimationTime = 0.0f;

        m_pCurrentModel = model;

        m_pCurrentAnimation = model->GetCurrentActiveAnimation();
        m_pNextAnimation = nullptr;
    }

    void Animator::UpdateAnimation(float _dt)
    {
        if (!m_pCurrentAnimation->IsValid())
        {
            return;
        }

        m_DeltaTime = _dt;

        if (m_pCurrentAnimation && m_pNextAnimation)
        {
            m_CurrentAnimationTime += m_pCurrentAnimation->GetTicksPerSecond() * _dt;
            m_CurrentAnimationTime = fmod(m_CurrentAnimationTime, m_pCurrentAnimation->GetDuration());

            m_CurrentNextAnimationTime += m_pNextAnimation->GetTicksPerSecond() * _dt;
            m_CurrentNextAnimationTime = fmod(m_CurrentNextAnimationTime, m_pNextAnimation->GetDuration());

            CalculateBoneTransform(&m_pCurrentAnimation->GetRootNode(), &m_pNextAnimation->GetRootNode(), Math::VQS::Identity());
        }
        else if (m_pCurrentAnimation)
        {
            m_CurrentAnimationTime += m_pCurrentAnimation->GetTicksPerSecond() * _dt;
            m_CurrentAnimationTime = fmod(m_CurrentAnimationTime, m_pCurrentAnimation->GetDuration());
            CalculateBoneTransform(&m_pCurrentAnimation->GetRootNode(), Math::VQS::Identity());
        }

        if (m_pCurrentPath)
        {
            m_CurrentLoopTime += _dt;
            m_CurrentLoopTime = fmod(m_CurrentLoopTime, m_PathLoopTime);
            MoveAlongPath();
        }

        // Solve IK
        Model::BoneNode* boneNode = m_pCurrentModel->GetBoneNode("mixamorig_LeftHandMiddle3");
        const size_t BONE_SIZE = 6;
        float boneLength[BONE_SIZE] = { 0 };

        Model::BoneNode* boneNodeList[BONE_SIZE + 1] = { nullptr };

        float fullLength = 0.0f;
        boneNodeList[BONE_SIZE] = boneNode;
        for (int i = BONE_SIZE - 1; i >= 0; i--)
        {
            const auto& currentBoneIt = m_pCurrentAnimation->GetBoneIDMap().find(boneNode->name);
            int index = currentBoneIt->second.id;
            glm::vec3 b1 = m_pCurrentModel->GetBonePosition(boneNode);

            const auto& nextBoneIt = m_pCurrentAnimation->GetBoneIDMap().find(boneNode->parent->name);
            int nextIndex = nextBoneIt->second.id;
            glm::vec3 b2 = m_pCurrentModel->GetBonePosition(boneNode->parent);

            boneLength[i] = glm::distance(b1, b2);
            fullLength += boneLength[i];

            boneNode = boneNode->parent;
            boneNodeList[i] = boneNode;
        }

        glm::vec3 tmpPosition[BONE_SIZE + 1];
        for (int i = 0; i <= BONE_SIZE; i++)
        {
            const auto& currentBoneIt = m_pCurrentAnimation->GetBoneIDMap().find(boneNodeList[i]->name);
            int index = currentBoneIt->second.id;
            tmpPosition[i] = m_pCurrentModel->GetBonePosition(boneNodeList[i]);
        }

        // FABRIK
        /*
        float sqrDistanceToTarget = glm::distance(m_pCurrentModel->focusPoint, tmpPosition[BONE_SIZE]);
        if (sqrDistanceToTarget >= fullLength * fullLength)
        {
            // Get the direction towards the target
            glm::vec3 dir = glm::normalize(m_pCurrentModel->focusPoint - tmpPosition[BONE_SIZE]);

            // Distribute bones along the direction towards the target
            for (int i = BONE_SIZE - 1; i >= 0; i--)
                tmpPosition[i] = tmpPosition[i + 1] - dir * boneLength[i];
        }
        */
        glm::vec3 newFocusPoint = m_pCurrentModel->focusPoint;
        if (glm::distance(tmpPosition[0], m_pCurrentModel->focusPoint) > fullLength)
        {
            newFocusPoint = tmpPosition[0] + glm::normalize(m_pCurrentModel->focusPoint - tmpPosition[0]) * fullLength;
        }

        glm::vec3 startDir[BONE_SIZE + 1] = { glm::vec3() };
        startDir[BONE_SIZE] = newFocusPoint - tmpPosition[BONE_SIZE];
        for (int i = 0; i < BONE_SIZE; i++)
        {
            startDir[i] = tmpPosition[i + 1] - tmpPosition[i];
        }


        int iterations = 10;
        float accuracy = 0.001f;
        for (int iteration = 0; iteration < iterations; iteration++)
        {
            // TODO: Back Propagation
            for (int i = BONE_SIZE; i > 0; i--)
            {
                if (i == BONE_SIZE)
                {
                    // Just set the effector to the target position
                    tmpPosition[i] = newFocusPoint;
                }
                else
                {
                    // Move the current bone to its new position on the line based on its length and the position of the next bone
                    glm::vec3 dir = glm::normalize(tmpPosition[i] - tmpPosition[i + 1]);
                    tmpPosition[i] = tmpPosition[i + 1] + dir * boneLength[i];
                }
            }

            // TODO: Front Propagation
            for (int i = 1; i <= BONE_SIZE; i++)
            {
                // This time set the current bone's position to the position on the line between itself and the previous bone, taking its length into consideration
                tmpPosition[i] = tmpPosition[i - 1] + glm::normalize(tmpPosition[i] - tmpPosition[i - 1]) * boneLength[i - 1];
            }

            // Stop iterating if we are close enough according to the accuracy value
            float sqdistance = glm::distance(tmpPosition[BONE_SIZE], newFocusPoint);
            if (sqdistance < accuracy * accuracy)
                break;
        }

        glm::vec3 fullTrans = glm::vec3(0.0f);
        for (int i = 0; i <= BONE_SIZE; i++)
        {
            const auto& currentBoneIt = m_pCurrentAnimation->GetBoneIDMap().find(boneNodeList[i]->name);
            int index = currentBoneIt->second.id;

            glm::vec3 currentBonePosition = m_pCurrentModel->GetBonePosition(boneNodeList[i]);
            glm::vec3 translate = tmpPosition[i] - currentBonePosition;

            glm::mat4 rot = glm::mat4(1.0f);
            glm::quat qRot = glm::quat();
            if (i < BONE_SIZE)
            {
                rot = glm::toMat4(FromToRotation(startDir[i], tmpPosition[i + 1] - tmpPosition[i]));
                qRot = FromToRotation(startDir[i], tmpPosition[i + 1] - tmpPosition[i]);
            }
            boneNodeList[i]->offset = glm::translate(boneNodeList[i]->offset, 1.0f / m_pCurrentModel->scale * translate);
            boneNodeList[i]->rot = rot;
            fullTrans += translate;
        }

        /*
        std::queue<std::pair<Model::BoneNode*, glm::mat4>> st;
        st.push({ boneNodeList[0], boneNodeList[0]->offset });
        while (!st.empty())
        {
            Model::BoneNode* currentBoneNode = st.front().first;
            glm::mat4 parentMat = st.front().second;
            st.pop();

            for (size_t i = 0; i < currentBoneNode->children.size(); i++)
            {
                const auto& currentBoneIt = m_pCurrentAnimation->GetBoneIDMap().find(currentBoneNode->children[i]->name);
                int index = currentBoneIt->second.id;
                if (currentBoneNode->children[i]->offset == glm::mat4(1.0f))
                {
                    std::cout << currentBoneNode->children[i]->name << "\n";
                    glm::mat4 tmp = m_pCurrentModel->m_FinalBoneMatrices[index];
                    m_pCurrentModel->m_FinalBoneMatrices[index] = parentMat * m_pCurrentModel->m_FinalBoneMatrices[index];
                    st.push({ currentBoneNode->children[i], parentMat * tmp });
                }
                else
                {
                    st.push({ currentBoneNode->children[i], parentMat * m_pCurrentModel->m_FinalBoneMatrices[index] });
                }
            }
        }
        std::cout << "\n";
        return;
        */
    }

    void Animator::PlayAnimation(Animation* _pAnimation)
    {
        m_pCurrentAnimation = _pAnimation;
        m_CurrentAnimationTime = 0.0f;
    }
    void Animator::PlayAnimation(const std::string& _animationName)
    {
        m_pCurrentModel->SetCurrentActiveAnimation(_animationName);
        PlayAnimation(m_pCurrentModel->GetCurrentActiveAnimation());
    }
    void Animator::BlendAnimation(const std::string& _newAnimationName, float _blendFactor)
    {
        if (_newAnimationName == "")
        {
            m_pNextAnimation = nullptr;
            m_BlendFactor = 0.0f;
            return;
        }

        m_pNextAnimation = m_pCurrentModel->GetAnimation(_newAnimationName);
        m_BlendFactor = glm::clamp(_blendFactor, 0.0f, 1.0f);
    }

    void Animator::CalculateBoneTransform(const AssimpNodeData* _node, Math::VQS _parentTransform)
    {
        const std::string& nodeName = _node->name;
        Math::VQS nodeTransform = _node->transformation;

        Bone* Bone = m_pCurrentAnimation->FindBone(nodeName);
        Model::BoneNode* boneNode = m_pCurrentModel->GetBoneNode(nodeName);

        if (Bone)
        {
            Bone->Update(m_CurrentAnimationTime);
            nodeTransform = Bone->GetLocalTransform();

            if (boneNode->IsValid())
                nodeTransform = Math::MatrixToVQS(boneNode->offset);
        }

        Math::VQS globalTransformation = _parentTransform * nodeTransform;

        auto& boneInfoMap = m_pCurrentAnimation->GetBoneIDMap();
        const auto& currentBoneIt = boneInfoMap.find(nodeName);
        if (currentBoneIt != boneInfoMap.end())
        {
            int index = currentBoneIt->second.id;
            glm::mat4 offset = currentBoneIt->second.offset;

            if (boneNode && boneNode->IsValid())
                m_pCurrentModel->m_FinalBoneMatrices[index] = nodeTransform.Matrix();
            else
                m_pCurrentModel->m_FinalBoneMatrices[index] = globalTransformation.Matrix() * offset;
        }

        for (int i = 0; i < _node->childrenCount; i++)
        {
            if (boneNode && boneNode->IsValid())
                CalculateBoneTransform(&_node->children[i], nodeTransform);
            else
                CalculateBoneTransform(&_node->children[i], globalTransformation);
        }
    }
    void Animator::CalculateBoneTransform(const AssimpNodeData* _currentAnimationNode, const AssimpNodeData* _nextAnimationNode, Math::VQS _parentTransform)
    {
        const std::string& nodeStartName = _currentAnimationNode->name;
        const std::string& nodeEndName = _nextAnimationNode->name;

        Math::VQS nodeTransform = Math::VQS::Identity();

        Bone* boneStart = m_pCurrentAnimation->FindBone(nodeStartName);
        Bone* boneEnd = m_pNextAnimation->FindBone(nodeEndName);

        nodeTransform = _currentAnimationNode->transformation;

        // Blending Animation using Interpolation in each component(position, rotation, scale)
        if (boneStart && boneEnd)
        {
            glm::vec3 startTranslation, endTranslation;
            Math::Quaternion startQuaternion, endQuaternion;
            glm::vec3 startScale, endScale;

            std::tie(startTranslation, startQuaternion, startScale) = boneStart->GetInterpolateTransform(m_CurrentAnimationTime);
            std::tie(endTranslation, endQuaternion, endScale) = boneEnd->GetInterpolateTransform(m_CurrentNextAnimationTime);
            
            glm::vec3 finalTranslation = glm::mix(startTranslation, endTranslation, m_BlendFactor);
            Math::Quaternion finalQuaternion = Math::Slerp(startQuaternion, endQuaternion, m_BlendFactor);
            glm::vec3 finalScale = glm::mix(startScale, endScale, m_BlendFactor);

            nodeTransform = Math::VQS(finalTranslation, finalQuaternion, finalScale);

            Model::BoneNode* boneNode = m_pCurrentModel->GetBoneNode(_currentAnimationNode->name);
            if (boneNode->IsValid())
            {
                if (boneNode->matrixType == Model::BoneNode::OVERRIDE)
                    nodeTransform = Math::MatrixToVQS(m_pCurrentModel->GetBoneNode(_currentAnimationNode->name)->offset);
                else
                    nodeTransform = nodeTransform * Math::MatrixToVQS(m_pCurrentModel->GetBoneNode(_currentAnimationNode->name)->offset);
            }
        }

        const Math::VQS& globalTransformation = _parentTransform * nodeTransform;

        auto& boneInfoMap = m_pCurrentAnimation->GetBoneIDMap();
        const auto& currentBoneIt = boneInfoMap.find(nodeStartName);
        const auto& nextBoneIt = boneInfoMap.find(nodeEndName);
        if (currentBoneIt != boneInfoMap.end())
        {
            int index1 = currentBoneIt->second.id;
            const glm::mat4& offset1 = currentBoneIt->second.offset;

            int index2 = nextBoneIt->second.id;
            const glm::mat4& offset2 = nextBoneIt->second.offset;

            glm::mat4 offset = (1.0f - m_BlendFactor) * offset1 + m_BlendFactor * offset2;

            m_pCurrentModel->m_FinalBoneMatrices[index1] = globalTransformation.Matrix() * offset;
        }

        // TODO: Assume currentAnimationNode & nextAnimationNode use same hierachy
        for (int i = 0; i < std::min(_currentAnimationNode->childrenCount, _nextAnimationNode->childrenCount); i++)
            CalculateBoneTransform(&_currentAnimationNode->children[i], &_nextAnimationNode->children[i], globalTransformation);
    }

    void Animator::MoveAlongPath()
    {
        float t = m_pCurrentPath->GetDistanceParameter(m_CurrentLoopTime / m_PathLoopTime);
        float next_t = m_pCurrentPath->GetDistanceParameter(m_CurrentLoopTime / m_PathLoopTime + 0.01f);
        glm::vec3 currentPoint = m_pCurrentPath->GetPointFromTable(t);
        glm::vec3 nextPoint = m_pCurrentPath->GetPointFromTable(next_t);

        float currentVelocity = m_pCurrentPath->GetVelocity(t);

        // Animation Blending rely on character velocity
        if (currentVelocity <= 0.25f)
        {
            BlendAnimation("Idle", 1.0f - (currentVelocity / 0.25f));
        }
        else if (currentVelocity >= 0.75f)
        {
            BlendAnimation("Run", (currentVelocity - 0.75f) / 0.25f);
        }
        else
        {
            BlendAnimation("", 0.0f);
        }

        // Skip Update Rotation/Position if currentPoint & nextPoint are same
        if (glm::distance(currentPoint, nextPoint) < 1e-6f || currentVelocity <= 0.0f)
            return;

        m_pCurrentModel->position = currentPoint;

        // Center Of Interest : Orientation Control 
        {
            glm::mat4 viewMatrix = glm::lookAt(m_pCurrentModel->position, nextPoint, glm::vec3(0.0f, 1.0f, 0.0f));
            // Extract the forward vector
            glm::vec3 forward = glm::normalize(glm::vec3(viewMatrix[2]));
            // Extract the up vector
            glm::vec3 upVec = glm::normalize(glm::vec3(viewMatrix[1]));

            // Calculate Euler angles
            float pitch = std::asin(-forward.y);
            float yaw = std::atan2(forward.z, forward.x);
            float roll = std::atan2(upVec.x, upVec.y);
            m_pCurrentModel->rotation = { AI_RAD_TO_DEG(pitch), AI_RAD_TO_DEG(yaw) + 90.0f, AI_RAD_TO_DEG(roll) };
        }

    }
}