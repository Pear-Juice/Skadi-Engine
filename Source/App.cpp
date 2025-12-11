#include "App.hpp"
#include "IdGen.hpp"
#include "Graphics/Rend.hpp"
#include "Graphics/Vertex.hpp"
#include "Resources/Loader.hpp"
#include "Input/Input.hpp"

#include <array>
#include <cstdint>
#include <glm/fwd.hpp>
#include <iostream>
#include <ostream>
#include <string>
#include <strings.h>
#include <thread>
#include <chrono>

#include "Core/ECS/Scene.hpp"

void initRend(Rend &renderer) {
	renderer.initVulkan();
	renderer.beginLoop();
}

void printMatrix(glm::mat4 mat) {
	glm::vec4 col0 = glm::vec4(mat[0]);
	glm::vec4 col1 = glm::vec4(mat[1]);
	glm::vec4 col2 = glm::vec4(mat[2]);
	glm::vec4 col3 = glm::vec4(mat[3]);

	std::cout << "----------\n";
	std::cout << col0[0] << " " << col1[0] << " " << col2[0] << " " << col3[0] << "\n";
	std::cout << col0[1] << " " << col1[1] << " " << col2[1] << " " << col3[1] << "\n";
	std::cout << col0[2] << " " << col1[2] << " " << col2[2] << " " << col3[2] << "\n";
	std::cout << col0[3] << " " << col1[3] << " " << col2[3] << " " << col3[3] << "\n";
}

App::App(std::string projectDirectory) {
	std::chrono::time_point<std::chrono::steady_clock> start, end;

	start = std::chrono::steady_clock::now();
	std::cout << "Created app " << projectDirectory << '\n';

	Rend rend;

	initRend(rend);

	std::string modelPath = "/home/vi/Documents/Game-Engines/Skadi-Engine/Models/Scene.glb";
	Loader loader;
	auto [meshes, materials] = loader.loadModels(modelPath);

	for (Material material : materials) {
		rend.registerMaterial(material);
	}

	Scene scene(10);

	SparseSet<Mesh>* meshComponents = scene.componentManager.getComponents<Mesh>();
	for (auto& mesh : meshes) {
		Entity box = scene.entityManager.allocEntity();
		meshComponents->add(box, mesh);
	}

	scene.enter(rend);

	end = std::chrono::steady_clock::now();
	auto elapsed_millis = std::chrono::duration_cast<std::chrono::duration<float,std::milli>>(end-start);
	std::cout << "Init time millis: " << elapsed_millis.count() << "\n";

	Input input;
	input.addKeyMapping("Up", Input::KEY_SPACE);
	input.addKeyMapping("Down", Input::KEY_SHIFT);
	input.addKeyMapping("Left", {Input::KEY_A, Input::LEFT_ARROW});
	input.addKeyMapping("Right", {Input::KEY_D, Input::RIGHT_ARROW});
	input.addKeyMapping("Forward", {Input::KEY_W, Input::UP_ARROW});
	input.addKeyMapping("Backward", {Input::KEY_S, Input::DOWN_ARROW});
	input.addKeyMapping("RotLeft", Input::KEY_Q);
	input.addKeyMapping("RotRight", Input::KEY_E);
	input.addKeyMapping("Flip", Input::KEY_F);

	float flyspeed = 0.2f;
	float turnspeed = 0.02f;

	glm::mat4 camMat(1);

	int flip = 0;

	input.getKeyEvent("Flip").add([&flip](Input::KeyData keyData) mutable {
		if (keyData.pressState == Input::JUST_PRESSED)
			flip = !flip;
	});

	auto origin_time = std::chrono::steady_clock::now();

	while (true) {
		auto start = std::chrono::steady_clock::now();

		Vector2 planeAxis = input.getKeyAxis("Left", "Right", "Forward", "Backward");
		int vertAxis = input.getKeyAxis("Down","Up");
		int rotAxis = -input.getKeyAxis("RotLeft", "RotRight");

		Vector3 moveDir(planeAxis.x, vertAxis, planeAxis.y);

		camMat = translate(camMat, moveDir.glm() * flyspeed);
		camMat = rotate(camMat, rotAxis * turnspeed, glm::vec3(0,1,0));

		if (flip) {
			rend.updateMeshTransform(meshes[0].id, camMat);
		}
		else {
			rend.camera.setTransform(camMat);
		}

		auto end = std::chrono::steady_clock::now();

		auto total_elapsed_millis = std::chrono::duration_cast<std::chrono::duration<float,std::milli>>(end-start).count();

		//DO not but any code after this line
		if (total_elapsed_millis < 16) {
			std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(16 - total_elapsed_millis)));
		}
	}

	// inputThread.detach();

	rend.renderThread.join();
}
