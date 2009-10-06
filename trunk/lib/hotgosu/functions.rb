module Gosu
  def self.random(min, max)
    ::ObjGosu.randomFrom min, to: max
  end
  
  def self.offset_x(angle, dist)
    ::ObjGosu.offsetXWithAngle angle, distance: dist
  end
  
  def self.offset_y(angle, dist)
    ::ObjGosu.offsetYWithAngle angle, distance: dist
  end
  
  def self.angle(x1, y1, x2, y2, default = 0)
    ::ObjGosu.angleFromX x1, y: y1, toX: x2, y: y2, default: default
  end
  
  def self.angle_diff(angle1, angle2)
    ::ObjGosu.angleDiffBetween angle1, andOtherAngle: angle2
  end
  
  def self.distance(x1, y1, x2, y2)
    ::ObjGosu.distanceFromX x1, y: y1, toX: x2, y: y2
  end
  
  def self.milliseconds
    ::ObjGosu.milliseconds
  end
  
  def self.default_font_name
    ::ObjGosu.defaultFontName
  end
  
  def self.screen_width
    ::ObjGosu.screenWidth
  end
  
  def self.screen_height
    ::ObjGosu.screenHeight
  end
end
