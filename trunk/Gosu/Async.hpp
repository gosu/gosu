// Undocumented for the first few iterations. Interface may change rapidly.
#ifdef GOSU_WITH_ASYNC

#ifndef GOSU_ASYNC_HPP
#define GOSU_ASYNC_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Window.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <string>

namespace Gosu
{
    template<typename Result>
    class AsyncResult
    {
        boost::shared_ptr<boost::try_mutex> mutex;
        boost::shared_ptr<std::auto_ptr<Result> > result;
        
    public:
        AsyncResult(const boost::shared_ptr<boost::try_mutex>& mutex,
                    const boost::shared_ptr<std::auto_ptr<Result> >& result)
        : mutex(mutex), result(result)
        {
        }
        
        bool hasValue() const
        {
            boost::try_mutex::scoped_try_lock lock(*mutex);
            return lock.locked() && result->get();
        }
        
        std::auto_ptr<Result> takeValue()
        {
            boost::try_mutex::scoped_lock lock(*mutex);
            return *result;
        }
    };
    
    void asyncNewImage_Impl(Window& window, std::wstring filename,
                            boost::shared_ptr<boost::try_mutex> mutex,
                            boost::shared_ptr<std::auto_ptr<Image> > result)
    {
        boost::try_mutex::scoped_lock lock(*mutex);
        Bitmap bmp = quickLoadBitmap(filename);
        boost::mutex::scoped_lock lock2(window.graphics().mutex());
        window.makeCurrent();
        result->reset(new Image(window.graphics(), filename));

        // TODO: Corner case: Will crash if graphics dies before operation finished.
    }
    
    AsyncResult<Image> asyncNewImage(Window& window, const std::wstring& filename)
    {
        boost::shared_ptr<boost::try_mutex> mutex(new boost::try_mutex);
        boost::shared_ptr<std::auto_ptr<Image> > image(new std::auto_ptr<Image>);
        boost::thread thread(boost::bind(asyncNewImage_Impl,
                                         boost::ref(window), filename,
                                         boost::ref(mutex), boost::ref(image)));
        return AsyncResult<Image>(mutex, image);
    }
}

#endif

#endif
