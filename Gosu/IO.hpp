//! \file IO.hpp
//! Contains everything related to input and output.

#ifndef GOSU_IO_HPP
#define GOSU_IO_HPP

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include <cstddef>
#include <algorithm>
#include <string>
#include <vector>

namespace Gosu
{
    class Resource;
    
    enum ByteOrder { boLittle, boBig, boDontCare };
#ifdef __BIG_ENDIAN__
    const ByteOrder nativeByteOrder = boBig, otherByteOrder = boLittle;
#else
    const ByteOrder nativeByteOrder = boLittle, otherByteOrder = boBig;
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

        void setPosition(std::size_t value)
        {
            // TODO: Check?
            pos = value;
        }

        void seek(std::ptrdiff_t offset)
        {
            // TODO: Check?
            pos += offset;
        }

        void read(void* destBuffer, std::size_t length);
        
        //! Convenience function; equivalent to read(&t, sizeof t).
        template<typename T>
        void readPod(T& t, ByteOrder bo = boDontCare)
        {
            read(&t, sizeof t);
            if (bo == otherByteOrder)
            {
                char* begin = reinterpret_cast<char*>(&t);
                std::reverse(begin, begin + sizeof t);
            }
        }

        //! Similar to readPod(T&), but returns the read value instead.
        template<typename T>
        T getPod(ByteOrder bo = boDontCare)
        {
            T t;
            readPod<T>(t, bo);
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

        void setPosition(std::size_t value)
        {
            // TODO: Check?
            pos = value;
        }

        void seek(std::ptrdiff_t offset)
        {
            // TODO: Check?
            pos += offset;
        }

        void write(const void* sourceBuffer, std::size_t length);

        //! Convenience function; equivalent to write(&t, sizeof t).
        template<typename T>
        void writePod(const T& t, ByteOrder bo = boDontCare)
        {
            if (bo == otherByteOrder)
            {
                char buf[sizeof t];
                const char* begin = reinterpret_cast<const char*>(&t);
                std::reverse_copy(begin, begin + sizeof t, buf);
                write(buf, sizeof buf);
            }
            else
                write(&t, sizeof t);
        }
    };

    //! Base class for resources. A resource in Gosu is nothing more but a
    //! piece of binary data that can be read or written, for example files
    //! or simply areas of allocated memory.
    //! A resource always knows its size and can resize itself, thereby either
    //! truncating its content or allocating room for more data.
    class Resource : boost::noncopyable
    {
    public:
        virtual ~Resource()
        {
        }

        //! Convenience: Creates a new Reader that reads from the start of
        //! the resource.
        Reader frontReader() const
        {
            return Reader(*this, 0);
        }

        //! Convenience: Creates a new Writer that appends data at the
        //! end of the resource.
        Writer backWriter()
        {
            return Writer(*this, size());
        }

        virtual std::size_t size() const = 0;

        virtual void resize(std::size_t newSize) = 0;

        virtual void read(std::size_t offset, std::size_t length,
            void* destBuffer) const = 0;

        virtual void write(std::size_t offset, std::size_t length,
            const void* sourceBuffer) = 0;
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
        : buf(other.buf)
        {
        }

        Buffer& operator=(const Buffer& other)
        {
            buf = other.buf;
            return *this;
        }

        std::size_t size() const;
        void resize(std::size_t newSize);

        void read(std::size_t offset, std::size_t length,
            void* destBuffer) const;

        void write(std::size_t offset, std::size_t length,
            const void* sourceBuffer);

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
        fmRead,
        //! Writes data to a file. If the file already exists, is emptied on
        //! opening. If the file does not exist, it is created.
        fmReplace,
        //! Opens or creates a file with writing access, but does not clear
        //! existing contents.
        fmAlter
    };

    //! File with the Resource interface.
    class File : public Resource
    {
        struct Impl;
        boost::scoped_ptr<Impl> pimpl;

    public:
        explicit File(const std::wstring& filename, FileMode mode = fmRead);
        ~File();

        std::size_t size() const;
        void resize(std::size_t newSize);
        void read(std::size_t offset, std::size_t length,
            void* destBuffer) const;
        void write(std::size_t offset, std::size_t length,
            const void* sourceBuffer);
    };

    //! Loads a whole file into a buffer.
    void loadFile(Buffer& buffer, const std::wstring& filename);
    //! Creates or overwrites a file with the contents of a buffer.
    void saveFile(const Buffer& buffer, const std::wstring& filename);
}

#endif
