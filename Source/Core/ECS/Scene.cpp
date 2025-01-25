#include "Scene.hpp"

#include <Source/Graphics/Rend.hpp>

#include "Source/Resources/Mesh.hpp"

void Scene::enter(Rend &renderer) {
	componentManager.operate<Mesh>([&renderer](auto data) {
		renderer.renderMesh(data);
	});
}

void Scene::exit(Rend &renderer) {
	componentManager.operate<Mesh>([&renderer](auto data) {
		renderer.eraseMesh(data.id);
	});
}

