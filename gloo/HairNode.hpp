//#ifndef GLOO_LAB3PT2_SCENE_NODE_H_
//#define GLOO_LAB3PT2_SCENE_NODE_H_

#include <vector>
#include <memory>
#include <unordered_map>
#include <iostream>
#include <typeinfo>
#include <stdexcept>

#include <glm/vec3.hpp>

#include "components/ComponentBase.hpp"
#include "components/ComponentType.hpp"
#include "Transform.hpp"
#include "SceneNode.hpp"
#include "assignment_code/assignment3/RodSystem.hpp"
#include "assignment_code/assignment3/ParticleState.hpp"
#include "assignment_code/assignment3/ForwardEulerIntegrator.hpp"
#include "debug/PrimitiveFactory.hpp"
#include "assignment_code/assignment3/IntegratorFactory.hpp"
#include "assignment_code/assignment3/IntegratorType.hpp"
#include "gloo/shaders/SimpleShader.hpp"
#include "gloo/InputManager.hpp"






namespace GLOO {
class HairNode : public SceneNode {
 public:
  HairNode(float step_size, IntegratorType integrator_type, glm::vec3 root_pos, float length, ){
    //initialize position, velocity, mass, and fixed particles
    length_ = length;
    root_pos_ = root_pos;
    std::vector<glm::vec3> positions = {};
    std::vector<glm::vec3> rest_positions = {};
    std::vector<glm::vec3> velocities = {};
    std::vector<float> masses = {};
    std::vector<bool> fixed_particle_list = {};
    auto indices = make_unique<IndexArray>();

    // https://www.cs.columbia.edu/cg/pdfs/143-rods.pdf for more on discrete Kirchoff rods (section 4.2)
    // See section 5.2 for 'discrete representation' of Kirchhoff rods (which are what we are using as our hair model)
    for (int i = 0; i < num_joints; i++){
        velocities.push_back(glm::vec3(0.0f));
        masses.push_back(1.0f);
        if (i == 0) {
            fixed_particle_list.push_back(true);
            rest_positions.push_back(root_pos_);
        }
        else{
            fixed_particle_list.push_back(false);
            rest_positions.push_back(root_pos_ - glm::vec3(0.f, float(i) * length / float(num_joints), 0.f));
        }
    }

    state_ = ParticleState{positions, velocities};
    system_ = RodSystem();
    system_.AddMasses(masses);
    system_.FixParticle(fixed_particle_list);

    //add springs
    // for (int i = 0; i < n; i++){
    //     for (int j = 0; j < n; j++){
            
    //         int current_index = i*n + j;
    //         indices->push_back(current_index);
    //         //don't draw anything at corner
    //         if (i == n-1 && j == n-1){
    //             break;
    //         }
    //         //only draw down; at vertical edge

    //         else if (j == n-1){
    //             indices->push_back(current_index + n);
    //             //indices->push_back(current_index);
    //             system_.AddSpring(current_index, current_index + n, 100.f, 1.f);

    //         }

    //         //only draw right
    //         else if (i == n-1){
    //             indices->push_back(current_index + 1);
    //             indices->push_back(current_index);
    //             system_.AddSpring(current_index, current_index + 1, 100.f, 1.f);

    //         }
    //         else{
    //         indices->push_back(current_index + 1);
    //         indices->push_back(current_index);
    //         indices->push_back(current_index + n);
    //         indices->push_back(current_index);

    //         system_.AddSpring(current_index, current_index + 1, 100.f, 1.f);
    //         system_.AddSpring(current_index, current_index + n, 100.f, 1.f);
    //         }

    //         //DIAGONAL SPRINGS
    //         //diagonal right
    //         if (j == 0 && i != n-1){
    //             indices->push_back(current_index + n + 1);
    //             indices->push_back(current_index);

    //             system_.AddSpring(current_index, current_index + n + 1, 100.f, std::sqrt(2.0));

    //         }
    //         //diagonal left
    //         else if ( j==n-1 && i != n-1){
    //             indices->push_back(current_index + n - 1);
    //             indices->push_back(current_index);
    //             system_.AddSpring(current_index, current_index + n - 1, 100.f, std::sqrt(2.0));

    //         }
    //         else if (i!= n-1){
    //             indices->push_back(current_index + n + 1);
    //             indices->push_back(current_index);
    //             system_.AddSpring(current_index, current_index + n + 1, 100.f, std::sqrt(2.0));
    //             indices->push_back(current_index + n - 1);
    //             indices->push_back(current_index);
    //             system_.AddSpring(current_index, current_index + n - 1, 100.f, std::sqrt(2.0));
    //         }
    //         //SHEAR SPRINGS
    //         if (i < n-2 && j < n - 2){
    //             indices->push_back(current_index + 2);
    //             indices->push_back(current_index);
    //             indices->push_back(current_index+2*n);
    //             indices->push_back(current_index);
    //             system_.AddSpring(current_index, current_index + 2, 100.f, 2.f);
    //             system_.AddSpring(current_index, current_index + 2*n, 100.f, 2.f);

    //         }
    //         else if (j <  n - 2){
    //             indices->push_back(current_index + 2);
    //             indices->push_back(current_index);
    //             system_.AddSpring(current_index, current_index + 2, 100.f, 2.f);
    //         }
    //         else if (i < n -2){
    //             indices->push_back(current_index + 2*n);
    //             indices->push_back(current_index);
    //             system_.AddSpring(current_index, current_index + 2*n, 100.f, 2.f);
    //         }
    //     }
    // }

    //DRAW SPRINGS
    // curve_polyline_ = std::make_shared<VertexObject>();
    // auto positions_ptr = make_unique<PositionArray>();
    // for (glm::vec3 pos : positions){
    //     positions_ptr->push_back(pos);
    // }
    // curve_polyline_->UpdatePositions(std::move(positions_ptr));
    // curve_polyline_->UpdateIndices(std::move(indices));
    // polyline_shader_ = std::make_shared<SimpleShader>();
    // auto line_node = make_unique<SceneNode>();
    // auto shader = std::make_shared<SimpleShader>();
    // line_node->CreateComponent<ShadingComponent>(polyline_shader_);
    // auto& rc = line_node->CreateComponent<RenderingComponent>(curve_polyline_);
    // rc.SetDrawMode(DrawMode::Lines);
    // //rc.SetDrawRange(0, (N_SUBDIV_ - 2)*2+2);
    // glm::vec3 color(1.f, 1.f, 1.f);
    // auto material = std::make_shared<Material>(color, color, color, 0);
    // line_node->CreateComponent<MaterialComponent>(material);
    // AddChild(std::move(line_node));

    //INITIALIZE OTHER VARIABLES
    // prev_released_ = true;
    // prev_released_2_ = true;
    // step_size_ = step_size;
    // integrator_ = IntegratorFactory::CreateIntegrator<PendulumSystem, ParticleState>(integrator_type); 
    // current_time_ = 0;
    // sphere_mesh_ = PrimitiveFactory::CreateSphere(0.1f, 25, 25);
    // shader_ = std::make_shared<PhongShader>();
    // sphere_node_ptrs_ = {};

    //DRAW SPHERES
//     for (int i =0; i < masses.size(); i++){
//         auto sphere_node = make_unique<SceneNode>();
//         sphere_node->CreateComponent<ShadingComponent>(shader_);
//         sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
//         sphere_node_ptrs_.push_back(sphere_node.get());
//         AddChild(std::move(sphere_node));
//     }
  }

  virtual void Update(double delta_time) {

    //RESET DYNAMICS BY PRESSING R
    if (InputManager::GetInstance().IsKeyPressed('R')) {
        if (prev_released_) {
            std::cout << "test" << std::endl;
            std::vector<glm::vec3> positions = {};
            std::vector<glm::vec3> velocities = {};
    
            for (int i = 0; i < n; i++){
                for (int j = 0; j < n; j++){
                    velocities.push_back(glm::vec3(0.0f));
                    if ((i == 0 & j == 0) || (i == 0 && j == n - 1)){
                        positions.push_back(glm::vec3(float(j), float(i), 0.f));
                    }
                    else{
                        positions.push_back(glm::vec3(float(j), -float(i), 0.f));
                    }
                }
            }

            state_ = ParticleState{positions, velocities};
        }       
        prev_released_ = false;
    }
    else {
        prev_released_ = true;
    }
     
    if (InputManager::GetInstance().IsKeyPressed('W')){
        if (prev_released_2_){
            system_.wind_on_ = ! system_.wind_on_;
            std::cout << system_.wind_on_ << std::endl;
        }
         prev_released_2_ = false;
    }  
    else {
        prev_released_2_ = true;
    }

    //INTEGRATE
    int number_steps = int(delta_time/step_size_);
    float remaining_step = delta_time - number_steps*step_size_;
    //regular sized steps
    for (int i = 0; i < number_steps; i++){
      state_ = integrator_->Integrate(system_, state_, current_time_, step_size_);
      current_time_ += step_size_;
    }
    //last remaining step
    state_ = integrator_->Integrate(system_, state_, current_time_, remaining_step);
    current_time_ += remaining_step;
    
    //REDRAW SPHERES
    // auto positions_ptr = make_unique<PositionArray>();
    // for (int i = 0; i < sphere_node_ptrs_.size(); i++){
    //     sphere_node_ptrs_[i]->GetTransform().SetPosition(state_.positions[i]);
    //     positions_ptr->push_back(state_.positions[i]);

    // }
    // curve_polyline_->UpdatePositions(std::move(positions_ptr));
    }

 private:
     int num_joints = 10; // for discretization of Kirchhoff rod
     glm::vec3 root_pos_;
     float length_;

     std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState> > integrator_;
     ParticleState state_;
     PendulumSystem system_;
     float step_size_;
     float current_time_;
     std::shared_ptr<VertexObject> sphere_mesh_;
     std::shared_ptr<ShaderProgram> shader_;
     std::vector<SceneNode *> sphere_node_ptrs_;
     std::shared_ptr<VertexObject> curve_polyline_;
     std::shared_ptr<ShaderProgram> polyline_shader_;
     bool prev_released_;
     bool prev_released_2_;

};  // namespace GLOO
}
