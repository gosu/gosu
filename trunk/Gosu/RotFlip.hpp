//! \file RotFlip.hpp
//! Interface of the RotFlip class.

#ifndef GOSU_ROTFLIP_HPP
#define GOSU_ROTFLIP_HPP

namespace Gosu
{
    //! Human-understandable descriptions of the variants of rotations and flips
    //! that can be constructed.
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
    
    //! Light-weight class that encodes a rotation by 0, 90, 180, or 270
    //! degrees as well as optional horizontal and/or vertical flips.
    //! In short, this convieniently encodes the eight ways in which a
    //! aligned rectangle can be linearly transformed to result in an aligned
    //! rectangle again.
    //! Especially useful for games with tile-based maps.
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
    
    //! Applies a RotFlip to a point x/y in a square of side length max.
    void applyToPoint(RotFlip rotFlip, int& x, int& y, int max);
}

#endif
