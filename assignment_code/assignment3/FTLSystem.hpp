#ifndef FTL_SYSTEM_BASE_H_
#define FTL_SYSTEM_BASE_H_

#include "ParticleState.hpp"
#include "ParticleSystemBase.hpp"

namespace GLOO {
class FTLSystem : public ParticleSystemBase {
 public:
    bool wind_on_ = false;
    std::vector<glm::vec3> collision_forces_ = {};
    FTLSystem() {
            for(int i = 0; i < 100; i++){
        collision_forces_.push_back(glm::vec3(0.f));
    }
    }; 
virtual ~FTLSystem(){}

void AddMasses(std::vector<float> masses){
    for (float mass: masses) {
        masses_.push_back(mass);
    }
}

void AddCollisionForce(glm::vec3 force){
    collision_forces_.push_back(force);
}

void AddRadii(std::vector<float> radii){
    for (float radius: radii) {
        radii_.push_back(radius);
    }
}

void FixParticle(std::vector<bool> fix){
    fixed_particles_ = fix;
}

virtual ParticleState ComputeTimeDerivative(const ParticleState& state,
                                            float time) const{
    std::vector<glm::vec3> new_particle_velocities;   
    std::vector<glm::vec3> new_particle_positions;                                        
                                    
    std::vector<glm::vec3> particle_positions = state.positions;
    std::vector<glm::vec3> particle_velocities = state.velocities;

    for (int i = 0; i < particle_positions.size(); i++){
        if (fixed_particles_[i]){
            new_particle_velocities.push_back(glm::vec3(0.f));
            new_particle_positions.push_back(state.positions[i]);
        }
        else{
            glm::vec3 gravity = glm::vec3(0.f,-1.f, 0.f) * masses_[i]*9.81f;
            glm::vec3 drag = - drag_ * particle_velocities[i];

            int x = rand() % 100 ;
            glm::vec3 wind = glm::vec3(0.4f, 0.2f, 0.3f) * float(x);

            glm::vec3 acceleration;
            if (wind_on_){
                acceleration = 1.0f/masses_[i] * (gravity  + drag + wind);
            }
            else{
                acceleration = 1.0f/masses_[i] * (gravity  + drag);
            }
            new_particle_velocities.push_back(acceleration);
            new_particle_positions.push_back(particle_velocities[i]);
        }
    }
    return ParticleState{new_particle_positions, new_particle_velocities}; 
}


std::vector<glm::vec3> GetExternalForces(const ParticleState& state, float time) {
    std::vector<glm::vec3> external_forces;                                     
                                    
    std::vector<glm::vec3> particle_positions = state.positions;
    std::vector<glm::vec3> particle_velocities = state.velocities;

    for (int i = 0; i < particle_positions.size(); i++){
        if (fixed_particles_[i]){
            external_forces.push_back(glm::vec3(0.f));
        } else {
            glm::vec3 gravity = glm::vec3(0.f,-1.f, 0.f) * masses_[i] * 9.81f;
            glm::vec3 drag = - drag_ * particle_velocities[i];

            // int x = (rand() % 100);
            // glm::vec3 wind = glm::vec3(0.4f, 0.2f, 0.3f) * float(x);

            glm::vec3 force;
            if (wind_on_) {
                glm::vec3 wind = WindForce(time);
                force = (gravity  + drag + wind + collision_forces_[i]);
            } else {
                force = (gravity  + drag + collision_forces_[i]);
            }
            external_forces.push_back(force);
        }
    }
    collision_forces_ = {};
    return external_forces; 
}
float s_damp_ = 0.9f;
std::vector<float> radii_;

private:
float drag_ = 0.1;
std::vector<float> masses_;
std::vector<bool> fixed_particles_;
glm::vec3 WindForce(float t) const {
    return glm::vec3(15.f + 15.f * std::cos(5000.f * t), 10.f + 10.f * std::cos(2500.f * (t - 0.3f)), 30.f + 20.f * std::cos(4000.f * (t - 0.5f)));
}
};


}  // namespace GLOO

#endif
