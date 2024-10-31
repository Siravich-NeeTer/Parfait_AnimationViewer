#include "Animator.h"

namespace Parfait
{
    Animator::Animator(Model* model)
    {
        m_CurrentTime = 0.0;
        m_pCurrentModel = model;
        m_pCurrentAnimation = model->GetCurrentActiveAnimation();

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
        if (m_pCurrentAnimation)
        {
            m_CurrentTime += m_pCurrentAnimation->GetTicksPerSecond() * _dt;
            m_CurrentTime = fmod(m_CurrentTime, m_pCurrentAnimation->GetDuration());
            CalculateBoneTransform(&m_pCurrentAnimation->GetRootNode(), Math::VQS::Identity());
        }
    }

    void Animator::PlayAnimation(Animation* _pAnimation)
    {
        m_pCurrentAnimation = _pAnimation;
        m_CurrentTime = 0.0f;
    }
    void Animator::PlayAnimation(const std::string& _animationName)
    {
        m_pCurrentModel->SetCurrentActiveAnimation(_animationName);
        PlayAnimation(m_pCurrentModel->GetCurrentActiveAnimation());
    }

    void Animator::CalculateBoneTransform(const AssimpNodeData* _node, Math::VQS _parentTransform)
    {
        const std::string& nodeName = _node->name;
        Math::VQS nodeTransform = _node->transformation;

        Bone* Bone = m_pCurrentAnimation->FindBone(nodeName);

        if (Bone)
        {
            Bone->Update(m_CurrentTime);
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
}