# Encoding: UTF-8
require_relative 'test_helper'

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

# This undoes the hack in rdoc/gosu.rb where constants like :KB_F1…KB_F12 are defined to keep the
# generated rdoc short. Given this constant, it would return [:KB_F1, ..., :KB_F12].
def unpack_range(constant)
  case constant
  when /^GP_(\d+)_(.+)…GP_(\d+)_(.+)$/
    # Special case: GP_0_BUTTON_0…GP_3_BUTTON_15 needs to be unpacked into 4 * 16 constants.
    ($1..$3).map { |n| ($2..$4).map { |suffix| :"GP_#{n}_#{suffix}" } }
  when /^KB_F(\d+)…KB_F(\d+)$/
    # Special case: The range from "F1" to "F12" is not what we want: "F9".succ == "G0".
    ($1..$2).map { |n| :"KB_F#{n}"}
  when /^(.+)…(.+)$/
    ($1..$2).map { |c| c.to_sym }
  else
    [constant]
  end
end

class TestInterface < Minitest::Test
  DOCUMENTED_CONSTANTS = GosuDocs.constants.map { |constant| unpack_range(constant) }.flatten

  def test_all_constants_exist
    DOCUMENTED_CONSTANTS.each do |constant|
      # There should be no deprecated constants in the docs
      assert_silent do
        assert Gosu.const_defined?(constant), "Expected constant Gosu::#{constant}"
      end
    end
  end
  
  def test_constant_types
    DOCUMENTED_CONSTANTS.each do |constant|
      case constant
      when /(KB_|GP_|MS_)/, /_VERSION/
        expected_class = Integer
      when :VERSION, :LICENSES
        expected_class = String
      else
        next
      end

      assert_kind_of expected_class, Gosu.const_get(constant),
        "Gosu::#{constant} must be #{expected_class}, but is #{Gosu.const_get(constant).class}"
    end
  end
  
  def test_no_extra_constants
    Gosu.constants.each do |constant|
      next if constant == :Button # backwards compatibility
      next if constant == :SampleInstance # backwards compatibility
      next if constant =~ /Kb|Gp|Ms/ # backwards compatibility
      next if constant == :GOSU_COPYRIGHT_NOTICE # backwards compatibility
      next if constant == :DEPRECATION_STACKTRACE_LINES # implementation detail for backwards compatibility
      next if constant == :ImmutableColor # implementation detail
      next if constant == :MAX_TEXTURE_SIZE # not sure if we still need this :/
      
      assert DOCUMENTED_CONSTANTS.include?(constant), "Unexpected Gosu::#{constant}"
    end
  end
  
  def test_methods_exist
    GosuDocs.constants.each do |constant|
      doc_class = GosuDocs.const_get(constant)
      next unless doc_class.is_a? Class
      
      real_class = Gosu.const_get(constant)
      
      doc_class.public_methods(false).each do |method|
        assert real_class.public_methods.include?(method), "Gosu::#{constant}.#{method} missing"
      end

      doc_class.instance_methods(false).each do |method|
        assert real_class.instance_methods.include?(method), "Gosu::#{constant}##{method} missing"
      end
    end
  end
  
  # Every string argument in Gosu must accept numbers and nil (= empty string) for backwards
  # compatibility.
  # https://www.reddit.com/r/gosu/comments/5t79pu/expected_argument_1_of_type_stdstring_const_but
  # This could easily be deprecated later...
  def test_everything_is_a_string
    font = Gosu::Font.new(20)
    assert_equal font.text_width(1234), font.text_width("1234")
    assert_equal font.text_width(nil), font.text_width("")
  end
end
