#include "CatmullRom.h"

CCatmullRom::CCatmullRom()
{}

CCatmullRom::~CCatmullRom()
{}

glm::vec3 CCatmullRom::Interpolate(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;

    glm::vec3 a = p1;
    glm::vec3 b = 0.5f * (-p0 + p2);
    glm::vec3 c = 0.5f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3);
    glm::vec3 d = 0.5f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3);

    return a + b * t + c * t2 + d * t3;
}

void CCatmullRom::CreatePath(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& p3)
{
    // Use VAO to store state associated with vertices
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    // Create a VBO
    CVertexBufferObject vbo;
    vbo.Create();
    vbo.Bind();
    glm::vec2 texCoord(0.0f, 0.0f);
    glm::vec3 normal(0.0f, 1.0f, 0.0f);
    for (unsigned int i = 0; i < 100; i++) {
        float t = (float)i / 100.0f;
        glm::vec3 v = Interpolate(p0, p1, p2, p3, t);
        vbo.AddData(&v, sizeof(glm::vec3));
        vbo.AddData(&texCoord, sizeof(glm::vec2));
        vbo.AddData(&normal, sizeof(glm::vec3));
    }
    // Upload the VBO to the GPU
    vbo.UploadDataToGPU(GL_STATIC_DRAW);
    // Set the vertex attribute locations
    GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);
    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    // Texture coordinates
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
    // Normal vectors
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
}

void CCatmullRom::RenderPath()
{
    glBindVertexArray(m_vao);
	glLineWidth(1.0f);
    glDrawArrays(GL_LINE_STRIP, 0, 100);
}