// Undocumented for the first few iterations. Interface may change rapidly.
// This is mainly a proof of concept. Stability will be the highest on OS X.

#ifndef GOSU_ASYNC_HPP
#define GOSU_ASYNC_HPP

#include <Gosu/Fwd.hpp>
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
            return lock && result->get();
        }
        
        std::auto_ptr<Result> takeValue()
        {
            boost::try_mutex::scoped_lock lock(*mutex);
            return *result;
        }
    };
    
	// TODO: Will only work if the window doesn't die inbetween.
	// TODO: More functions to come; or a general interface?

	AsyncResult<Image>
		asyncNewImage(Window& window, const std::wstring& filename);
}

#endif
