#include "Animation.h"

namespace Parfait
{
    Animation::Animation(const std::string& _animationPath, std::map<std::string, BoneInfo>& _boneInfoMap, int& _boneCounter, size_t _animationIndex)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(_animationPath, aiProcess_Triangulate);
        assert(scene && scene->mRootNode);

        m_IsAnimationValid = scene->HasAnimations();
        if (scene->HasAnimations())
        {
            scene->mRootNode->mTransformation = aiMatrix4x4();

            aiAnimation* animation = scene->mAnimations[_animationIndex];
            m_Duration = animation->mDuration;
            m_TicksPerSecond = animation->mTicksPerSecond;
            ReadHeirarchyData(m_RootNode, scene->mRootNode);
            ReadMissingBones(animation, _boneInfoMap, _boneCounter);
        }
    }
    Animation::Animation(const aiScene* _scene, const aiAnimation* _animation, std::map<std::string, BoneInfo>& _boneInfoMap, int& _boneCounter)
    {
        if (_animation)
        {
            m_IsAnimationValid = true;
            _scene->mRootNode->mTransformation = aiMatrix4x4();

            m_Duration = _animation->mDuration;
            m_TicksPerSecond = _animation->mTicksPerSecond;
            ReadHeirarchyData(m_RootNode, _scene->mRootNode);
            ReadMissingBones(_animation, _boneInfoMap, _boneCounter);
        }
    }

    Animation::~Animation()
    {
    }

    Bone* Animation::FindBone(const std::string& _name)
    {
        auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
            [&](const Bone& Bone)
            {
                return Bone.GetBoneName() == _name;
            }
        );
        if (iter == m_Bones.end()) 
            return nullptr;
        else 
            return &(*iter);
    }

    void Animation::ReadMissingBones(const aiAnimation* _animation, std::map<std::string, BoneInfo>& _boneInfoMap, int& _boneCounter)
    {
        int size = _animation->mNumChannels;

        //reading channels(bones engaged in an animation and their keyframes)
        for (int i = 0; i < size; i++)
        {
            auto channel = _animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;

            if (_boneInfoMap.find(boneName) == _boneInfoMap.end())
            {
                _boneInfoMap[boneName].id = _boneCounter;
                _boneCounter++;
            }
            m_Bones.push_back(Bone(channel->mNodeName.data,
                _boneInfoMap[channel->mNodeName.data].id, channel));
        }

        m_BoneInfoMap = _boneInfoMap;
    }

    void Animation::ReadHeirarchyData(AssimpNodeData& _dest, const aiNode* _src)
    {
        assert(_src);

        _dest.name = _src->mName.data;
        _dest.transformation = Math::MatrixToVQS(AssimpGLMHelpers::ConvertMatrixToGLMFormat(_src->mTransformation));
        _dest.childrenCount = _src->mNumChildren;

        for (int i = 0; i < _src->mNumChildren; i++)
        {
            AssimpNodeData newData;
            ReadHeirarchyData(newData, _src->mChildren[i]);
            _dest.children.push_back(newData);
        }
    }
}