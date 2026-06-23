#include "main.h"

#include <cstdio>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <filesystem>

#include "game.h"
#include "lc_client/loop.h"
#include "lc_client/eng_graphics/i_window.h"
#include "lc_client/eng_graphics/opengl/gl_window.h"
#include "lc_client/tier0/tier0.h"
#include "lc_client/tier0/log.h"

namespace fs = std::filesystem;

void printException(const std::exception& e, int level = 0) {
	LE_GAME_CRITICAL("{}Exception: {}", std::string(level, ' '), e.what());
	try {
		std::rethrow_if_nested(e);
	}
	catch (const std::exception& nestedException) {
		printException(nestedException, level + 1);
	}
	catch (...) {
	}
}

std::optional<fs::path> resolveResourceDir(int argc, char** argv) {
	// 1) Check CLI args: --res-dir=...
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		std::string prefix = "--res-dir=";
		if (arg.rfind(prefix, 0) == 0) { // starts with
			return fs::absolute(arg.substr(prefix.size()));
		}
	}

	// 2) Check env var
	if (const char* env = std::getenv("GAME_RES_DIR")) {
		return fs::absolute(env);
	}

	return {};
}

int main(int argc, char** argv) {
	std::string title = "Local` Engine";
	// int width = 1080;
	// int height = 720;
	int width = 1920;
	int height = 1080;
	bool vSync = true;

	int targetFPS = 60;
	int targetUPS = 60;

	Tier0* pTier0 = nullptr;

	IWindow* pWindow = nullptr;
	IGameLogic* pGameLogic = nullptr;
	Loop* pLoop = nullptr;
	const auto maybeResourceDir = resolveResourceDir(argc, argv);
	if (!maybeResourceDir.has_value()) {
		throw std::runtime_error("resource directory is not specified");
	}
	const auto resourceDir = maybeResourceDir.value();
	// try {
	pTier0 = new Tier0(resourceDir);

	pWindow = new WindowGL(title, width, height, new int[2]{16, 9});
	pGameLogic = new Game(pWindow, pTier0, resourceDir);
	pLoop = Loop::createInstance(pWindow, pGameLogic, targetFPS, targetUPS);

	pLoop->init();
	pLoop->startLoop();

	pLoop->cleanUp();
	//}
	// catch (std::runtime_error& exception) {
	//	printException(exception);
	//	//abort();
	//	exit(-1);
	//}

	if (pLoop) {
		delete pLoop;
	}

	if (pWindow) {
		delete pWindow;
	}
	if (pGameLogic) {
		delete pGameLogic;
	}
	if (pTier0) {
		delete pTier0;
	}

	exit(0);
}
// #include <iostream>

// #include <glad/glad.h>   // glad must be included before glfw3.h
// #include <GLFW/glfw3.h>

// void framebuffer_size_callback(GLFWwindow* window, int width, int height)
// {
//     glViewport(0, 0, width, height);
// }

// int main()
// {
//     glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
//     // Init GLFW
//     if (!glfwInit())
//     {
//         std::cerr << "Failed to initialize GLFW\n";
//         return -1;
//     }

//     // Ask for OpenGL 3.3 Core (change if you need other version)
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//     // For macOS you’d also need:
//     // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

//     GLFWwindow* window = glfwCreateWindow(800, 600, "GLFW + GLAD Test", nullptr, nullptr);
//     if (!window)
//     {
//         std::cerr << "Failed to create GLFW window\n";
//         glfwTerminate();
//         return -1;
//     }

//     glfwMakeContextCurrent(window);

//     // Load OpenGL functions with GLAD
//     if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//     {
//         std::cerr << "Failed to initialize GLAD\n";
//         glfwDestroyWindow(window);
//         glfwTerminate();
//         return -1;
//     }

//     glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

//     // Set initial viewport
//     int width, height;
//     glfwGetFramebufferSize(window, &width, &height);
//     glViewport(0, 0, width, height);

//     // Main loop
//     while (!glfwWindowShouldClose(window))
//     {
//         // Handle input (Esc to close)
//         if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//             glfwSetWindowShouldClose(window, true);

//         // Render
//         glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
//         glClear(GL_COLOR_BUFFER_BIT);

//         // Swap buffers & poll events
//         glfwSwapBuffers(window);
//         glfwPollEvents();
//     }

//     glfwDestroyWindow(window);
//     glfwTerminate();
//     return 0;
// }
