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

	void Model::LoadModel(const std::filesystem::path& _path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(_path.string(),
			aiProcess_Triangulate |
			aiProcess_FlipUVs |
			aiProcess_JoinIdenticalVertices |
			aiProcess_PreTransformVertices |
			aiProcess_TransformUVCoords
		);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			throw std::runtime_error("ERROR::ASSIMP " + std::string(importer.GetErrorString()));
		}

		for (unsigned int i = 0; i < scene->mNumMaterials; i++) 
		{
			aiMaterial* material = scene->mMaterials[i];

			aiTextureType textureTypes[] = {
				aiTextureType_DIFFUSE,
				aiTextureType_SPECULAR,
				aiTextureType_NORMALS,
				aiTextureType_HEIGHT,
				// Add other texture types you may need
			};

			for (auto type : textureTypes) 
			{
				if (material->GetTextureCount(type) > 0) 
				{
					aiString texturePath;
					material->GetTexture(type, 0, &texturePath);
					std::string fullPath = texturePath.C_Str();
					// Load the texture into Vulkan

					std::cout << fullPath << "\n";

					m_Descriptor->AddDescriptorSets({ { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr } });
					m_Textures.push_back(std::make_unique<Graphics::VulkanTexture>(m_VulkanContextRef, m_VulkanCommandPool));
					m_Textures.back().get()->LoadTexture("Models/" + fullPath);
				}
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

		ProcessNode(scene->mRootNode, scene, nullptr);
	}
	void Model::ProcessNode(aiNode * _node, const aiScene * _scene, Node* _parent)
	{
		// Create Node & Add to list
		Node* node = new Node();
		node->matrix = Graphics::AssimpGLMHelpers::ConvertMatrixToGLMFormat(_node->mTransformation);
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

		uint32_t startIdx = m_Vertices.size();
		for (unsigned int i = 0; i < _mesh->mNumVertices; i++)
		{
			Graphics::Vertex vertex;
			vertex.position = Graphics::AssimpGLMHelpers::GetGLMVec(_mesh->mVertices[i]);

			if (_mesh->mTextureCoords[0]) // Check for texture coordinates
			{
				glm::vec2 vec;
				vec.x = _mesh->mTextureCoords[0][i].x;
				vec.y = _mesh->mTextureCoords[0][i].y;
				vertex.uv = vec;
				primitive.materialIndex = _mesh->mMaterialIndex;
			}
			else
			{
				vertex.uv = glm::vec2(0.0f, 0.0f);
			}

			m_Vertices.push_back(vertex);
		}

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
			model *= glm::scale(glm::mat4(1.0f), scale);
			model *= glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
			model *= glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
			model *= glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
			model *= glm::translate(glm::mat4(1.0f), position);

			// Pass the final matrix to the vertex shader using push constants
			// vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &nodeMatrix);
			for (Primitive primitive : _node->mesh.primitives)
			{
				if (primitive.indexCount > 0) 
				{
					vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayoutRef, 1, 1, &m_Descriptor->GetDescriptorSets(primitive.materialIndex)[1], 0, nullptr);
					vkCmdPushConstants(commandBuffer, m_PipelineLayoutRef, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &model);

					vkCmdDrawIndexed(commandBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
				}
			}
		}
		for (Node* child : _node->children) 
		{
			DrawNode(commandBuffer, child);
		}
	}
}