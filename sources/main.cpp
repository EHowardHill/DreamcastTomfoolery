#include "simulant/simulant.h"
#include "simulant/shortcuts.h"
#include "simulant/extra.h"
#include "simulant/macros.h"
#include "simulant/utils/dreamcast.h"

#include <math.h>
#include <vector>
#include <string.h>

using namespace smlt;

#define M_PI 3.14159265358979323846

class GameScene : public smlt::PhysicsScene<GameScene>
{
public:
    GameScene(Window *window) : smlt::PhysicsScene<GameScene>(window) {}
    CameraPtr camera_;
    StagePtr stage_;

    ActorPtr player;
    ActorPtr actor_floor;

    TexturePtr txt_grass;
    MaterialPtr mat_grass;

    behaviours::RigidBody *controller;

    bool activate_z = false;
    bool activate_a = false;
    bool activate_l = false;
    bool activate_r = false;

    int ticker = 0;
    int side = 0;

    void load()
    {
        stage_ = new_stage(PARTITIONER_NULL);
        stage_->enable_debug();

        camera_ = stage_->new_camera();
        auto pipeline = compositor->render(stage_, camera_);
        link_pipeline(pipeline);

        camera_->set_perspective_projection(
            Degrees(45.0),
            float(window->width()) / float(window->height()),
            0.1,
            1000.0);

        stage_->set_ambient_light(Colour(0.25f, 0.25f, 0.25f, 1.0f));

        txt_grass = stage_->assets->new_texture_from_file("sample_data/grass3.png");
        txt_grass->set_texture_filter(TEXTURE_FILTER_BILINEAR);
        mat_grass = stage_->assets->new_material_from_texture(txt_grass);
        mat_grass->pass(0)->set_lighting_enabled(true);

        auto cube = stage_->assets->new_mesh_from_file("sample_data/tank.obj");
        player = stage_->new_actor_with_mesh(cube);
        player->move_to(0.0, 3.0, 0.0);
        player->rotate_by(Degrees(15), Degrees(15), Degrees(15));
        // controller = player->new_behaviour<behaviours::RigidBody>(physics);
        // controller->add_box_collider(player->aabb().dimensions(), behaviours::PhysicsMaterial::WOOD);

        auto floor = stage_->assets->new_mesh(VertexSpecification::DEFAULT);
        floor->new_submesh_as_cube("floor", mat_grass, 12.0);
        actor_floor = stage_->new_actor_with_mesh(floor);
        actor_floor->move_to(0.0, -6.0, 0.0);

        auto c = actor_floor->new_behaviour<behaviours::StaticBody>(physics);
        c->add_box_collider(actor_floor->aabb().dimensions(), behaviours::PhysicsMaterial::STONE);

        // Set up custom axises
        auto l_trigger = input->new_axis("Left Trigger");
        l_trigger->set_joystick_axis(JOYSTICK_AXIS_LTRIGGER);

        auto r_trigger = input->new_axis("Right Trigger");
        r_trigger->set_joystick_axis(JOYSTICK_AXIS_RTRIGGER);

        auto a_button = input->new_axis("A Button");
        a_button->set_positive_joystick_button(JOYSTICK_BUTTON_A);

        stage_->new_light_as_directional(Vec3(1, 0, 0), Colour::WHITE);
    }

    void update(float dt)
    {
        _S_UNUSED(dt);

        /*
        // Camera Controls
        if (input->axis_value_hard("Left Trigger") != 0) //
        {
            if (!activate_z)
            {
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
        */

        /*
        0   10, 10
        1   -10, 10
        2   -10, -10
        3   10, -10
        */

        if (input->axis_value_hard("Left Trigger") != 0)
        {
            if (!activate_l)
            {
                activate_l = true;
                side -= 1;
                if (side < 0)
                    side = 3;
            }
        }
        else
        {
            activate_l = false;
        }

        if (input->axis_value_hard("Right Trigger") != 0)
        {
            if (!activate_r)
            {
                activate_r = true;
                side += 1;
                if (side > 3)
                    side = 0;
            }
        }
        else
        {
            activate_r = false;
        }

        int mx = -15;
        int mz = -15;
        if (side == 0 || side == 3)
            mx = 15;
        if (side == 2 || side == 3)
            mz = 15;
        camera_->move_to(Vec3(lerp(camera_->absolute_position().x, mx, 3.0f * dt),
                              10,
                              lerp(camera_->absolute_position().z, mz, 3.0f * dt)));

        camera_->look_at(Vec3(0, 3, 0));

        if (input->axis_value_hard("A Button") == 1)
        {
            if (!activate_a)
            {
                activate_a = true;
                controller = player->new_behaviour<behaviours::RigidBody>(physics);
                controller->add_box_collider(player->aabb().dimensions(), behaviours::PhysicsMaterial::WOOD);
            }
        }

        if (activate_a) {
            auto deets = controller->angular_velocity();
            S_INFO("{0}, {1}, {2}", deets.x, deets.y, deets.z);
        }
    }

    void fixed_update(float dt)
    {
        // S_INFO("Ticker: {0}", ticker);

        // Player Controls
        auto horz = input->axis_value("Horizontal") * 5.0f;
        auto vert = input->axis_value("Vertical") * -5.0f;

        player->move_by((camera_->up() * vert * dt * -1) + (camera_->right() * horz * dt));

        /*

        auto c_horz = horz;
        auto c_vert = vert;
        if (c_horz < 0) c_horz *= -1;
        if (c_vert < 0) c_vert *= -1;
        auto c_total = c_horz + c_vert;
        if (c_total > 1.75f) {
            auto factor = (1.75f) / c_total;
            horz = horz * factor;
            vert = vert * factor;
        }

        if (input->axis_value_hard("Left Trigger") != 0)
        {
            auto forw = camera_->forward() * vert * -75.0f * dt;
            auto stos = camera_->right() * horz * -75.0f * dt;
            auto newd = forw - stos;

            controller->add_impulse(
                Vec3(
                    newd.x * 5.0f * dt,
                    0.0f,
                    newd.z * 5.0f * dt));
        }
        else
        {
            auto forw = player->position() - camera_->position();
            forw = forw.normalized() * vert * -75.0f * dt;
            auto stos = camera_->right() * horz * -75.0f * dt;
            auto newd = forw - stos;

            controller->add_impulse(
                Vec3(
                    newd.x * 3.5f * dt,
                    0.0f,
                    newd.z * 3.5f * dt));
        }

        auto cameraAngle = atan2(camera_->forward().x, camera_->forward().z) * (180.0 / M_PI);
        auto analogAngle = atan2(vert, -horz) * (180.0 / M_PI);

        auto idealQuat = Quaternion(Euler(0.0f, cameraAngle + analogAngle + 90.0f, 0.0f));
        if (vert != 0.0f || horz != 0.0f)
        {
            controller->rotate_to(idealQuat);
        }


        */

        // Collisions
        /*
        if (!actor_floor->transformed_aabb().contains_point(player->absolute_position())) {
            controller->move_by(Vec3(0.0f, -0.05f, 0.0f));
        }
        */
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
    config.title = "Tank Example";
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