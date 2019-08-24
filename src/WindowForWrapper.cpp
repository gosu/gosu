#include <Gosu/Gosu.hpp>
#include <Gosu/WindowForWrapper.hpp>

namespace Gosu
{
  class GosuWindowForWrapper : Gosu::Window
  {
    struct Callbacks
    {
      void* update;
      void* draw;
    } callbacks;

    public:
      GosuWindowForWrapper(int width, int height, bool fullscreen, double update_interval, bool resizable)
      : Window(width, height, fullscreen, update_interval, resizable)
      {
      }

      void set_update(void function())
      {
        callbacks.update = &function;
      }

      void set_draw(void function())
      {
        callbacks.draw = &function;
      }

      void update() override
      {
        if (callbacks.update != nullptr)
        {
          callbacks.update;
        }
      }

      void draw() override
      {
        if (callbacks.draw != nullptr)
        {
          callbacks.draw;
        }
      }
  };
}