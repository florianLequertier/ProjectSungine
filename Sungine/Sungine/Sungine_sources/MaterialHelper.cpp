#include "MaterialHelper.h"

////////////////////////////////////////////////////
/////BEGIN : Helpers
namespace MaterialHelper {

	GLuint getUniform(GLuint programId, const std::string& uniformName)
	{
		return glGetUniformLocation(programId, uniformName.data());
	}

	std::vector<GLuint> getUniforms(GLuint programId, const std::string& uniformName, int count)
	{
		std::vector<GLuint> outUniformIds;
		std::string name = uniformName + "[0]";;

		for (int i = 0; i < count; i++)
		{
			name[name.size() - 2] = std::to_string(i)[0];
			outUniformIds.push_back(glGetUniformLocation(programId, name.data()));
		}
		return outUniformIds;
	}

	GLuint getUniformStruct(GLuint programId, const std::string& uniformName, int index, const std::string& memberName)
	{
		std::string name = uniformName + "[" + std::to_string(index) + "]." + memberName;
		return glGetUniformLocation(programId, name.data());
	}

	GLuint findUniform(const std::string& uniformName, const std::vector<std::shared_ptr<ExternalShaderParameterBase>>& externalParameters)
	{
		auto& found = std::find_if(externalParameters.begin(), externalParameters.end(), [uniformName](const std::shared_ptr<ExternalShaderParameterBase>& item) { return item->getName() == uniformName; });
		assert(found != externalParameters.end());
		if (found != externalParameters.end())
			return (*found)->getUniformId();
		else
			return 0;
	}

	std::vector<GLuint> findUniforms(const std::string& uniformName, int count, const std::vector<std::shared_ptr<ExternalShaderParameterBase>>& externalParameters)
	{
		std::vector<GLuint> foundUniforms;


		auto& found = std::find_if(externalParameters.begin(), externalParameters.end(), [uniformName](const std::shared_ptr<ExternalShaderParameterBase>& item) { return item->getName() == uniformName; });
		assert(found != externalParameters.end());
		if (found != externalParameters.end())
			(*found)->getUniformIds(foundUniforms);

		return foundUniforms;
	}

}

/////END : Helpers
////////////////////////////////////////////////////
