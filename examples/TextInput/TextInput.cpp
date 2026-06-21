// This example demonstrates the use of the TextInput functionality.
// The user can click into the text fields and change their contents, or use the tab key to cycle
// through them.

// To start processing keyboard text input, you need to create a new TextInput instance and
// pass it to your window's Input object via set_text_input.
// Until you call this function again with a nullptr value, the TextInput object will then
// build a text that can be accessed via TextInput::text().

// A TextInput object has no built-in UI. In this example, we create a subclass that has a position
// and renders a rectangular input area.

#include <Gosu/Gosu.hpp>
#include <array>
#include <memory>
#include <string>

class TextField : public Gosu::TextInput
{
    Gosu::Window& m_window;
    Gosu::Font m_font;
    double m_x, m_y;

public:
    // Some constants that define the appearance of text fields.
    inline static const Gosu::Color INACTIVE_COLOR = 0xcc'666666;
    inline static const Gosu::Color ACTIVE_COLOR = 0xcc'ff6666;
    inline static const Gosu::Color SELECTION_COLOR = 0xcc'0000ff;
    inline static const Gosu::Color CARET_COLOR = 0xff'ffffff;
    inline static const int PADDING = 5;

    TextField(Gosu::Window& window, Gosu::Font font, double x, double y)
    : m_window{window},
      m_font{std::move(font)},
      m_x{x},
      m_y{y}
    {
        // Start with a self-explanatory text in each field.
        set_text("Click to change text");
    }

    // Example filter member function: Replace numbers by fancy Unicode numbers in circles.
    std::string filter(std::string str) const override
    {
        std::array<std::string, 10> numbers_in_circles{"⓪", "①", "②", "③", "④",
                                                       "⑤", "⑥", "⑦", "⑧", "⑨"};
        std::string::size_type index = 0;
        while ((index = str.find_first_of("0123456789", index)) != std::string::npos) {
            // Replace all numbers with Unicode "number in circle" characters.
            str.replace(index, 1, numbers_in_circles.at(std::stoi(str.substr(index, 1))));
        }
        return str;
    }

    void draw() const
    {
        Gosu::Color background_color = INACTIVE_COLOR;
        // Highlight this text field if it is currently selected.
        if (m_window.input().text_input() == this) {
            background_color = ACTIVE_COLOR;
        }

        Gosu::draw_rect(m_x - PADDING, m_y - PADDING, width() + 2 * PADDING, height() + 2 * PADDING,
                        background_color, 0);

        // Calculate the position of the caret and the selection start.
        double pos_x = m_x + m_font.text_width(text().substr(0, caret_pos()));
        double sel_x = m_x + m_font.text_width(text().substr(0, selection_start()));

        // Draw the selection background, if any; if not, sel_x and pos_x will be
        // the same value, making this rect empty and invisible.
        Gosu::draw_rect(sel_x, m_y, pos_x - sel_x, height(), SELECTION_COLOR, 0);

        // Draw the caret if this is the currently selected field.
        if (m_window.input().text_input() == this) {
            Gosu::draw_rect(pos_x, m_y, 1, height(), CARET_COLOR, 0);
        }

        // Finally, draw the text itself!
        m_font.draw_text(text(), m_x, m_y, 0);
    }

    // This text field grows with the text in it.
    // (Usually one would use clip_to and implement scrolling for the text field.)
    double width() const { return m_font.text_width(text()); }

    double height() const { return m_font.height(); }

    bool is_under_point(double mouse_x, double mouse_y)
    {
        return mouse_x > m_x - PADDING && mouse_x < m_x + width() + PADDING &&
               mouse_y > m_y - PADDING && mouse_y < m_y + height() + PADDING;
    }
};

class Window : public Gosu::Window
{
    std::array<std::unique_ptr<TextField>, 3> m_text_fields;

public:
    Window()
    : Gosu::Window(300, 200, Gosu::WF_RESIZABLE)
    {
        set_caption("Text Input Example");

        Gosu::Font font(20);

        for (int index = 0; index < m_text_fields.size(); ++index) {
            m_text_fields.at(index).reset(new TextField(*this, font, 50, 30 + index * 50));
        }
    }

    bool needs_cursor() const override //
    {
        return true;
    }

    void draw() override
    {
        for (auto& text_field : m_text_fields) {
            text_field->draw();
        }
    }

    void button_down(Gosu::Button btn) override
    {
        if (btn == Gosu::KB_TAB) {
            // Tab key will not be consumed by Gosu::TextInput.
            // Move focus to next text field.
            int index = -1;
            for (int i = 0; i < m_text_fields.size(); ++i) {
                if (input().text_input() == m_text_fields.at(i).get()) {
                    index = i;
                }
            }
            input().set_text_input(m_text_fields.at((index + 1) % m_text_fields.size()).get());
        }
        else if (btn == Gosu::KB_ESCAPE) {
            // Escape key will not be consumed by Gosu::TextInput.
            // Deselect all text fields.
            if (input().text_input()) {
                input().set_text_input(nullptr);
            }
            else {
                close();
            }
        }
        else if (btn == Gosu::MS_LEFT) {
            // Mouse click: Select text field based on mouse position.
            for (const auto& text_field_ptr : m_text_fields) {
                if (text_field_ptr->is_under_point(input().mouse_x(), input().mouse_y())) {
                    input().set_text_input(text_field_ptr.get());
                    return;
                }
            }
            // No button at this position - exit.
            input().set_text_input(nullptr);
        }
        else {
            Gosu::Window::button_down(btn);
        }
    }
};

int main()
{
    Window window;
    window.show();
    return 0;
}
