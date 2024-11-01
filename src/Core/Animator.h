#pragma once

#include "Core/Model.h"
#include "Core/Animation.h"

namespace Parfait
{
    class Animator
    {
        public:
            Animator(Model* model);

            void UpdateAnimation(float _dt);
            void PlayAnimation(Animation* _pAnimation);
            void PlayAnimation(const std::string& _animationName);
            void BlendAnimation(const std::string& _newAnimationName, float _blendFactor);

            void CalculateBoneTransform(const AssimpNodeData* _node, Math::VQS _parentTransform);
            void CalculateBoneTransform(const AssimpNodeData* _currentAnimationNode, const AssimpNodeData* _nextAnimationNode, Math::VQS _parentTransform);

            Animation* GetAnimation() const { return m_pCurrentAnimation; }
            Model* GetModel() const { return m_pCurrentModel; }
            float GetBlendFactor() const { return m_BlendFactor; }
            const std::vector<glm::mat4>& GetFinalBoneMatrices() const { return m_FinalBoneMatrices;  }

        private:
            std::vector<glm::mat4> m_FinalBoneMatrices;
            Animation* m_pCurrentAnimation;
            Animation* m_pNextAnimation;
            Model* m_pCurrentModel;
            float m_CurrentAnimationTime;
            float m_CurrentNextAnimationTime;
            float m_DeltaTime;
            float m_BlendFactor;
    };
}