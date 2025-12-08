#pragma once

#include <stdexcept>

class GlfwWindowFailException : public std::runtime_error {
public:
	GlfwWindowFailException() : std::runtime_error("Failed to create GLFW window.") { }
	GlfwWindowFailException(std::string message) : std::runtime_error("Failed to create GLFW window: " + message) { }
};

class GlfwInitFailException : public std::runtime_error {
public:
	GlfwInitFailException() : std::runtime_error("Failed to initialize GLFW.") { }
};
