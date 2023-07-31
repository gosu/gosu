#include <gtest/gtest.h>

#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include "../src/BinPacker.hpp"
#include "../src/TexChunk.hpp"
#include "../src/Texture.hpp"
#include <random>

class TextureTests : public testing::Test
{
};

TEST_F(TextureTests, creation)
{
    ASSERT_THROW(Gosu::Texture(2, -5, false), std::invalid_argument);
    Gosu::Texture texture(128, 256, false);
    ASSERT_EQ(texture.width(), 128);
    ASSERT_EQ(texture.height(), 256);
    ASSERT_EQ(texture.try_alloc(Gosu::Bitmap(150, 150), 4), nullptr);
}

TEST_F(TextureTests, tex_chunks)
{
    const int tex_size = 64;
    const auto texture_ptr = std::make_shared<Gosu::Texture>(tex_size, tex_size, false);

    // Prepare a yellow rectangle with a 1px red border.
    Gosu::Bitmap yellow_with_red_frame(10, 7, Gosu::Color::RED);
    yellow_with_red_frame.insert(Gosu::Bitmap(8, 5, Gosu::Color::YELLOW), 1, 1);

    // Create a TexChunk that refers to the yellow area in the bitmap. (The red border will be
    // ignored because of padding=1).
    const auto chunk_ptr = texture_ptr->try_alloc(yellow_with_red_frame, 1);
    ASSERT_NE(chunk_ptr, nullptr);
    ASSERT_EQ(chunk_ptr->width(), 8);
    ASSERT_EQ(chunk_ptr->height(), 5);

    // Overwrite the bottom 2x2 pixels with green.
    chunk_ptr->insert(Gosu::Bitmap(10, 40, Gosu::Color::GREEN), 6, 3);
    // (No-op)
    chunk_ptr->insert(Gosu::Bitmap(10, 40), -34, 2);

    const Gosu::Bitmap chunk_bitmap = chunk_ptr->to_bitmap();
    ASSERT_EQ(chunk_bitmap.width(), 8);
    ASSERT_EQ(chunk_bitmap.height(), 5);
    for (int y = 0; y < chunk_bitmap.height(); ++y) {
        for (int x = 0; x < chunk_bitmap.width(); ++x) {
            ASSERT_EQ(chunk_bitmap.pixel(x, y),
                      x >= 6 && y >= 3 ? Gosu::Color::GREEN : Gosu::Color::YELLOW);
        }
    }

    const Gosu::GLTexInfo* info = chunk_ptr->gl_tex_info();
    ASSERT_NE(info, nullptr);
    ASSERT_EQ(info->tex_name, texture_ptr->tex_name());
    ASSERT_LT(info->left, info->right);
    ASSERT_LT(info->top, info->bottom);
    const Gosu::Bitmap texture_bitmap = texture_ptr->to_bitmap(Gosu::Rect {
        static_cast<int>(std::round(info->left * tex_size) - 1),
        static_cast<int>(std::round(info->top * tex_size) - 1),
        yellow_with_red_frame.width(),
        yellow_with_red_frame.height(),
    });
    // Simulate the green rectangle insertion on our source bitmap as well.
    yellow_with_red_frame.insert(Gosu::Bitmap(2, 2, Gosu::Color::GREEN), 7, 4);
    // Now the image should have the same content.
    ASSERT_EQ(texture_bitmap, yellow_with_red_frame);

    ASSERT_THROW(texture_ptr->to_bitmap(Gosu::Rect { -4, -6, 10, 5 }), //
                 std::invalid_argument);
    ASSERT_THROW(Gosu::TexChunk(texture_ptr, Gosu::Rect { -3, 5, 10, 10 }, nullptr),
                 std::invalid_argument);
    ASSERT_THROW(Gosu::TexChunk(texture_ptr, Gosu::Rect { 14, 3, 0, 75 }, nullptr),
                 std::invalid_argument);
    ASSERT_THROW(chunk_ptr->subimage(Gosu::Rect { 3, -5, 3, 5 }), //
                 std::invalid_argument);
}

TEST_F(TextureTests, bin_packing_benchmark)
{
    std::random_device rd;
    std::mt19937_64 mt(rd());

    const auto next_size = [&mt] {
        static std::uniform_int_distribution size_distribution(1, 256);
        return size_distribution(mt);
    };
    const auto next_bool = [&mt] {
        static std::uniform_int_distribution bool_distribution(0, 1);
        return bool_distribution(mt) == 1;
    };
    const Gosu::Bitmap bitmap(256, 256, Gosu::Color::RED);

    std::vector<std::unique_ptr<Gosu::Drawable>> images;

    for (int i = 0; i < 5'000; ++i) {
        Gosu::Rect source_rect { 0, 0, next_size(), next_size() };
        if (next_bool()) {
            source_rect.width = source_rect.height;
        }
        unsigned image_flags = 0;
        if (next_bool()) {
            image_flags |= Gosu::IF_RETRO;
        }
        if (next_bool()) {
            image_flags |= Gosu::IF_TILEABLE;
        }
        images.push_back(Gosu::create_drawable(bitmap, source_rect, image_flags));

        if (i % 3 == 0) {
            images.erase(images.begin());
        }
    }
}
