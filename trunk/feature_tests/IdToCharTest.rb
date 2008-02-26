require '../gosu'

class Test < Gosu::Window
  def initialize 
    super(300, 200, false, 20)
  end
    
  def button_down(id)        
    self.caption = "#{id} = #{button_id_to_char(id)} = #{char_to_button_id(button_id_to_char(id))}"
  end
end

Test.new.show
