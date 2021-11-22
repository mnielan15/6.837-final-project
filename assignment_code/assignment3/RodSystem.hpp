#ifndef ROD_SYSTEM_BASE_H_
#define ROD_SYSTEM_BASE_H_

#include "ParticleState.hpp"
#include "ParticleSystemBase.hpp"
#include "external/src/glm-0.9.9.8/glm/gtx/vector_angle.hpp"

namespace GLOO {
class RodSystem : public ParticleSystemBase {
 public:
    bool wind_on_ = false;
    RodSystem() {}; 
virtual ~RodSystem(){}


void AddMasses(std::vector<float> masses){
    for (float mass: masses){
        masses_.push_back(mass);
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
            new_particle_positions.push_back(glm::vec3(0.f));
        }
        else{
            glm::vec3 gravity = glm::vec3(0.f,-1.f, 0.f) * masses_[i]*9.81f;
            glm::vec3 drag = - k * particle_velocities[i];


            // TODO: These bending and twisting energies are summations over each node, so we might
            //       need to compute these in a for loop before this for loop.

            // TODO: compute 'discrete bending energy'--Elastic Rods, section 4.2.1
            // TODO: do something different for the i == 0 and i == last
            glm::vec3 prev_vec = particle_positions[i] - particle_positions[i-1];
            glm::vec3 this_vec = particle_positions[i+1] - particle_positions[i];
            float phi_i = glm::angle(glm::normalize(prev_vec), glm::normalize(this_vec)); // angle of the kink at joint i
            float discrete_curvature = 2.f * glm::tan(phi_i);
            glm::vec3 discrete_curvature_binormal = 2.f * glm::cross(prev_vec, this_vec) / (glm::length(prev_vec) * glm::length(this_vec) + glm::dot(prev_vec, this_vec));
            glm::vec3 m_1, m_2; // TODO: make these in HairNode.hpp and then get them here--these are the cross sections of the rod segment
            glm::vec2 material_curvature = glm::vec2(glm::dot(discrete_curvature_binormal, m_2), -1.f * glm::dot(discrete_curvature_binormal, m_1));
            float bending_energy; // TODO: compute this

            // TODO: compute 'discrete twisting energy'--Elastic Rods, section 4.2.2
            glm::mat3 P_i; // TODO: compute this--this is the discrete parallel transport matrix
            glm::vec3 u_i1; // TODO: this is u_(i-1)     need to create these in HairNode
            glm::vec3 u_i = P_i * u_i1;
            glm::vec3 t_i; // TODO: create these tangents in HairNode, then get t_i here
            glm::vec3 v_i = glm::cross(t_i, u_i);
            // TODO: update the Bishop frame here
            float theta_i; // TODO: compute this
            float theta_i1; // theta_(i-1)--i.e. the theta from the previous node   TODO: get this
            float twisting_energy; // TODO: compute this

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

            // Inextensibility property (i.e. rods always stay the same length)
            // glm::vec3 prev_old_pos = particle_positions[i-1];
            // glm::vec3 this_old_pos = particle_positions[i];
            float old_length = glm::length(prev_vec);

            new_particle_positions.push_back(glm::normalize(particle_velocities[i]) * old_length);
        }
    }
    return ParticleState{new_particle_positions, new_particle_velocities}; 
    }
private:
    float k = 0.8;
    std::vector<float> masses_;
    std::vector<bool> fixed_particles_;
};


}  // namespace GLOO

#endif
