#pragma once

#include "Animation.h"

namespace Parfait
{
    class Animator
    {
        public:
            Animator(Animation* _animation);

            void UpdateAnimation(float _dt);
            void PlayAnimation(Animation* _pAnimation);

            void CalculateBoneTransform(const AssimpNodeData* _node, Math::VQS _parentTransform);

            const std::vector<glm::mat4>& GetFinalBoneMatrices() const { return m_FinalBoneMatrices;  }

        private:
            std::vector<glm::mat4> m_FinalBoneMatrices;
            Animation* m_CurrentAnimation;
            float m_CurrentTime;
            float m_DeltaTime;
    };
}