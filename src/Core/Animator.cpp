#include "Animator.h"

namespace Parfait
{
    Animator::Animator(Model* model)
    {
        m_CurrentAnimationTime = 0.0f;
        m_CurrentNextAnimationTime = 0.0f;

        m_pCurrentModel = model;

        m_pCurrentAnimation = model->GetCurrentActiveAnimation();
        m_pNextAnimation = nullptr;

        m_FinalBoneMatrices.reserve(model->GetCurrentActiveAnimation()->GetBoneIDMap().size());

        for (int i = 0; i < model->GetCurrentActiveAnimation()->GetBoneIDMap().size(); i++)
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
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
        m_BlendFactor = _blendFactor;
    }

    void Animator::CalculateBoneTransform(const AssimpNodeData* _node, Math::VQS _parentTransform)
    {
        const std::string& nodeName = _node->name;
        Math::VQS nodeTransform = _node->transformation;

        Bone* Bone = m_pCurrentAnimation->FindBone(nodeName);

        if (Bone)
        {
            Bone->Update(m_CurrentAnimationTime);
            nodeTransform = Bone->GetLocalTransform();
        }

        const Math::VQS& globalTransformation = _parentTransform * nodeTransform;

        auto& boneInfoMap = m_pCurrentAnimation->GetBoneIDMap();
        const auto& currentBoneIt = boneInfoMap.find(nodeName);
        if (currentBoneIt != boneInfoMap.end())
        {
            int index = currentBoneIt->second.id;
            const glm::mat4& offset = currentBoneIt->second.offset;
            m_FinalBoneMatrices[index] = globalTransformation.Matrix() * offset;
        }

        for (int i = 0; i < _node->childrenCount; i++)
            CalculateBoneTransform(&_node->children[i], globalTransformation);
    }
    void Animator::CalculateBoneTransform(const AssimpNodeData* _currentAnimationNode, const AssimpNodeData* _nextAnimationNode, Math::VQS _parentTransform)
    {
        const std::string& nodeStartName = _currentAnimationNode->name;
        const std::string& nodeEndName = _nextAnimationNode->name;

        Math::VQS nodeTransform = Math::VQS::Identity();

        Bone* boneStart = m_pCurrentAnimation->FindBone(nodeStartName);
        Bone* boneEnd = m_pNextAnimation->FindBone(nodeEndName);

        nodeTransform = _currentAnimationNode->transformation;

        if (boneStart && boneEnd)
        {
            glm::vec3 startTranslation, endTranslation;
            Math::Quaternion startQuaternion, endQuaternion;
            glm::vec3 startScale, endScale;

            std::tie(startTranslation, startQuaternion, startScale) = boneStart->GetInterpolateTransform(m_CurrentAnimationTime);
            std::tie(endTranslation, endQuaternion, endScale) = boneEnd->GetInterpolateTransform(m_CurrentNextAnimationTime);
            
            glm::vec3 finalTranslation = glm::mix(startTranslation, endTranslation, m_BlendFactor);
            Math::Quaternion finalQuaternion = Math::Lerp(startQuaternion, endQuaternion, m_BlendFactor);
            glm::vec3 finalScale = glm::mix(startScale, endScale, m_BlendFactor);

            nodeTransform = Math::VQS(finalTranslation, finalQuaternion, finalScale);
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

            m_FinalBoneMatrices[index1] = globalTransformation.Matrix() * offset1;
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

        if (currentVelocity <= 0.25f)
        {
            BlendAnimation("Idle", currentVelocity / 0.25f);
        }
        else if (currentVelocity >= 0.75f)
        {
            BlendAnimation("Run", (currentVelocity - 0.75f) / 0.25f);
        }
        else
        {
            BlendAnimation("", 0.0f);
        }

        if (glm::distance(currentPoint, nextPoint) < 1e-6f)
            return;

        m_pCurrentModel->position = currentPoint;

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