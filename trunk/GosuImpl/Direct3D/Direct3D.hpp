#ifndef GOSUIMPL_DIRECT3D_DIRECT3D_HPP
#define GOSUIMPL_DIRECT3D_DIRECT3D_HPP

#include <GosuImpl/Graphics/Graphics.hpp>
#include <d3d8.h>
#include <Gosu/Platform.hpp>
#include <Gosu/ImageData.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <string>
#include <vector>

namespace Gosu
{
    namespace Direct3D
    {
        GOSU_NORETURN void throwError(HRESULT retVal, const std::string& action);

        inline HRESULT check(HRESULT retVal, const std::string& action = "")
        {
            if (FAILED(retVal))
                throwError(retVal, action);
            return retVal;
        }

        struct Blit
        {
            IDirect3DDevice8* device;
            IDirect3DTexture8* texture;
            AlphaMode mode;

            enum
            {
                btQuad,
                btTri,
                btLine
            } type;

            struct Vertex
            {
                FLOAT x, y, z, rhw;
                DWORD diffuse;
                FLOAT u, v;
            };
            Vertex vertices[4];

            void perform() const;
        };

        // Wrapper class for a Direct3D texture on which ImageDatas can
        // be allocated.
        class Texture
        {
            struct Impl;
            boost::scoped_ptr<Impl> pimpl;

        public:
            Texture(IDirect3DDevice8* device, unsigned size);
            ~Texture();

            IDirect3DTexture8* texture() const;
            IDirect3DDevice8* device() const;
            unsigned size() const;

            // Tries to allocate space for a bitmap on the texture and returns
            // the position where it was placed on success.
            boost::optional<RECT> allocBlock(const Bitmap& bmp);

            // Marks the rectangle previously occupied by a bitmap as being
            // free again.
            void freeBlock(unsigned x, unsigned y);
        };

        class GraphicsImpl;

        class ImageDataImpl : public ImageData
        {
            Graphics* graphics;
            BlitQueue<Blit>* queue;
            boost::shared_ptr<Texture> texture;
            unsigned texX, texY;
            unsigned blockWidth, blockHeight;
            float srcX, srcY, srcX2, srcY2;

            ImageDataImpl();

        public:
            static std::auto_ptr<ImageData>
                tryToCreate(Graphics& graphics, BlitQueue<Blit>& queue,
                    const Bitmap& source,
                    const boost::shared_ptr<Texture>& texture,
                    unsigned paddingLeft, unsigned paddingRight,
                    unsigned paddingTop, unsigned paddingBottom);
            ~ImageDataImpl();

            unsigned width() const;
            unsigned height() const;

            void draw(double x1, double y1, Color c1,
                double x2, double y2, Color c2, double x3, double y3, Color c3,
                double x4, double y4, Color c4, ZPos z, AlphaMode mode) const;
        };
    }
}

#endif
