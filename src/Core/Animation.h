#pragma once

#include "Bone.h"
#include "Model.h"

#include "Math/AssimpGLMHelpers.h"

#include "Renderer/Utilities/VulkanUtilities.h"

namespace Parfait
{
    struct AssimpNodeData
    {
        Math::VQS transformation;
        std::string name;
        int childrenCount;
        std::vector<AssimpNodeData> children;
    };

    class Animation
    {
        public:
            Animation() = default;
            Animation(const std::string& _animationPath, Model* _model);

            ~Animation();

            Bone* FindBone(const std::string& _name);

            float GetTicksPerSecond() const { return m_TicksPerSecond; }
            float GetDuration() const { return m_Duration; }
            const AssimpNodeData& GetRootNode() const { return m_RootNode; }
            const std::map<std::string, BoneInfo>& GetBoneIDMap() const { return m_BoneInfoMap; }

        private:
            float m_Duration;
            int m_TicksPerSecond;
            std::vector<Bone> m_Bones;
            AssimpNodeData m_RootNode;
            std::map<std::string, BoneInfo> m_BoneInfoMap;

            void ReadMissingBones(const aiAnimation* _animation, Model& _model);
            void ReadHeirarchyData(AssimpNodeData& _dest, const aiNode* _src);
    };
}