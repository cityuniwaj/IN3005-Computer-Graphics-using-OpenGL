#pragma once
#include "Common.h"
#include "VertexBufferObject.h"

//class CCatmullRom
//{
//public:
//	CCatmullRom();
//	~CCatmullRom();
//	glm::vec3 Interpolate(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, float t);
//};

class CCatmullRom
{
public:
	CCatmullRom();
	~CCatmullRom();
	glm::vec3 Interpolate(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, float t);
	void CreatePath(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& p3);
	void RenderPath();
private:
	GLuint m_vao;
};