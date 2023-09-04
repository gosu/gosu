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
#include <utility>
#include <vector>

class Gosu::DrawOpQueue
{
    const QueueMode queue_mode;

    TransformStack transform_stack;
    ClipRectStack clip_rect_stack;

    std::vector<DrawOp> ops;
    std::vector<std::function<void ()>> gl_blocks;

public:
    explicit DrawOpQueue(QueueMode mode)
    : queue_mode(mode)
    {
    }
    
    QueueMode mode() const
    {
        return queue_mode;
    }
    
    void schedule_draw_op(DrawOp op)
    {
#ifdef GOSU_IS_OPENGLES
        // No triangles, no lines supported
        assert(op.vertices_or_block_index == 4);
#endif

        op.render_state.transform = &transform_stack.current();
        op.render_state.clip_rect = clip_rect_stack.effective_rect();
        ops.push_back(op);
    }

    void gl(std::function<void ()> gl_block, ZPos z)
    {
        int complement_of_block_index = ~(int)gl_blocks.size();
        gl_blocks.push_back(std::move(gl_block));

        DrawOp op;
        op.vertices_or_block_index = complement_of_block_index;
        op.render_state.transform = &transform_stack.current();
        op.render_state.clip_rect = clip_rect_stack.effective_rect();
        op.z = z;
        ops.push_back(op);
    }

    void begin_clipping(double x, double y, double width, double height,
                        std::optional<int> viewport_height)
    {
        if (mode() == QM_RECORD_MACRO) {
            throw std::logic_error("Clipping is not allowed while creating a macro");
        }
        
        // Apply current transformation.

        double left = x, right = x + width;
        double top = y, bottom = y + height;

        transform_stack.current().apply(left, top);
        transform_stack.current().apply(right, bottom);

        Rect clip_rect {
            .x = static_cast<int>(std::min(left, right)),
            .y = static_cast<int>(std::min(top, bottom)),
            .width = static_cast<int>(std::abs(left - right)),
            .height = static_cast<int>(std::abs(top - bottom)),
        };
        // Adjust for OpenGL having the wrong idea of where y=0 is.
        if (viewport_height) {
            clip_rect.y = *viewport_height - clip_rect.y - clip_rect.height;
        }

        clip_rect_stack.push(clip_rect);
    }

    void end_clipping() { clip_rect_stack.pop(); }

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

    void perform_draw_ops_and_code()
    {
        if (mode() == QM_RECORD_MACRO) {
            throw std::logic_error("Flushing to the screen is not allowed while recording a macro");
        }
        
        // Apply Z-Ordering.
        std::stable_sort(ops.begin(), ops.end(),
                         [](const DrawOp& lhs, const DrawOp& rhs) { return lhs.z < rhs.z; });

        RenderStateManager manager;
        
    #ifdef GOSU_IS_OPENGLES
        if (ops.empty()) return;

        auto current = ops.begin(), last = ops.end() - 1;
        for (; current != last; ++current) {
            manager.set_render_state(current->render_state);
            current->perform(&*(current + 1));
        }
        manager.set_render_state(last->render_state);
        last->perform(nullptr);
    #else
        for (const auto& op : ops) {
            manager.set_render_state(op.render_state);
            if (op.vertices_or_block_index >= 0) {
                op.perform(nullptr);
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

        std::stable_sort(ops.begin(), ops.end(), [](const DrawOp& lhs, const DrawOp& rhs) { return lhs.z < rhs.z; });
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
