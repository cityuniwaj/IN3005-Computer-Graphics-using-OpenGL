#include "../Common.h"

#define _USE_MATH_DEFINES

#include "../Cylinder.h"
#include <math.h>

CCylinder::CCylinder()
{}

CCylinder::~CCylinder()
{}

void CCylinder::Create(string directory, string filename, float radius, float height, int slices)
{
    m_texture.Load(directory + filename);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    m_vbo.Create();
    m_vbo.Bind();

    m_numTriangles = 0;

    // Side wall
    unsigned int sideStart = 0;
    for (int i = 0; i <= slices; i++) {
        float angle = (i / (float)slices) * 2.0f * (float)M_PI;
        float c = cos(angle);
        float s = sin(angle);

        glm::vec3 vBottom(radius * c, 0.0f, radius * s);
        glm::vec2 tBottom(i / (float)slices, 0.0f);
        glm::vec3 nSide(c, 0.0f, s);

        glm::vec3 vTop(radius * c, height, radius * s);
        glm::vec2 tTop(i / (float)slices, 1.0f);

        m_vbo.AddVertexData(&vBottom, sizeof(glm::vec3));
        m_vbo.AddVertexData(&tBottom, sizeof(glm::vec2));
        m_vbo.AddVertexData(&nSide, sizeof(glm::vec3));

        m_vbo.AddVertexData(&vTop, sizeof(glm::vec3));
        m_vbo.AddVertexData(&tTop, sizeof(glm::vec2));
        m_vbo.AddVertexData(&nSide, sizeof(glm::vec3));
    }

    for (int i = 0; i < slices; i++) {
        unsigned int b0 = sideStart + i * 2;
        unsigned int t0 = b0 + 1;
        unsigned int b1 = b0 + 2;
        unsigned int t1 = b0 + 3;

        m_vbo.AddIndexData(&b0, sizeof(unsigned int));
        m_vbo.AddIndexData(&t0, sizeof(unsigned int));
        m_vbo.AddIndexData(&b1, sizeof(unsigned int));
        m_numTriangles++;

        m_vbo.AddIndexData(&b1, sizeof(unsigned int));
        m_vbo.AddIndexData(&t0, sizeof(unsigned int));
        m_vbo.AddIndexData(&t1, sizeof(unsigned int));
        m_numTriangles++;
    }

    // Top cap
    unsigned int topCenterIndex = (unsigned int)((slices + 1) * 2);
    glm::vec3 vTopCenter(0.0f, height, 0.0f);
    glm::vec2 tTopCenter(0.5f, 0.5f);
    glm::vec3 nTop(0.0f, 1.0f, 0.0f);
    m_vbo.AddVertexData(&vTopCenter, sizeof(glm::vec3));
    m_vbo.AddVertexData(&tTopCenter, sizeof(glm::vec2));
    m_vbo.AddVertexData(&nTop, sizeof(glm::vec3));

    for (int i = 0; i <= slices; i++) {
        float angle = (i / (float)slices) * 2.0f * (float)M_PI;
        float c = cos(angle);
        float s = sin(angle);

        glm::vec3 v(radius * c, height, radius * s);
        glm::vec2 t((c + 1.0f) * 0.5f, (s + 1.0f) * 0.5f);

        m_vbo.AddVertexData(&v, sizeof(glm::vec3));
        m_vbo.AddVertexData(&t, sizeof(glm::vec2));
        m_vbo.AddVertexData(&nTop, sizeof(glm::vec3));
    }

    for (int i = 0; i < slices; i++) {
        unsigned int i0 = topCenterIndex;
        unsigned int i1 = topCenterIndex + 1 + i;
        unsigned int i2 = topCenterIndex + 1 + i + 1;

        m_vbo.AddIndexData(&i0, sizeof(unsigned int));
        m_vbo.AddIndexData(&i2, sizeof(unsigned int));
        m_vbo.AddIndexData(&i1, sizeof(unsigned int));
        m_numTriangles++;
    }

    // Bottom cap
    unsigned int bottomCenterIndex = topCenterIndex + 1 + (slices + 1);
    glm::vec3 vBottomCenter(0.0f, 0.0f, 0.0f);
    glm::vec2 tBottomCenter(0.5f, 0.5f);
    glm::vec3 nBottom(0.0f, -1.0f, 0.0f);
    m_vbo.AddVertexData(&vBottomCenter, sizeof(glm::vec3));
    m_vbo.AddVertexData(&tBottomCenter, sizeof(glm::vec2));
    m_vbo.AddVertexData(&nBottom, sizeof(glm::vec3));

    for (int i = 0; i <= slices; i++) {
        float angle = (i / (float)slices) * 2.0f * (float)M_PI;
        float c = cos(angle);
        float s = sin(angle);

        glm::vec3 v(radius * c, 0.0f, radius * s);
        glm::vec2 t((c + 1.0f) * 0.5f, (s + 1.0f) * 0.5f);

        m_vbo.AddVertexData(&v, sizeof(glm::vec3));
        m_vbo.AddVertexData(&t, sizeof(glm::vec2));
        m_vbo.AddVertexData(&nBottom, sizeof(glm::vec3));
    }

    for (int i = 0; i < slices; i++) {
        unsigned int i0 = bottomCenterIndex;
        unsigned int i1 = bottomCenterIndex + 1 + i;
        unsigned int i2 = bottomCenterIndex + 1 + i + 1;

        m_vbo.AddIndexData(&i0, sizeof(unsigned int));
        m_vbo.AddIndexData(&i1, sizeof(unsigned int));
        m_vbo.AddIndexData(&i2, sizeof(unsigned int));
        m_numTriangles++;
    }

    m_vbo.UploadDataToGPU(GL_STATIC_DRAW);

    GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
}

void CCylinder::Render()
{
    glBindVertexArray(m_vao);
    m_texture.Bind();
    glDrawElements(GL_TRIANGLES, m_numTriangles * 3, GL_UNSIGNED_INT, 0);
}

void CCylinder::Release()
{
    m_texture.Release();
    glDeleteVertexArrays(1, &m_vao);
    m_vbo.Release();
}
