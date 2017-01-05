//! \file IO.hpp
//! Contains everything related to input and output.

#pragma once

#include <Gosu/Platform.hpp>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace Gosu
{
    class Resource;
    
    enum ByteOrder { BO_LITTLE, BO_BIG, BO_DONT_CARE };
#ifdef __BIG_ENDIAN__
    const ByteOrder BO_NATIVE = BO_BIG, BO_OTHER = BO_LITTLE;
#else
    const ByteOrder BO_NATIVE = BO_LITTLE, BO_OTHER = BO_BIG;
#endif

    //! Utility class that points to a specific position in a resource
    //! and offers an interface for sequential reading.
    class Reader
    {
        const Resource* res;
        std::size_t pos;

    public:
        Reader(const Resource& resource, std::size_t position)
        : res(&resource), pos(position)
        {
        }

        const Resource& resource() const
        {
            return *res;
        }

        std::size_t position() const
        {
            return pos;
        }

        void set_position(std::size_t value)
        {
            // TODO: Check?
            pos = value;
        }

        void seek(std::ptrdiff_t offset)
        {
            // TODO: Check?
            pos += offset;
        }

        void read(void* dest_buffer, std::size_t length);
        
        //! Convenience function; equivalent to read(&t, sizeof t).
        template<typename T>
        void read_pod(T& t, ByteOrder bo = BO_DONT_CARE)
        {
            read(&t, sizeof t);
            if (bo == BO_OTHER) {
                char* begin = reinterpret_cast<char*>(&t);
                std::reverse(begin, begin + sizeof t);
            }
        }

        //! Similar to read_pod(T&), but returns the read value instead.
        template<typename T>
        T get_pod(ByteOrder bo = BO_DONT_CARE)
        {
            T t;
            read_pod<T>(t, bo);
            return t;
        }
    };
    
    //! Utility class that points to a specific position in a resource
    //! and offers an interface for sequential writing.
    class Writer
    {
        Resource* res;
        std::size_t pos;

    public:
        Writer(Resource& resource, std::size_t position)
        : res(&resource), pos(position)
        {
        }

        Resource& resource() const
        {
            return *res;
        }

        std::size_t position() const
        {
            return pos;
        }

        void set_position(std::size_t value)
        {
            // TODO: Check?
            pos = value;
        }

        void seek(std::ptrdiff_t offset)
        {
            // TODO: Check?
            pos += offset;
        }

        void write(const void* source_buffer, std::size_t length);

        //! Convenience function; equivalent to write(&t, sizeof t).
        template<typename T>
        void write_pod(const T& t, ByteOrder bo = BO_DONT_CARE)
        {
            if (bo == BO_OTHER) {
                char buf[sizeof t];
                const char* begin = reinterpret_cast<const char*>(&t);
                std::reverse_copy(begin, begin + sizeof t, buf);
                write(buf, sizeof buf);
            }
            else {
                write(&t, sizeof t);
            }
        }
    };

    //! Base class for resources. A resource in Gosu is nothing more but a
    //! piece of binary data that can be read or written, for example files
    //! or simply areas of allocated memory.
    //! A resource always knows its size and can resize itself, thereby either
    //! truncating its content or allocating room for more data.
    class Resource
    {
        // Non-copyable and non-movable to avoid slicing.
        Resource(const Resource&) = delete;
        Resource& operator=(const Resource&) = delete;
        Resource(Resource&&) = delete;
        Resource& operator=(Resource&&) = delete;

    public:
        Resource()
        {
        }
        
        virtual ~Resource()
        {
        }

        //! Convenience: Creates a new Reader that reads from the start of
        //! the resource.
        Reader front_reader() const
        {
            return Reader(*this, 0);
        }

        //! Convenience: Creates a new Writer that appends data at the
        //! end of the resource.
        Writer back_writer()
        {
            return Writer(*this, size());
        }

        virtual std::size_t size() const = 0;

        virtual void resize(std::size_t new_size) = 0;

        virtual void read(std::size_t offset, std::size_t length, void* dest_buffer) const = 0;

        virtual void write(std::size_t offset, std::size_t length, const void* source_buffer) = 0;
    };

    //! Piece of memory with the Resource interface.
    class Buffer : public Resource
    {
        std::vector<char> buf;

    public:
        Buffer()
        {
        }

        Buffer(const Buffer& other)
        : Resource()
        , buf(other.buf)
        {
        }

        Buffer& operator=(const Buffer& other)
        {
            buf = other.buf;
            return *this;
        }

        std::size_t size() const override;

        void resize(std::size_t new_size) override;

        void read(std::size_t offset, std::size_t length, void* dest_buffer) const override;
        
        void write(std::size_t offset, std::size_t length, const void* source_buffer) override;

        const void* data() const
        {
            return &buf[0];
        }

        void* data()
        {
            return &buf[0];
        }
    };

    enum FileMode
    {
        //! Opens an existing file for reading; throws an exception if the file
        //! cannot be found.
        FM_READ,
        //! Writes data to a file. If the file already exists, is emptied on
        //! opening. If the file does not exist, it is created.
        FM_REPLACE,
        //! Opens or creates a file with writing access, but does not clear
        //! existing contents.
        FM_ALTER
    };

    //! File with the Resource interface.
    class File : public Resource
    {
        struct Impl;
        const std::unique_ptr<Impl> pimpl;

    public:
        explicit File(const std::string& filename, FileMode mode = FM_READ);
        ~File();

        std::size_t size() const override;
        
        void resize(std::size_t new_size) override;
        
        void read(std::size_t offset, std::size_t length, void* dest_buffer) const override;
        
        void write(std::size_t offset, std::size_t length, const void* source_buffer) override;
    };

    //! Loads a whole file into a buffer.
    void load_file(Buffer& buffer, const std::string& filename);
    //! Creates or overwrites a file with the contents of a buffer.
    void save_file(const Buffer& buffer, const std::string& filename);
}
