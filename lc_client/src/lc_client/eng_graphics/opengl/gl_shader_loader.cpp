#include "gl_shader_loader.h"

#include <iostream>

#include <glad/glad.h>

#include "lc_client/tier0/log.h"

void ShaderLoaderGl::loadShaders(entt::registry* pRegistry, entt::entity entity, const std::string vertexShaderName,
							   const std::string fragmentShaderName) {

	pRegistry->emplace_or_replace<ShaderGl>(entity, (int) createShaderProgram(vertexShaderName, fragmentShaderName));
}

unsigned int ShaderLoaderGl::createShaderProgram(std::string vertexShaderName, std::string fragmentShaderName) {
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();

	try {
		glAttachShader(shaderProgram, m_pShaderManager->getFragmentShader(fragmentShaderName));
	}
	catch (const std::out_of_range& exception) {
		LE_CORE_WARN("can't find fragment shader '{}' for program", fragmentShaderName);
	}
	try {
		glAttachShader(shaderProgram, m_pShaderManager->getVertexShader(vertexShaderName));
	}
	catch (const std::out_of_range& exception) {
	    LE_CORE_WARN("can't find vertex shader '{}' for program", vertexShaderName);
	}

	glLinkProgram(shaderProgram);

	int success;
	char infoLog[512];
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

	if (success == GL_TRUE) {
		LE_CORE_DEBUG("gl_shader_loader: shader program linked successfully: {}", shaderProgram);
	}
	else {
		glGetProgramInfoLog(shaderProgram, 512, 0, infoLog);
		LE_CORE_ERROR("gl_shader_loader: shader program link failure: \n{}", infoLog);
	}

	return shaderProgram;
}

void ShaderLoaderGl::setShaderManager(ShaderManagerGl* pShaderManager) { m_pShaderManager = pShaderManager; }
