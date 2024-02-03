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
		glm::vec2 Scale{1,1};
		glm::vec4 Color = {1,0,1,1};
	};
	struct QuadRangle
	{
		std::vector<Vertex> Verticies;
	};
	
	struct Sphere
	{
		glm::vec3 Pos;
		glm::vec3 Scale = {1,1,1};
		glm::vec4 Color = { 1,0,1,1 };
		glm::vec3& Rot = glm::vec3{ 0,0,0 };
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
