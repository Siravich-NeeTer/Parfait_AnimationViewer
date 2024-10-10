#include "Model.h"

namespace Parfait
{
	Model::Model(const Graphics::VulkanContext& _vulkanContext, const Graphics::VulkanCommandPool& _vulkanCommandPool, const std::filesystem::path& _path)
		: m_VulkanContextRef(_vulkanContext),
		m_VulkanCommandPool(_vulkanCommandPool),
		m_Descriptor(std::make_unique<Graphics::VulkanDescriptor>(_vulkanContext))
	{
		m_Directory = _path.root_directory().string();
		LoadModel(_path);

		m_VertexBuffer = std::make_unique<Graphics::VulkanVertexBuffer<Graphics::Vertex>>(_vulkanContext, _vulkanCommandPool, m_Vertices.data(), m_Vertices.size());
		m_IndexBuffer = std::make_unique<Graphics::VulkanIndexBuffer>(_vulkanContext, _vulkanCommandPool, m_Indices.data(), m_Indices.size());

		if (m_BoneVertices.size() > 0)
		{
			for (size_t i = 0; i < m_BoneVertices.size(); i++)
			{
				m_BoneIndices.push_back(i);
				//m_BoneIndices.push_back(i + 1);
			}
			m_BoneVertexBuffer = std::make_unique<Graphics::VulkanVertexBuffer<Graphics::BoneVertex>>(_vulkanContext, _vulkanCommandPool, m_BoneVertices.data(), m_BoneVertices.size());
			m_BoneIndexBuffer = std::make_unique<Graphics::VulkanIndexBuffer>(_vulkanContext, _vulkanCommandPool, m_BoneIndices.data(), m_BoneIndices.size());
		}
	}
	void Model::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout _pipelineLayout)
	{
		m_PipelineLayoutRef = _pipelineLayout;

		const VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_VertexBuffer->GetBuffer(), offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

		for (Node* node: m_Nodes) 
		{
			DrawNode(commandBuffer, node);
		}
	}
	void Model::DrawBone(VkCommandBuffer commandBuffer, VkPipelineLayout _pipelineLayout)
	{
		/*
		for (unsigned int i = 0; i < scene->mNumMeshes; i++) 
		{
			const aiMesh* mesh = scene->mMeshes[i];

			for (unsigned int j = 0; j < mesh->mNumBones; j++) 
			{
				const aiBone* bone = mesh->mBones[j];

				// Transform for the current bone
				aiMatrix4x4 boneTransform = bone->mOffsetMatrix;

				// Get the bone position (assuming it’s at the origin of the bone)
				aiVector3D bonePosition(boneTransform.a4, boneTransform.b4, boneTransform.c4);

				// If you want to draw to the child bones (if available)
				for (unsigned int k = 0; k < bone->mNumWeights; k++) {
					// Here you can assume some logic to get a child bone's position
					// Example: aiVector3D childBonePosition = ...; // get from bone data or a defined structure
					aiVector3D childBonePosition = /* Logic to find child position ;

					// Draw a line from bonePosition to childBonePosition
					DrawLine(bonePosition, childBonePosition);
				}
			}
		}
		*/

		m_PipelineLayoutRef = _pipelineLayout;

		glm::mat4 model = glm::mat4(1.0f);
		model *= glm::translate(glm::mat4(1.0f), position);
		model *= glm::scale(glm::mat4(1.0f), scale);
		model *= glm::toMat4(rotation);
		//model *= glm::translate(glm::mat4(1.0f), -center);

		Graphics::MeshPushConstants meshConstants;
		meshConstants.model = model;
		meshConstants.numBones = m_BoneCounter;

		const VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_BoneVertexBuffer->GetBuffer(), offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_BoneIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
		
		// vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayoutRef, 1, 1, &m_Descriptor->GetDescriptorSets(primitive.materialIndex)[1], 0, nullptr);
		vkCmdPushConstants(commandBuffer, m_PipelineLayoutRef, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Graphics::MeshPushConstants), &meshConstants);
		vkCmdDrawIndexed(commandBuffer, m_BoneIndices.size(), 1, 0, 0, 0);
	}

	void Model::LoadModel(const std::filesystem::path& _path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(_path.string(),
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_FlipUVs |
			aiProcess_JoinIdenticalVertices |
			aiProcess_TransformUVCoords | 
			aiProcess_PopulateArmatureData
		);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			throw std::runtime_error("ERROR::ASSIMP " + std::string(importer.GetErrorString()));
		}

		for (unsigned int i = 0; i < scene->mNumMaterials; i++) 
		{
			aiMaterial* material = scene->mMaterials[i];
			bool isFoundSuitableTexture = false;

			aiTextureType textureTypes[] = {
				aiTextureType_DIFFUSE,
				aiTextureType_SPECULAR,
				aiTextureType_NORMALS,
				aiTextureType_HEIGHT,
				aiTextureType_EMISSIVE,

				aiTextureType_BASE_COLOR,
				aiTextureType_NORMAL_CAMERA,
				aiTextureType_EMISSION_COLOR,
				aiTextureType_METALNESS,
				aiTextureType_DIFFUSE_ROUGHNESS,
				aiTextureType_AMBIENT_OCCLUSION,
				// Add other texture types you may need
			};

			for (auto type : textureTypes) 
			{
				if (material->GetTextureCount(type) > 0) 
				{
					aiString texturePath;
					material->GetTexture(type, 0, &texturePath);
					// Load the texture into Vulkan
					m_Descriptor->AddDescriptorSets({ { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr } });
					m_Textures.push_back(std::make_unique<Graphics::VulkanTexture>(m_VulkanContextRef, m_VulkanCommandPool));
					m_Textures.back().get()->LoadTexture(GetDirectory(_path) + texturePath.C_Str());

					std::cout << "LOAD: " << GetDirectory(_path) + texturePath.C_Str() << "\n";

					isFoundSuitableTexture = true;
					break;
				}
			}

			if (!isFoundSuitableTexture)
			{
				m_Descriptor->AddDescriptorSets({ { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr } });
				m_Textures.push_back(std::make_unique<Graphics::VulkanTexture>(m_VulkanContextRef, m_VulkanCommandPool));
				m_Textures.back().get()->LoadTexture("Models/null.png");
			}
		}

		m_Descriptor->Init();

		for (size_t i = 0; i < m_Textures.size(); i++)
		{
			m_Descriptor->SetCurrentDescriptorIndex(i);
			for (size_t j = 0; j < Graphics::MAX_FRAMES_IN_FLIGHT; j++)
			{
				m_Descriptor->WriteImageBuffer(0, m_Textures[i]->GetImageView(), m_Textures[i]->GetSampler(), j);
				m_Descriptor->UpdateDescriptorSet();
			}
		}

		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(AssimpGLMHelpers::ConvertMatrixToGLMFormat(scene->mRootNode->mTransformation), scale, rotation, position, skew, perspective);
		//scene->mRootNode->mTransformation = aiMatrix4x4();
		scale = glm::vec3(1.0f);

		ProcessNode(scene->mRootNode, scene, nullptr);
	}
	void Model::ProcessNode(aiNode * _node, const aiScene * _scene, Node* _parent)
	{
		// Create Node & Add to list
		Node* node = new Node();
		node->matrix = AssimpGLMHelpers::ConvertMatrixToGLMFormat(_node->mTransformation);
		node->parent = _parent;

		if (_parent != nullptr)
			_parent->children.push_back(node);
		else
			m_Nodes.push_back(node);

		// process each mesh located at the current node
		for (unsigned int i = 0; i < _node->mNumMeshes; i++)
		{
			// the node object only contains indices to index the actual objects in the scene. 
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = _scene->mMeshes[_node->mMeshes[i]];
			ProcessMesh(mesh, _scene, node);
		}
		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < _node->mNumChildren; i++)
		{
			ProcessNode(_node->mChildren[i], _scene, node);
		}
	}
	void Model::ProcessMesh(aiMesh * _mesh, const aiScene * _scene, Node* _currentNode)
	{
		if (_mesh->mNumVertices == 0)
			return;

		Primitive primitive;
		primitive.firstIndex = m_Indices.size();
		curMat = _currentNode->matrix;

		uint32_t startIdx = m_Vertices.size();
		for (unsigned int i = 0; i < _mesh->mNumVertices; i++)
		{
			Graphics::Vertex vertex;

			SetVertexBoneDataToDefault(vertex);

			vertex.position = AssimpGLMHelpers::GetGLMVec(_mesh->mVertices[i]);
			vertex.normal = AssimpGLMHelpers::GetGLMVec(_mesh->mNormals[i]);

			if (_mesh->HasTextureCoords(0)) // Check for texture coordinates
			{
				glm::vec2 vec;
				vec.x = _mesh->mTextureCoords[0][i].x;
				vec.y = _mesh->mTextureCoords[0][i].y;
				vertex.uv = vec;
			}
			else
			{
				vertex.uv = glm::vec2(0.0f, 0.0f);
			}

			m_Vertices.push_back(vertex);
		}
		primitive.materialIndex = _mesh->mMaterialIndex;

		uint32_t indexCount = 0;
		for (unsigned int i = 0; i < _mesh->mNumFaces; i++)
		{
			aiFace face = _mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				m_Indices.push_back(startIdx + face.mIndices[j]);
				indexCount++;
			}
		}
		ExtractBoneWeightForVertices(m_Vertices, startIdx, _mesh, _scene);

		primitive.indexCount = indexCount;

		_currentNode->mesh.primitives.push_back(primitive);
	}

	void Model::DrawNode(VkCommandBuffer commandBuffer, Node* _node)
	{
		if (_node->mesh.primitives.size() > 0) 
		{
			// Pass the node's matrix via push constants
			// Traverse the node hierarchy to the top-most parent to get the final matrix of the current node
			glm::mat4 nodeMatrix = _node->matrix;
			Node* currentParent = _node->parent;
			while (currentParent) 
			{
				nodeMatrix = currentParent->matrix * nodeMatrix;
				currentParent = currentParent->parent;
			}


			glm::mat4 model = glm::mat4(1.0f);
			model *= glm::translate(glm::mat4(1.0f), position);
			model *= glm::scale(glm::mat4(1.0f), scale);
			model *= glm::toMat4(rotation);

			Graphics::MeshPushConstants meshConstants;
			meshConstants.model = model;
			meshConstants.numBones = m_BoneCounter;

			// Pass the final matrix to the vertex shader using push constants
			// vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &nodeMatrix);
			for (Primitive primitive : _node->mesh.primitives)
			{
				if (primitive.indexCount > 0) 
				{
					vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayoutRef, 1, 1, &m_Descriptor->GetDescriptorSets(primitive.materialIndex)[1], 0, nullptr);
					vkCmdPushConstants(commandBuffer, m_PipelineLayoutRef, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Graphics::MeshPushConstants), &meshConstants);

					vkCmdDrawIndexed(commandBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
				}
			}
		}
		for (Node* child : _node->children) 
		{
			DrawNode(commandBuffer, child);
		}
	}

	void Model::SetVertexBoneDataToDefault(Graphics::Vertex& _vertex)
	{
		for (int i = 0; i < Graphics::MAX_BONE_INFLUENCE; i++)
		{
			_vertex.boneIDs[i] = -1;
			_vertex.weights[i] = 0.0f;
		}
	}
	void Model::SetVertexBoneData(Graphics::Vertex& _vertex, int _boneID, float _weight)
	{
		for (int i = 0; i < Graphics::MAX_BONE_INFLUENCE; ++i)
		{
			if (_vertex.boneIDs[i] < 0)
			{
				_vertex.weights[i] = _weight;
				_vertex.boneIDs[i] = _boneID;
				return;
			}
		}
		//assert(0);
	}
	void Model::ExtractBoneWeightForVertices(std::vector<Graphics::Vertex>& _vertices, uint32_t _startIdx, aiMesh* _mesh, const aiScene* _scene)
	{
		for (int boneIndex = 0; boneIndex < _mesh->mNumBones; ++boneIndex)
		{
			int boneID = -1;
			std::string boneName = _mesh->mBones[boneIndex]->mName.C_Str();
			if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
			{
				BoneInfo newBoneInfo;
				newBoneInfo.id = m_BoneCounter;
				newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(_mesh->mBones[boneIndex]->mOffsetMatrix);
				m_BoneInfoMap[boneName] = newBoneInfo;
				boneID = m_BoneCounter;
				m_BoneCounter++;
				
				m_BoneVertices.push_back({ glm::vec3(curMat * glm::inverse(newBoneInfo.offset) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), {1.0f, 0.0f, 0.0f} });
			}
			else
			{
				boneID = m_BoneInfoMap[boneName].id;
			}
			assert(boneID != -1);
			aiVertexWeight* weights = _mesh->mBones[boneIndex]->mWeights;
			int numWeights = _mesh->mBones[boneIndex]->mNumWeights;

			for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
			{
				int vertexId = weights[weightIndex].mVertexId;
				float weight = weights[weightIndex].mWeight;
				assert(vertexId <= _vertices.size());
				SetVertexBoneData(_vertices[_startIdx + vertexId], boneID, weight);
			}
		}

		/*
		for (int boneIndex = 0; boneIndex < _mesh->mNumBones; ++boneIndex)
		{
			aiNode* node = _mesh->mBones[boneIndex]->mNode;
			if (m_BoneInfoMap.find(node->mParent->mName.C_Str()) != m_BoneInfoMap.end())
			{
				m_BoneVertices.push_back({ glm::vec3(AssimpGLMHelpers::ConvertMatrixToGLMFormat(node->mParent->mTransformation) * glm::inverse(m_BoneInfoMap[node->mParent->mName.C_Str()].offset) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), {1.0f, 0.0f, 0.0f} });
				m_BoneVertices.push_back({ glm::vec3(AssimpGLMHelpers::ConvertMatrixToGLMFormat(node->mTransformation) * glm::inverse(m_BoneInfoMap[node->mName.C_Str()].offset) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), {1.0f, 0.0f, 0.0f}});
			}
		}
		*/
	}

	std::string Model::GetDirectory(const std::filesystem::path& _path)
	{
		size_t pos = _path.string().find_last_of("\\/");
		return (std::string::npos == pos ? "" : _path.string().substr(0, pos + 1));
	}
}