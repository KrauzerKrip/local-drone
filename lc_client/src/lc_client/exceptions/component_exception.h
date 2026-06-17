#pragma once

#include <format>
#include <stdexcept>
#include <string>


class ComponentNotFoundException : public std::runtime_error {
public:
	ComponentNotFoundException(std::string component)
		: std::runtime_error(std::format("Component {} not found", component)) {}
};
