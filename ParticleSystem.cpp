#include "ParticleSystem.h"
#include "Actor.h"
#include "ImageDB.h"
#include "glm/glm.hpp"
#include <algorithm>

void ParticleSystem::OnStart() {
    if (image.empty()) {
        ImageDB::CreateDefaultParticleTextureWithName(sprite_name);
    }
    else {
        sprite_name = image;
    }

    frames_between_bursts = std::max(1, frames_between_bursts);
    burst_quantity = std::max(1, burst_quantity);
    duration_frames = std::max(1, duration_frames);

    emit_angle_distribution = RandomEngine(emit_angle_min, emit_angle_max, 298);
    emit_radius_distribution = RandomEngine(emit_radius_min, emit_radius_max, 404);
    rotation_distribution = RandomEngine(rotation_min, rotation_max, 440);
    scale_distribution = RandomEngine(start_scale_min, start_scale_max, 494);
    speed_distribution = RandomEngine(start_speed_min, start_speed_max, 498);
    rotation_speed_distribution = RandomEngine(rotation_speed_min, rotation_speed_max, 305);

    // Cache optional end values as plain bools+floats — eliminates has_value() in hot loop
    has_end_scale = end_scale.has_value();
    end_scale_val = has_end_scale ? end_scale.value() : 1.0f;
    has_end_r = end_color_r.has_value(); end_r_val = has_end_r ? (float)end_color_r.value() : 255.f;
    has_end_g = end_color_g.has_value(); end_g_val = has_end_g ? (float)end_color_g.value() : 255.f;
    has_end_b = end_color_b.has_value(); end_b_val = has_end_b ? (float)end_color_b.value() : 255.f;
    has_end_a = end_color_a.has_value(); end_a_val = has_end_a ? (float)end_color_a.value() : 255.f;

    // Reserve capacity upfront to avoid repeated reallocation during warmup
    int estimated_max = burst_quantity * duration_frames / frames_between_bursts + burst_quantity;
    is_active.reserve(estimated_max);
    start_frame.reserve(estimated_max);
    pos_x.reserve(estimated_max);      pos_y.reserve(estimated_max);
    vel_x.reserve(estimated_max);      vel_y.reserve(estimated_max);
    rotation.reserve(estimated_max);
    rotation_speed.reserve(estimated_max);
    initial_scale.reserve(estimated_max);
    init_r.reserve(estimated_max);     init_g.reserve(estimated_max);
    init_b.reserve(estimated_max);     init_a.reserve(estimated_max);

    active_systems.push_back(this);
}

void ParticleSystem::EmitParticle() {
    float angle_radians = glm::radians(emit_angle_distribution.Sample());
    float radius = emit_radius_distribution.Sample();
    float cos_a = glm::cos(angle_radians);
    float sin_a = glm::sin(angle_radians);

    float sx = x + cos_a * radius;
    float sy = y + sin_a * radius;
    float speed = speed_distribution.Sample();
    float svx = cos_a * speed;
    float svy = sin_a * speed;
    float srot = rotation_distribution.Sample();
    float srspeed = rotation_speed_distribution.Sample();
    float sscale = scale_distribution.Sample();

    if (!free_list.empty()) {
        int i = free_list.front(); free_list.pop();
        is_active[i] = true;
        start_frame[i] = local_frame_number;
        pos_x[i] = sx;   pos_y[i] = sy;
        vel_x[i] = svx;  vel_y[i] = svy;
        rotation[i] = srot;
        rotation_speed[i] = srspeed;
        initial_scale[i] = sscale;
        init_r[i] = start_color_r; init_g[i] = start_color_g;
        init_b[i] = start_color_b; init_a[i] = start_color_a;
    }
    else {
        is_active.push_back(true);
        start_frame.push_back(local_frame_number);
        pos_x.push_back(sx);        pos_y.push_back(sy);
        vel_x.push_back(svx);       vel_y.push_back(svy);
        rotation.push_back(srot);
        rotation_speed.push_back(srspeed);
        initial_scale.push_back(sscale);
        init_r.push_back(start_color_r); init_g.push_back(start_color_g);
        init_b.push_back(start_color_b); init_a.push_back(start_color_a);
        num_slots++;
    }
    num_particles++;
}

void ParticleSystem::OnUpdate() {
    // Burst
    if (local_frame_number % frames_between_bursts == 0 && emission_allowed) {
        for (int i = 0; i < burst_quantity; i++)
            EmitParticle();
    }

    // Hoist member accesses to stack locals so compiler can register-allocate them
    const float gx = gravity_scale_x;
    const float gy = gravity_scale_y;
    const float df = drag_factor;
    const float adf = angular_drag_factor;
    const int   cur_frame = local_frame_number;
    const int   dur = duration_frames;
    const float sort = (float)sorting_order;

    for (int i = 0; i < num_slots; i++) {
        if (!is_active[i]) continue;

        int frames_alive = cur_frame - start_frame[i];
        if (frames_alive >= dur) {
            is_active[i] = false;
            free_list.push(i);
            num_particles--;
            continue;
        }

        // 1. Gravity
        vel_x[i] += gx;
        vel_y[i] += gy;

        // 2. Drag
        vel_x[i] *= df;
        vel_y[i] *= df;
        rotation_speed[i] *= adf;

        // 3. Apply velocities
        pos_x[i] += vel_x[i];
        pos_y[i] += vel_y[i];
        rotation[i] += rotation_speed[i];

        // 4. Color and scale — use precomputed bools to skip has_value() checks,
        //    but keep glm::mix and the original division to preserve exact FP results
        float lifetime_progress = static_cast<float>(frames_alive) / dur;

        float cur_scale = initial_scale[i];
        if (has_end_scale)
            cur_scale = glm::mix(initial_scale[i], end_scale_val, lifetime_progress);

        float cur_r = (float)init_r[i], cur_g = (float)init_g[i],
            cur_b = (float)init_b[i], cur_a = (float)init_a[i];
        if (has_end_r) cur_r = glm::mix((float)init_r[i], end_r_val, lifetime_progress);
        if (has_end_g) cur_g = glm::mix((float)init_g[i], end_g_val, lifetime_progress);
        if (has_end_b) cur_b = glm::mix((float)init_b[i], end_b_val, lifetime_progress);
        if (has_end_a) cur_a = glm::mix((float)init_a[i], end_a_val, lifetime_progress);

        ImageDB::DrawEx(
            sprite_name,
            pos_x[i], pos_y[i],
            rotation[i],
            cur_scale, cur_scale,
            0.5f, 0.5f,
            cur_r, cur_g, cur_b, cur_a,
            sort
        );
    }

    local_frame_number++;
}

void ParticleSystem::Stop() {
    emission_allowed = false;
}

void ParticleSystem::Play() {
    emission_allowed = true;
}

void ParticleSystem::Burst() {
    for (int i = 0; i < burst_quantity; i++)
        EmitParticle();
}

void ParticleSystem::OnDestroy() {
    is_active.clear();
    start_frame.clear();
    pos_x.clear(); pos_y.clear();
    vel_x.clear(); vel_y.clear();
    rotation.clear();
    rotation_speed.clear();
    initial_scale.clear();
    init_r.clear(); init_g.clear(); init_b.clear(); init_a.clear();
    while (!free_list.empty()) free_list.pop();
    num_slots = 0;
    num_particles = 0;

    active_systems.erase(
        std::remove(active_systems.begin(), active_systems.end(), this),
        active_systems.end()
    );
}