#include <Gosu/RotFlip.hpp>
#include <algorithm>

const Gosu::RotFlip::TransTable Gosu::RotFlip::flipXTable =
{
    // rfDefault ->
    rfFlipX,
    // rfRotate90 ->
    rfRotate90FlipX,
    // rfRotate180 ->
    rfRotate180FlipX,
    // rfRotate270 ->
    rfRotate270FlipX,
    // rfFlipX ->
    rfDefault,
    // rfRotate90FlipX ->
    rfRotate90,
    // rfRotate180FlipX ->
    rfRotate180,
    // rfRotate270FlipX ->
    rfRotate270
};

const Gosu::RotFlip::TransTable Gosu::RotFlip::flipYTable =
{
    // rfDefault ->
    rfFlipY,
    // rfRotate90 ->
    rfRotate90FlipY,
    // rfRotate180 ->
    rfRotate180FlipY,
    // rfRotate270 ->
    rfRotate270FlipY,
    // rfFlipX ->
    rfRotate180,
    // rfRotate90FlipX ->
    rfRotate270,
    // rfRotate180FlipX ->
    rfDefault,
    // rfRotate270FlipX ->
    rfRotate90
};

const Gosu::RotFlip::TransTable Gosu::RotFlip::rotate90Table =
{
    // rfDefault ->
    rfRotate90,
    // rfRotate90 ->
    rfRotate180,
    // rfRotate180 ->
    rfRotate270,
    // rfRotate270 ->
    rfDefault,
    // rfFlipX ->
    rfFlipXRotate90,
    // rfRotate90FlipX ->
    rfFlipX,
    // rfRotate180FlipX ->
    rfFlipYRotate90,
    // rfRotate270FlipX ->
    rfFlipY
};

const Gosu::RotFlip::TransTable Gosu::RotFlip::rotate180Table =
{
    // rfDefault ->
    rotate90Table[rotate90Table[rfDefault]],
    // rfRotate90 ->
    rotate90Table[rotate90Table[rfRotate90]],
    // rfRotate180 ->
    rotate90Table[rotate90Table[rfRotate180]],
    // rfRotate270 ->
    rotate90Table[rotate90Table[rfRotate270]],
    // rfFlipX ->
    rotate90Table[rotate90Table[rfFlipX]],
    // rfRotate90FlipX ->
    rotate90Table[rotate90Table[rfRotate90FlipX]],
    // rfRotate180FlipX ->
    rotate90Table[rotate90Table[rfRotate180FlipX]],
    // rfRotate270FlipX ->
    rotate90Table[rotate90Table[rfRotate270FlipX]],
};

const Gosu::RotFlip::TransTable Gosu::RotFlip::rotate270Table =
{
    // rfDefault ->
    rotate90Table[rotate180Table[rfDefault]],
    // rfRotate90 ->
    rotate90Table[rotate180Table[rfRotate90]],
    // rfRotate180 ->
    rotate90Table[rotate180Table[rfRotate180]],
    // rfRotate270 ->
    rotate90Table[rotate180Table[rfRotate270]],
    // rfFlipX ->
    rotate90Table[rotate180Table[rfFlipX]],
    // rfRotate90FlipX ->
    rotate90Table[rotate180Table[rfRotate90FlipX]],
    // rfRotate180FlipX ->
    rotate90Table[rotate180Table[rfRotate180FlipX]],
    // rfRotate270FlipX ->
    rotate90Table[rotate180Table[rfRotate270FlipX]],
};

const Gosu::RotFlip::CornerTable Gosu::RotFlip::mapCornerTable =
{
    // rfDefault ->
    { 0, 1, 2, 3 },
    // rfRotate90 ->
    { 1, 3, 0, 2 },
    // rfRotate180 ->
    { 3, 2, 1, 0 },
    // rfRotate270 ->
    { 2, 0, 3, 1 },
    // rfFlipX ->
    { 1, 0, 3, 2 },
    // rfRotate90FlipX ->
    { 0, 2, 1, 3 },
    // rfRotate180FlipX ->
    { 2, 3, 0, 1 },
    // rfRotate270FlipX ->
    { 3, 1, 2, 0 }
};

const Gosu::RotFlip::CornerTable Gosu::RotFlip::realCornerTable =
{
    // rfDefault ->
    { 0, 1, 2, 3 },
    // rfRotate90 ->
    { 1, 3, 0, 2 },
    // rfRotate180 ->
    { 3, 2, 1, 0 },
    // rfRotate270 ->
    { 1, 3, 0, 2 },
    // rfFlipX ->
    { 1, 0, 3, 2 },
    // rfRotate90FlipX ->
    { 3, 1, 2, 0 },
    // rfRotate180FlipX ->
    { 2, 3, 0, 1 },
    // rfRotate270FlipX ->
    { 0, 2, 1, 3 }
};

void Gosu::applyToPoint(Gosu::RotFlip rotFlip, int& x, int& y, int max)
{
    switch (rotFlip.name())
    {
    case rfDefault:
        return;

    case rfRotate90:
        std::swap(x, y);
        y = max - y;
        return;

    case rfRotate180:
        x = max - x;
        y = max - y;
        return;

    case rfRotate270:
        std::swap(x, y);
        x = max - x;
        return;

    case rfFlipX:
        x = max - x;
        return;

    case rfRotate90FlipX:
        std::swap(x, y);
        return;

    case rfRotate180FlipX: // == rfFlipY
        y = max - y;
        return;

    case rfRotate270FlipX:
        int temp = x;
        x = max - y;
        y = max - temp;
        return;
    }
}
