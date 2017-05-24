#pragma once

#include <vector>
#include <memory>
#include "glew/glew.h"
#include "glm/common.hpp"
#include "ShaderParameters.h"

//Helpers :
namespace MaterialHelper {

	GLuint getUniform(GLuint programId, const std::string& uniformName);
	std::vector<GLuint> getUniforms(GLuint programId, const std::string& uniformName, int count);
	GLuint getUniformStruct(GLuint programId, const std::string& uniformName, int index, const std::string& memberName);
	GLuint findUniform(const std::string& uniformName, const std::vector<std::shared_ptr<ExternalShaderParameterBase>>& externalParameters);
	std::vector<GLuint> findUniforms(const std::string& uniformName, int count, const std::vector<std::shared_ptr<ExternalShaderParameterBase>>& externalParameters);

}