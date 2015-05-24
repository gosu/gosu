#ifndef GOSUIMPL_GRAPHICS_MACRO_HPP
#define GOSUIMPL_GRAPHICS_MACRO_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/ImageData.hpp>
#include <Gosu/TR1.hpp>
#include <Gosu/Platform.hpp>
#include "Common.hpp"

class Gosu::Macro : public Gosu::ImageData
{
    struct Impl;
    std::tr1::shared_ptr<Impl> pimpl;
    
public:
    Macro(DrawOpQueue& queue, int width, int height);
    
    int width() const;
    int height() const;
    
    void draw(double x1, double y1, Color c1,
        double x2, double y2, Color c2,
        double x3, double y3, Color c3,
        double x4, double y4, Color c4,
        ZPos z, AlphaMode mode) const;
    
    const Gosu::GLTexInfo* glTexInfo() const;
    
    Gosu::Bitmap toBitmap() const;
    
    GOSU_UNIQUE_PTR<ImageData> subimage(int x, int y, int width, int height) const;
    
    void insert(const Bitmap& bitmap, int x, int y);
};

#endif
