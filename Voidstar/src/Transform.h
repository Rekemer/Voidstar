#pragma once
#include "glm.hpp"
class Transform
{
	void Move(glm::vec3 trans)
	{
		auto newPos = globalPosition + trans;
		globalPosition = newPos;
		dirty = true;
	}
	void SetGlobalPosition(glm::vec3 position)
	{


		globalPosition = position;
		dirty = true;
	}
	void SetGlobalRotation(glm::vec3 rotation)
	{

		globalRotation = rotation;
		dirty = true;
	}
	void SetGlobalScale(glm::vec3 scale)
	{

		globalScale = scale;
		dirty = true;
	}
	void Scale(glm::vec3 scale)
	{
		globalScale += scale;
		dirty = true;
	}
	void UpdateLocalPosition(glm::vec3 parentPosition)
	{
		localPosition = globalPosition - parentPosition;
	}

	void SetLocalPosition(glm::vec3 localPos)
	{
		localPosition = localPos;
		localDirty = true;
	}
	void SetLocalRotation(glm::vec3 localRot)
	{
		localRotation = localRot;
		localDirty = true;
	}
	void SetLocalScale(glm::vec3 localScal)
	{
		localScale = localScal;
		localDirty = true;
	}
	glm::vec3 GetGlobalPosition()
	{
		return globalPosition;
	}
	glm::vec3 GetGlobalRotation()
	{
		return globalRotation;
	}
	glm::vec3 GetGlobalScale()
	{
		return globalScale;
	}
	const glm::mat4x4& GetGlobalMatrix()
	{
		return worldMatrix;
	}

	glm::vec3 GetLocalPosition()
	{
		return localPosition;
	}
	glm::vec3 GetLocalRotation()
	{
		return localRotation;
	}
	glm::vec3 GetLocalScale()
	{
		return localScale;
	}

	void SetLocalDirty()
	{
		localDirty = true;
	}
	void SetGlobalDirty()
	{
		localDirty = true;
	}


private:
	glm::vec3 globalPosition{ 0,0,0 };
	glm::vec3 globalRotation{ 0,0,0 };
	glm::vec3 globalScale{ 1, 1, 1 };
	// maybe can be computed on the fly?
	glm::vec3 localPosition{ 0,0,0 };
	glm::vec3 localRotation;
	glm::vec3 localScale{ 1, 1, 1 };
	glm::mat4x4 worldMatrix;
	bool dirty{ true };
	bool localDirty;

};