#pragma once

#include <memory>
#include <tuple>
#include <vector>
#include <set>
#include <array>
#include <algorithm>
#include <functional>
#include <typeinfo>
#include <typeindex>
#include <list>

#include "components/Entity.h"
#include "utils/Math.h"

namespace core { namespace ECS {

	const uint32_t MAX_COMPONENTS = 16;
	const uint32_t CHUNK_SIZE = 16 * 1024;

	typedef size_t ComponentHash;
	typedef uint32_t ComponentSetHash;

	template<typename T>
	ComponentHash GetComponentHash()
	{
		return std::type_index(typeid(T)).hash_code();
	}

	typedef std::function<void(uint64_t, EntityAddress)> EntityAddressChangedCallback;

	// Data that allows to locate component array memory location inside the chunk
	struct ComponentData
	{
		friend bool operator<(const ComponentData& l, const ComponentData& r)
		{
			return l.hash < r.hash;
		}

		bool operator==(const ComponentData& other) const
		{
			return hash == other.hash && offset == other.offset && size == other.size;
		}

		ComponentHash hash;
		uint32_t offset;
		uint32_t size;
	};

	class ComponentLayout
	{
	public:
		ComponentLayout(const ComponentLayout&) = default;

		explicit ComponentLayout(size_t chunk_size)
			: chunk_size(chunk_size)
		{
			memset(components.data(), 0, sizeof(components));
		}

		uint32_t GetMaxEntityCount() const { return max_entity_count; }
		uint32_t GetComponentCount() const { return component_count; }
		ComponentData* GetComponents() { return components.data(); }
		const ComponentData* GetComponents() const { return components.data(); }
		uint32_t GetHash() const { return hash; }

		template<typename T>
		bool AddComponent()
		{
			auto hash = GetComponentHash<T>();
			auto* existing_component = GetComponentData(hash);
			if (existing_component)
				return false;

			components[component_count] = ComponentData{ hash, 0, sizeof(T) };
			component_count += 1;
			assert(component_count <= MAX_COMPONENTS);
			std::sort(components.begin(), components.begin() + component_count);
			RecalculateOffsets();
			UpdateHash();

			return true;
		}

		template<typename T>
		bool RemoveComponent()
		{
			auto hash = GetComponentHash<T>();
			auto* existing_component = GetComponentData(hash);
			if (!existing_component)
				return false;

			auto new_end = std::remove(components.begin(), components.begin() + component_count, *existing_component);
			component_count -= 1;
			assert(component_count >= 0);
			memset(&*(components.begin() + component_count), 0, sizeof(ComponentData));
			
			RecalculateOffsets();
			UpdateHash();

			return true;
		}

		const ComponentData* GetComponentData(ComponentHash hash) const
		{
			const ComponentData* result = nullptr;

			auto end = components.begin() + component_count;
			auto data = ComponentData{ hash, 0, 0 };
			auto it = std::lower_bound(components.begin(), end, data);

			if (it != end && it->hash == hash)
			{
				result = &*it;
			}

			return result;
		}

		void RecalculateOffsets()
		{
			uint32_t total_size = 0;
			for (auto& component : components)
				total_size += component.size;

			max_entity_count = (uint32_t)chunk_size / total_size;

			auto last_offset = 0;
			for (auto& component : components)
			{
				component.offset = last_offset;
				last_offset += component.size * max_entity_count;
			}

			assert(max_entity_count > 0);
		}

	private:
		void UpdateHash()
		{
			hash = 0;
			for (auto& component : components)
			{
				auto pair = std::make_pair(hash, component.hash);
				hash = FastHash(&pair, sizeof(pair));
			}
		}

		std::array<ComponentData, MAX_COMPONENTS> components;
		uint32_t component_count = 0;
		uint32_t max_entity_count = 0;
		uint32_t hash = 0;
		size_t chunk_size;
	};

	class Chunk
	{
	public:
		Chunk(const ComponentLayout& layout, EntityAddressChangedCallback entity_address_callback)
			: layout(layout) 
			, entity_address_callback(entity_address_callback)
			, entity_count(0)
			, memory(malloc(CHUNK_SIZE))
		{}

		Chunk(const ComponentLayout&& layout, EntityAddressChangedCallback entity_address_callback) = delete;
		
		~Chunk()
		{
			if (memory)
				free(memory);
		}
		
		void* GetMemory() const { return memory; }
		Chunk* GetNextChunk() const { return next.get(); };
		
		void* GetComponentPointer(uint32_t index, ComponentHash hash)
		{
			auto* data = layout.GetComponentData(hash);
			return GetComponentPointer(index, data);
		}

		// TODO: getting components in systems without referencing layouts and chunk instances (pass pointer and ComponentData into a static function)
		void* GetComponentPointer(uint32_t index, const ComponentData* data)
		{
			if (!data)
				return nullptr;

			return (char*)memory + data->offset + (size_t)data->size * (size_t)index;
		}

		uint32_t GetEntityCount() const { return entity_count; }

		EntityAddress AllocateAddress()
		{
			if (entity_count < layout.GetMaxEntityCount())
			{
				EntityAddress result = { this, entity_count };
				entity_count += 1;
				return result;
			}
			else
			{
				if (!next)
					next = std::make_unique<Chunk>(layout, entity_address_callback);

				return next->AllocateAddress(); // todo: make non-recursive
			}
		}

		const ComponentLayout& GetComponentLayout() const { return layout; }

		EntityAddress AddEntity(EntityID id, const EntityAddress* previous_address = nullptr)
		{
			auto new_address = AllocateAddress();

			// Copy data
			if (previous_address)
			{
				auto* previous_chunk = previous_address->chunk;
				auto& previous_layout = previous_chunk->GetComponentLayout();

				for (int i = 0; i < layout.GetComponentCount(); i++)
				{
					auto* data = &layout.GetComponents()[i];

					auto* previous_data = previous_layout.GetComponentData(data->hash);
					if (previous_data)
					{
						auto* new_pointer = new_address.chunk->GetComponentPointer(new_address.index, data);
						auto previous_pointer = previous_chunk->GetComponentPointer(previous_address->index, previous_data);
						memcpy(new_pointer, previous_pointer, data->size);
					}
				}
			}

			auto* entity = (EntityData*)new_address.chunk->GetComponentPointer(new_address.index, GetComponentHash<EntityData>());
			// Entity is new, need to setup EntityData component
			if (!previous_address)
			{
				entity->address = new_address;
				entity->id = id;
			}

			assert(entity->id == id);

			// Every time entity is moved to a different chunk the layout is updated
			entity->layout = &layout;

			entity_address_callback(entity->id, new_address); // Notify address changed

			return new_address;
		}

		void RemoveEntity(uint32_t index)
		{
			assert(index < entity_count);

			auto last_entity_index = entity_count - 1;
			
			// Swap deleted component data with the last one
			if (index < last_entity_index)
			{
				for (int i = 0; i < layout.GetComponentCount(); i++)
				{
					auto& component_data = layout.GetComponents()[i];

					auto* remove_pointer = GetComponentPointer(index, &component_data);
					auto* last_pointer = GetComponentPointer(last_entity_index, &component_data);
					memcpy(remove_pointer, last_pointer, component_data.size);
				}

				// entity address index changes as well
				auto* entity = (EntityData*)GetComponentPointer(index, GetComponentHash<EntityData>());
				entity_address_callback(entity->id, EntityAddress{ this, index });
			}

			entity_count -= 1;
		}

	private:
		const ComponentLayout& layout;
		void* memory;
		std::unique_ptr<Chunk> next;
		uint32_t entity_count;
		EntityAddressChangedCallback entity_address_callback;
	};

	class ChunkList
	{
	public:
		typedef std::list<ChunkList*> List;

		ChunkList(const ComponentLayout& layout, EntityAddressChangedCallback entity_address_callback)
			: layout(layout)
			, first(std::make_unique<Chunk>(this->layout, entity_address_callback)) {};

		~ChunkList() = default;

		const ComponentLayout& GetLayout() const { return layout; }
		Chunk* GetFirstChunk() const { return first.get(); }

	private:
		ComponentLayout layout; // TODO: move layout into the chunk memory, offset components
		std::unique_ptr<Chunk> first;
	};

} }