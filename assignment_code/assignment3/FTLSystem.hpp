#ifndef FTL_SYSTEM_BASE_H_
#define FTL_SYSTEM_BASE_H_

#include "ParticleState.hpp"
#include "ParticleSystemBase.hpp"

namespace GLOO {
class FTLSystem : public ParticleSystemBase {
 public:
    bool wind_on_ = false;
    FTLSystem() {}; 
virtual ~FTLSystem(){}

void AddMasses(std::vector<float> masses){
    for (float mass: masses) {
        masses_.push_back(mass);
    }
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

            int x = rand() % 100 + 1 - 50;
            glm::vec3 wind = glm::vec3(0.0f, 0.0f, 1.0f) * float(x);

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


std::vector<glm::vec3> GetExternalForces(const ParticleState& state) {
    std::vector<glm::vec3> external_forces;                                     
                                    
    std::vector<glm::vec3> particle_positions = state.positions;
    std::vector<glm::vec3> particle_velocities = state.velocities;

    for (int i = 0; i < particle_positions.size(); i++){
        if (fixed_particles_[i]){
            external_forces.push_back(glm::vec3(0.f));
        } else {
            glm::vec3 gravity = glm::vec3(0.f,-1.f, 0.f) * masses_[i] * 9.81f;
            glm::vec3 drag = - drag_ * particle_velocities[i];

            int x = rand() % 100 + 1 - 50;
            glm::vec3 wind = glm::vec3(0.0f, 0.0f, 1.0f) * float(x);

            glm::vec3 force;
            if (wind_on_) {
                force = (gravity  + drag + wind);
            } else {
                force = (gravity  + drag);
            }
            external_forces.push_back(force);
        }
    }
    return external_forces; 
}
float s_damp_ = 0.9;
std::vector<float> radii_;

private:
float drag_ = 0.8;
std::vector<float> masses_;
std::vector<bool> fixed_particles_;


};


}  // namespace GLOO

#endif
