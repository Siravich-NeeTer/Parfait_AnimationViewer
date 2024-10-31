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

            void CalculateBoneTransform(const AssimpNodeData* _node, Math::VQS _parentTransform);

            Animation* GetAnimation() const { return m_pCurrentAnimation; }
            Model* GetModel() const { return m_pCurrentModel; }
            const std::vector<glm::mat4>& GetFinalBoneMatrices() const { return m_FinalBoneMatrices;  }

        private:
            std::vector<glm::mat4> m_FinalBoneMatrices;
            Animation* m_pCurrentAnimation;
            Model* m_pCurrentModel;
            float m_CurrentTime;
            float m_DeltaTime;
    };
}