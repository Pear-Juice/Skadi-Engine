#ifndef SCENE_HPP
#define SCENE_HPP

#include <Source/Graphics/Rend.hpp>

#include "Definitions.hpp"
#include "ComponentManager.hpp"
#include "EntityManager.hpp"

class Scene {


	const uint32_t MAX_ENTITIES;

public:
	EntityManager entityManager;
	ComponentManager componentManager;

	explicit Scene(uint32_t maxEntities) : entityManager(maxEntities), componentManager({maxEntities}), MAX_ENTITIES(maxEntities){}

	void enter(Rend &renderer);
	void exit(Rend &renderer);
	///Rendering should be handled by the renderer
	///Object data should only be sent to the renderer when they are changed
	///
	///The engine should handle rendering
	///Scene load function which run through all render components and registers them
};

#endif //SCENE_HPP
