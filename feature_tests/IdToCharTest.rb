$LOAD_PATH << '../lib'
require 'gosu'

class Test < Gosu::Window
  def needs_cursor?; true; end
  
  def initialize 
    super(300, 200, false, 20)
  end
    
  def button_down(id)
    self.caption = "#{id} = #{button_id_to_char(id)} / #{(button_id_to_char(id)||"\0")[0]} = #{char_to_button_id(button_id_to_char(id))}"
    name = Gosu.constants.grep(/^Kb/).find { |name| Gosu.const_get(name) == id }
    self.caption += " called: #{name}" unless name.nil?
  end
end

Test.new.show
