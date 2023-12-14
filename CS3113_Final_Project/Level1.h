#pragma once
#include "Scene.h"

class Level1 : public Scene {
public:
    // ————— STATIC ATTRIBUTES ————— //
    int ENEMY_COUNT = 2;
    
    // ————— CONSTRUCTOR ————— //
    ~Level1();
    
    // ————— METHODS ————— //
    void initialize() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
    int get_number_of_enemies() { return ENEMY_COUNT; }
};
