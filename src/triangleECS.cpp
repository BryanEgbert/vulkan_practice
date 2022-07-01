#include <algorithm>
#include "triangleECS.hpp"

triangle::Entity::Entity() : id{++g_EntityID}, componentMask { std::bitset<maxComponents>() } {
	std::cout << "construct new entity\n";
}

bool triangle::Entity::operator==(const triangle::Entity& other)
{
	return this->id == other.id;
}

void triangle::ECS::addEntity(Entity &a_Entity)
{
	g_Entities.emplace_back(a_Entity);
}

bool triangle::ECS::deleteEntity(Entity& a_Entity)
{
	if (a_Entity.id == 0)
		return false;

	auto entityIndex = std::find(g_Entities.begin(), g_Entities.end(), a_Entity);
	g_Entities.erase(entityIndex);

	a_Entity.id = 0;
	a_Entity.componentMask.reset();

	return true;
}

template <typename T>
bool triangle::ECS::assignComponent(Entity& a_Entity, const T& a_Component)
{
	if (a_Entity.id == 0)
		return false;
	
	for (auto& entity : g_Entities)
	{
		if (a_Entity == entity)
		{
			ComponentID compID = ++ g_ComponentID;

			g_Components<T>.insert({compID, a_Component});

			entity.componentMask.set(compID);
			a_Entity.componentMask.set(compID);

			return true;
		}
	}

	return false;
}

template <typename T>
std::optional<std::reference_wrapper<T>> triangle::ECS::getComponent(Entity &a_Entity)
{
	if(a_Entity.id == 0)
		return std::nullopt;
	
	for (const auto& component : g_Components<T>)
	{
		if (a_Entity.componentMask.test(component.first))
			return std::optional<std::reference_wrapper<T>>{g_Components<T>.at(component.first)};
		else
			continue;
	}
}

template <typename T>
bool triangle::ECS::deleteComponent(Entity& a_Entity)
{
	if (a_Entity.id == 0)
		return false;

	for (auto &component : g_Components<T>)
	{
		if (a_Entity.componentMask.test(component.first))
		{
			g_Components<T>.erase(component.first);
			return true;
		}
	}
}