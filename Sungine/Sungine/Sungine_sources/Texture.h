#pragma once

#include <string>
#include <vector>

#include "stb/stb_image.h"
#include "glm/common.hpp"

#include "glew/glew.h"
#include "FileHandler.h"

#include "AssetManager.h"
#include "Asset.h"
#include "AssetPool.h"

enum class AlphaMode
{
	WITH_ALPHA,
	WITHOUT_ALPHA,
	AUTOMATIC
};

class Texture final : public Asset
{
public:
	static std::string s_extention;

private:

	int w;
	int h;
	int comp;

	unsigned char* pixels;

	GLuint glId;

	GLint internalFormat;
	GLenum format;
	GLenum type;
	bool generateMipMap;
	GLint textureWrapping_u;
	GLint textureWrapping_v;
	GLint minFilter;
	GLint magFilter;

	int m_textureUseCounts;

public:

	//////////////////////////////////////////////////////////
	// Constructors / Operators
	//////////////////////////////////////////////////////////
	Texture(int width = 0, int height = 0, bool useAlpha = false);
	Texture(unsigned char* _pixels, int width, int height, bool useAlpha, int _comp);
	Texture::Texture(const glm::vec3& color);
	Texture::Texture(const glm::vec4& color);
	Texture(int width, int height, const glm::vec4& color);
	Texture(int width, int height, const glm::vec3& color);
	Texture(const FileHandler::CompletePath& _path, AlphaMode alphaMode = AlphaMode::AUTOMATIC);
	~Texture();
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// Asset override
	//////////////////////////////////////////////////////////
	void createNewAssetFile(const FileHandler::CompletePath& filePath) override;
	void loadFromFile(const FileHandler::CompletePath& filePath) override;
	void saveToFile(const FileHandler::CompletePath& filePath) override;
	void saveMetas(const FileHandler::CompletePath& filePath) override;
	void loadMetas(const FileHandler::CompletePath& filePath) override;
	void drawIconeInResourceTree() override;
	void drawUIOnHovered() override;
	void drawIconeInResourceField() override;
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// Setters / Getters / Actions
	//////////////////////////////////////////////////////////
	int getWidth() const;
	int getHeight() const;
	int getComp() const;
	const unsigned char* getPixels() const;
	unsigned char getPixel(int index) const;
	void setTextureParameters(GLint _internalFormat = GL_RGB, GLenum _format = GL_RGB, GLenum _type = GL_UNSIGNED_BYTE, bool _generateMipMap = true);
	void setTextureMinMaxFilters(GLint _maxFilter = GL_NEAREST, GLint _format = GL_NEAREST);
	void setTextureWrapping(GLint _uWrapping = GL_CLAMP_TO_EDGE, GLint _vWrapping = GL_CLAMP_TO_EDGE);
	void setPixels(const std::vector<unsigned char>& _pixels, int _width, int _height, int _comp);
	void setPixels(const std::vector<glm::vec3>& _pixels, int _width, int _height);
	void setPixels(const std::vector<glm::vec4>& _pixels, int _width, int _height);
	void resizePixelArray(int width, int height, const glm::vec4& color);
	void resizePixelArray(int width, int height, const glm::vec3& color);
	// This fonction only change width and height without touching pixels
	void resizeTexture(int width, int height);
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// Opengl asset
	//////////////////////////////////////////////////////////
	void initGL();
	void freeGL();
	//////////////////////////////////////////////////////////
};

class CubeTexture final : public Asset
{
public:
	static std::string s_extention;

private:
	std::string name;

	bool m_isInitialized;

	//FileHandler::CompletePath paths[6];
	AssetHandle<Texture> m_textures[6];

	int w;
	int h;
	int comp;

	unsigned char* pixels[6];

	GLuint glId;

	GLint internalFormat;
	GLenum format;
	GLenum type;
	bool generateMipMap;
	GLint textureWrapping_s;
	GLint textureWrapping_t;
	GLint textureWrapping_r;
	GLint minFilter;
	GLint magFilter;

	int m_textureUseCounts;

public:

	//////////////////////////////////////////////////////////
	// Constructors / Operators
	//////////////////////////////////////////////////////////
	CubeTexture();
	CubeTexture(const FileHandler::CompletePath& path);
	CubeTexture(char r, char g, char b);
	CubeTexture(const std::vector<AssetHandle<Texture>>& textures);
	~CubeTexture();
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// Asset override
	//////////////////////////////////////////////////////////
	void createNewAssetFile(const FileHandler::CompletePath& filePath) override;
	void loadFromFile(const FileHandler::CompletePath& filePath) override;
	void saveToFile(const FileHandler::CompletePath& filePath) override;
	void saveMetas(const FileHandler::CompletePath& filePath) override;
	void loadMetas(const FileHandler::CompletePath& filePath) override;
	void drawIconeInResourceTree() override;
	void drawUIOnHovered() override;
	void drawIconeInResourceField() override;
	void drawInInspector() override;
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// Getters / Setters / Actions
	//////////////////////////////////////////////////////////
	void setTextureParameters(GLint _internalFormat = GL_RGB, GLenum _format = GL_RGB, GLenum _type = GL_UNSIGNED_BYTE, bool _generateMipMap = false);
	void setTextureMinMaxFilters(GLint _maxFilter = GL_LINEAR, GLint _format = GL_LINEAR);
	void setTextureWrapping(GLint _sWrapping = GL_CLAMP_TO_EDGE, GLint _tWrapping = GL_CLAMP_TO_EDGE, GLint _rWrapping = GL_CLAMP_TO_EDGE);
	void resizePixelArrays(int width, int height, const glm::vec4& color);
	void resizePixelArrays(int width, int height, const glm::vec3& color);
	// This fonction only change width and height without touching pixels
	void resizeTexture(int width, int height);
	void setTextureWithoutCheck(int index, AssetHandle<Texture> texture);
	bool setTexture(int index, AssetHandle<Texture> texture);
	bool initFromTextures(const std::vector<AssetHandle<Texture>>& textures);
	bool checkTextureCanBeAdded(const AssetHandle<Texture>& textureToAdd) const;
	void clearTextures();
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// Opengl asset
	//////////////////////////////////////////////////////////
	void initGL();
	void freeGL();
	//////////////////////////////////////////////////////////
};

namespace GlHelper{

	//make a basic color texture with GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE
	Texture* makeNewColorTexture(float width, float height);
	//make a basic color texture with GL_RGB16F, GL_RGB, GL_FLOAT
	Texture* makeNewFloatColorTexture(float width, float height);
	//make a basic normal texture with GL_RGBA16, GL_RGBA, GL_FLOAT
	Texture* makeNewNormalTexture(float width, float height);
	//make a basic depth texture with GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT
	Texture* makeNewDepthTexture(float width, float height);

	//make a basic color texture with GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE
	void makeColorTexture(Texture& texture, float width, float height);
	//make a basic color texture with GL_RGB16F, GL_RGB, GL_FLOAT
	void makeFloatColorTexture(Texture& texture, float width, float height);
	//make a basic normal texture with GL_RGBA16, GL_RGBA, GL_FLOAT
	void makeNormalTexture(Texture& texture, float width, float height);
	//make a basic depth texture with GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT
	void makeDepthTexture(Texture& texture, float width, float height);
	//make a basic depth texture with GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT
	void makeDepthStencilTexture(Texture& texture, float width, float height);
	//make a basic depth texture with GL_RED, GL_RGB, GL_FLOAT
	void makeRedTexture(Texture& texture, float width, float height);
}