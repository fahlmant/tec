#include "graphics/vertex-buffer-object.hpp"
#include "graphics/shader.hpp"
#include "graphics/material.hpp"
#include "graphics/texture-object.hpp"
#include "components/transforms.hpp"
#include "components/collisionbody.hpp"
#include "components/camera.hpp"
#include "resources/md5mesh.hpp"
#include "resources/pixel-buffer.hpp"
#include "resources/md5anim.hpp"
#include "entity.hpp"
#include "component-update-system.hpp"
#include "render-system.hpp"
#include "physics-system.hpp"
#include "voxelvolume.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace tec {
	void IntializeComponents() {
		ComponentUpdateSystem<Velocity>::Initialize();
		ComponentUpdateSystem<Position>::Initialize();
		ComponentUpdateSystem<Orientation>::Initialize();
		ComponentUpdateSystem<Scale>::Initialize();
		ComponentUpdateSystem<Camera>::Initialize();
		ComponentUpdateSystem<Renderable>::Initialize();
		ComponentUpdateSystem<View>::Initialize();
		ComponentUpdateSystem<Animation>::Initialize();
		ComponentUpdateSystem<CollisionBody>::Initialize();
	}

	void BuildTestEntities() {
		auto shader_files = std::list < std::pair<Shader::ShaderType, std::string> > {
			std::make_pair(Shader::VERTEX, "assets/basic.vert"), std::make_pair(Shader::FRAGMENT, "assets/basic.frag"),
		};
		auto s = Shader::CreateFromFile("shader1", shader_files);
		auto basic_fill = Material::Create("material_basic", s);

		shader_files = std::list < std::pair<Shader::ShaderType, std::string> > {
			std::make_pair(Shader::VERTEX, "assets/basic.vert"), std::make_pair(Shader::FRAGMENT, "assets/overlay.frag"),
		};
		auto s_overlay = Shader::CreateFromFile("shader_overlay", shader_files);
		auto overlay = Material::Create("material_overlay", s_overlay);
		overlay->SetPolygonMode(GL_LINE);

		auto voxvol = VoxelVolume::Create(100, "bob", 0);
		auto voxvol_shared = voxvol.lock();
		Entity voxel1(100);
		voxel1.Add<Position>();
		voxel1.Add<Orientation>();

		VoxelCommand add_voxel(
			[ ] (VoxelVolume* vox_vol) {
			vox_vol->AddVoxel(-1, 1, 1);
			vox_vol->AddVoxel(-1, -1, 1);
			vox_vol->AddVoxel(-1, -1, 0);
			vox_vol->AddVoxel(-1, -1, -1);
			vox_vol->AddVoxel(-1, 0, 1);
			vox_vol->AddVoxel(-1, 0, 0);
			vox_vol->AddVoxel(-1, 0, -1);
			vox_vol->AddVoxel(1, -1, 1);
		});
		VoxelVolume::QueueCommand(std::move(add_voxel));
		voxvol_shared->Update(0.0);
		auto voxvol_vert_buffer = std::make_shared<VertexBufferObject>();
		voxel1.Add<Renderable>(voxvol_vert_buffer);
		{
			std::shared_ptr<CollisionBody> colbody = std::make_shared<CollisionMesh>(100, voxvol_shared->GetMesh().lock());
			voxel1.Add(colbody);
		}

		RenderCommand buffer_func([voxvol_vert_buffer, voxvol_shared, s] (RenderSystem* sys) {
			auto mesh = voxvol_shared->GetMesh().lock();
			voxvol_vert_buffer->Load(mesh, s);
			auto voxel1Renderable = Entity(100).Get<Renderable>().lock();
			for (size_t i = 0; i < voxvol_vert_buffer->GetVertexGroupCount(); ++i) {
				voxel1Renderable->vertex_groups.insert(voxvol_vert_buffer->GetVertexGroup(i));
			}
		});
		RenderSystem::QueueCommand(std::move(buffer_func));

		Entity bob(99);
		auto mesh1 = MD5Mesh::Create("assets/bob/bob.md5mesh");
		{
			auto renderable = std::make_shared<Renderable>(std::make_shared<VertexBufferObject>());
			renderable->buffer->Load(mesh1, s);
			for (size_t i = 0; i < renderable->buffer->GetVertexGroupCount(); ++i) {
				renderable->vertex_groups.insert(renderable->buffer->GetVertexGroup(i));
			}
			bob.Add<Renderable>(renderable);
		}

		auto anim1 = MD5Anim::Create("assets/bob/bob.md5anim", mesh1);
		bob.Add<Animation>(anim1);
		{
			std::shared_ptr<CollisionBody> colbody = std::make_shared<CollisionMesh>(99, mesh1, true);
			bob.Add(colbody);
		}
		bob.Add<Position>(glm::vec3(0.0, 0.0, -1.0));
		bob.Add<Orientation>(glm::vec3(glm::radians(-90.0), 0.0, 0.0));

		Entity camera(1);
		camera.Add<Position>();
		camera.Add<Orientation>();
		camera.Add<Camera>(1);
		camera.Add<Renderable>(voxvol_vert_buffer);
		Entity camera2(2);
		camera2.Add<Position>();
		camera2.Add<Orientation>();
		camera2.Add<Camera>(2);
	}
}
