#include <iostream>
#include <vector>
#include <bitset>
#include <unordered_map>
#include <optional>

namespace triangle
{
	using EntityID = uint32_t;
	using ComponentID = uint32_t;

	static constexpr ComponentID maxComponents = 50;

	static EntityID g_EntityID = 0;
	static ComponentID g_ComponentID = 0;
	struct Entity
	{
		Entity();

		EntityID id;
		std::bitset<maxComponents> componentMask;

		bool operator==(const Entity &other);
	};

	template <typename T>
	static std::unordered_map<ComponentID, T> g_Components;

	class ECS
	{
	public:
		bool deleteEntity(Entity& a_Entity);
		void addEntity(Entity& a_Entity);
		template <typename T>
		bool assignComponent(Entity& a_Entity, const T &a_Component);
		template <typename T>
		std::optional<std::reference_wrapper<T>> getComponent(Entity& a_Entity);
		template <typename T>
		bool deleteComponent(Entity& a_Entity);
	private:
		std::vector<Entity> g_Entities;
	};
}
