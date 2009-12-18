# Mac-specific constants extracted from SWIG port via:

# Gosu.constants.sort.each do |name|
#   value = eval "Gosu::#{name}"
#   puts "  #{name} = #{value.inspect}" unless value.is_a? Module
# end

module Gosu
  GpButton0 = 65546
  GpButton1 = 65547
  GpButton10 = 65556
  GpButton11 = 65557
  GpButton12 = 65558
  GpButton13 = 65559
  GpButton14 = 65560
  GpButton15 = 65561
  GpButton2 = 65548
  GpButton3 = 65549
  GpButton4 = 65550
  GpButton5 = 65551
  GpButton6 = 65552
  GpButton7 = 65553
  GpButton8 = 65554
  GpButton9 = 65555
  GpDown = 65545
  GpLeft = 65542
  GpNum = 20
  GpRangeBegin = 65542
  GpRangeEnd = 65561
  GpRight = 65543
  GpUp = 65544
  Kb0 = 29
  Kb1 = 18
  Kb2 = 19
  Kb3 = 20
  Kb4 = 21
  Kb5 = 23
  Kb6 = 22
  Kb7 = 26
  Kb8 = 28
  Kb9 = 25
  KbA = 0
  KbB = 11
  KbBackspace = 51
  KbC = 8
  KbD = 2
  KbDelete = 117
  KbDown = 125
  KbE = 14
  KbEnd = 119
  KbEnter = 76
  KbEscape = 53
  KbF = 3
  KbF1 = 122
  KbF10 = 109
  KbF11 = 103
  KbF12 = 111
  KbF2 = 120
  KbF3 = 99
  KbF4 = 118
  KbF5 = 96
  KbF6 = 97
  KbF7 = 98
  KbF8 = 100
  KbF9 = 101
  KbG = 5
  KbH = 4
  KbHome = 115
  KbI = 34
  KbInsert = 114
  KbJ = 38
  KbK = 40
  KbL = 37
  KbLeft = 123
  KbLeftAlt = 58
  KbLeftControl = 59
  KbLeftMeta = 55
  KbLeftShift = 56
  KbM = 46
  KbN = 45
  KbNum = 65535
  KbNumpad0 = 82
  KbNumpad1 = 83
  KbNumpad2 = 84
  KbNumpad3 = 85
  KbNumpad4 = 86
  KbNumpad5 = 87
  KbNumpad6 = 88
  KbNumpad7 = 89
  KbNumpad8 = 91
  KbNumpad9 = 92
  KbNumpadAdd = 69
  KbNumpadDivide = 75
  KbNumpadMultiply = 67
  KbNumpadSubtract = 78
  KbO = 31
  KbP = 35
  KbPageDown = 121
  KbPageUp = 116
  KbQ = 12
  KbR = 15
  KbRangeBegin = 1
  KbRangeEnd = 65535
  KbReturn = 36
  KbRight = 124
  KbRightAlt = 61
  KbRightControl = 62
  KbRightMeta = 54
  KbRightShift = 60
  KbS = 1
  KbSpace = 49
  KbT = 17
  KbTab = 48
  KbU = 32
  KbUp = 126
  KbV = 9
  KbW = 13
  KbX = 7
  KbY = 16
  KbZ = 6
  MAJOR_VERSION = 0
  MINOR_VERSION = 7
  MsLeft = 65536
  MsMiddle = 65538
  MsNum = 6
  MsRangeBegin = 65536
  MsRangeEnd = 65541
  MsRight = 65537
  MsWheelDown = 65540
  MsWheelUp = 65539
  NoButton = -1
  NumButtons = 65562
  POINT_VERSION = 16
  VERSION = "0.7.16"
end

module Gosu
  module Button
    Gosu.constants.grep(/^(Gp|Ms|Kb)/).each do |constant|
      self.const_set constant, Gosu.const_get(constant)
    end
  end
end
