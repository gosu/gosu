#pragma once

#ifndef __cplusplus
#error This file is only for internal use within the Gosu FFI implementation.
#endif

#include "Gosu.h"
#include <Gosu/Gosu.hpp>
#include <exception>
#include <functional>
#include <string>

// Error handling

std::string& Gosu_internal_error();

template<typename Functor>
auto Gosu_translate_exceptions(Functor functor)
{
    try {
        // The functor may call back into Ruby, which may in turn call other Gosu methods that fail.
        // If the Ruby callback handles these errors itself, they must not be reported again.
        // => Reset it not only before, but also after the functor.
        Gosu_internal_error().clear();
        const auto clear_again = Gosu::finally([] { Gosu_internal_error().clear(); });

        return functor();
    }
    catch (const std::exception& e) {
        Gosu_internal_error() = e.what();
        using R = decltype(functor());
        return R();
    }
}

// C-compatible wrapper structs for Gosu classes. (No inheritance because of missing virtual dtors.)

struct Gosu_Channel
{
    Gosu::Channel channel;
};

struct Gosu_Font
{
    Gosu::Font font;
};

struct Gosu_Image
{
    Gosu::Image image;
};

struct Gosu_Sample
{
    Gosu::Sample sample;
};

// Use inheritance where composition is not feasible (because we want to compare pointers).

struct Gosu_Song : Gosu::Song
{
    using Song::Song;
};

struct Gosu_TextInput : public Gosu::TextInput
{
    std::string filter(std::string text) const override
    {
        if (filter_callback) {
            filter_callback(text.c_str());
            // We now expect the result to be stored using Gosu_TextInput_set_filter_result.
            return filter_result;
        }
        return Gosu::TextInput::filter(text);
    }

    std::function<void(const char* text)> filter_callback;
    std::string filter_result;
};

struct Gosu_Window : public Gosu::Window
{
    using Window::Window;

    void update() override
    {
        if (update_callback) update_callback();
    }

    void draw() override
    {
        if (draw_callback) draw_callback();
    }

    void button_down(Gosu::Button btn) override
    {
        if (button_down_callback) button_down_callback(btn);
    }

    void button_up(Gosu::Button btn) override
    {
        if (button_up_callback) button_up_callback(btn);
    }

    void gamepad_connected(int id) override
    {
        if (gamepad_connected_callback) gamepad_connected_callback(id);
    }

    void gamepad_disconnected(int id) override
    {
        if (gamepad_disconnected_callback) gamepad_disconnected_callback(id);
    }

    void drop(const std::string& filename) override
    {
        if (drop_callback) drop_callback(filename.c_str());
    }

    bool needs_redraw() const override
    {
        if (needs_redraw_callback) return needs_redraw_callback();
        return Window::needs_redraw();
    }

    bool needs_cursor() const override
    {
        if (needs_cursor_callback) return needs_cursor_callback();
        return Window::needs_cursor();
    }

    void close() override
    {
        if (close_callback != nullptr) return close_callback();
        Gosu::Window::close();
    }

    void gain_focus() override
    {
        if (gain_focus_callback) gain_focus_callback();
    }

    void lose_focus() override
    {
        if (lose_focus_callback) lose_focus_callback();
    }

    std::function<void()> update_callback;
    std::function<void()> draw_callback;
    std::function<void(unsigned id)> button_down_callback;
    std::function<void(unsigned id)> button_up_callback;
    std::function<void(int index)> gamepad_connected_callback;
    std::function<void(int index)> gamepad_disconnected_callback;
    std::function<void(const char* filename)> drop_callback;
    std::function<bool()> needs_redraw_callback;
    std::function<bool()> needs_cursor_callback;
    std::function<void()> close_callback;
    std::function<void()> gain_focus_callback;
    std::function<void()> lose_focus_callback;
};
