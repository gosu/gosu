#pragma once

#include "ClipRectStack.hpp"
#include "DrawOp.hpp"
#include "GraphicsImpl.hpp"
#include "TransformStack.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <map>
#include <vector>

class Gosu::DrawOpQueue
{
    TransformStack transform_stack;
    ClipRectStack clip_rect_stack;
    bool rec;

    std::vector<DrawOp> ops;
    std::vector<std::function<void ()>> gl_blocks;

public:
    DrawOpQueue()
    : rec(false)
    {
    }
    
    bool recording() const
    {
        return rec;
    }
    
    void set_recording()
    {
        rec = true;
    }
    
    void schedule_draw_op(DrawOp op)
    {
        if (clip_rect_stack.clipped_world_away()) {
            return;
        }

        op.render_state.transform = &transform_stack.current();
        if (const ClipRect* cr = clip_rect_stack.maybe_effective_rect()) {
            op.render_state.clip_rect = *cr;
        }
        ops.push_back(op);
    }

    void gl(std::function<void ()> gl_block, ZPos z)
    {
        // TODO: Document this case: Clipped-away GL blocks are *not* being run.
        if (clip_rect_stack.clipped_world_away()) {
            return;
        }

        int complement_of_block_index = ~(int)gl_blocks.size();
        gl_blocks.push_back(gl_block);

        DrawOp op;
        op.vertices_or_block_index = complement_of_block_index;
        op.render_state.transform = &transform_stack.current();
        if (const ClipRect* cr = clip_rect_stack.maybe_effective_rect()) {
            op.render_state.clip_rect = *cr;
        }
        op.z = z;
        ops.push_back(op);
    }

    void begin_clipping(double x, double y, double width, double height, double screen_height)
    {
        if (recording()) {
            throw std::logic_error("Clipping is not allowed while creating a macro");
        }
        
        // Apply current transformation.

        double left = x, right = x + width;
        double top = y, bottom = y + height;

        apply_transform(transform_stack.current(), left, top);
        apply_transform(transform_stack.current(), right, bottom);

        double phys_x      = std::min(left, right);
        double phys_y      = std::min(top, bottom);
        double phys_width  = std::abs(left - right);
        double phys_height = std::abs(top - bottom);

        // Adjust for OpenGL having the wrong idea of where y=0 is.
        phys_y = screen_height - phys_y - phys_height;

        clip_rect_stack.begin_clipping(phys_x, phys_y, phys_width, phys_height);
    }

    void end_clipping()
    {
        clip_rect_stack.end_clipping();
    }

    void set_base_transform(const Transform& base_transform)
    {
        transform_stack.set_base_transform(base_transform);
    }

    void push_transform(const Transform& transform)
    {
        transform_stack.push(transform);
    }

    void pop_transform()
    {
        transform_stack.pop();
    }

    void perform_draw_ops_andCode()
    {
        if (recording()) {
            throw std::logic_error("Flushing to the screen is not allowed while recording a macro");
        }
        
        // Apply Z-Ordering.
        std::stable_sort(ops.begin(), ops.end());

        RenderStateManager manager;
        
    #ifdef GOSU_IS_OPENGLES
        if (ops.empty()) {
            return;
        }

        auto current = ops.begin(), last = ops.end() - 1;
        for (; current != last; ++current) {
            manager.set_render_state(current->render_state);
            current->perform(&*(current + 1));
        }
        manager.set_render_state(last->render_state);
        last->perform(0);
    #else
        for (const auto& op : ops) {
            manager.set_render_state(op.render_state);
            if (op.vertices_or_block_index >= 0) {
                op.perform(0);
            }
            else {
                // GL code
                int block_index = ~op.vertices_or_block_index;
                assert (block_index >= 0);
                assert (block_index < gl_blocks.size());
                gl_blocks[block_index]();
                manager.enforce_after_untrusted_gL();
            }
        }
    #endif
    }

    void compile_to(VertexArrays& vas)
    {
        if (!gl_blocks.empty()) {
            throw std::logic_error("Custom OpenGL code cannot be recorded as a macro");
        }

        std::stable_sort(ops.begin(), ops.end());
        for (const auto& op : ops) {
            op.compile_to(vas);
        }
    }

    // This retains the current stack of transforms and clippings.
    void clear_queue()
    {
        gl_blocks.clear();
        ops.clear();
    }

    // This clears the queue and starts with new stacks. This must not be called
    // when end_clipping/pop_transform calls might still be pending.
    void reset()
    {
        transform_stack.reset();
        clip_rect_stack.clear();
        clear_queue();
    }
};
