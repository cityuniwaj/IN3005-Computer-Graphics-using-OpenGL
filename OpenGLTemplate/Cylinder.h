#pragma once
#include "Texture.h"
#include "VertexBufferObjectIndexed.h"

class CCylinder
{
public:
    CCylinder();
    ~CCylinder();
    void Create(string directory, string filename, float radius, float height, int slices);
    void Render();
    void Release();
private:
    UINT m_vao;
    CVertexBufferObjectIndexed m_vbo;
    CTexture m_texture;
    int m_numTriangles;
};
