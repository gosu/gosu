class ::Numeric
  def degrees_to_radians
    self * Math::PI / 180.0
  end

  def radians_to_degrees
    self * 180.0 / Math::PI
  end

  def gosu_to_radians
    (self - 90) * Math::PI / 180.0
  end

  def radians_to_gosu
    self * 180.0 / Math::PI + 90
  end
end
