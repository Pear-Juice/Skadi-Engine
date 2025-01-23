#ifndef ENTITYMANAGER_HPP
#define ENTITYMANAGER_HPP

#include <bitset>
#include <cstdint>
#include <queue>
#include <valarray>

#include "Types.hpp"

class EntityManager {
	const uint32_t maxEntities = 0;
	const uint16_t maxComponents = 0;
	uint32_t numEntities = 0;
	std::queue<Entity> freeEntities;

	SparseSet<Signature> activeEntities;
public:

	EntityManager(uint32_t maxEntities);

	Entity allocEntity();
	std::vector<Entity> getEntities(Signature signature, uint16_t threadMax = 8) const;
	void freeEntity(Entity entity);
	uint32_t getNumEntities() const;

	void setEntitySignature(Entity entity, Signature signature);
	Signature getEntitySignature(Entity entity);

	~EntityManager();
};



#endif //ENTITYMANAGER_HPP
