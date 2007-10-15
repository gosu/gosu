// Undocumented for the first few iterations. Interface may change rapidly.

#ifndef GOSU_ASYNC_HPP
#define GOSU_ASYNC_HPP

#include <Gosu/Fwd.hpp>
#include <boost/bind.hpp>
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
    
    void asyncNewImage_Impl(Graphics& graphics, std::wstring filename,
                            boost::shared_ptr<boost::try_mutex> mutex,
                            boost::shared_ptr<std::auto_ptr<Image> > result)
    {
        boost::try_mutex::scoped_lock lock(*mutex);
        result->reset(new Gosu::Image(graphics, filename));
        // TODO: Corner case: Will crash if graphics dies before operation finished.
    }
    
    AsyncResult<Image> asyncNewImage(Graphics& graphics, const std::wstring& filename)
    {
        boost::shared_ptr<boost::try_mutex> mutex(new boost::try_mutex);
        boost::shared_ptr<std::auto_ptr<Image> > image(new std::auto_ptr<Image>);
        boost::thread thread(boost::bind(asyncNewImage_Impl,
                                         boost::ref(graphics), filename,
                                         boost::ref(mutex), boost::ref(image)));
    }
}

#endif
