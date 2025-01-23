//
// Created by blankitte on 1/1/25.
//

#ifndef COMPONENTMANAGER_HPP
#define COMPONENTMANAGER_HPP

#include <bitset>
#include <cstdint>
#include <functional>
#include <optional>
#include <queue>
#include <variant>
#include <Source/Core/Messaging/Lambda.hpp>

#include "Types.hpp"
#include "Source/Core/DataStorage/SparseSet.hpp"

class ComponentManager {
public:
	std::queue<ComponentType> freeComponentTypes;
	std::unordered_map<std::string, ComponentType> typeMap;

	SparseSet<char*> componentData;

	uint32_t maxEntities;

	ComponentManager(const uint32_t maxEntities, const uint32_t maxComponents) : componentData(maxComponents), maxEntities(maxEntities) {
		for (int i = 0; i < maxComponents; i++) {
			freeComponentTypes.push(i);
		}
	}

	~ComponentManager() {
		for (int i = 0; i < componentData.size(); i++) {
			delete[] componentData.dense;
			componentData.dense = nullptr;
		}
	}

	template <typename T>
	void registerComponentType() {
		std::string typeName = typeid(T).name();
		if (typeMap.contains(typeName)) return;

		ComponentType componentType = freeComponentTypes.front();
		freeComponentTypes.pop();

		typeMap[typeName] = componentType;

		auto data = new SparseSet<T>(maxEntities);
		componentData.add(componentType, reinterpret_cast<char*>(data));
	}

	template<typename T>
	std::optional<ComponentType> getComponentType() {
		std::string typeName = typeid(T).name();
		if (!typeMap.contains(typeName))
			return {};

		return typeMap[typeName];
	}

	template<typename T>
	SparseSet<T>* getComponents() {
		std::optional<ComponentType> type = getComponentType<T>();
		if (!type) return nullptr;

		char* data = componentData.get(type.value());
		return reinterpret_cast<SparseSet<T>*>(data);
	}

	template<typename T>
	void operate(std::vector<Entity> entities, std::function<void(T& data)> func) {
		SparseSet<T>* set = getComponents<T>();
		if (set == nullptr) return;

		for (const Entity entity : entities) {
			func(set->get(entity));
		}
	}

	template<typename T>
	void operate(std::function<void(T &data)> func) {
		SparseSet<T>* set = getComponents<T>();
		if (set == nullptr) return;

		for (uint32_t i = 0; i < set->size(); ++i) {
			func(set->dense[i].val);
		}
	}

	template<typename T>
	void operate(std::function<void(T &data)> func, std::function<bool(const T& data)> filter) {
		SparseSet<T>* set = getComponents<T>();
		if (set == nullptr) return;

		for (uint32_t i = 0; i < set->size(); ++i) {
			if (T& data = set->dense[i].val; filter(data))
				func(data);
		}
	}

	template<typename T>
	void operate(std::function<void(Entity entity, T &data)> func) {
        SparseSet<T>* set = getComponents<T>();
		if (set == nullptr) return;

		for (uint32_t i = 0; i < set->size(); ++i) {
			auto& element = set->dense[i];
			func(element.sparseID, element.val);
		}
	}

    template<typename T>
	void operate(std::function<void(Entity entity, T &data)> func, std::function<bool(Entity entity, const T& data)> filter) {
        SparseSet<T>* set = getComponents<T>();
		if (set == nullptr) return;

		for (uint32_t i = 0; i < set->size(); ++i) {
			auto element = set->dense[i];
			auto entity = element.sparseID;
			auto& value = element.val;

			if (filter(entity, value))
				func(entity,value);
		}
	}

	template <typename T>
	void unregisterComponents() {
		std::optional<ComponentType> type = getComponentType<T>();
		if (!type) return;

		delete[] componentData.get(type.value());
		componentData.del(type.value());

		typeMap.erase(typeid(T).name());
	}

	static Signature componentTypesToSignature(std::vector<ComponentType> types) {
		Signature signature;

		Signature flag;
		for (const auto& type : types) {
			flag = 1;
			flag = flag << type;
			signature = signature | flag;
		}

		return signature;
	}

	static std::vector<ComponentType> signatureToComponentTypes(Signature signature) {
		std::vector<ComponentType> componentTypes;
		componentTypes.reserve(signature.count());

		Signature mask = 1;
		for (uint32_t i = 0; i < COMPONENT_COUNT; ++i) {
			if ((mask & signature).any())
				componentTypes.push_back(i);

			mask = mask << 1;
		}

		return componentTypes;
	}
};



#endif //COMPONENTMANAGER_HPP
