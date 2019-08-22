#include <Gosu/Gosu.hpp>

extern "C" {
  #include <Gosu/Image.h>

  Gosu_Image* Gosu_Image_create(char* filename, unsigned image_flags)
  {
    return reinterpret_cast<Gosu_Image*>( new Gosu::Image(std::string(filename), image_flags) );
  };

  void Gosu_Image_save(Gosu_Image* image, char* filename)
  {
    Gosu::save_image_file(reinterpret_cast<Gosu::Image*>( image )->data().to_bitmap(), filename);
  }

  void Gosu_Image_destroy(Gosu_Image* image)
  {
    delete( reinterpret_cast<Gosu::Image*>( image ) );
  }
}