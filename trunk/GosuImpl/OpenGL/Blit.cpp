#include <GosuImpl/OpenGL/Blit.hpp>
#include <GosuImpl/OpenGL/TexChunk.hpp>
#include <GosuImpl/OpenGL/Texture.hpp>

void Gosu::DrawOp::perform() const
{
    glEnable(GL_BLEND);

    if (mode == amAdditive)
        glBlendFunc(GL_ONE, GL_ONE);
    else
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (chunk)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, chunk->texName());
    }

    if (usedVertices == 2)
        glBegin(GL_LINES);
    else if (usedVertices == 3)
        glBegin(GL_TRIANGLES);
    else if (usedVertices == 4)
        glBegin(GL_QUADS);

    double left, top, right, bottom;
    if (chunk)
        chunk->getCoords(left, top, right, bottom);

    for (unsigned i = 0; i < usedVertices; i++)
    {
        glColor4f(vertices[i].c.red() / 255.0, vertices[i].c.green() / 255.0,
                  vertices[i].c.blue() / 255.0, vertices[i].c.alpha() / 255.0);
        if (chunk)
            switch (i)
            {
            case 0:
                glTexCoord2d(left, top);
                break;
            case 1:
                glTexCoord2d(right, top);
                break;
            case 2:
                glTexCoord2d(right, bottom);
                break;
            case 3:
                glTexCoord2d(left, bottom);
                break;
            }
        glVertex2d(vertices[i].x, vertices[i].y);
    }

    glEnd();

    if (chunk)
        glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}
