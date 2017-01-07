require "minitest/autorun"
require "gosu" unless defined? Gosu

# Make a backup of the Gosu modules and its helpers on Numeric.
OrigGosu = Gosu
Object.send :remove_const, :Gosu
%w(gosu_to_radians radians_to_gosu degrees_to_radians radians_to_degrees).each do |helper|
  Numeric.send :alias_method, :"orig_#{helper}", :"#{helper}"
  Numeric.send :undef_method, :"#{helper}"
end

# Now load Gosu's documentation, a module that is only filled with method stubs, into GosuDocs.
require_relative "../rdoc/gosu.rb"
GosuDocs = Gosu
Object.send :remove_const, :Gosu

# And finally, restore the real Gosu module.
Gosu = OrigGosu
Object.send :remove_const, :OrigGosu
%w(gosu_to_radians radians_to_gosu degrees_to_radians radians_to_degrees).each do |helper|
  Numeric.send :undef_method, :"#{helper}"
  Numeric.send :alias_method, :"#{helper}", :"orig_#{helper}"
  Numeric.send :undef_method, :"orig_#{helper}"
end

class TestInterface < Minitest::Test
  def test_all_constants_exist
    GosuDocs.constants.each do |constant|
      assert Gosu.constants.include?(constant),
        "Expected constant Gosu::#{constant}"
    end
  end
  
  def test_constant_types
    GosuDocs.constants.each do |constant|
      case constant
      when /(Kb|Gp|Ms)/, /(KB_|GP_|MS_)/, /_VERSION/
        expected_class = Integer
      when :VERSION, :LICENSES
        expected_class = String
      else
        next
      end

      assert_kind_of expected_class, Gosu.const_get(constant),
        "Gosu::#{constant} must be #{expected_class}, is #{Gosu.const_get(constant).class}"
    end
  end
  
  def test_no_extra_constants
    Gosu.constants.each do |constant|
      next if constant =~ /KB_|GP_|MS_/ # TODO: not yet documented
      
      next if constant == :Button # backwards compatibility
      
      next if constant == :ImmutableColor # implementation detail
      
      next if constant == :MAX_TEXTURE_SIZE # not sure if we still need this :/
      
      assert GosuDocs.constants.include?(constant),
        "Unexpected Gosu::#{constant}"
    end
  end
end
