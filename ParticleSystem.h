#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <string>
#include <vector>
#include <queue>
#include <optional>
#include "Helper.h"

class Actor;

class ParticleSystem {
public:
    Actor* actor = nullptr;
    bool enabled = true;
    std::string key = "";
    std::string type = "ParticleSystem";

    static inline std::vector<ParticleSystem*> active_systems;

    // Position
    float x = 0.0f;
    float y = 0.0f;

    // Emission shape
    float emit_angle_min = 0.0f;
    float emit_angle_max = 360.0f;
    float emit_radius_min = 0.0f;
    float emit_radius_max = 0.5f;

    // Burst config
    int frames_between_bursts = 1;
    int burst_quantity = 1;

    // Scale
    float start_scale_min = 1.0f;
    float start_scale_max = 1.0f;
    std::optional<float> end_scale;  // unset = no interpolation

    // Rotation
    float rotation_min = 0.0f;
    float rotation_max = 0.0f;
    float rotation_speed_min = 0.0f;
    float rotation_speed_max = 0.0f;

    // Speed
    float start_speed_min = 0.0f;
    float start_speed_max = 0.0f;

    // Gravity & drag
    float gravity_scale_x = 0.0f;
    float gravity_scale_y = 0.0f;
    float drag_factor = 1.0f;
    float angular_drag_factor = 1.0f;

    // Color
    int start_color_r = 255, start_color_g = 255, start_color_b = 255, start_color_a = 255;
    std::optional<int> end_color_r, end_color_g, end_color_b, end_color_a;

    // Lifetime
    int duration_frames = 300;

    // Rendering
    int sorting_order = 9999;
    std::string image = "";
    std::string sprite_name = "__default_particle";

    int local_frame_number = 0;

    bool emission_allowed = true;

    void OnStart();
    void OnUpdate();
    void OnDestroy();

    void Stop();
    void Play();
    void Burst();

private:
    // Parallel arrays (data-oriented)
    std::vector<bool>  is_active;
    std::vector<int>   start_frame;
    std::vector<float> pos_x, pos_y;
    std::vector<float> vel_x, vel_y;
    std::vector<float> rotation;
    std::vector<float> rotation_speed;
    std::vector<float> initial_scale;
    // scale[] removed — cur_scale is computed inline in OnUpdate and never stored
    std::vector<int>   init_r, init_g, init_b, init_a;

    std::queue<int> free_list;
    int num_slots = 0;
    int num_particles = 0;

    // Precomputed once in OnStart to eliminate optional::has_value() calls from the hot loop
    bool  has_end_scale = false;
    float end_scale_val = 1.0f;
    bool  has_end_r = false, has_end_g = false, has_end_b = false, has_end_a = false;
    float end_r_val = 255.f, end_g_val = 255.f, end_b_val = 255.f, end_a_val = 255.f;

    RandomEngine emit_angle_distribution;
    RandomEngine emit_radius_distribution;
    RandomEngine rotation_distribution;
    RandomEngine scale_distribution;
    RandomEngine speed_distribution;
    RandomEngine rotation_speed_distribution;

    void EmitParticle();
};

#endif