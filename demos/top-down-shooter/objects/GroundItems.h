#pragma once

#include <scene/Behaviour.h>
#include "TopDownShooter.h"
#include <unordered_map>
#include "utils/Math.h"
#include "ecs/components/Transform.h"
#include "ecs/components/MultiMeshRenderer.h"
#include <magic_enum/magic_enum.hpp>
#include "resources/MaterialResource.h"

namespace scene
{
	class GrassTile : public Behaviour
	{
		glm::ivec2 position;

	public:
		GrassTile(glm::ivec2 position);
		void Awake() override;
	};


	class GroundItemsCache
	{
	public:
		enum class GrassType
		{
			Kentucky,
			Lamium
		};

		struct GrassHandles
		{
			Resources::MaterialResource::Handle material;
			Resources::MultiMesh::Handle mesh;
		};

		static constexpr float TILE_SIZE = 6.0f;
		static constexpr int32_t TILE_VISIBLE_RADIUS = 1;
		static constexpr int32_t TILE_VISIBLE_DIAMETER = TILE_VISIBLE_RADIUS * 2 + 1;
		static constexpr float SPLATMAP_SIZE = 6.0f;

		GroundItemsCache(ECS::EntityManager& manager);
		void Update(float dt);
		void OnMovedToTile(glm::ivec2 tile);

		static ivec2 TileFromCoord(vec3 coord);
		static AABBIVec2 VisibleAABBForTile(ivec2);
		static void ForEachTile(AABBIVec2 aabb, std::function<void(ivec2)> callback) 
		{ 
			for (int i = aabb.min.x; i <= aabb.max.x; i++)
				for (int j = aabb.min.y; j <= aabb.max.y; j++)
					callback(ivec2(i, j));
		};
		static vec3 GetTileOrigin(glm::ivec2 tile) { return vec3(tile.x * TILE_SIZE, 0, tile.y * TILE_SIZE); }

		static GroundItemsCache* GetInstance() { return instance; }

		const auto& GetGrassHandle(GrassType type) const { return grass_handles[(size_t)type]; }

	private:
		void AwakeTile(glm::ivec2 tile);
		void SleepTile(glm::ivec2 tile);

	private:
		static inline const auto INIT_TILE = glm::ivec2(std::numeric_limits<glm::ivec2::value_type>::max());
		static inline GroundItemsCache* instance = nullptr;

		struct TileData
		{
			bool is_sleeping = true;
			GrassTile* tile = nullptr;
			
			struct Hasher
			{
				size_t operator()(glm::ivec2 data) const { return FastHash(&data, sizeof(data)); }
			};
		};

		std::array<GrassHandles, magic_enum::enum_count<GrassType>()> grass_handles;

		std::unordered_map<glm::ivec2, TileData, TileData::Hasher> tiles;

		glm::ivec2 current_tile = INIT_TILE;
		ECS::EntityManager& manager;
	};
}