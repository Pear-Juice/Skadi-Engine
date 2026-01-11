#include "EntityManager.hpp"

#include <iostream>

EntityManager::EntityManager(uint32_t maxEntities) : maxEntities(maxEntities), activeEntities(maxEntities) {
	for (Entity entity = 0; entity < maxEntities; ++entity) {
		freeEntities.push(entity);
	}
}

uint32_t EntityManager::getNumEntities() const {
	return numEntities;
}

Entity EntityManager::allocEntity() {
	if (freeEntities.empty()) throw std::runtime_error("Cannot get entity, all entities are in use");

	const Entity entity = freeEntities.front();
	freeEntities.pop();
	numEntities++;

	activeEntities.add(entity, Signature());
	return entity;
}

std::vector<Entity> filterEntities(SparseSet<Signature>::DenseElement *elements, uint32_t begin, uint32_t end, Signature filter) {
	std::vector<Entity> entities;
	for (uint32_t i = begin; i < end; ++i) {
		if ((elements[i].val & filter) == filter) {
			entities.push_back(elements[i].sparseID);
		}
	}

	return entities;
}

std::vector<Entity> EntityManager::getEntities(Signature signature, const uint16_t threadMax) const {
	return filterEntities(activeEntities.dense, 0 , activeEntities.size(), signature);


	// return matchingEntities;
}



void EntityManager::freeEntity(Entity entity) {
	if (numEntities <= 0) throw std::runtime_error("Cannot return Entity, all entities are returned");

	numEntities--;

	freeEntities.push(entity);
	activeEntities.del(entity);
}

Signature EntityManager::getEntitySignature(Entity entity) {
	if (entity > maxEntities) throw std::runtime_error("Entity out of range");
	return activeEntities.get(entity);
}

void EntityManager::setEntitySignature(Entity entity, Signature signature) {
	if (entity > maxEntities) throw std::runtime_error("Entity out of range");
	if (!activeEntities.contains(entity)) return;

	activeEntities.set(entity, signature);
}

EntityManager::~EntityManager() {
}