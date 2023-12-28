#pragma once
#include "glm.hpp"
#include "Prereq.h"
#include "Renderer.h"
#include "Vertex.h"
namespace Voidstar
{
	struct Quad
	{
		glm::vec3 Pos;
		glm::vec2 Scale;
		glm::vec4 Color;
	};
	struct QuadRangle
	{
		std::vector<Vertex> Verticies;
	};
	
	class Drawable
	{
	public:
		template<typename T>
		Drawable(T data) : m_Self{ CreateUPtr<DrawableObj<T>>(std::move(data)) }
		{

		}
	private:
		friend class Renderer;
		struct IDrawable
		{
			void virtual Draw() = 0;
		};
		template<typename T>
		class DrawableObj : public IDrawable
		{
		public:
			DrawableObj(T data) : m_Self(CreateSPtr<T>(data)) {

			}
			void Draw() override
			{
				Renderer::Instance()->Draw(*m_Self);
			}
		private:
			SPtr<T> m_Self;
		};
		SPtr<IDrawable> m_Self;
	};
}
