#include <Gosu/Drawable.hpp>
#include <Gosu/Utility.hpp>
#include "EmptyDrawable.hpp"
#include "Texture.hpp"
#include "TiledDrawable.hpp"
#include <list>
#include <mutex>

namespace Gosu
{
    // This variable has been declared in multiple places. Define it here, where it will be used.
    // This is a compatibility hack for old versions of Ruby/Gosu that didn't yet support :retro.
    bool undocumented_retrofication = false; // NOLINT(*-avoid-non-const-global-variables)
}

std::unique_ptr<Gosu::Drawable> Gosu::create_drawable(const Bitmap& source, const Rect& source_rect,
                                                      unsigned image_flags)
{
    if (!Rect::covering(source).contains(source_rect)) {
        throw std::invalid_argument("Source rectangle exceeds bitmap");
    }

    if (source_rect.empty()) {
        // It can happen that Gosu::Font wants us to create an empty drawable when rendering
        // invisible characters (e.g. '\r'). In this case, return an empty drawable that is not
        // backed by any kind of OpenGL texture.
        return std::make_unique<EmptyDrawable>(source_rect.width, source_rect.height);
    }

    // Backward compatibility: This used to be 'bool tileable', help users that still pass 'true'.
    if (image_flags == 1) {
        image_flags = IF_TILEABLE;
    }

    bool wants_retro = (image_flags & IF_RETRO) || undocumented_retrofication;

    // Special case: If the texture is supposed to be tileable, is quadratic, has a size that is at
    // least 64 pixels but no more than MAX_TEXTURE_SIZE pixels and a power of two, create a single
    // texture just for this image.
    // This is not just an optimization, but a feature of Gosu so that one can use Gosu for loading
    // textures for use in 3D scenes, where it is important that the full u/v range is dedicated to
    // a single image so that texture repetition works as expected.
    if ((image_flags & IF_TILEABLE) == IF_TILEABLE && source_rect.width == source_rect.height
        && (source_rect.width & (source_rect.width - 1)) == 0 && source_rect.width >= 64
        && source_rect.width <= MAX_TEXTURE_SIZE) {

        const std::shared_ptr<Texture> texture
            = std::make_shared<Texture>(source_rect.width, source_rect.height, wants_retro);

        // Use the source bitmap directly if the source area completely covers it.
        if (source_rect == Rect::covering(source)) {
            return texture->try_alloc(source, 0);
        }
        else {
            Bitmap trimmed_source(source_rect.width, source_rect.height);
            trimmed_source.insert(source, 0, 0, source_rect);
            return texture->try_alloc(trimmed_source, 0);
        }
    }

    const int max_size = MAX_TEXTURE_SIZE;

    // Too large to fit on a single texture? -> Create a tiled representation.
    if (source_rect.width > max_size - 2 || source_rect.height > max_size - 2) {
        return std::make_unique<TiledDrawable>(source, source_rect, max_size - 2, image_flags);
    }

    Bitmap source_with_borders = apply_border_flags(image_flags, source, source_rect);

    // Try to put the bitmap into one of the already allocated textures.
    static std::list<std::weak_ptr<Texture>> texture_pool;
    static std::mutex mutex;
    std::scoped_lock lock(mutex);

    texture_pool.remove_if([](const auto& weak_ptr) { return weak_ptr.expired(); });

    for (const std::weak_ptr<Texture>& weak_texture : texture_pool) {
        const auto texture = weak_texture.lock();
        if (!texture || texture->retro() != wants_retro) {
            continue;
        }

        std::unique_ptr<Drawable> data = texture->try_alloc(source_with_borders, 1);
        if (data) {
            return data;
        }
    }

    // All textures are full: Create a new one.

    std::shared_ptr<Texture> texture
        = std::make_shared<Texture>(MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, wants_retro);
    texture_pool.push_back(texture);
    return texture->try_alloc(source_with_borders, 1);
}
