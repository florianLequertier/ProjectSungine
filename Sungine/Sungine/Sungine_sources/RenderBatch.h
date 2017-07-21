#pragma once

#include <glew/glew.h>
#include <vector>
#include <map>

#include "IDrawable.h"
#include "BatchableWith.h"
#include "Materials.h"
#include "Lights.h"
#include "RenderDatas.h"

class IRenderBatch
{
protected: 
	GLuint m_programId;
	std::vector<const IDrawable*> m_drawables;
public:
	virtual void add(const IDrawable* drawable, const MaterialInstance* material) = 0;
	virtual void clear() = 0;
	virtual void render(const glm::mat4& projection, const glm::mat4& view, const RenderDatas& renderDatas) const = 0;
	//virtual void renderForward(const glm::mat4& projection, const glm::mat4& view, const RenderDatas& renderDatas) const = 0;
	const std::vector<const IDrawable*>& getDrawables() const
	{
		return m_drawables;
	}
	GLuint getProgramId() const
	{
		return m_programId;
	}
};

//////////////////////////////////////////////

template<typename MaterialType>
class RenderBatch : public IRenderBatch
{
protected:	
	std::map<const MaterialType*, std::vector<const IDrawable*>> m_container;

public:
	void add(const IDrawable* drawable, const MaterialInstance* material) override;
	void clear() override;
	void render(const glm::mat4& projection, const glm::mat4& view, const RenderDatas& renderDatas) const override;
	//void renderForward(const glm::mat4& projection, const glm::mat4& view, const RenderDatas& renderDatas) const override;
};

//////////////////////////////////////////////


template<typename MaterialType>
inline void RenderBatch<MaterialType>::add(const IDrawable * drawable, const MaterialInstance * material)
{
	assert(drawable != nullptr && material != nullptr);

	if (m_container.size() > 0)
		assert(material->getGLId() == m_programId);
	else
		m_programId = material->getGLId();

	m_container[static_cast<const MaterialType*>(material)].push_back(drawable);

	m_drawables.push_back(drawable);
}

template<typename MaterialType>
inline void RenderBatch<MaterialType>::clear()
{
	m_container.clear();
	m_drawables.clear();
}

template<typename MaterialType>
inline void RenderBatch<MaterialType>::render(const glm::mat4& projection, const glm::mat4& view, const RenderDatas& renderDatas) const
{
	const MaterialType* material = m_container.begin()->first;

	// Use material
	material->use();

	//TODO RENDERING
	// Push globals to GPU
	//pushGlobalsToGPU(*material);
	material->pushGlobalsToGPU(renderDatas);

	// For each material instance...
	int texCount = 0;
	for (auto& item : m_container)
	{
		const MaterialType* materialInstance = item.first;

		// Push internals to GPU
		materialInstance->pushInternalsToGPU(texCount);

		// For each drawable...
		for (auto& drawable : item.second)
		{
			// Push externals to GPU
			//static_cast<const IBatchableWith<MaterialType>* const>(drawable)->setExternalsOf(*materialInstance, projection, view, &texCount);
			materialInstance->pushExternalsToGPU(*drawable, renderDatas, texCount);
			//pushExternalsToPGU(*materialInstance, *drawable, projection, view);

			// Draw the drawable
			drawable->draw();
		}

		texCount = 0;
	}
}

//
//template<typename MaterialType>
//inline void RenderBatch<MaterialType>::renderForward(const glm::mat4& projection, const glm::mat4& view, const RenderDatas& renderDatas) const
//{
//	const MaterialType* material = m_container.begin()->first;
//	const MaterialAggregation* forwardAggregate = material->getAggregation("forward").get();
//	// Use material
//	material->use();
//
//	forwardAggregate->pushParametersToGPU(renderDatas);
//
//	//TODO RENDERING
//	// Push globals to GPU
//	//pushGlobalsToGPU(*material);
//
//	// For each material instance...
//	int texCount = 0;
//	for (auto& item : m_container)
//	{
//		const MaterialType* materialInstance = item.first;
//
//		// Push internals to GPU
//		materialInstance->pushInternalsToGPU(texCount);
//
//		// For each drawable...
//		for (auto& drawable : item.second)
//		{
//			// Push externals to GPU
//			static_cast<const IBatchableWith<MaterialType>* const>(drawable)->setExternalsOf(*materialInstance, projection, view, &texCount);
//			//pushExternalsToPGU(*materialInstance, *drawable, projection, view);
//
//			// Draw the drawable
//			drawable->draw();
//		}
//
//		texCount = 0;
//	}
//}


/////////////////////////////////////////////

inline std::shared_ptr<IRenderBatch> MaterialInstance::makeSharedRenderBatch() const
{
	return std::make_shared<RenderBatch<MaterialInstance>>();
}

//template<typename BatchMaterialType, typename ParentMaterialType>
//inline std::shared_ptr<IRenderBatch> BatchableMaterial<BatchMaterialType, ParentMaterialType>::makeSharedRenderBatch() const
//{
//	return std::make_shared<RenderBatch<BatchMaterialType>>();
//}


//template<typename DrawableType, typename MaterialType>
//class RenderBatch : public IRenderBatch
//{
//protected:
//	GLuint m_programId;
//	std::map<const MaterialType*, std::vector<const DrawableType*>> m_container;
//
//public:
//	void add(const IDrawable* drawable, const MaterialInstance* material) override;
//	void clear() override;
//	void render(const glm::mat4& projection, const glm::mat4& view) const override;
//
//	virtual void pushGlobalsToGPU(const MaterialType& materialInstance) const = 0;
//	virtual void pushExternalsToPGU(const MaterialType& materialInstance, const DrawableType& drawable, const glm::mat4& projection, const glm::mat4& view) const = 0;
//};

//////////////////////////////////////////////

//class RenderBatchConstructor
//{
//public:
//	template<typename DrawableType, typename MaterialType>
//	static std::shared_ptr<RenderBatch<DrawableType, MaterialType>> makeSharedRenderBatch(const DrawableType* drawable, const MaterialType* material)
//	{ 
//		std::shared_ptr<RenderBatch<DrawableType, MaterialType>> newRenderBatch = std::make_shared<RenderBatch<DrawableType, MaterialType>>();
//		newRenderBatch->add(drawable, material);
//		return newRenderBatch;
//	}
//};

//////////////////////////////////////////////
//
//template<typename DrawableType, typename MaterialType>
//inline void RenderBatch<DrawableType, MaterialType>::add(const IDrawable * drawable, const MaterialInstance * material)
//{
//	assert(drawable != nullptr && material != nullptr);
//
//	m_container[static_cast<const MaterialType*>(material)].push_back(static_cast<const DrawableType*>(drawable));
//
//	m_drawables.push_back(drawable);
//}
//
//template<typename DrawableType, typename MaterialType>
//inline void RenderBatch<DrawableType, MaterialType>::clear()
//{
//	m_container.clear();
//	m_drawables.clear();
//}
//
//template<typename DrawableType, typename MaterialType>
//inline void RenderBatch<DrawableType, MaterialType>::render(const glm::mat4& projection, const glm::mat4& view) const
//{
//	const MaterialType* material = m_container.begin()->first;
//
//	// Use material
//	material->use();
//
//	// Push globals to GPU
//	pushGlobalsToGPU(*material);
//
//	// For each material instance...
//	int texCount = 0;
//	for (auto& item : m_container)
//	{
//		const MaterialType* materialInstance = item.first;
//
//		// Push internals to GPU
//		materialInstance->pushInternalsToGPU(texCount);
//
//		// For each drawable...
//		for (auto& drawable : item.second)
//		{
//			// Push externals to GPU
//			pushExternalsToPGU(*materialInstance, *drawable, projection, view);
//
//			// Draw the drawable
//			drawable->draw();
//		}
//	}
//}


