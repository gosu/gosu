//! \file RotFlip.hpp
//! Interface of the RotFlip class.

#ifndef GOSU_ROTFLIP_HPP
#define GOSU_ROTFLIP_HPP

namespace Gosu
{
    //! Constants that can be used in place of real RotFlip objects. They
    //! exist for every possible combinations of both a rotation and a flip,
    //! executed in the order in which they appear in the name. If you call
    //! flipX() and then rotate270() on a default-constructed RotFlip object,
    //! it is equivalent to rfFlipXRotate270. 
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

    //! Represents a state in which an object can be after being flipped
    //! horizontally, vertically or rotated in 90° steps, in any order. If, for
    //! example, the tiles in your game can be flipped or rotated, it is
    //! convenient to store a RotFlip object together with each tile and draw
    //! the tile using Image::drawRotFlip.
    //! RotFlip objects are small and cheap to copy (they just wrap the
    //! RotFlipName enumeration).
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

        //! Returns the RotFlipName member that represents the current state.
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

        //! Returns whether the current state cannot be reached using only
        //! rotations.
        bool flipped() const
        {
            return name_ >= rfFlipX;
        }

        //! Returns whether the current state cannot be reached using only
        //! flips.
        bool rotated() const
        {
            return name_ % 2 == 1;
        }

        // IMPR: Strange explanation :/
        //! Returns the new corner index of the nth corner of an image
        //! after the RotFlip has been applied to it.
        unsigned mapCorner(unsigned n) const
        {
            return mapCornerTable[name_][n];
        }

        // IMPR: Strange explanation :/
        //! Reverses mapCorner.
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

    // IMPR: Strange explanation :/
    //! Applies a RotFlip object to a point. For that, the point is assumed to
    //! be on a square area with the side length of max that starts at
    //! (0; 0).
    void applyToPoint(RotFlip rotFlip, int& x, int& y, int max);
}

#endif
