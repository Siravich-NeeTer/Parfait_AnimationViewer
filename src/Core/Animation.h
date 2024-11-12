#pragma once

#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Math/AssimpGLMHelpers.h"
#include "Math/VQS.h"

#include "Core/Bone.h"

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
            Animation(const std::string& _animationPath, std::map<std::string, BoneInfo>& _boneInfoMap, int& _boneCounter, size_t _animationIndex = 0);
            Animation(const aiScene* _scene, const aiAnimation* _animation, std::map<std::string, BoneInfo>& _boneInfoMap, int& _boneCounter);

            ~Animation();

            Bone* FindBone(const std::string& _name);

            float GetTicksPerSecond() const { return m_TicksPerSecond; }
            float GetDuration() const { return m_Duration; }
            const AssimpNodeData& GetRootNode() const { return m_RootNode; }
            std::map<std::string, BoneInfo>& GetBoneIDMap() { return m_BoneInfoMap; }
            bool IsValid() const { return m_IsAnimationValid; }

        private:
            float m_Duration;
            int m_TicksPerSecond;
            std::vector<Bone> m_Bones;
            AssimpNodeData m_RootNode;
            std::map<std::string, BoneInfo> m_BoneInfoMap;
            bool m_IsAnimationValid = false;

            void ReadMissingBones(const aiAnimation* _animation, std::map<std::string, BoneInfo>& _boneInfoMap, int& _boneCounter);
            void ReadHeirarchyData(AssimpNodeData& _dest, const aiNode* _src);
    };
}