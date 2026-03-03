#include "Common.h"
#include "Cube.h"

CCube::CCube()
{}

CCube::~CCube()
{}

void CCube::Create(string a_sDirectory, string a_sFilename)
{
    // Load texture
    m_texture.Load(a_sDirectory + a_sFilename);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    m_vbo.Create();
    m_vbo.Bind();

    // Lambda to add one vertex cleanly
    auto addVertex = [&](glm::vec3 pos, glm::vec2 tex, glm::vec3 norm) {
        m_vbo.AddVertexData(&pos, sizeof(glm::vec3));
        m_vbo.AddVertexData(&tex, sizeof(glm::vec2));
        m_vbo.AddVertexData(&norm, sizeof(glm::vec3));
        };

    // --- FRONT FACE (normal = 0, 0, +1) ---
    addVertex({ -1,-1,+1 }, { 0,0 }, { 0,0,1 }); // V0
    addVertex({ +1,-1,+1 }, { 1,0 }, { 0,0,1 }); // V1
    addVertex({ +1,+1,+1 }, { 1,1 }, { 0,0,1 }); // V2
    addVertex({ -1,+1,+1 }, { 0,1 }, { 0,0,1 }); // V3

    // --- BACK FACE (normal = 0, 0, -1) ---
    addVertex({ +1,-1,-1 }, { 0,0 }, { 0,0,-1 }); // V4
    addVertex({ -1,-1,-1 }, { 1,0 }, { 0,0,-1 }); // V5
    addVertex({ -1,+1,-1 }, { 1,1 }, { 0,0,-1 }); // V6
    addVertex({ +1,+1,-1 }, { 0,1 }, { 0,0,-1 }); // V7

    // --- LEFT FACE (normal = -1, 0, 0) ---
    addVertex({ -1,-1,-1 }, { 0,0 }, { -1,0,0 }); // V8
    addVertex({ -1,-1,+1 }, { 1,0 }, { -1,0,0 }); // V9
    addVertex({ -1,+1,+1 }, { 1,1 }, { -1,0,0 }); // V10
    addVertex({ -1,+1,-1 }, { 0,1 }, { -1,0,0 }); // V11

    // --- RIGHT FACE (normal = +1, 0, 0) ---
    addVertex({ +1,-1,+1 }, { 0,0 }, { 1,0,0 }); // V12
    addVertex({ +1,-1,-1 }, { 1,0 }, { 1,0,0 }); // V13
    addVertex({ +1,+1,-1 }, { 1,1 }, { 1,0,0 }); // V14
    addVertex({ +1,+1,+1 }, { 0,1 }, { 1,0,0 }); // V15

    // --- TOP FACE (normal = 0, +1, 0) ---
    addVertex({ -1,+1,+1 }, { 0,0 }, { 0,1,0 }); // V16
    addVertex({ +1,+1,+1 }, { 1,0 }, { 0,1,0 }); // V17
    addVertex({ +1,+1,-1 }, { 1,1 }, { 0,1,0 }); // V18
    addVertex({ -1,+1,-1 }, { 0,1 }, { 0,1,0 }); // V19

    // --- BOTTOM FACE (normal = 0, -1, 0) ---
    addVertex({ -1,-1,-1 }, { 0,0 }, { 0,-1,0 }); // V20
    addVertex({ +1,-1,-1 }, { 1,0 }, { 0,-1,0 }); // V21
    addVertex({ +1,-1,+1 }, { 1,1 }, { 0,-1,0 }); // V22
    addVertex({ -1,-1,+1 }, { 0,1 }, { 0,-1,0 }); // V23

    // Index data — 12 triangles, CCW winding
    unsigned int indices[] = {
         0, 1, 2,   0, 2, 3,   // Front
         4, 5, 6,   4, 6, 7,   // Back
         8, 9,10,   8,10,11,   // Left
        12,13,14,  12,14,15,   // Right
        16,17,18,  16,18,19,   // Top
        20,21,22,  20,22,23    // Bottom
    };

    for (int i = 0; i < 36; i++)
        m_vbo.AddIndexData(&indices[i], sizeof(unsigned int));

    // Upload to GPU
    m_vbo.UploadDataToGPU(GL_STATIC_DRAW);

    // Stride = vec3 + vec2 + vec3 = 32 bytes
    GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);

    // Position — location 0, offset 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);

    // Texture Coordinates — location 1, offset 12
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));

    // Normal — location 2, offset 20
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
}

void CCube::Render()
{
    glBindVertexArray(m_vao);
    m_texture.Bind();
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void CCube::Release()
{
    m_texture.Release();
    glDeleteVertexArrays(1, &m_vao);
    m_vbo.Release();
}
