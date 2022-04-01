#include "ParticleSystem.h"
#include "Engine.h"
#include "camera/ViewerCamera.h"
#include "system/Input.h"
#include "ecs/components/Static.h"
#include "ecs/components/AnimationController.h"
#include "ecs/components/BoneAttachment.h"
#include "ecs/components/MultiMeshRenderer.h"
#include "ecs/components/Particles.h"
#include "scene/Scene.h"
#include "render/debug/DebugDraw.h"
#include "render/texture/Texture.h"
#include "render/renderer/SceneRenderer.h"
#include "resources/MultiMesh.h"
#include "resources/EntityResource.h"
#include "resources/SkeletonResource.h"
#include "resources/SkeletalAnimationResource.h"
#include "system/Input.h"
#include "objects/Camera.h"
#include "utils/Math.h"
#include "render/renderer/EnvironmentSettings.h"
#include "render/renderer/RenderGraph.h"
#include "render/buffer/GPUBuffer.h"
#include "render/device/VulkanRenderState.h"
#include "imgui/imgui.h"

using namespace System;
using namespace ECS;
using namespace ECS::systems;
using namespace physx;

struct Game::Handles
{
	render::SceneRenderer::RenderDispatcher::Handle render_callback;
};

namespace
{
	constexpr uint32_t MAX_RESOLUTION = 200;
}

Game::Game() = default;
Game::~Game() = default;

void Game::init()
{
	OPTICK_EVENT();

	handles = std::make_unique<Handles>();

	auto* engine = Engine::Get();
	manager = engine->GetEntityManager();

	camera = std::make_unique<ViewerCamera>();
	Engine::Get()->GetScene()->GetCamera()->Transform().position = vec3(0, 2, -4);

	auto scifi_box_handle = Resources::EntityResource::Handle(L"assets/Entities/Basic/cube.entity");
	box_id = scifi_box_handle->Spawn(vec3(0, 0, 0));
	auto box_transform = manager->GetComponent<components::Transform>(box_id);
	box_transform->scale = vec3(1.0f);
	box_transform->bounds = AABB::Infinity();
	auto box_renderer = manager->GetComponent<components::MultiMeshRenderer>(box_id);

	auto material = Material::Create();
	material->LightingEnabled(false);
	material->SetColor(vec4(0.6, 2, 0.6, 0.2));
	material->SetShaderPath(L"shaders/particles/particle_material_default.hlsl");
	material->SetRenderQueue(RenderQueue::Additive);
	material->SetTexture0Resource(Resources::TextureResource::SRGB(L"assets/Textures/effects/light.dds"));

	auto emitter_initializer = components::ParticleEmitter::Initializer(10000, material)
		.SetEmitterGeometry(components::ParticleEmitter::EmitterGeometrySphere())
		.SetEmissionParams(components::ParticleEmitter::EmissionParams().SetSize({ 0.5f, 2 }).SetEmissionRate(1000).SetLife({ 0.5f, 4 }).SetConeAngle(M_PI / 2));

	auto emitter = manager->AddComponent<components::ParticleEmitter>(box_id, emitter_initializer);

	manager->RemoveComponent<components::MultiMeshRenderer>(box_id);

}

void Game::update(float dt)
{
	Engine::Get()->GetDebugDraw()->DrawAxis(vec3(0), glm::quat(), 15);

	camera->Update(dt);
}