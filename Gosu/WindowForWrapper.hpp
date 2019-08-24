#pragma once

namespace Gosu
{
  class WindowForWrapper : public Gosu::Window
  {
  public:
      WindowForWrapper(int width, int height, bool fullscreen, double update_interval, bool resizable);
      void set_draw(void function());
      void set_update(void function());
  };
}