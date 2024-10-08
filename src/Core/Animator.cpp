#include "Animator.h"

namespace Parfait
{
    Animator::Animator(Animation* _animation)
    {
        m_CurrentTime = 0.0;
        m_CurrentAnimation = _animation;

        m_FinalBoneMatrices.reserve(_animation->GetBoneIDMap().size());

        for (int i = 0; i < _animation->GetBoneIDMap().size(); i++)
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    void Animator::UpdateAnimation(float _dt)
    {
        m_DeltaTime = _dt;
        if (m_CurrentAnimation)
        {
            m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * _dt;
            m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
            CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), Math::VQS::Identity());
        }
    }

    void Animator::PlayAnimation(Animation* _pAnimation)
    {
        m_CurrentAnimation = _pAnimation;
        m_CurrentTime = 0.0f;
    }

    void Animator::CalculateBoneTransform(const AssimpNodeData* _node, Math::VQS _parentTransform)
    {
        std::string nodeName = _node->name;
        Math::VQS nodeTransform = _node->transformation;

        Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

        if (Bone)
        {
            Bone->Update(m_CurrentTime);
            nodeTransform = Bone->GetLocalTransform();
        }

        Math::VQS globalTransformation = _parentTransform * nodeTransform;

        auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
        if (boneInfoMap.find(nodeName) != boneInfoMap.end())
        {
            int index = boneInfoMap[nodeName].id;
            glm::mat4 offset = boneInfoMap[nodeName].offset;
            m_FinalBoneMatrices[index] = globalTransformation.Matrix() * offset;
        }

        for (int i = 0; i < _node->childrenCount; i++)
            CalculateBoneTransform(&_node->children[i], globalTransformation);
    }
}