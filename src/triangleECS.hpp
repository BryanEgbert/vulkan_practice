#pragma once

#include <iostream>
#include <vector>
#include <bitset>
#include <unordered_map>
#include <optional>
#include <algorithm>

namespace triangle
{
	using EntityID = uint32_t;
	using ComponentID = uint32_t;

	static constexpr ComponentID maxComponents = 50;

	static EntityID g_EntityID = 0;
	static ComponentID g_ComponentID = 0;
	struct Entity
	{
		Entity() : id{++g_EntityID}
		{};

		EntityID id;

		bool operator==(const Entity &other)
		{
			return this->id == other.id;
		}
	};

	template <typename T>
	static std::unordered_map<ComponentID, T> g_Components;

	class ECS
	{
	public:
		std::vector<Entity> getEntities() { return m_Entities; }
		uint32_t getEntitySize() { return g_EntityID; }
		bool deleteEntity(Entity& a_Entity)
		{
			if (a_Entity.id == 0)
				return false;

			auto entityIndex = std::find(m_Entities.begin(), m_Entities.end(), a_Entity);
			m_Entities.erase(entityIndex);

			a_Entity.id = 0;

			return true;
		}

		void addEntity(Entity& a_Entity)
		{
			m_Entities.emplace_back(a_Entity);
		}

		template <typename T>
		bool assignComponent(Entity& a_Entity, const T &a_Component)
		{
			if (a_Entity.id == 0)
				return false;

			for (auto &entity : m_Entities)
			{
				if (a_Entity == entity)
				{
					g_Components<T>.insert({a_Entity.id, a_Component});

					return true;
				}
			}

			return false;
		}

		template <typename T>
		T* getComponent(const Entity& a_Entity)
		{
			if (a_Entity.id == 0)
				return nullptr;

			auto search = g_Components<T>.find(a_Entity.id);
			if (search == g_Components<T>.end())
				return nullptr;
			else
				return &(search->second);
		}

		template <typename T>
		bool deleteComponent(Entity& a_Entity)
		{
			if (a_Entity.id == 0)
				return false;

			g_Components<T>.erase(a_Entity.id);
			
			return true;
		}
	private:
		std::vector<Entity> m_Entities;
	};
}
