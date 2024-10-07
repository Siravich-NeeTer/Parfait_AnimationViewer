#include "Animation.h"

namespace Parfait
{
    Animation::Animation(const std::string& _animationPath, Model* _model)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(_animationPath, aiProcess_Triangulate);
        assert(scene && scene->mRootNode);
        aiAnimation* animation = scene->mAnimations[0];
        m_Duration = animation->mDuration;
        m_TicksPerSecond = animation->mTicksPerSecond;
        ReadHeirarchyData(m_RootNode, scene->mRootNode);
        ReadMissingBones(animation, *_model);
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


    void Animation::ReadMissingBones(const aiAnimation* _animation, Model& _model)
    {
        int size = _animation->mNumChannels;

        auto& boneInfoMap = _model.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
        int& boneCount = _model.GetBoneCount(); //getting the m_BoneCounter from Model class

        //reading channels(bones engaged in an animation and their keyframes)
        for (int i = 0; i < size; i++)
        {
            auto channel = _animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;

            if (boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                boneInfoMap[boneName].id = boneCount;
                boneCount++;
            }
            m_Bones.push_back(Bone(channel->mNodeName.data,
                boneInfoMap[channel->mNodeName.data].id, channel));
        }

        m_BoneInfoMap = boneInfoMap;
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