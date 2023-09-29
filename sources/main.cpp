#include "simulant/simulant.h"
#include "simulant/shortcuts.h"
#include "simulant/extra.h"
#include "simulant/macros.h"
#include "simulant/utils/dreamcast.h"
#include <math.h>

using namespace smlt;

#define M_PI 3.14159265358979323846

class GameScene : public Scene<GameScene>
{
public:
    GameScene(Window *window) : Scene<GameScene>(window) {}
    CameraPtr camera_;
    StagePtr stage_;

    ActorPtr player;
    ActorPtr actor_floor;

    TexturePtr txt_crate;
    MaterialPtr mat_crate;

    TexturePtr txt_grass;
    MaterialPtr mat_grass;

    SoundPtr sound_;

    int prev_degrees = 0;
    int ticker = 0;
    bool activate_z = false;

    float lerp(float a, int b, float t)
    {
        return a * (1 - t) + b * t;
    }

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

        txt_crate = stage_->assets->new_texture_from_file("sample_data/crate.png");
        txt_crate->set_texture_filter(TEXTURE_FILTER_BILINEAR);
        mat_crate = stage_->assets->new_material_from_texture(txt_crate);
        mat_crate->pass(0)->set_lighting_enabled(true);

        txt_grass = stage_->assets->new_texture_from_file("sample_data/grass3.png");
        txt_grass->set_texture_filter(TEXTURE_FILTER_BILINEAR);
        mat_grass = stage_->assets->new_material_from_texture(txt_grass);
        mat_grass->pass(0)->set_lighting_enabled(true);

        auto cube = stage_->assets->new_mesh_from_file("sample_data/tank.obj");
        //cube->new_submesh_as_cube("tank", mat_crate, 2.0);
        player = stage_->new_actor_with_mesh(cube);
        player->move_to(0.0, 0.0, -5.0);

        auto floor = stage_->assets->new_mesh(VertexSpecification::DEFAULT);
        floor->new_submesh_as_cube("rect", mat_grass, 12.0);
        actor_floor = stage_->new_actor_with_mesh(floor);
        actor_floor->move_to(0.0, -6.0, -5.0);

        // Set up custom axises
        auto l_trigger = input->new_axis("Left Trigger");
        l_trigger->set_joystick_axis(JOYSTICK_AXIS_LTRIGGER);

        auto r_trigger = input->new_axis("Right Trigger");
        r_trigger->set_joystick_axis(JOYSTICK_AXIS_RTRIGGER);

        auto a_button = input->new_axis("A Button");
        a_button->set_positive_joystick_button(JOYSTICK_BUTTON_A);

        // window->set_audio_listener(camera_);
        // sound_ = stage_->assets->new_sound_from_file("sample_data/stillalive.ogg");
        // camera_->play_sound(sound_, AUDIO_REPEAT_FOREVER, DISTANCE_MODEL_AMBIENT);

        stage_->new_light_as_directional(Vec3(1, 0, 0), Colour::WHITE);
        // stage_->debug->draw_line(player->absolute_position(), Vec3(0, -1, 0), Colour::RED);
    }

    void update(float dt)
    {
        _S_UNUSED(dt);

        //S_INFO("Axis: {0}", input->axis_value_hard("Left Trigger"));

        // Camera Controls
        ticker++;

        if (input->axis_value_hard("Left Trigger") != 0) // 
        {
            if (!activate_z) {
                activate_z = true;
                auto newPos = player->position() - (player->forward() * 7.0f) + Vec3(0, 1.5f, 0);
                camera_->move_to(newPos);
            }
        }
        else
        {
            activate_z = false;
            auto offset = camera_->position() - player->position();
            offset = offset.normalized() * 7.0f;
            auto newPos = player->position() + offset;
            newPos.y = player->position().y + 1.5f;
            camera_->move_to(newPos);
        }

        camera_->look_at(player->absolute_position());
    }

    void fixed_update(float dt)
    {
        // Player Controls
        auto horz = input->axis_value("Horizontal") * 1.5f * dt;
        auto vert = input->axis_value("Vertical") * -1.5f * dt;

        if (input->axis_value_hard("Left Trigger") != 0) {
            auto forw = camera_->forward() * vert * -75.0f * dt;
            auto stos = camera_->right() * horz * -75.0f * dt;
            auto newd = forw - stos;

            player->move_by(
                newd.x * 200.0f * dt,
                0.0f,
                newd.z * 200.0f * dt);
        } else {
            auto forw = player->position() - camera_->position();
            forw = forw.normalized() * vert * -75.0f * dt;
            auto stos = camera_->right() * horz * -75.0f * dt;
            auto newd = forw - stos;

            player->move_by(
                newd.x * 200.0f * dt,
                0.0f,
                newd.z * 200.0f * dt);
        }

        auto cameraAngle = atan2(camera_->forward().x, camera_->forward().z) * (180.0 / M_PI);
        auto analogAngle = atan2(vert, -horz) * (180.0 / M_PI);

        auto idealQuat = Quaternion(Euler(0.0f, cameraAngle + analogAngle + 90.0f, 0.0f));
        if (vert != 0.0f || horz != 0.0f)
        {
            player->rotate_to(idealQuat);
        }

        auto down_ray = Ray(player->position(), Vec3(0, -3, 0));

        if (down_ray.intersects_aabb(actor_floor->aabb())) {
            S_INFO("Intersects!");
        }

        // if(input->axis_value_hard("Start") == 1) {
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
    config.width = 640;
    config.height = 480;
#endif

    config.desktop.enable_virtual_screen = false;

    ApplicationInst app(config);
    return app.run();
}