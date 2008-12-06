require '../lib/gosu'

class Test < Gosu::Window
  def initialize
    super(800, 600, false)
    
    Gosu::Image.new(self, "Umläuts.png", false) rescue nil
    
    self.caption = "Huhu, Ümläuts!"
    @font = Gosu::Font.new(self, "Monaco", 50)
    @sys_text = Gosu::Image.from_text(self, "Huhu, 地下鉄chikatetsuでde行きますikimasu!", "Helvetica", 15)
    @loc_text = Gosu::Image.from_text(self, "Huhu, 地下鉄で行きます", "media/Vera.ttf", 30)
    @japanese_text = Gosu::Image.from_text(self, "Achtung, hier kommts: " +
                                                 "このテクストはスペエクがありません。このテクストはスペエクがありません。このテクストはスペエクがありません。" +
                                                 "このテクストはスペエクがありません。このテクストはスペエクがありません。このテクストはスペエクがありません。" +
                                                 "このテクストはスペエクがありません。このテクストはスペエクがありません。このテクストはスペエクがありません。",
                                                 Gosu::default_font_name, 30, 0, 200, :center)
  end
  
  def draw
    draw_line(100, 300, 0xffffffff, 300, 300, 0xffffffff, 255)
    draw_line(300, 304, 0xffffffff, 100, 304, 0xffffffff, 255)
    @sys_text.draw(100, 100, 0)
    @loc_text.draw(400, 100, 0)
    @japanese_text.draw(100, 400, 0)
    @font.draw(('A'..'Z').to_a.join + ('a'..'z').to_a.join, 10, 10, 0)
    @font.draw_rel("Alfons läuft durch Bayern und fühlt sich pudelwohl dabei! Na sowas.", 800, 600, 0, 0.5, 0.5)
  end
  
  def button_down(id)
    if id == Gosu::KbEscape or id == char_to_button_id("ä") then
      close
    else
      @text ||= ""
      @text += button_id_to_char(id)
      self.caption = @text
    end
  end
end

Test.new.show
