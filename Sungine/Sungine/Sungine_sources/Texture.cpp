

#include "Texture.h"
#include "Utils.h"
#include "ErrorHandler.h"
#include "Project.h"
#include "EditorGUI.h"

#include "jsoncpp/json/json.h"

Texture::Texture(int width, int height, bool useAlpha)
	: glId(0)
	, type(GL_UNSIGNED_BYTE)
	, generateMipMap(true)
	, m_textureUseCounts(0)
	, comp(3)
	, pixels(nullptr)
	, w(width)
	, h(height)
	, textureWrapping_u(GL_REPEAT)
	, textureWrapping_v(GL_REPEAT)
	, minFilter(GL_LINEAR)
	, magFilter(GL_LINEAR)
{
	if (!useAlpha)
	{
		internalFormat = GL_RGB;
		format = GL_RGB;
		comp = 3;
	}
	else
	{
		internalFormat = GL_RGBA;
		format = GL_RGBA;
		comp = 4;
	}

	//pixels = new unsigned char[3*width*height];
	//for (int i = 0; i < 3*width*height; i++) {
	//	pixels[i] = 255;
	//}
}

Texture::Texture(unsigned char * _pixels, int width, int height, bool useAlpha, int _comp) 
	: Texture(width, height, useAlpha)
{
	comp = _comp;
	pixels = _pixels;
}

Texture::Texture(const glm::vec4& color)
	: Texture(1, 1, color)
{

}

Texture::Texture(const glm::vec3& color)
	: Texture(1, 1, color)
{

}

Texture::Texture(int width, int height, const glm::vec4& color) 
	: Texture(width, height, true)
{
	comp = 4;
	pixels = new unsigned char[4*width*height];
	for (int i = 0; i < width * height * 4; i += 4)
	{
		pixels[i] = color.r;
		pixels[i + 1] = color.g;
		pixels[i + 2] = color.b;
		pixels[i + 3] = color.a;
	}
}

Texture::Texture(int width, int height, const glm::vec3& color) 
	: Texture(width, height, false)
{
	comp = 3;
	pixels = new unsigned char[3 * width*height];
	for (int i = 0; i < width * height * 3; i += 3)
	{
		pixels[i] = color.r;
		pixels[i + 1] = color.g;
		pixels[i + 2] = color.b;
	}
}

Texture::Texture(const FileHandler::CompletePath& _path, AlphaMode alphaMode)
	: Resource(_path)
	, glId(0)
	, generateMipMap(true)
	, m_textureUseCounts(0)
	, textureWrapping_u(GL_REPEAT)
	, textureWrapping_v(GL_REPEAT)
	, minFilter(GL_LINEAR)
	, magFilter(GL_LINEAR)
{
	if (alphaMode == AlphaMode::WITHOUT_ALPHA)
	{
		internalFormat = GL_RGB;
		format = GL_RGB;
		type = GL_UNSIGNED_BYTE;
		comp = 3;
		int texComp;
		pixels = stbi_load(_path.c_str(), &w, &h, &texComp, comp);
		assert(pixels != nullptr);
	}
	else if(alphaMode == AlphaMode::WITH_ALPHA)
	{
		internalFormat = GL_RGBA;
		format = GL_RGBA;
		type = GL_UNSIGNED_BYTE;
		comp = 4;
		int texComp;
		pixels = stbi_load(_path.c_str(), &w, &h, &texComp, comp);
		assert(pixels != nullptr);
	}
	else
	{
		type = GL_UNSIGNED_BYTE;
		int texComp;
		pixels = stbi_load(_path.c_str(), &w, &h, &texComp, 0);
		assert(pixels != nullptr);
		comp = texComp;
		if (comp == 4)
		{
			internalFormat = GL_RGBA;
			format = GL_RGBA;
		}
		else if (comp == 3)
		{
			internalFormat = GL_RGB;
			format = GL_RGB;
		}
		else if (comp == 1)
		{
			internalFormat = GL_RED;
			format = GL_RED;
		}
		else
		{
			ASSERT(false, "You are initalizing a texture wing wrong number of component. Number of component supported : 1, 3 or4");
		}
	}
}

void Texture::init(const FileHandler::CompletePath& path, const ID& id)
{
	Resource::init(path, id);

	assert(!Project::isPathPointingInsideProjectFolder(path));
	FileHandler::CompletePath absolutePath = Project::getAbsolutePathFromRelativePath(path);

	type = GL_UNSIGNED_BYTE;
	int texComp;
	pixels = stbi_load(absolutePath.c_str(), &w, &h, &texComp, 0);
	CHECK_STBI_ERROR("Error when loading texture : %s", absolutePath.c_str());
	//ErrorHandler::error("Error when loading texture : %d, %f", 10, 4.5);

	assert(pixels != nullptr);
	comp = texComp;
	if (comp == 4)
	{
		internalFormat = GL_RGBA;
		format = GL_RGBA;
	}
	else if (comp == 3)
	{
		internalFormat = GL_RGB;
		format = GL_RGB;
	}
	else if (comp == 1)
	{
		internalFormat = GL_RED;
		format = GL_RED;
	}
	else
	{
		ASSERT(false, "You are initalizing a texture wing wrong number of component. Number of component supported : 1, 3 or4");
	}

	initGL();
}

void Texture::save()
{
	// TODO : metadatas
	//assert(false && "metadatas for textures aren't yet implamented.");
	PRINT_WARNING("metadatas for textures aren't yet implamented.");
}

void Texture::resolvePointersLoading()
{
	// No pointers.
}

void Texture::drawInInspector(Scene & scene)
{
	Resource::drawInInspector(scene);

	ImGui::Image((void*)glId, ImVec2(w, h), ImVec2(0, 1), ImVec2(1, 0));
}

void Texture::drawIconeInResourceTree()
{
	ImGui::Image((void*)glId, ImVec2(30, 30), ImVec2(0, 1), ImVec2(1, 0));
}

void Texture::drawUIOnHovered()
{
	ImGui::BeginTooltip();
	ImGui::Text("Texture : \n Size = %d, %d. \n Comp = %d. \n GLID = %u.", w, h, comp, glId);
	ImGui::Image((void*)glId, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
	ImGui::EndTooltip();
}

void Texture::drawIconeInResourceField()
{
	ImGui::Image((void*)glId, ImVec2(40, 40), ImVec2(0, 1), ImVec2(1, 0));
}

Texture::~Texture()
{
	delete[] pixels;
	freeGL();
}

void Texture::setTextureParameters(GLint _internalFormat, GLenum _format, GLenum _type, bool _generateMipMap)
{
	internalFormat = _internalFormat;
	format = _format;
	type = _type;
	generateMipMap = _generateMipMap;
}

void Texture::setTextureMinMaxFilters(GLint _magFilter, GLint _minFilter)
{
	magFilter = _magFilter;
	minFilter = _minFilter;
}

void Texture::setTextureWrapping(GLint _uWrapping, GLint _vWrapping)
{
	textureWrapping_u = _uWrapping;
	textureWrapping_v = _vWrapping;
}

void Texture::setPixels(const std::vector<unsigned char>& _pixels, int _width, int _height, int _comp)
{
	if (pixels != nullptr)
		delete[] pixels;

	pixels = new unsigned char[_pixels.size()];
	for (int i = 0; i < _pixels.size(); i++)
	{
		pixels[i] = _pixels[i];
	}

	w = _width;
	h = _height;
	comp = _comp;
}

void Texture::setPixels(const std::vector<glm::vec3>& _pixels, int _width, int _height)
{
	if (pixels != nullptr)
		delete[] pixels;

	pixels = new unsigned char[_pixels.size()];
	for (int i = 0, j = 0; i < _pixels.size(); i+=3, j++)
	{
		for(int k = 0; k < 3; k++)
			pixels[i + k] = _pixels[j][k];
	}

	w = _width;
	h = _height;
	comp = 3;
}

void Texture::setPixels(const std::vector<glm::vec4>& _pixels, int _width, int _height)
{
	if (pixels != nullptr)
		delete[] pixels;

	pixels = new unsigned char[_pixels.size()];
	for (int i = 0, j = 0; i < _pixels.size(); i += 4, j++)
	{
		for (int k = 0; k < 4; k++)
			pixels[i + k] = _pixels[j][k];
	}

	w = _width;
	h = _height;
	comp = 4;
}

void Texture::resizePixelArray(int width, int height, const glm::vec4& color)
{
	comp = 4;
	w = width;
	h = height;
	delete[] pixels;
	pixels = new unsigned char[comp * width*height];

	for (int i = 0; i < width * height * comp; i += comp)
	{
		for (int k = 0; k < comp; k++)
		{
			pixels[i + k] = color[k];
		}
	}
}

void Texture::resizePixelArray(int width, int height, const glm::vec3& color)
{
	comp = 4;
	w = width;
	h = height;
	delete[] pixels;
	pixels = new unsigned char[comp * width*height];

	for (int i = 0; i < width * height * comp; i += comp)
	{
		for (int k = 0; k < comp; k++)
		{
			pixels[i + k] = color[k];
		}
	}
}

void Texture::resizeTexture(int width, int height)
{
	w = width;
	h = height;
}

void Texture::initGL()
{

	//m_textureUseCounts++;

	//if (m_textureUseCounts == 1)
	if(glId <= 0)
	{
		glGenTextures(1, &glId);

		glBindTexture(GL_TEXTURE_2D, glId);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrapping_u);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrapping_v);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

		if (generateMipMap)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);

		CHECK_GL_ERROR("error in texture initialization.");
	}
}

void Texture::freeGL()
{
	//m_textureUseCounts--;

	//if (m_textureUseCounts <= 0)
	if(glId > 0)
	{
		glDeleteTextures(1, &glId);
		glId = 0;
	}
}

//////////////////////////////////////////

CubeTexture::CubeTexture()
	: CubeTexture(0, 0, 0)
{

}

CubeTexture::CubeTexture(const FileHandler::CompletePath& path)
	: Resource(path)
	, glId(0)
	, internalFormat(GL_RGB)
	, format(GL_RGB)
	, type(GL_UNSIGNED_BYTE)
	, generateMipMap(false)
	, m_textureUseCounts(0)
	, magFilter(GL_LINEAR)
	, minFilter(GL_LINEAR)
	, textureWrapping_s(GL_CLAMP_TO_EDGE)
	, textureWrapping_t(GL_CLAMP_TO_EDGE)
	, textureWrapping_r(GL_CLAMP_TO_EDGE)
	, m_isInitialized(false)
{
	comp = 3;
	w = 1;
	h = 1;

	for (int i = 0; i < 6; i++)
	{
		m_textures[i] = ResourcePtr<Texture>();
	}

	for (int i = 0; i < 6; i++)
	{
		pixels[i] = new unsigned char[3];
		pixels[i][0] = 0;
		pixels[i][1] = 0;
		pixels[i][2] = 0;
	}
}

CubeTexture::CubeTexture(char r, char g, char b) 
	: glId(0)
	, internalFormat(GL_RGB)
	, format(GL_RGB)
	, type(GL_UNSIGNED_BYTE)
	, generateMipMap(false)
	, m_textureUseCounts(0)
	, magFilter(GL_LINEAR)
	, minFilter(GL_LINEAR)
	, textureWrapping_s(GL_CLAMP_TO_EDGE)
	, textureWrapping_t(GL_CLAMP_TO_EDGE)
	, textureWrapping_r(GL_CLAMP_TO_EDGE)
	, m_isInitialized(false)
{
	comp = 3;
	w = 1;
	h = 1;

	for (int i = 0; i < 6; i++)
	{
		m_textures[i] = ResourcePtr<Texture>();
	}

	for (int i = 0; i < 6; i++)
	{
		pixels[i] = new unsigned char[3];
		pixels[i][0] = r;
		pixels[i][1] = g;
		pixels[i][2] = b;
	}
}

CubeTexture::CubeTexture(const std::vector<ResourcePtr<Texture>>& textures) 
	: glId(0)
	, internalFormat(GL_RGB)
	, format(GL_RGB)
	, type(GL_UNSIGNED_BYTE)
	, generateMipMap(true)
	, m_textureUseCounts(0)
	, magFilter(GL_LINEAR)
	, minFilter(GL_LINEAR)
	, textureWrapping_s(GL_CLAMP_TO_EDGE)
	, textureWrapping_t(GL_CLAMP_TO_EDGE)
	, textureWrapping_r(GL_CLAMP_TO_EDGE)
	, m_isInitialized(false)

{
	for (int i = 0; i < 6; i++)
		pixels[i] = nullptr;

	initFromTextures(textures);
}


CubeTexture::~CubeTexture()
{
	for (int i = 0; i < 6; i++)
	{
		delete[] pixels[i];
	}
	freeGL();
}

void CubeTexture::setTextureParameters(GLint _internalFormat, GLenum _format, GLenum _type, bool _generateMipMap)
{
	internalFormat = _internalFormat;
	format = _format;
	type = _type;
	generateMipMap = _generateMipMap;
}

void CubeTexture::setTextureMinMaxFilters(GLint _magFilter, GLint _minFilter)
{
	magFilter = _magFilter;
	minFilter = _minFilter;
}

void CubeTexture::setTextureWrapping(GLint _sWrapping, GLint _tWrapping, GLint _rWrapping)
{
	textureWrapping_s = _sWrapping;
	textureWrapping_t = _tWrapping;
	textureWrapping_r = _rWrapping;
}

void CubeTexture::resizePixelArrays(int width, int height, const glm::vec4& color)
{
	comp = 4;
	w = width;
	h = height;
	delete[] pixels;

	for (int j = 0; j < 6; j++)
	{
		delete[] pixels[j];
		pixels[j] = new unsigned char[comp*width*height];

		for (int i = 0; i < width * height * comp; i += comp)
		{
			pixels[j][i] = color.r;
			pixels[j][i + 1] = color.g;
			pixels[j][i + 2] = color.b;
			pixels[j][i + 3] = color.a;
		}
	}
}

void CubeTexture::resizePixelArrays(int width, int height, const glm::vec3& color)
{
	comp = 3;
	w = width;
	h = height;
	delete[] pixels;

	for (int j = 0; j < 6; j++)
	{
		delete[] pixels[j];
		pixels[j] = new unsigned char[comp*width*height];

		for (int i = 0; i < width * height * comp; i += comp)
		{
			pixels[j][i] = color.r;
			pixels[j][i + 1] = color.g;
			pixels[j][i + 2] = color.b;
		}
	}
}

void CubeTexture::resizeTexture(int width, int height)
{
	w = width;
	h = height;
}

void CubeTexture::setTextureWithoutCheck(int index, ResourcePtr<Texture> texture)
{
	// Check if given texture have been initialized
	if (!m_textures[index].isValid() || m_textures[index]->pixels == nullptr)
		return;

	w = texture->w;
	h = texture->h;
	comp = texture->comp;
	m_textures[index] = texture;
	if (pixels[index] != nullptr)
		delete[] pixels[index];
	pixels[index] = new unsigned char[w * h * comp];
	for (int j = 0; j < w*h*comp; j += comp)
	{
		for (int k = 0; k < comp; k++)
		{
			pixels[index][j + k] = m_textures[index]->pixels[j + k];
		}
	}
}

bool CubeTexture::setTexture(int index, ResourcePtr<Texture> texture)
{
	if (!checkTextureCanBeAdded(texture))
	{
		m_textures[index].reset();
		return false;
	}
	else
	{
		setTextureWithoutCheck(index, texture);
		return true;
	}
}

bool CubeTexture::initFromTextures(const std::vector<ResourcePtr<Texture>>& textures)
{
	assert(textures.size() > 0);

	bool sameSize = true;
	w = textures[0]->w;
	h = textures[0]->h;
	comp = textures[0]->comp;
	for (int i = 0; i < 6; i++)
	{
		if (!checkTextureCanBeAdded(textures[i]))
		{
			w = 0;
			h = 0;
			comp = 0;
			return false;
		}
	}

	for (int i = 0; i < 6; i++)
	{
		setTextureWithoutCheck(i, textures[i]);
	}
	m_isInitialized = true;

	return true;
}

bool CubeTexture::checkTextureCanBeAdded(const ResourcePtr<Texture>& textureToAdd) const
{
	if (!textureToAdd.isValid())
		return false;
	else if (textureToAdd->w != textureToAdd->h)
	{
		PRINT_ERROR("You are trying to create a cubeTexture with a textures with width != height.")
			return false;
	}
	else if (!m_isInitialized)
		return true;
	if (textureToAdd->w != w || textureToAdd->h != h)
	{
		PRINT_ERROR("You are trying to create a cubeTexture with textures with different size.")
			return false;
	}
	else if (textureToAdd->comp != comp)
	{
		PRINT_ERROR("You are trying to create a cubeTexture with textures with different format.")
			return false;
	}
	return true;
}

void CubeTexture::clearTextures()
{
	m_isInitialized = false;
	for (int i = 0; i < 6; i++)
	{
		m_textures[i].reset();
	}

	comp = 3;
	w = 1;
	h = 1;

	for (int i = 0; i < 6; i++)
	{
		if (pixels[i] != nullptr)
			delete pixels[i];

		pixels[i] = new unsigned char[3];
		pixels[i][0] = 255;
		pixels[i][1] = 255;
		pixels[i][2] = 255;
	}
}

void CubeTexture::initGL()
{
	if (glId <= 0){

		glGenTextures(1, &glId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, glId);

		for (int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, w, h, 0, format, type, pixels[i]);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, textureWrapping_s);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, textureWrapping_t);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, textureWrapping_r);
		

		if (generateMipMap)
		{
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}
		else
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		CHECK_GL_ERROR("error in texture initialization.");
	}
}

void CubeTexture::freeGL()
{
	if (glId > 0)
	{
		glDeleteTextures(1, &glId);
		glId = 0;
	}
}

void CubeTexture::init(const FileHandler::CompletePath & path, const ID& id)
{
	Resource::init(path, id);

	assert(!Project::isPathPointingInsideProjectFolder(path)); //path should be relative
	FileHandler::CompletePath absolutePath = Project::getAbsolutePathFromRelativePath(path);

	std::ifstream stream;
	stream.open(absolutePath.toString());
	if (!stream.is_open())
	{
		std::cout << "error, can't load cube texture at path : " << absolutePath.toString() << std::endl;
		return;
	}
	Json::Value root;
	stream >> root;

	load(root);

	initGL();
}

void CubeTexture::save()
{
	assert(!Project::isPathPointingInsideProjectFolder(m_completePath)); //path should be relative
	FileHandler::CompletePath absolutePath = Project::getAbsolutePathFromRelativePath(m_completePath);

	std::ofstream stream;
	stream.open(absolutePath.toString());
	if (!stream.is_open())
	{
		std::cout << "error, can't load material at path : " << absolutePath.toString() << std::endl;
		return;
	}
	Json::Value root;

	save(root);

	stream << root;
}

void CubeTexture::resolvePointersLoading()
{
	assert(!Project::isPathPointingInsideProjectFolder(m_completePath)); //path should be relative
	FileHandler::CompletePath absolutePath = Project::getAbsolutePathFromRelativePath(m_completePath);

	std::ifstream stream;
	stream.open(absolutePath.toString());
	if (!stream.is_open())
	{
		std::cout << "error, can't load material at path : " << absolutePath.toString() << std::endl;
		return;
	}
	Json::Value root;
	stream >> root;

	/////////////////

	bool allTexturesOk = true;
	for (int i = 0; i < 6; i++)
	{
		m_textures[i].load(root["textures"][i]);
		allTexturesOk &= checkTextureCanBeAdded(m_textures[i]);
	}
	if (allTexturesOk)
	{
		for (int i = 0; i < 6; i++)
		{
			setTextureWithoutCheck(i, m_textures[i]);
		}
	}
	else
	{
		for (int i = 0; i < 6; i++)
		{
			m_textures[i] = ResourcePtr<Texture>();
		}
		PRINT_ERROR("error in cubeTexture loading !")
	}
}

void CubeTexture::load(const Json::Value& root)
{
	/*bool allTexturesOk = true;
	for (int i = 0; i < 6; i++)
	{
		m_textures[i].load(root["textures"][i]);
		allTexturesOk &= checkTextureCanBeAdded(m_textures[i]);
	}
	if (allTexturesOk)
	{
		for (int i = 0; i < 6; i++)
		{
			setTextureWithoutCheck(i, m_textures[i]);
		}
	}
	else
	{
		for (int i = 0; i < 6; i++)
		{
			m_textures[i] = ResourcePtr<Texture>();
		}
		PRINT_ERROR("error in cubeTexture loading !")
	}*/

	internalFormat = root["internalFormat"].asInt();
	format = root["format"].asInt();
	type = root["type"].asInt();
	generateMipMap = root["generateMipMap"].asBool();
}

void CubeTexture::save(Json::Value& root) const
{
	root["textures"] = Json::Value(Json::arrayValue);
	for (int i = 0; i < 6; i++)
	{
		m_textures[i].save(root["textures"][i]);
	}

	root["internalFormat"] = (int)internalFormat;
	root["format"] = (int)format;
	root["type"] = (int)type;
	root["generateMipMap"] = generateMipMap;
}

void CubeTexture::drawInInspector(Scene & scene)
{
	Resource::drawInInspector(scene);

	if (ImGui::Button("Clear"))
	{
		clearTextures();
	}

	bool modified = false;
	if (EditorGUI::ResourceField<Texture>("texturePositiveX", m_textures[0]))
	{
		setTexture(0, m_textures[0]);
		modified = true;
	}
	if (EditorGUI::ResourceField<Texture>("textureNegativeX", m_textures[1]))
	{
		setTexture(1, m_textures[1]);
		modified = true;
	}
	if (EditorGUI::ResourceField<Texture>("texturePositiveY", m_textures[2]))
	{
		setTexture(2, m_textures[2]);
		modified = true;
	}
	if (EditorGUI::ResourceField<Texture>("textureNegativeY", m_textures[3]))
	{
		setTexture(3, m_textures[3]);
		modified = true;
	}
	if (EditorGUI::ResourceField<Texture>("texturePositiveZ", m_textures[4]))
	{
		setTexture(4, m_textures[4]);
		modified = true;
	}
	if (EditorGUI::ResourceField<Texture>("textureNegativeZ", m_textures[5]))
	{
		setTexture(5, m_textures[5]);
		modified = true;
	}

	if (modified)
	{
		freeGL();
		initGL();
	}
}

///////////////////////////////////////////
//// BEGIN : texture helpers



namespace GlHelper {

	Texture* makeNewColorTexture(float width, float height)
	{
		// Create color texture
		Texture* newTexture = new Texture(width, height, true);
		newTexture->setTextureParameters(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, false);
		newTexture->setTextureMinMaxFilters(GL_NEAREST, GL_NEAREST);
		newTexture->setTextureWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		return newTexture;
	}

	
	Texture* makeNewFloatColorTexture(float width, float height)
	{
		// Create color texture
		Texture* newTexture = new Texture(width, height, true);
		newTexture->setTextureParameters(GL_RGB16F, GL_RGB, GL_FLOAT, false);
		newTexture->setTextureMinMaxFilters(GL_NEAREST, GL_NEAREST);
		newTexture->setTextureWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		return newTexture;
	}

	Texture* makeNewNormalTexture(float width, float height)
	{
		// Create normal texture
		Texture* newTexture = new Texture(width, height, true);
		newTexture->setTextureParameters(GL_RGBA16, GL_RGBA, GL_FLOAT, false);
		newTexture->setTextureMinMaxFilters(GL_NEAREST, GL_NEAREST);
		newTexture->setTextureWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		return newTexture;
	}

	Texture* makeNewDepthTexture(float width, float height)
	{
		// Create depth texture
		Texture* newTexture = new Texture(width, height, true);
		newTexture->setTextureParameters(GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT, false);
		newTexture->setTextureMinMaxFilters(GL_NEAREST, GL_NEAREST);
		newTexture->setTextureWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		return newTexture;
	}

	void makeColorTexture(Texture& texture, float width, float height)
	{
		texture.resizeTexture(width, height);
		texture.setTextureParameters(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, false);
		texture.setTextureMinMaxFilters(GL_NEAREST, GL_NEAREST);
		texture.setTextureWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}

	void makeFloatColorTexture(Texture& texture, float width, float height)
	{
		texture.resizeTexture(width, height);
		texture.setTextureParameters(GL_RGB16F, GL_RGB, GL_FLOAT, false);
		texture.setTextureMinMaxFilters(GL_NEAREST, GL_NEAREST);
		texture.setTextureWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}

	void makeNormalTexture(Texture& texture, float width, float height)
	{
		texture.resizeTexture(width, height);
		texture.setTextureParameters(GL_RGBA16, GL_RGBA, GL_FLOAT, false);
		texture.setTextureMinMaxFilters(GL_NEAREST, GL_NEAREST);
		texture.setTextureWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}

	void makeDepthTexture(Texture& texture, float width, float height)
	{
		texture.resizeTexture(width, height);
		texture.setTextureParameters(GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT, false);
		texture.setTextureMinMaxFilters(GL_NEAREST, GL_NEAREST);
		texture.setTextureWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}

	void makeDepthStencilTexture(Texture& texture, float width, float height)
	{
		texture.resizeTexture(width, height);
		texture.setTextureParameters(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT, false);
		texture.setTextureMinMaxFilters(GL_NEAREST, GL_NEAREST);
		texture.setTextureWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}

	void makeRedTexture(Texture& texture, float width, float height)
	{
		texture.resizeTexture(width, height);
		texture.setTextureParameters(GL_RED, GL_RGB, GL_FLOAT, false);
		texture.setTextureMinMaxFilters(GL_NEAREST, GL_NEAREST);
		texture.setTextureWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}
}

//// END : texture helpers
///////////////////////////////////////////