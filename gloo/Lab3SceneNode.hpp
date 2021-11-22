//#ifndef GLOO_SCENE_NODE_H_
//#define GLOO_SCENE_NODE_H_

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
#include "assignment_code/assignment3/ParticleSystemSimple.hpp"
#include "assignment_code/assignment3/ParticleState.hpp"
#include "assignment_code/assignment3/ForwardEulerIntegrator.hpp"
#include "assignment_code/assignment3/TrapezoidIntegrator.hpp"
#include "assignment_code/assignment3/RK4Integrator.hpp"

#include "debug/PrimitiveFactory.hpp"
#include "assignment_code/assignment3/IntegratorFactory.hpp"
#include "assignment_code/assignment3/IntegratorType.hpp"





namespace GLOO {
class Lab3SceneNode : public SceneNode {
 public:
 //constructor?
 //
  Lab3SceneNode(float step_size, IntegratorType integrator_type){
    step_size_ = step_size;
    std::vector<glm::vec3> positions = {glm::vec3(1.0f, 1.0f, 0.0f)};
    std::vector<glm::vec3> velocities = {glm::vec3(0.0f, 0.0f, 0.0f)};

    state_ = ParticleState{positions, velocities};
    system_ = ParticleSystemSimple();
    integrator_ = IntegratorFactory::CreateIntegrator<ParticleSystemSimple, ParticleState>(integrator_type); //ForwardEulerIntegrator<ParticleState, ParticleSystemSimple>(state_, system_);
    current_time_ = 0;
    sphere_mesh_ = PrimitiveFactory::CreateSphere(0.1f, 25, 25);
    shader_ = std::make_shared<PhongShader>();
    sphere_node_ptr_ = nullptr;

    std::cout << "help" << std::endl;
    auto sphere_node = make_unique<SceneNode>();
    sphere_node->CreateComponent<ShadingComponent>(shader_);
    sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
    glm::vec3 color(0.f, 0.f, 1.f);
    auto material = std::make_shared<Material>(color, color, color, 0);
    sphere_node->CreateComponent<MaterialComponent>(material);
    sphere_node_ptr_ = sphere_node.get();
    AddChild(std::move(sphere_node));

   
        //auto child_node_position = child_node->GetTransform().GetPosition();

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
    sphere_node_ptr_->GetTransform().SetPosition(state_.positions[0]);
    }

 private:
  //I added these
  //where do I save stepsize from command line? 
  std::unique_ptr<IntegratorBase<ParticleSystemSimple, ParticleState>> integrator_;
  ParticleState state_;
  ParticleSystemSimple system_;
  float step_size_;
  float current_time_;
  std::shared_ptr<VertexObject> sphere_mesh_;
  std::shared_ptr<ShaderProgram> shader_;
  SceneNode* sphere_node_ptr_;

  //put step size

};  // namespace GLOO
}
