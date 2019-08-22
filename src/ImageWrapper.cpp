#include <Gosu/Gosu.hpp>

extern "C" {
  #include <Gosu/Image.h>

  // Constructors
  Gosu_Image* Gosu_Image_create(const char* filename, unsigned image_flags)
  {
    return reinterpret_cast<Gosu_Image*>( new Gosu::Image(filename, image_flags) );
  };

  // Properties
  int Gosu_Image_width(Gosu_Image* image)
  {
    return reinterpret_cast<Gosu::Image*>( image )->width();
  }

  int Gosu_Image_height(Gosu_Image* image)
  {
    return reinterpret_cast<Gosu::Image*>( image )->height();
  }

  Gosu_GLTexInfo* Gosu_Image_gl_texinfo(Gosu_Image* image)
  {
    Gosu_GLTexInfo* tex_info;
    const Gosu::GLTexInfo* gosu_texture_info = reinterpret_cast<Gosu::Image*>( image )->data().gl_tex_info();

    tex_info->texture_name = gosu_texture_info->tex_name;
    tex_info->left         = gosu_texture_info->left;
    tex_info->right        = gosu_texture_info->right;
    tex_info->top          = gosu_texture_info->top;
    tex_info->bottom       = gosu_texture_info->bottom;
    return tex_info;
  }

  // Rendering
  void Gosu_Image_draw(Gosu_Image* image, double x, double y, double z, unsigned color, int mode)
  {
    reinterpret_cast<Gosu::Image*>( image )->draw(x, y, z, 1.0, 1.0, color, (Gosu::AlphaMode)mode);
  }

  void Gosu_Image_draw_rot(Gosu_Image* image, double x, double y, double z,
                           double angle, double center_x, double center_y,
                           double scale_x, double scale_y, unsigned color, int mode)
  {
    reinterpret_cast<Gosu::Image*>( image )->draw_rot(x, y, z,
                                                      angle, center_x, center_y,
                                                      scale_x, scale_y,
                                                      color, (Gosu::AlphaMode)mode);
  }

  void Gosu_Image_draw_as_quad(Gosu_Image* image,
                               double x1, double y1, unsigned color1,
                               double x2, double y2, unsigned color2,
                               double x3, double y3, unsigned color3,
                               double x4, double y4, unsigned color4,
                               double z, int mode)
  {
    Gosu::Image* gosu_image = reinterpret_cast<Gosu::Image*>( image );
    gosu_image->data().draw(
      x1, y1, color1,
      x2, y2, color2,
      x3, y3, color3,
      x4, y4, color4,
      z, (Gosu::AlphaMode)mode
    );
  }

  // Image operations
  Gosu_Image* Gosu_Image_subimage(Gosu_Image* image, int left, int top, int width, int height)
  {
    Gosu::Image* gosu_image = reinterpret_cast<Gosu::Image*>( image );
    std::unique_ptr<Gosu::ImageData> image_data = gosu_image->data().subimage(left, top, width, height);

    return reinterpret_cast<Gosu_Image*>( image_data.get() ? new Gosu::Image(std::move(image_data)) : nullptr );
  }

  void Gosu_Image_insert(Gosu_Image* image, Gosu_Image* source, int x, int y)
  {
    Gosu::Bitmap bmp;
    bmp = reinterpret_cast<Gosu::Image*>( source )->data().to_bitmap();
    reinterpret_cast<Gosu::Image*>( image )->data().insert(bmp, x, y);
  }

  const char* Gosu_Image_to_blob(Gosu_Image* image)
  {
    Gosu::Image* gosu_image = reinterpret_cast<Gosu::Image*>( image );

    return reinterpret_cast<const char*>( gosu_image->data().to_bitmap().data() );
  }

  void Gosu_Image_save(Gosu_Image* image, const char* filename)
  {
    Gosu::save_image_file(reinterpret_cast<Gosu::Image*>( image )->data().to_bitmap(), filename);
  }

  // Destructor
  void Gosu_Image_destroy(Gosu_Image* image)
  {
    delete( reinterpret_cast<Gosu::Image*>( image ) );
  }
}