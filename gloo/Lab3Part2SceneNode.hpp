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
#include "assignment_code/assignment3/PendulumSystem.hpp"
#include "assignment_code/assignment3/ParticleState.hpp"
#include "assignment_code/assignment3/ForwardEulerIntegrator.hpp"
#include "debug/PrimitiveFactory.hpp"
#include "assignment_code/assignment3/IntegratorFactory.hpp"
#include "assignment_code/assignment3/IntegratorType.hpp"





namespace GLOO {
class Lab3Part2SceneNode : public SceneNode {
 public:
  Lab3Part2SceneNode(float step_size, IntegratorType integrator_type){
    step_size_ = step_size;
    std::vector<glm::vec3> positions = {glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(2.0f, 1.0f, 0.0f), glm::vec3(3.0f, 1.0f, 0.0f)};
    std::vector<glm::vec3> velocities = {glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)};
    std::vector<float> masses = {1.0f, 1.0f, 1.0f, 1.0f};

    state_ = ParticleState{positions, velocities};
    system_ = PendulumSystem();
    system_.AddMasses(masses);
    system_.AddSpring(0, 1, 10.0f, 0.1f);
    system_.AddSpring(1, 2, 10.0f, 0.1f);
    system_.AddSpring(2, 3, 10.0f, 0.1f);

    system_.FixParticle({true, false, false, false});

    integrator_ = IntegratorFactory::CreateIntegrator<PendulumSystem, ParticleState>(integrator_type); 
    current_time_ = 0;
    sphere_mesh_ = PrimitiveFactory::CreateSphere(0.1f, 25, 25);
    shader_ = std::make_shared<PhongShader>();
    sphere_node_ptrs_ = {};

    for (int i =0; i < masses.size(); i++){
        auto sphere_node = make_unique<SceneNode>();
        sphere_node->CreateComponent<ShadingComponent>(shader_);
        sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
         glm::vec3 color(1.f, 0.f, 1.f);
        auto material = std::make_shared<Material>(color, color, color, 0);
        sphere_node->CreateComponent<MaterialComponent>(material);
        sphere_node_ptrs_.push_back(sphere_node.get());
        AddChild(std::move(sphere_node));
    }
  }

  virtual void Update(double delta_time) {
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
    
    //redraw sphere
    for (int i = 0; i < sphere_node_ptrs_.size(); i++){
        sphere_node_ptrs_[i]->GetTransform().SetPosition(state_.positions[i]);
    }
    }

 private:
  //I added these
  std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator_;
  ParticleState state_;
  PendulumSystem system_;
  float step_size_;
  float current_time_;
  std::shared_ptr<VertexObject> sphere_mesh_;
  std::shared_ptr<ShaderProgram> shader_;
  std::vector<SceneNode* > sphere_node_ptrs_;

  //put step size

};  // namespace GLOO
}
