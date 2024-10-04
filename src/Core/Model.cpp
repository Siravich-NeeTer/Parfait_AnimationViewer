#include "Model.h"

namespace Parfait
{
	Model::Model(const Graphics::VulkanContext& _vulkanContext, const Graphics::VulkanCommandPool& _vulkanCommandPool, const std::filesystem::path& _path)
	{
		m_Directory = _path.root_directory().string();
		LoadModel(_path);

		m_VertexBuffer = std::make_unique<Graphics::VulkanVertexBuffer<Graphics::Vertex>>(_vulkanContext, _vulkanCommandPool, m_Vertices.data(), m_Vertices.size());
		m_IndexBuffer = std::make_unique<Graphics::VulkanIndexBuffer>(_vulkanContext, _vulkanCommandPool, m_Indices.data(), m_Indices.size());
	}
	void Model::Draw(VkCommandBuffer commandBuffer)
	{
		const VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_VertexBuffer->GetBuffer(), offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(commandBuffer, m_Indices.size(), 1, 0, 0, 0);
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

		/*
		for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
			aiMaterial* material = scene->mMaterials[i];

			aiTextureType textureTypes[] = {
				aiTextureType_DIFFUSE
				// aiTextureType_SPECULAR,
				// aiTextureType_NORMALS,
				// aiTextureType_HEIGHT,
				// Add other texture types you may need
			};

			for (auto type : textureTypes) {
				if (material->GetTextureCount(type) > 0) {
					aiString texturePath;
					material->GetTexture(type, 0, &texturePath);
					std::string fullPath = texturePath.C_Str();
					// Load the texture into Vulkan

					std::cout << fullPath << "\n";

					m_Textures.push_back(std::make_unique<VulkanTexture>(m_VkContextRef, *m_CommandPool));
					m_Textures.back().get()->LoadTexture("Models/" + fullPath);
				}
			}
		}
		*/

		ProcessNode(scene->mRootNode, scene);
	}
	void Model::ProcessNode(aiNode * _node, const aiScene * _scene)
	{
		// process each mesh located at the current node
		for (unsigned int i = 0; i < _node->mNumMeshes; i++)
		{
			// the node object only contains indices to index the actual objects in the scene. 
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = _scene->mMeshes[_node->mMeshes[i]];
			ProcessMesh(mesh, _scene);
		}
		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < _node->mNumChildren; i++)
		{
			ProcessNode(_node->mChildren[i], _scene);
		}
	}
	void Model::ProcessMesh(aiMesh * _mesh, const aiScene * _scene)
	{
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
				//vertex.texIndex = _mesh->mMaterialIndex;
			}
			else
			{
				vertex.uv = glm::vec2(0.0f, 0.0f);
			}

			m_Vertices.push_back(vertex);
		}
		for (unsigned int i = 0; i < _mesh->mNumFaces; i++)
		{
			aiFace face = _mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				m_Indices.push_back(startIdx + face.mIndices[j]);
		}
	}
}