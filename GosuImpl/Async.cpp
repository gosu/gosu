#include <Gosu/Async.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/TR1.hpp>
#include <Gosu/Window.hpp>

using namespace std;
using namespace std::tr1;

namespace Gosu
{
    namespace
    {
        void asyncNewImage_Impl(Window& window, std::wstring filename,
                            Window::SharedContext context,
                            shared_ptr<try_mutex> mutex,
                            shared_ptr<std::auto_ptr<Image> > result)
        {
            try_mutex::scoped_lock lock(*mutex);
            (*context)();
            result->reset(new Image(window.graphics(), filename));
        }
    }
}

Gosu::AsyncResult<Gosu::Image>
    Gosu::asyncNewImage(Window& window, const std::wstring& filename)
{
	shared_ptr<try_mutex> mutex(new try_mutex);
	shared_ptr<auto_ptr<Image> > image(new std::auto_ptr<Image>);
	thread thread(bind(asyncNewImage_Impl,
						ref(window),
                        filename,
						window.createSharedContext(),
						mutex,
                        image));
	return AsyncResult<Image>(mutex, image);
}
