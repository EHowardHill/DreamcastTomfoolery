#pragma once

#include <EGL/egl.h>
#include "../../window.h"

namespace smlt {

class AndroidWindow : public Window {
public:
    static Window::ptr create(Application* app) {
        return Window::create<AndroidWindow>(app);
    }

    AndroidWindow();

    void set_title(const std::string&) override {} // No-op
    void cursor_position(int32_t&, int32_t&) override {} // No-op
    void show_cursor(bool) override {} // No-op
    void lock_cursor(bool) override {} // No-op

    void swap_buffers() override;
    void destroy_window() override;
    void check_events() override;

    void initialize_input_controller(InputState &controller) override;

    std::shared_ptr<SoundDriver> create_sound_driver(const std::string& from_config) override;

private:
    EGLDisplay dpy_;
    EGLSurface surface_;
    EGLContext ctx_;

    bool _init_window() override;
    bool _init_renderer(Renderer *renderer) override;
};

}
