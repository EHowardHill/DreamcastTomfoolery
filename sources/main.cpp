#include "simulant/simulant.h"
#include "simulant/shortcuts.h"
#include "simulant/extra.h"
#include "simulant/macros.h"
#include "simulant/utils/dreamcast.h"

using namespace smlt;

class GameScene : public Scene<GameScene>
{
public:
    GameScene(Window *window) : Scene<GameScene>(window) {}
    CameraPtr camera_;
    StagePtr stage_;
    ActorPtr actor_;
    TexturePtr texture_;
    MaterialPtr material_;
    SoundPtr sound_;

    void load()
    {
        stage_ = new_stage(PARTITIONER_NULL);
        camera_ = stage_->new_camera();
        auto pipeline = compositor->render(stage_, camera_);
        link_pipeline(pipeline);

        camera_->set_perspective_projection(
            Degrees(45.0),
            float(window->width()) / float(window->height()),
            0.1,
            1000.0);

        stage_->set_ambient_light(Colour(0.25f, 0.25f, 0.25f, 1.0f));

        sound_ = stage_->assets->new_sound_from_file("sample_data/stillalive.ogg");

        texture_ = stage_->assets->new_texture_from_file("sample_data/ac_face.png");
        texture_->set_texture_filter(TEXTURE_FILTER_BILINEAR);

        material_ = stage_->assets->new_material_from_texture(texture_);
        material_->pass(0)->set_lighting_enabled(true);

        auto cube = stage_->assets->new_mesh(VertexSpecification::DEFAULT);
        cube->new_submesh_as_cube("rect", material_, 2.0);
        actor_ = stage_->new_actor_with_mesh(cube);
        actor_->move_to(0.0, 0.0, -5.0);

        window->set_audio_listener(camera_);
        camera_->play_sound(sound_, AUDIO_REPEAT_FOREVER, DISTANCE_MODEL_AMBIENT);

        camera_->look_at(actor_->absolute_position());
        stage_->new_light_as_directional(Vec3(1, 0, 0), Colour::WHITE);
    }

    void update(float dt)
    {
        _S_UNUSED(dt);
    }

    void fixed_update(float dt)
    {
        actor_->rotate_x_by(Degrees(input->axis_value("Vertical") * 32.0f * dt));
        actor_->rotate_y_by(Degrees(input->axis_value("Horizontal") * 32.0f * dt));
    }
};

class ApplicationInst : public Application
{
public:
    ApplicationInst(const AppConfig &config) : Application(config)
    {
    }

    bool init()
    {
        scenes->register_scene<GameScene>("main");
        return true;
    }
};

int main(int argc, char *argv[])
{
    _S_UNUSED(argc);
    _S_UNUSED(argv);

    AppConfig config;
    config.title = "Example";
    config.fullscreen = false;

#ifdef __DREAMCAST__
    config.width = 640;
    config.height = 480;
#else
    config.width = 1280;
    config.height = 960;
#endif

    config.desktop.enable_virtual_screen = true;

    ApplicationInst app(config);
    return app.run();
}