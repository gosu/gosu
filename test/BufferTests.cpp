#include <gtest/gtest.h>

#include <Gosu/Buffer.hpp>
#include <cstring> // for std::memcmp
#include <filesystem> // for std::filesystem::temp_directory_path

class BufferTests : public testing::Test
{
};

TEST_F(BufferTests, from_existing_memory)
{
    // nullptr with non-zero length.
    ASSERT_THROW(Gosu::Buffer(nullptr, 5, nullptr), std::invalid_argument);

    // Negative size.
    std::uint8_t bytes[] = { 0, 1, 2, 3 };
    ASSERT_THROW(Gosu::Buffer(bytes, -4, nullptr), std::length_error);

    bool deleted = false;
    auto deleter = [&](void* pointer) {
        ASSERT_EQ(pointer, bytes);
        deleted = true;
    };

    // Success!
    Gosu::Buffer buffer(bytes, sizeof bytes, deleter);
    ASSERT_EQ(buffer.data(), bytes);
    ASSERT_EQ(buffer.size(), sizeof bytes);

    // Move into another buffer.
    Gosu::Buffer buffer2 = std::move(buffer);
    ASSERT_EQ(buffer.data(), nullptr); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved)
    ASSERT_EQ(buffer.size(), 0);
    ASSERT_EQ(buffer2.data(), bytes);
    ASSERT_EQ(buffer2.size(), sizeof bytes);

    // Moving into another buffer does not yet delete memory.
    ASSERT_FALSE(deleted);
    // But moving again (overwriting buffer2) will do that.
    buffer2 = std::move(buffer);
    ASSERT_TRUE(deleted);
}

TEST_F(BufferTests, from_size)
{
    ASSERT_THROW(Gosu::Buffer(-3), std::length_error);
    Gosu::Buffer buffer(367);
    ASSERT_EQ(buffer.size(), 367);

    // Assigning a buffer to itself is a no-op.
    buffer = buffer;
    ASSERT_EQ(buffer.size(), 367);
    // So is moving into itself.
    buffer = std::move(buffer);

    // Copying a buffer has to make another copy of the memory.
    Gosu::Buffer buffer2;
    buffer2 = buffer;
    ASSERT_EQ(buffer2.size(), 367);
    ASSERT_NE(buffer.data(), buffer2.data());
}

TEST_F(BufferTests, file_io)
{
    const Gosu::Buffer buffer = Gosu::load_file("test_image_io/alpha-bmp24.bmp");
    ASSERT_GT(buffer.size(), 2);
    ASSERT_EQ(buffer.data()[0], 'B');
    ASSERT_EQ(buffer.data()[1], 'M');

    const std::string temp_filename = std::filesystem::temp_directory_path() / "file_io_test.bin";
    Gosu::save_file(buffer, temp_filename);
    const Gosu::Buffer buffer2 = Gosu::load_file(temp_filename);
    ASSERT_EQ(buffer.size(), buffer2.size());
    ASSERT_EQ(0, std::memcmp(buffer.data(), buffer2.data(), buffer.size()));

    ASSERT_THROW(Gosu::save_file(buffer2, "/does/not/exist"), std::runtime_error);
}
