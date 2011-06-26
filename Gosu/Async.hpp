// Undocumented for the first few iterations. Interface may change rapidly.
// This is mainly a proof of concept. Stability will be the highest on OS X.

#if 0
#ifndef GOSU_ASYNC_HPP
#define GOSU_ASYNC_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/TR1.hpp>
#include <thread>
#include <memory>
#include <string>

namespace Gosu
{
    template<typename Result>
    class AsyncResult
    {
        std::tr1::shared_ptr<std::tr1::try_mutex> mutex;
        std::tr1::shared_ptr<std::auto_ptr<Result> > result;
        
    public:
        AsyncResult(const std::tr1::shared_ptr<std::tr1::try_mutex>& mutex,
                    const std::tr1::shared_ptr<std::auto_ptr<Result> >& result)
        : mutex(mutex), result(result)
        {
        }
        
        bool hasValue() const
        {
            std::tr1::try_mutex::scoped_try_lock lock(*mutex);
            return lock && result->get();
        }
        
        std::auto_ptr<Result> takeValue()
        {
            std::tr1::try_mutex::scoped_lock lock(*mutex);
            return *result;
        }
    };
    
	// TODO: Will only work if the window doesn't die inbetween.
	// TODO: More functions to come; or a general interface?

	AsyncResult<Image>
		asyncNewImage(Window& window, const std::wstring& filename);
}

#endif
#endif
