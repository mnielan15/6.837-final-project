#ifndef PENDULUM_SYSTEM_BASE_H_
#define PENDULUM_SYSTEM_BASE_H_

#include "ParticleState.hpp"
#include "ParticleSystemBase.hpp"

namespace GLOO {
class PendulumSystem : public ParticleSystemBase {
 public:
    bool wind_on_ = false;
    PendulumSystem() {}; 
virtual ~PendulumSystem(){}


void AddMasses(std::vector<float> masses){
    for (float mass: masses){
        masses_.push_back(mass);
        spring_rest_lengths_.push_back({});
        spring_constants_.push_back({});

    }
}

void AddSpring(int i, int j, float spring_constant, float spring_rest_length){
    spring_rest_lengths_[i].push_back(std::make_pair(j, spring_rest_length));
    spring_constants_[i].push_back(std::make_pair(j, spring_constant));
    
    spring_rest_lengths_[j].push_back(std::make_pair(i, spring_rest_length));
    spring_constants_[j].push_back(std::make_pair(i, spring_constant));

   


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
                //std::cout << i << glm::to_string(state.positions[i]) << std::endl;
                
                new_particle_velocities.push_back(glm::vec3(0.f));
                new_particle_positions.push_back(glm::vec3(0.f));
            }
            else{
                glm::vec3 gravity = glm::vec3(0.f,-1.f, 0.f) * masses_[i]*9.81f;
                glm::vec3 drag = - k * particle_velocities[i];

                int x = rand() % 100 + 1 - 50;
                glm::vec3 wind = glm::vec3(0.0f, 0.0f, 1.0f) * float(x);
                //calculate spring constants
                glm::vec3 spring_force(0.f, 0.f, 0.f);                   

                for (int j = 0;  j < spring_constants_[i].size(); j++){
                    //pair = (j, spring_constant or spring rest)

                    std::pair<int, float> current_pair_k = spring_constants_[i][j];
                    std::pair<int, float> current_pair_r = spring_rest_lengths_[i][j];

                    glm::vec3 d = particle_positions[i] - particle_positions[current_pair_k.first];
                    //std::cout << glm::to_string(d) << std::endl;
                    spring_force += -current_pair_k.second * (glm::length(d) - current_pair_r.second) * d / glm::length(d);
                }
                glm::vec3 acceleration;
                if (wind_on_){
                    acceleration = 1.0f/masses_[i] * (gravity  + drag + spring_force + wind);
                }
                else{
                    acceleration = 1.0f/masses_[i] * (gravity  + drag + spring_force);
                }
                new_particle_velocities.push_back(acceleration);
                new_particle_positions.push_back(particle_velocities[i]);
            }
        }
        //where is the constructor for Particle state
        return ParticleState{new_particle_positions, new_particle_velocities}; 
        }
private:
     float k = 0.8;
 std::vector<float> masses_;
 std::vector<std::vector<std::pair<int, float>>> spring_constants_ ;
std::vector<std::vector<std::pair<int, float>>> spring_rest_lengths_;
std::vector<bool> fixed_particles_;


};


}  // namespace GLOO

#endif
