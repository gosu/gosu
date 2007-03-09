#include <GosuImpl/Direct3D/Direct3D.hpp>

void Gosu::Direct3D::Blit::perform() const
{
    if (FAILED(device->SetTexture(0, texture)))
        return;

    switch (mode)
    {
    case amDefault:
        device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        break;
    case amAdditive:
        device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        break;
    }

    switch (type)
    {
    case btQuad:
        device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2,
            vertices, sizeof(Vertex));
        break;

    case btLine:
        device->DrawPrimitiveUP(D3DPT_LINELIST, 1,
            vertices, sizeof(Vertex));
        break;

    case btTri:
        device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1,
            vertices, sizeof(Vertex));
        break;
    }
}
