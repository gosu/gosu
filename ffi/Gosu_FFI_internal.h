#pragma once

#ifndef __cplusplus
#error This file is only for internal use within the Gosu FFI implementation.
#endif

#include "Gosu.h"
#include <Gosu/Gosu.hpp>
#include <stdexcept>

// Error handling

std::string& Gosu_internal_error();

template<typename Functor>
auto Gosu_translate_exceptions(Functor functor)
{
    try {
        Gosu_internal_error().clear();
        return functor();
    }
    catch (const std::exception& e) {
        Gosu_internal_error() = e.what();
        using R = decltype(functor());
        return R();
    }
}

// C-compatible wrapper structs for Gosu classes

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

// Use inheritance where composition is not feasible

struct Gosu_Song : Gosu::Song
{
    [[maybe_unused]] // silence buggy warning, this is being used in Gosu_Song.cpp.
    explicit Gosu_Song(const std::string& filename)
    : Gosu::Song{filename}
    {
    }
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
    [[maybe_unused]] // silence buggy warning, this is being used in Gosu_Window.cpp.
    Gosu_Window(int width, int height, unsigned window_flags, double update_interval)
    : Gosu::Window(width, height, window_flags, update_interval)
    {
    }

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
