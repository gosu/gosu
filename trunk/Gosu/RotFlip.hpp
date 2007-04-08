//! \file RotFlip.hpp
//! Interface of the undocumented RotFlip class.

#ifndef GOSU_ROTFLIP_HPP
#define GOSU_ROTFLIP_HPP

namespace Gosu
{
    enum RotFlipName
    {
        rfDefault,
        rfRotate90,
        rfRotate180,
        rfRotate270,

        rfFlipX,
        rfRotate90FlipX,
        rfRotate180FlipX,
        rfRotate270FlipX,
        
        rfFlipY          = rfRotate180FlipX,
        rfRotate90FlipY  = rfRotate270FlipX,
        rfRotate180FlipY = rfFlipX,
        rfRotate270FlipY = rfRotate90FlipX,

        rfFlipXRotate90  = rfRotate270FlipX,
        rfFlipXRotate180 = rfRotate180FlipX,
        rfFlipXRotate270 = rfRotate90FlipX,

        rfFlipYRotate90  = rfRotate90FlipX,
        rfFlipYRotate180 = rfFlipX,
        rfFlipYRotate270 = rfRotate270FlipX
    };

    class RotFlip
    {
        RotFlipName name_;

        typedef RotFlipName TransTable[8];
        static const TransTable flipXTable, flipYTable, rotate90Table,
            rotate180Table, rotate270Table;

        typedef unsigned CornerTable[8][4];
        static const CornerTable mapCornerTable, realCornerTable;

    public:
        RotFlip(RotFlipName name = rfDefault)
        : name_(name)
        {
        }

        RotFlipName name() const
        {
            return name_;
        }

        void flipX()
        {
            name_ = flipXTable[name_];
        }

        void flipY()
        {
            name_ = flipYTable[name_];
        }

        void rotate90()
        {
            name_ = rotate90Table[name_];
        }

        void rotate180()
        {
            name_ = rotate180Table[name_];
        }

        void rotate270()
        {
            name_ = rotate270Table[name_];
        }

        bool flipped() const
        {
            return name_ >= rfFlipX;
        }

        bool rotated() const
        {
            return name_ % 2 == 1;
        }

        unsigned mapCorner(unsigned n) const
        {
            return mapCornerTable[name_][n];
        }

        unsigned realCorner(unsigned n) const
        {
            return realCornerTable[name_][n];
        }
    };

    inline bool operator==(RotFlip a, RotFlip b)
    {
        return a.name() == b.name();
    }

    inline bool operator!=(RotFlip a, RotFlip b)
    {
        return a.name() != b.name();
    }

    void applyToPoint(RotFlip rotFlip, int& x, int& y, int max);
}

#endif
