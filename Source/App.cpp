#include "App.hpp"
#include "IdGen.hpp"
#include "Graphics/Rend.hpp"
#include "Graphics/Vertex.hpp"
#include "Resources/Loader.hpp"
#include "Resources/Model.hpp"
#include "Source/Resources/Mesh.hpp"
#include "Source/Input/Input.hpp"

#include <array>
#include <cstdint>
#include <glm/fwd.hpp>
#include <iostream>
#include <ostream>
#include <string>
#include <strings.h>
#include <thread>
#include <chrono>

Input initInput() {
	Input input;
	input.addAction("Jump", Input::KEY_SPACE);
	input.addAction("Jump", Input::UP_ARROW);
	input.addMouseAction("Shoot", Input::MOUSE_1);
	input.addMouseAction("Move", Input::MOUSE_MOVE);

	input.pushKeyCallback("Jump", [](Input::KeyData keyData) {
		std::cout << keyData.actionName << "\n";
	});

	input.pushMouseCallback("Shoot", [](auto mouseData) {
		std::cout << "Shoot " << Input::pressStateToString(mouseData.pressState) << "\n";
	});

	input.pushMouseCallback("Move", [](auto mouseData) {
		std::cout << "Move: " << mouseData.xPos << " " << mouseData.yPos << "\n";
	});

	return input;
}

void initRend() {
	std::string modelPath = "/home/blankitte/Documents/Engines/SkadiEngine/Models/model.glb";


	Rend renderer;
	renderer.initVulkan();

	Loader loader;

	// const auto [models, materials] = loader.loadModels(modelPath);
	//
	// for (Material material : materials) {
	// 	renderer.registerMaterial(material);
	// }

	std::thread rendererThread(&Rend::beginLoop, &renderer);

	// for (auto& model : models) {
	// 	std::cout << "Render name: " << model.name << "\n";
	// 	renderer.renderModel(model);
	// }

	rendererThread.join();

	//try {
//		m
  //	} catch (const std::exception &e) {
    //	std::cerr << e.what() << std::endl;
	//}

}

App::App(std::string projectDirectory) {
	std::cout << "Created app " << projectDirectory << '\n';

	initInput();
	initRend();
}
