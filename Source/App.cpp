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

void initRend(Rend &renderer) {
	renderer.initVulkan();
	renderer.beginLoop();

	// try {
	//
 //  	} catch (const std::exception &e) {
 //    	std::cerr << e.what() << std::endl;
	// }

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

	std::string modelPath = "/home/blankitte/Documents/Engines/SkadiEngine/Models/Cube.glb";
	Loader loader;
	auto [meshes, materials] = loader.loadModels(modelPath);

	for (Material material : materials) {
		rend.registerMaterial(material);
	}

	for (auto& mesh : meshes) {
	 	rend.renderMesh(mesh);
	    std::cout << "Render " << mesh.id << "\n";
	}

	end = std::chrono::steady_clock::now();
	auto elapsed_millis = std::chrono::duration_cast<std::chrono::duration<float,std::milli>>(end-start);
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::cout << "Init time: " << elapsed_seconds.count() << "\n";
	std::cout << "Init time millis: " << elapsed_millis.count() << "\n";

	Input input;
	input.addAction("Up", Input::KEY_SPACE);
	input.addAction("Down", Input::KEY_SHIFT);
	input.addAction("Left", {Input::KEY_A, Input::LEFT_ARROW});
	input.addAction("Right", {Input::KEY_D, Input::RIGHT_ARROW});
	input.addAction("Forward", {Input::KEY_W, Input::UP_ARROW});
	input.addAction("Backward", {Input::KEY_S, Input::DOWN_ARROW});
	input.addAction("MoveLeft", Input::KEY_Q);
	input.addAction("MoveRight", Input::KEY_E);

	auto upAction = input.getAction("Up");
	auto downAction = input.getAction("Down");
	auto forwardAction = input.getAction("Forward");
	auto backAction = input.getAction("Backward");
	auto leftAction = input.getAction("Left");
	auto rightAction = input.getAction("Right");
	auto moveLeftAction = input.getAction("MoveLeft");
	auto moveRightAction = input.getAction("MoveRight");

	float flyspeed = 0.2f;
	float turnspeed = 0.02f;

	glm::mat4 camRotMat(1);
	glm::mat4 camPosMat(1);

	while (true) {
		auto start = std::chrono::steady_clock::now();

		glm::vec3 moveDir(0);
		int rotateDir = 0;
		if (leftAction->mapping.data.pressState == Input::PRESSED || leftAction->mapping.data.pressState == Input::HELD) {
			rotateDir = -1;
		}

		if (rightAction->mapping.data.pressState == Input::PRESSED || rightAction->mapping.data.pressState == Input::HELD) {
			rotateDir = 1;
		}

		if (upAction->mapping.data.pressState == Input::PRESSED || upAction->mapping.data.pressState == Input::HELD) {
			moveDir =  glm::vec3(0,-flyspeed,0);
		}

		if (downAction->mapping.data.pressState == Input::PRESSED || downAction->mapping.data.pressState == Input::HELD) {
			moveDir = glm::vec3(0,flyspeed,0);
		}

		if (forwardAction->mapping.data.pressState == Input::PRESSED || forwardAction->mapping.data.pressState == Input::HELD) {
			moveDir = glm::vec3(0,-0,flyspeed);
		}

		if (backAction->mapping.data.pressState == Input::PRESSED || backAction->mapping.data.pressState == Input::HELD) {
			moveDir =  glm::vec3(0,0,-flyspeed);
		}

		if (moveLeftAction->mapping.data.pressState == Input::PRESSED || moveLeftAction->mapping.data.pressState == Input::HELD) {
			moveDir = glm::vec3(flyspeed,00,0);
		}

		if (moveRightAction->mapping.data.pressState == Input::PRESSED || moveRightAction->mapping.data.pressState == Input::HELD) {
			moveDir =  glm::vec3(-flyspeed,0,0);
		}

		camPosMat = translate(camPosMat, -moveDir * flyspeed);
		camPosMat = rotate(camPosMat, -rotateDir * turnspeed, glm::vec3(0,1,0));

		// glm::mat4 newMat =  inverse(camPosMat);

		// rend.camera.setTransform(newMat);

		rend.updateMeshTransform(meshes[0].id, camPosMat);

		auto end = std::chrono::steady_clock::now();

		auto total_elapsed_millis = std::chrono::duration_cast<std::chrono::duration<float,std::milli>>(end-start).count();

		if (total_elapsed_millis < 16) {
			std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(16 - total_elapsed_millis)));
		}
	}

	// inputThread.detach();

	rend.renderThread.join();
}
