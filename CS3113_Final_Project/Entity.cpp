﻿/**
* Author: Jaden Thakur
* Assignment: Platformer
* Date due: 2023-12-2, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << std::endl;

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"
#include "Utility.h"


const float GRAVITY = 1.0f;


Entity::Entity() {

    dead = false;
    fire_height = 0;
    weapon = nullptr;
    hit = false;

    // PHYSICS (GRAVITY) 
    m_position = glm::vec3(0.0f);
    m_velocity = glm::vec3(0.0f);
    m_acceleration = glm::vec3(0.0f);

    // TRANSFORMATIONS
    m_movement = glm::vec3(0.0f);
    m_speed = 0.0;
    m_model_matrix = glm::mat4(1.0f);
}

Entity::~Entity() {
    delete[] m_animation;
    m_animation = nullptr;
    delete[] m_left;
    m_left = nullptr;
    delete[] m_right;
    m_right = nullptr;
};

void Entity::scale() {
    // scales model to current height and width, yes you have to set the height and width first Jaden was lazy
    m_model_matrix = glm::scale(m_model_matrix, glm::vec3(m_width, m_height, 1.0));
}

void Entity::give_sword(Entity* sword) {
    this->weapon = sword;
}

void Entity::activate() {
    this->m_is_active = true;
};
void Entity::deactivate() {
    this->m_is_active = false;
};


// relocate to individual class files for each kind of enemy
void Entity::activate_ai(Entity* player, float delta_time) 
{
    switch (m_enemy_type) 
    {
    /*case JUMPY:
        if (glm::distance(m_position, player->get_position()) > 10) {
            set_mode(PATROL);
            ai_patrol(delta_time);
        }
        else {
            set_mode(AGGRO);
            ai_attack(player);
        }
        break;
    case SPIKY:
        m_can_damage = false;
        set_mode(IDLE);
        ai_idle(delta_time);
        
        break;*/
    case DASHY:
        if (glm::distance(m_position, player->get_position()) > 3) {
            set_mode(PATROL);
            ai_patrol(delta_time);
            m_speed = 0.5f;
        }
        else {
            set_mode(AGGRO);
            ai_attack(player);
            m_speed = player->m_speed/2;
        }
        break;
    }
}

void Entity::ai_attack(Entity* player) {
    switch (m_ai_mode) {
    case AGGRO:
        if (m_enemy_type == DASHY) {
            if (m_position.x > player->get_position().x + player->get_width()) {
                move_left();
                m_animation_indices = m_animation[RIGHT];
            }
            else if (m_position.x < player->get_position().x - player->get_width()) {
                move_right();
                m_animation_indices = m_animation[LEFT];
            }
            if (m_position.y > player->get_position().y + player->get_height()) {
                move_down();
            }
            else if (m_position.y < player->get_position().y - player->get_height()) {
                move_up();
            }
        }

        /*if (m_enemy_type == JUMPY && m_collided_bottom) {
            if (m_position.x > player->get_position().x) {
                move_right();
                m_animation_indices = m_animation[RIGHT];
            }
            else {
                move_left();
                m_animation_indices = m_animation[LEFT];
            }
        }
        else if (m_enemy_type == JUMPY && !m_collided_bottom) {
            if (m_position.x > player->get_position().x) {
                move_left();
                m_animation_indices = m_animation[RIGHT];
            }
            else {
                move_right();
                m_animation_indices = m_animation[LEFT];
            }
        }*/
        break;
    }
}

void Entity::ai_patrol(float delta_time) {
    switch (m_ai_mode) {
    case PATROL:
        if (timer > 5) {
            dir = -dir;
            timer = 0;

        }
        m_movement = glm::vec3(1.0f, 0.0f, 0.0f) * dir;
        if (dir == 1) {
            m_animation_indices = m_animation[LEFT];
        }
        else {
            m_animation_indices = m_animation[RIGHT];
        }
        timer += 2 * delta_time;
    }
}

void Entity::ai_idle(float delta_time) {
    switch (m_ai_mode) {
    case IDLE:
        if (timer > 5) {
            dir = -dir;
            timer = 0;

        }
   
        if (dir == 1) {
            m_animation_indices = m_animation[LEFT];
            m_can_damage = true;
        }
        else {
            m_animation_indices = m_animation[RIGHT];
            m_can_damage = false;
        }

        timer += 2 * delta_time;
    }
}

void Entity::update(float delta_time, Entity* player, Entity* collidable_entities1, int collidable_entity_count, Map* map)
{
    if (!m_is_active) return;

    if (m_enemy_type == LIFE) {
        m_model_matrix = glm::mat4(1.0f);
        m_model_matrix = glm::translate(m_model_matrix, m_position);
        return;
    }

    if (m_entity_type == ENEMY) {
        activate_ai(player, delta_time);
    }

    if (m_entity_type == SWORD) {
        if (flying) {
            if (fire_dir == LEFT) {
                m_movement.x = -1.0f;
            }
            else if (fire_dir == RIGHT) {
                m_movement.x = 1.0f;
            }
            else if (fire_dir == UP) {
                m_movement.y = 1.0f;
            }
            else if (fire_dir == DOWN) {
                m_movement.y = -1.0f;
            }
        }
        else {
            m_position = player->m_position;
            m_position.y = player->m_position.y - player->m_width / 4;
            m_movement = glm::vec3(0.0f);
        }   
    }
    
    if (m_animation_indices != NULL)
    {
        if (glm::length(m_movement) != 0)
        {
            m_animation_time += delta_time;
            float frames_per_second = (float)1 / SECONDS_PER_FRAME;

            if (m_animation_time >= frames_per_second)
            {
                m_animation_time = 0.0f;
                m_animation_index++;

                if (m_animation_index >= m_animation_frames)
                {
                    m_animation_index = 0;
                }
            }
        }

        // collision checks
        m_collided_top = false;
        m_collided_bottom = false;
        m_collided_left = false;
        m_collided_right = false;

        m_velocity.x = m_movement.x * m_speed;
        m_velocity.y = m_movement.y * m_speed;

        m_position.x += m_velocity.x * delta_time;
        check_collision_x(collidable_entities1, collidable_entity_count);
        check_collision_x(map);

        m_position.y += m_velocity.y * delta_time;
        check_collision_y(collidable_entities1, collidable_entity_count);
        check_collision_y(map);

        m_position += m_movement * delta_time;

        //����� TRANSFORMATIONS ����� //
        m_model_matrix = glm::mat4(1.0f);
        m_model_matrix = glm::translate(m_model_matrix, m_position);
    }

   

}

void const Entity::check_collision_y(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float y_distance = fabs(m_position.y - collidable_entity->get_position().y);
            float y_overlap = fabs(y_distance - (m_height / 2.0f) - (collidable_entity->get_height() / 2.0f));
            if (m_velocity.y > 0) {
                m_position.y -= y_overlap;
                m_velocity.y = 0;
                m_collided_top = true;
                
            }
            else if (m_velocity.y < 0) {
                m_position.y += y_overlap;
                m_velocity.y = 0;
                m_collided_entity_bottom = true;
                
            }

            if (m_entity_type == PLAYER) {
                dead = true;
            }

            if (m_entity_type == SWORD) {
                hit = true;
                collidable_entity->deactivate();
            }
        }


    }
}

void const Entity::check_collision_x(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float x_distance = fabs(m_position.x - collidable_entity->get_position().x);
            float x_overlap = fabs(x_distance - (m_width / 2.0f) - (collidable_entity->get_width() / 2.0f));
            if (m_velocity.x > 0) {
                m_position.x -= x_overlap;
                m_velocity.x = 0;
                m_collided_right = true;
                
            }
            else if (m_velocity.x < 0) {
                m_position.x += x_overlap;
                m_velocity.x = 0;
                m_collided_left = true;
                
            }
            
            if (m_entity_type == PLAYER) {
                dead = true;
            }

            if (m_entity_type == SWORD) {
                hit = true;
                collidable_entity->deactivate();
            }

        }

       
    }
}

void const Entity::check_collision_y(Map* map)
{
    // Probes for tiles above
    glm::vec3 top = glm::vec3(m_position.x, m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_left = glm::vec3(m_position.x - (5 * m_width / 12), m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_right = glm::vec3(m_position.x + (5 * m_width / 12), m_position.y + (m_height / 2), m_position.z);

    // Probes for tiles below
    glm::vec3 bottom = glm::vec3(m_position.x, m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_left = glm::vec3(m_position.x - (5 * m_width / 12), m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_right = glm::vec3(m_position.x + (5 * m_width / 12), m_position.y - (m_height / 2), m_position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    // If the map is solid, check the top three points
    if (map->is_solid(top, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
        if (m_entity_type == SWORD) {
            hit = true;
        }
    }
    else if (map->is_solid(top_left, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
        if (m_entity_type == SWORD) {
            hit = true;
        }
    }
    else if (map->is_solid(top_right, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
        if (m_entity_type == SWORD) {
            hit = true;
        }
    }

    // And the bottom three points
    if (map->is_solid(bottom, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
        if (m_entity_type == SWORD) {
            hit = true;
        }

    }
    else if (map->is_solid(bottom_left, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
        if (m_entity_type == SWORD) {
            hit = true;
        }

    }
    else if (map->is_solid(bottom_right, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
        if (m_entity_type == SWORD) {
            hit = true;
        }
    }

}

void const Entity::check_collision_x(Map* map)
{
    // Probes for tiles on left
    glm::vec3 left = glm::vec3(m_position.x - (5 * m_width / 12), m_position.y, m_position.z);
    glm::vec3 top_left = glm::vec3(m_position.x - (5 * m_width / 12), m_position.y + (m_height / 3), m_position.z);
    glm::vec3 bottom_left = glm::vec3(m_position.x - (5 * m_width / 12), m_position.y - (m_height / 3), m_position.z);

    // Probes for tiles on the right
    glm::vec3 right = glm::vec3(m_position.x + (5 * m_width / 12), m_position.y, m_position.z);
    glm::vec3 top_right = glm::vec3(m_position.x + (5 * m_width / 12), m_position.y + (m_height / 3), m_position.z);
    glm::vec3 bottom_right = glm::vec3(m_position.x + (5 * m_width / 12), m_position.y - (m_height / 3), m_position.z);

    float penetration_x = 0;
    float penetration_y = 0;


    if (map->is_solid(left, &penetration_x, &penetration_y) && m_velocity.x < 0)
    {
        
        m_position.x += penetration_x;
        m_velocity.x = 0;
        m_collided_left = true;
        if (m_entity_type == SWORD) {
            hit = true;
        }
    }
    if (map->is_solid(top_left, &penetration_x, &penetration_y) && m_velocity.x < 0)
    {

        m_position.x += penetration_x;
        m_velocity.x = 0;
        m_collided_left = true;
        if (m_entity_type == SWORD) {
            hit = true;
        }
    }
    if (map->is_solid(bottom_left, &penetration_x, &penetration_y) && m_velocity.x < 0)
    {

        m_position.x += penetration_x;
        m_velocity.x = 0;
        m_collided_left = true;
        if (m_entity_type == SWORD) {
            hit = true;
        }
    }
    if (map->is_solid(right, &penetration_x, &penetration_y) && m_velocity.x > 0)
    {
        
        m_position.x -= penetration_x;
        m_velocity.x = 0;
        m_collided_right = true;
        if (m_entity_type == SWORD) {
            hit = true;
        }
    }
    if (map->is_solid(top_right, &penetration_x, &penetration_y) && m_velocity.x > 0)
    {

        m_position.x -= penetration_x;
        m_velocity.x = 0;
        m_collided_right = true;
        if (m_entity_type == SWORD) {
            hit = true;
        }
    }
    if (map->is_solid(bottom_right, &penetration_x, &penetration_y) && m_velocity.x > 0)
    {

        m_position.x -= penetration_x;
        m_velocity.x = 0;
        m_collided_right = true;
        if (m_entity_type == SWORD) {
            hit = true;
        }
    }

}

void Entity::render(ShaderProgram* program)
{
    program->set_model_matrix(m_model_matrix);

    if (!m_is_active) { return; }

    if (m_animation_indices != NULL)
    {
        draw_sprite_from_texture_atlas(program, m_texture_id, m_animation_indices[m_animation_index]);
        return;
    }

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());

    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    scale();
}

bool const Entity::check_collision(Entity* other) const
{
    if (other == this) return false;
    // If either entity is inactive, there shouldn't be any collision
    if (!m_is_active || !other->m_is_active) return false;

    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % m_animation_cols) / (float)m_animation_cols;
    float v_coord = (float)(index / m_animation_cols) / (float)m_animation_rows;

    // Step 2: Calculate its UV size
    float width = 1.0f / (float)m_animation_cols;
    float height = 1.0f / (float)m_animation_rows;

    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}