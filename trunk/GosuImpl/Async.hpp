// Undocumented. Interface may change rapidly.

#ifndef GOSU_ASYNC_HPP
#define GOSU_ASYNC_HPP

#include <Gosu/Fwd.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <string>

namespace Gosu
{
    template<typename Result>
    class AsyncResult
    {
        boost::shared_ptr<boost::thread> thread;
        
    public:
        
        
        bool isReady() const;
        
        std::auto_ptr<Result> takeValue()
        {
            thread->join();
            
        }
    };
    
    AsyncResult<Image> asyncNewImage(Graphics& graphics, const std::wstring& filename)
    {
    }
}

#endif
