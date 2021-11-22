#ifndef PARTICLE_SYSTEM_SIMPLE_H_
#define PARTICLE_SYSTEM_SIMPLE_H_

#include "ParticleSystemBase.hpp"
#include "ParticleState.hpp"

namespace GLOO {
class ParticleSystemSimple : public ParticleSystemBase {
 public:
  virtual ~ParticleSystemSimple() {}

    ParticleState ComputeTimeDerivative(const ParticleState& state,
                                              float time) const {
                                 
        // f(x, y, z) = (-y, x, 0)   
        std::vector<glm::vec3> new_particle_positions;                                        
        std::vector<glm::vec3> particles = state.positions;
        for (glm::vec3 particle : particles){
            glm::vec3 new_positions(-1.f*particle[1], particle[0], 0*particle[2]);
            new_particle_positions.push_back(new_positions);
        }

        //where is the constructor for Particle state
        return ParticleState{new_particle_positions, state.velocities};                         
};
                                              
                                              };
                                              };// namespace GLOO
#endif
