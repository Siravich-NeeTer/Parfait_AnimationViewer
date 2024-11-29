#pragma once

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <map>

#include "Object.h"
#include "Bone.h"
#include "Animation.h"

#include "Math/AssimpGLMHelpers.h"

#include "Renderer/VulkanDescriptor.h"
#include "Renderer/VulkanTexture.h"

#include "Renderer/Buffers/VulkanVertexBuffer.h"
#include "Renderer/Buffers/VulkanIndexBuffer.h"
#include "Renderer/Utilities/VulkanUtilities.h"

namespace Parfait
{
	class Model : public Object
	{
		public:
			// TODO: REMOVE THIS TEMP
			glm::vec3 focusPoint = glm::vec3(0.8f, 1.47f, 1.0f);
			std::vector<glm::mat4> m_FinalBoneMatrices;

			struct Primitive
			{
				uint32_t firstIndex;
				uint32_t indexCount;
				int32_t materialIndex;
			};
			struct Mesh
			{
				std::vector<Primitive> primitives;
			};
			struct Node
			{
				Node* parent;
				std::vector<Node*> children;
				Mesh mesh;
				glm::mat4 matrix;

				~Node()
				{
					for (Node* child : children)
					{
						delete child;
					}
				}
			};
			struct BoneNode
			{
				BoneNode* parent = nullptr;
				std::vector<BoneNode*> children;

				std::string name;
				glm::mat4 offset = glm::mat4(1.0f);
				glm::mat4 rot = glm::mat4(1.0f);
				int depth = 0;

				enum MatrixType
				{
					OVERRIDE = 0,
					MULTIPLY
				};
				MatrixType matrixType = MULTIPLY;

				bool IsValid() const
				{
					return offset != glm::mat4(1.0f);
				}

				glm::mat4 GetModelMatrix()
				{
					glm::mat4 ret(1.0f);
					BoneNode* cur = this;
					while (cur)
					{
						ret *= cur->offset;
						cur = cur->parent;
					}
					return ret;
				}
			};

		public:
			Model(const Graphics::VulkanContext& _vulkanContext, const Graphics::VulkanCommandPool& _vulkanCommandPool, const std::filesystem::path& _path, uint32_t _id, const std::string& _objectName, bool _isAnimation = false);
			void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout _pipelineLayout);
			void DrawBone(VkCommandBuffer commandBuffer, VkPipelineLayout _pipelineLayout);
			void DrawJoint(VkCommandBuffer commandBuffer, VkPipelineLayout _pipelineLayout);
			void DrawPicking(VkCommandBuffer commandBuffer, VkPipelineLayout _pipelineLayout);

			void SetBoneTransformOffset(int offset) { m_BoneTransformOffset = offset; }
			void SetIsAnimation(bool active) { m_IsAnimation = active; }
			void SetCurrentActiveAnimation(const std::string& _animationName) 
			{ 
				m_FinalBoneMatrices.clear();
				m_CurrentActiveAnimation = &m_Animations[_animationName]; 
				m_FinalBoneMatrices.reserve(m_CurrentActiveAnimation->GetBoneIDMap().size());

				for (int i = 0; i < m_CurrentActiveAnimation->GetBoneIDMap().size(); i++)
					m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
			}
			void AddAnimation(const std::filesystem::path& _path, const std::string& customName = "");

			std::map<std::string, BoneInfo>& GetBoneInfoMap() { return m_BoneInfoMap; }
			int GetBoneTransformOffset() const { return m_BoneTransformOffset; }
			int& GetBoneCount() { return m_BoneCounter; }
			Animation* GetCurrentActiveAnimation() { return m_CurrentActiveAnimation; }
			Animation* GetAnimation(const std::string& _animationName) { return &m_Animations[_animationName]; }
			const std::vector<std::string>& GetAnimationNameList() const { return m_AnimationNameList; }
			BoneNode* GetBoneNode(std::string _boneNodeName) 
			{
				if (m_BoneNodeMap.find(_boneNodeName) != m_BoneNodeMap.end())
					return m_BoneNodeMap[_boneNodeName];
				return nullptr;
			}
			const glm::vec3& GetBonePosition(BoneNode* _boneNode) { return GetBonePosition(_boneNode->name); }
			const glm::vec3& GetBonePosition(std::string _boneNodeName)
			{
				if (m_BoneNodeMap.find(_boneNodeName) != m_BoneNodeMap.end())
					return glm::vec3(GetModelMatrix() * m_BoneNodeMap[_boneNodeName]->offset * glm::inverse(m_BoneInfoMap[_boneNodeName].offset) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
				return glm::vec3(0.0f);
			}

			const Graphics::VulkanDescriptor& GetDescriptor() const { return *m_Descriptor; }

			bool IsAnimation() const { return m_IsAnimation; }

		private:
			const Graphics::VulkanContext& m_VulkanContextRef;
			const Graphics::VulkanCommandPool& m_VulkanCommandPool;
			VkPipelineLayout m_PipelineLayoutRef;

			std::vector<Graphics::Vertex> m_Vertices;
			std::vector<uint32_t> m_Indices;

			std::vector<Graphics::BoneVertex> m_BoneVertices;
			std::vector<Graphics::ObjectPickingVertex> m_ObjectPickingVertices;

			std::vector<Node*> m_Nodes;
			std::map<std::string, BoneInfo> m_BoneInfoMap;
			int m_BoneCounter = 0;
			int m_BoneTransformOffset = 0;

			std::string m_Directory;
			std::map<std::string, Animation> m_Animations;
			std::vector<std::string> m_AnimationNameList;
			Animation* m_CurrentActiveAnimation;

			std::unique_ptr<Graphics::VulkanDescriptor> m_Descriptor;
			bool m_IsAnimation = false;

			std::unique_ptr<Graphics::VulkanVertexBuffer<Graphics::Vertex>> m_VertexBuffer;
			std::unique_ptr<Graphics::VulkanIndexBuffer> m_IndexBuffer;
			std::vector<std::unique_ptr<Graphics::VulkanTexture>> m_Textures;

			std::unique_ptr<Graphics::VulkanVertexBuffer<Graphics::BoneVertex>> m_BoneVertexBuffer;
			std::unique_ptr<Graphics::VulkanVertexBuffer<Graphics::ObjectPickingVertex>> m_ObjectPickingVertexBuffer;

			std::vector<glm::vec3> m_SphereVertices;
			std::unique_ptr<Graphics::VulkanVertexBuffer<glm::vec3>> m_SpherePointBuffer;

			// Temp: For Inverse Kinematic
			std::map<std::string, std::vector<std::string>> m_BoneHierachy;
			std::set<std::string> m_BoneNameList;

			std::vector<BoneNode*> m_RootBoneList;
			std::map<std::string, BoneNode*> m_BoneNodeMap;


			void LoadModel(const std::filesystem::path& _path);
			void ProcessNode(aiNode* _node, const aiScene* _scene, Node* _parent);
			void ProcessMesh(aiMesh* _mesh, const aiScene* _scene, Node* _currentNode);

			void DrawNode(VkCommandBuffer commandBuffer, Node* _node);
			void DrawPickingNode(VkCommandBuffer commandBuffer, Node* _node);

			void SetVertexBoneDataToDefault(Graphics::Vertex& _vertex);
			void SetVertexBoneData(Graphics::Vertex& _vertex, int _boneID, float _weight);
			void ExtractBoneWeightForVertices(std::vector<Graphics::Vertex>& _vertices, uint32_t _startIdx, aiMesh* _mesh, const aiScene* _scene);

			void ExtractBoneHierachy();
			void ProcessBoneNodeHierachy(BoneNode* _boneNode);
			void PrintBoneHierachy();

			std::string GetDirectory(const std::filesystem::path& _path);
	};
}