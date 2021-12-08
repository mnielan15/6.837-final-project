#include "SimulationApp.hpp"

#include "glm/gtx/string_cast.hpp"

#include "gloo/shaders/PhongShader.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/CameraComponent.hpp"
#include "gloo/components/LightComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/MeshLoader.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/lights/AmbientLight.hpp"
#include "gloo/cameras/ArcBallCameraNode.hpp"
#include "gloo/debug/AxisNode.hpp"
#include "gloo/Lab3SceneNode.hpp"
#include "gloo/Lab3Part2SceneNode.hpp"
#include "gloo/Lab3Part3SceneNode.hpp"
#include "gloo/HairNode.hpp"
#define _USE_MATH_DEFINES
 
#include <cmath>
#include <iostream>



namespace GLOO {
SimulationApp::SimulationApp(const std::string& app_name,
                             glm::ivec2 window_size,
                             IntegratorType integrator_type,
                             float integration_step)
    : Application(app_name, window_size),
      integrator_type_(integrator_type),
      integration_step_(integration_step) {
  // TODO: remove the following two lines and use integrator type and step to
  // create integrators; the lines below exist only to suppress compiler
  // warnings.

  //UNUSED(integrator_type_);
  //UNUSED(integration_step_);
}

void SimulationApp::SetupScene() {
  SceneNode& root = scene_->GetRootNode();

  auto camera_node = make_unique<ArcBallCameraNode>(45.f, 0.75f, 5.0f);
  scene_->ActivateCamera(camera_node->GetComponentPtr<CameraComponent>());
  root.AddChild(std::move(camera_node));

  root.AddChild(make_unique<AxisNode>('A'));

  auto ambient_light = std::make_shared<AmbientLight>();
  ambient_light->SetAmbientColor(glm::vec3(0.2f));
  root.CreateComponent<LightComponent>(ambient_light);

  auto point_light = std::make_shared<PointLight>();
  point_light->SetDiffuseColor(glm::vec3(0.8f, 0.8f, 0.8f));
  point_light->SetSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
  point_light->SetAttenuation(glm::vec3(1.0f, 0.09f, 0.032f));
  auto point_light_node = make_unique<SceneNode>();
  point_light_node->CreateComponent<LightComponent>(point_light);
  point_light_node->GetTransform().SetPosition(glm::vec3(0.0f, 2.0f, 4.f));
  root.AddChild(std::move(point_light_node));

  //make new lab3scenenode
  //TODO fix constructor
  //state
  //systemstate

  //auto spiral_node = make_unique<Lab3SceneNode>(integration_step_, integrator_type_);
  //root.AddChild(std::move(spiral_node));

  // auto pendulum_node = make_unique<Lab3Part2SceneNode>(integration_step_, integrator_type_);
  // pendulum_node->shift();
  // root.AddChild(std::move(pendulum_node));

  // auto pendulum_node2 = make_unique<Lab3Part2SceneNode>(integration_step_, integrator_type_);
  // root.AddChild(std::move(pendulum_node2));

  // auto pendulum_node3 = make_unique<Lab3Part2SceneNode>(integration_step_, integrator_type_);
  // root.AddChild(std::move(pendulum_node3));

  //auto hair_node = make_unique<HairNode>(integration_step_, integrator_type_, glm::vec3(1.f), 1.5f);
  //root.AddChild(std::move(hair_node));

  // for (float i = -1; i <=1; i+= 0.1){
  //   for (float j = -1; j <= 1; j+= 0.1){
  //     float z = glm::sqrt(1-i*i + j*j);
  //     auto hair_node = make_unique<HairNode>(integration_step_, integrator_type_, glm::vec3(i, j, z), 1.0f);
  //     root.AddChild(std::move(hair_node));
  //   }
  // }

  int N = 100;
  int n = 0;
  float r = 1;
  float a = 4*M_PI* r*r / float(N);
  float d = sqrt(a);
  float M_theta = round(M_PI/d);
  float d_theta = M_PI / M_theta;
  float d_phi = a / d_theta;
  float M_phi;
  for (int m = 0; m< M_theta; m++){
    float theta = M_PI*(m + 0.5)/M_theta;
    M_phi = round(2*M_PI*sin(theta)/d_phi);
    for (n = 0; n < M_phi; n++){
      float phi = 2*M_PI*n/M_phi;
      glm::vec3 point = {r*sin(theta)*cos(phi), r*sin(theta)*sin(phi), r*cos(theta)};
      auto hair_node = make_unique<HairNode>(integration_step_, integrator_type_, point, 1.0f);
      root.AddChild(std::move(hair_node));
      n+=1;
    }
  }
  // auto cloth_node = make_unique<Lab3Part3SceneNode>(integration_step_, integrator_type_);
  // root.AddChild(std::move(cloth_node));


}
}  // namespace GLOO
