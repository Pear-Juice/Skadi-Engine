#include "App.hpp"
#include "IdGen.hpp"
#include "Graphics/Rend.hpp"
#include "Graphics/Vertex.hpp"
#include "Resources/Loader.hpp"
#include "Resources/Model.hpp"
#include "Source/Resources/Mesh.hpp"

#include <array>
#include <cstdint>
#include <glm/fwd.hpp>
#include <iostream>
#include <ostream>
#include <string>
#include <strings.h>
#include <thread>
#include <chrono>

void initRend() {
	std::string modelPath = "/home/blankitte/Documents/Engines/SkadiEngine/Models/model.glb";

	Rend renderer;
	renderer.initVulkan({});

	Loader loader;

	std::vector<Model> models = loader.loadModels(modelPath);

	for (Model model : models) {
		renderer.registerMaterial(model.material);
	}

	std::thread rendererThread(&Rend::beginLoop, &renderer);

	for (auto& model : models) {
		std::cout << "Render name: " << model.name << "\n";
		renderer.renderModel(model);
	}

	rendererThread.join();

	//try {
//		m
  //	} catch (const std::exception &e) {
    //	std::cerr << e.what() << std::endl;
	//}

}

App::App(std::string projectDirectory) {
	std::cout << "Created app " << projectDirectory << '\n';

	initRend();
}
