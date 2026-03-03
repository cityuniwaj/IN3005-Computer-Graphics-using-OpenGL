#pragma once

#include "Texture.h"
#include "VertexBufferObjectIndexed.h"

// Class for generating a unit cube
class CCube
{
public:
    CCube();
    ~CCube();
    void Create(string directory, string filename);
    void Render();
    void Release();
private:
    UINT m_vao;
    CVertexBufferObjectIndexed m_vbo;
    CTexture m_texture;
};