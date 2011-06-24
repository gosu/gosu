# Encoding: UTF-8

$LOAD_PATH << '../lib'
require 'gosu'

class Test < Gosu::Window
  def initialize
    super(800, 700, false)
    
    Gosu::Image.new(self, "Umläuts.png", false) rescue nil
    
    self.caption = "Huhu, Ümläuts!"
    @font = Gosu::Font.new(self, "Monaco", 50)
    @sys_text = Gosu::Image.from_text(self, "Huhu, 地下鉄chika\ntetsuでde行きますikimasu!", "Helvetica", 15)
    @loc_text = Gosu::Image.from_text(self, "Huhu, 地下鉄で行きます", "media/Vera.ttf", 30)
    @german_text = Gosu::Image.from_text(self, "Unicode [ˈjuːnɪkoʊd] ist ein internationaler Standard, in dem langfristig für jedes sinntragende Schriftzeichen oder Textelement aller bekannten Schriftkulturen und Zeichensysteme ein digitaler Code festgelegt wird. Ziel ist es, die Verwendung unterschiedlicher und inkompatibler Kodierungen in verschiedenen Ländern oder Kulturkreisen zu beseitigen. Unicode wird laufend um Zeichen weiterer Schriftsysteme ergänzt. ISO 10646 ist die von ISO verwendete praktisch bedeutungsgleiche Bezeichnung des Unicode-Zeichensatzes; er wird dort als Universal Character Set (UCS) bezeichnet.",
                                               Gosu::default_font_name, 20, 0, 300, :center)
    @japanese_text = Gosu::Image.from_text(self, "Unicode（ユニコード）とはコンピュータ上で多言語の文字を単一の文字コードで取り扱うために1980年代に提唱された文字コードである。" +
                                                 "ゼロックス社が提唱し、マイクロソフト、アップル、IBM、サン・マイクロシステムズ、ヒューレット・パッカード、ジャストシステムなどが参加するユニコードコンソーシアムにより作られた。1993年にISOでもISO/IEC 10646の一部として標準化されたが、Unicodeと完全に同一の規格ではない。",
                                                 Gosu::default_font_name, 20, 0, 300, :justify)
  end
  
  def draw
    @sys_text.draw(100, 100, 0)
    @loc_text.draw(400, 100, 0)
    @japanese_text.draw(50, 250, 0)
    @german_text.draw(450, 250, 0)
    @font.draw(('A'..'Z').to_a.join + ('a'..'z').to_a.join, 10, 10, 0)
    @font.draw_rel("Alfons läuft durch Bayern und fühlt sich pudelwohl dabei! Na sowas.", 800, 800, 0, 0.5, 0.5)
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
