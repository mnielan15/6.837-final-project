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
#include "assignment_code/assignment3/FTLSystem.hpp"
#include "assignment_code/assignment3/ParticleState.hpp"
#include "assignment_code/assignment3/ForwardEulerIntegrator.hpp"
#include "debug/PrimitiveFactory.hpp"
#include "assignment_code/assignment3/IntegratorFactory.hpp"
#include "assignment_code/assignment3/IntegratorType.hpp"
#include "gloo/shaders/SimpleShader.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/InputManager.hpp"
#include "external/src/glm-0.9.9.8/glm/gtx/string_cast.hpp"
#include "external/src/glm-0.9.9.8/glm/gtx/perpendicular.hpp"
#include  "glm/gtx/string_cast.hpp"






namespace GLOO {
class HairNode : public SceneNode {
 public:
  HairNode(float step_size, IntegratorType integrator_type, glm::vec3 root_pos, float length, bool is_curly){
    //initialize position, velocity, mass, and fixed particles
    length_ = length;
    root_pos_ = root_pos;
    is_curly_ = is_curly;
    root_normal_ = glm::vec3(1.0f, 0.f, 0.f); // TODO: make this a field for the constructor
    std::vector<glm::vec3> positions = {};
    std::vector<glm::vec3> velocities = {};
    std::vector<float> masses = {};
    std::vector<bool> fixed_particle_list = {};
    radii_ = length_ / num_joints_;
    auto indices = make_unique<IndexArray>();

    for (int i = 0; i < num_joints_; i++){
        velocities.push_back(glm::vec3(0.0f));
        masses.push_back(1.0f);
        if (i == 0) {
            fixed_particle_list.push_back(true);
            positions.push_back(root_pos);
        }
        else{
            fixed_particle_list.push_back(false);
            positions.push_back(root_pos + glm::vec3(radii_ * i / glm::sqrt(3.f)));
        }
    }

    state_ = ParticleState{positions, velocities};
    system_ = FTLSystem();
    system_.AddMasses(masses);
    system_.FixParticle(fixed_particle_list);


    //DRAW LINE SEGMENTS
    // first, set indices for the line segments
    if (is_curly_) {
        int t = 0;
        for (int i = 0; i < positions.size(); i++) {
            indices->push_back(t);
            if (i != 0 && i < positions.size() - 1) {
                indices->push_back(t);
            }
            t++;
            if (i < positions.size() - 1) {
                for (int j = 1; j < normal_samples_; j++) {
                    indices->push_back(t);
                    indices->push_back(t);
                    t++;
                }
            }
        }
    } else {
        for (int i = 0; i < positions.size(); i++) {
            indices->push_back(i);
            if (i != 0 && i < positions.size() - 1) {
                indices->push_back(i);
            }
        }
    }

    // then create the polyline
    curve_polyline_ = std::make_shared<VertexObject>();
    auto positions_ptr = make_unique<PositionArray>();
    for (glm::vec3 pos : positions){
        positions_ptr->push_back(pos);
    }

    if (is_curly_) {
        UpdateCurlPositions();
    } else {
        curve_polyline_->UpdatePositions(std::move(positions_ptr));
    }
    curve_polyline_->UpdateIndices(std::move(indices));
    polyline_shader_ = std::make_shared<SimpleShader>();
    auto line_node = make_unique<SceneNode>();
    auto shader = std::make_shared<SimpleShader>();
    line_node->CreateComponent<ShadingComponent>(polyline_shader_);
    auto& rc = line_node->CreateComponent<RenderingComponent>(curve_polyline_);
    rc.SetDrawMode(DrawMode::Lines);
    // rc.SetDrawRange(0, (N_SUBDIV_ - 2)*2+2);

    // Some brown hair colors (from dark to light):
    //      0x23120B = (35, 18, 11) -> (0.136, 0.07, 0.043)
    //      0x3D2314 = (61, 35, 20) -> (0.238, 0.136, 0.078)
    //      0x5A3825 = (90, 56, 37) -> (0.352, 0.219, 0.145)
    //      0xCC9966 = (204, 153, 102) -> (0.797, 0.598, 0.398)
    glm::vec3 color(0.238, 0.136, 0.078);
    auto material = std::make_shared<Material>(color, color, color, 0);
    line_node->CreateComponent<MaterialComponent>(material);
    AddChild(std::move(line_node));

    //INITIALIZE OTHER VARIABLES
    prev_released_ = true;
    prev_released_2_ = true;
    step_size_ = step_size;
    integrator_ = IntegratorFactory::CreateIntegrator<FTLSystem, ParticleState>(integrator_type); 
    current_time_ = 0;
    sphere_mesh_ = PrimitiveFactory::CreateSphere(0.01f, 25, 25);
    shader_ = std::make_shared<SimpleShader>();
    sphere_node_ptrs_ = {};

    //DRAW JOINTS
    // for (int i =0; i < num_joints_; i++){
    //     auto sphere_node = make_unique<SceneNode>();
    //     sphere_node->CreateComponent<ShadingComponent>(shader_);
    //     sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
    //     sphere_node_ptrs_.push_back(sphere_node.get());
    //     AddChild(std::move(sphere_node));
    // }

    // STARTING ENDPOINT
    // auto sphere_node = make_unique<SceneNode>();
    // sphere_node->CreateComponent<ShadingComponent>(shader_);
    // sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
    // sphere_node.get()->GetTransform().SetPosition(root_pos + glm::vec3(length / glm::sqrt(3.f)));
    // AddChild(std::move(sphere_node));

    // // REST ENDPOINT
    // auto sphere_node2 = make_unique<SceneNode>();
    // sphere_node2->CreateComponent<ShadingComponent>(shader_);
    // sphere_node2->CreateComponent<RenderingComponent>(sphere_mesh_);
    // sphere_node2.get()->GetTransform().SetPosition(root_pos + glm::vec3(0.f, -1.f * length, 0.f));
    // AddChild(std::move(sphere_node2));
  }

  virtual void Update(double delta_time) {

    //RESET DYNAMICS BY PRESSING R
    if (InputManager::GetInstance().IsKeyPressed('R')) {
        if (prev_released_) {
            std::cout << "reset positions" << std::endl;
            std::vector<glm::vec3> positions = {};
            std::vector<glm::vec3> velocities = {};
            for (int i = 0; i < num_joints_; i++) {
                velocities.push_back(glm::vec3(0.f));
                if (i == 0) {
                    positions.push_back(state_.positions[0]);
                } else {
                    positions.push_back(state_.positions[0] + glm::vec3(radii_ * i / glm::sqrt(3.f)));
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
            std::cout << "wind toggled ";
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
        std::vector<glm::vec3> external_forces = system_.GetExternalForces(state_, current_time_);
        std::vector<glm::vec3> new_velocities = {};
        std::vector<glm::vec3> new_positions = {};
        std::vector<glm::vec3> d_is = {};
        std::vector<glm::vec3> p_x = {};
        p_x.push_back(glm::vec3(0.f));
        new_velocities.push_back(glm::vec3(0.f));
        new_positions.push_back(state_.positions[0]);
        d_is.push_back(glm::vec3(0.f));
        
        for (int i = 1; i < state_.positions.size(); i++) {
            glm::vec3 x = state_.positions[i];
            glm::vec3 v = state_.velocities[i];
            glm::vec3 f = external_forces[i];
            glm::vec3 p = x + step_size_ * v + step_size_ * step_size_ * f;
            glm::vec3 prev_node = new_positions[i - 1];
            // glm::vec3 prev_node = state_.positions[i - 1];

            // Project p onto sphere around state_.positions[i-1] with radius system_.radii[i-1]
            glm::vec3 from_i1_to_p = glm::normalize(p - prev_node);
            glm::vec3 projected_p = prev_node + radii_ * from_i1_to_p;

            // store the d_i's and then use d_{i+1} in the new_v calculation instead
            glm::vec3 d = projected_p - p; // correction vector for the above line
            d_is.push_back(d);
            p_x.push_back(projected_p - x);

            // Dynamic FTL:
            // glm::vec3 new_v = (projected_p - x) / float(step_size_);
            // new_velocities.push_back(new_v);

            new_positions.push_back(projected_p);

            //Hair - object collision
            if (glm::dot(projected_p, projected_p) < 1.0){
                glm::vec3 point_on_sphere = glm::normalize(projected_p - sphere_center_)*sphere_radius_;
                float d = glm::distance(point_on_sphere, projected_p);
                float k = 300.f;
                //std::cout << glm::to_string(point_on_sphere) << std::endl;
               // std::cout << glm::to_string(projected_p) << std::endl;

                glm::vec3 normal = glm::normalize(projected_p - sphere_center_);
                glm::vec3 penalty_force = k * d * normal;
                system_.AddCollisionForce(penalty_force);
            }
            else{
                system_.AddCollisionForce(glm::vec3(0.f));

            }
        }

        // Velocity correction:
        for (int i = 1; i < state_.positions.size(); i++) {
            glm::vec3 d_i1 = glm::vec3(0.f);
            glm::vec3 d_i = p_x[i];
            if (i < state_.positions.size() - 1) {
                d_i1 += d_is[i + 1];
            }
            glm::vec3 new_v = (d_i - system_.s_damp_ * d_i1) / float(step_size_);
            new_velocities.push_back(new_v);
        }

        state_.velocities = new_velocities;
        state_.positions = new_positions;
        current_time_ += step_size_;
    }

    // last remaining step
    std::vector<glm::vec3> external_forces = system_.GetExternalForces(state_, current_time_);
    std::vector<glm::vec3> new_velocities = {};
    std::vector<glm::vec3> new_positions = {};
    std::vector<glm::vec3> d_is = {};
    std::vector<glm::vec3> p_x = {};
    p_x.push_back(glm::vec3(0.f));
    new_velocities.push_back(glm::vec3(0.f));
    new_positions.push_back(state_.positions[0]);
    d_is.push_back(glm::vec3(0.f));
    for (int i = 1; i < state_.positions.size(); i++) {
        glm::vec3 x = state_.positions[i];
        glm::vec3 v = state_.velocities[i];
        glm::vec3 f = external_forces[i];
        glm::vec3 p = x + remaining_step * v + remaining_step * remaining_step * f;
        // glm::vec3 prev_node = state_.positions[i - 1];
        glm::vec3 prev_node = new_positions[i - 1];

        // Project p onto sphere around state_.positions[i-1] with radius system_.radii[i-1]
        glm::vec3 from_i1_to_p = glm::normalize(p - prev_node);
        glm::vec3 projected_p = prev_node + radii_ * from_i1_to_p;
        glm::vec3 d = projected_p - p; // correction vector for the above line
        d_is.push_back(d);
        p_x.push_back(projected_p - x);

        // Dynamic FTL:
        // glm::vec3 new_v = ((projected_p - x) - system_.s_damp_ * d) / float(remaining_step);
        // new_velocities.push_back(new_v);

        new_positions.push_back(projected_p);
                    if (glm::dot(projected_p, projected_p) < 1.0){
                glm::vec3 point_on_sphere = glm::normalize(projected_p - sphere_center_)*sphere_radius_;
                float d = glm::distance(point_on_sphere, projected_p);
                float k = 300.f;
                //std::cout << glm::to_string(point_on_sphere) << std::endl;
               // std::cout << glm::to_string(projected_p) << std::endl;

                glm::vec3 normal = glm::normalize(projected_p - sphere_center_);
                glm::vec3 penalty_force = k * d * normal;
                system_.AddCollisionForce(penalty_force);
            }
            else{
                system_.AddCollisionForce(glm::vec3(0.f));

            }
    }

    // Velocity correction:
    for (int i = 1; i < state_.positions.size(); i++) {
        glm::vec3 d_i1 = glm::vec3(0.f);
        glm::vec3 d_i = p_x[i];
        if (i < state_.positions.size() - 1) {
            d_i1 += d_is[i + 1];
        }
        glm::vec3 new_v = (d_i - system_.s_damp_ * d_i1) / float(remaining_step);
        new_velocities.push_back(new_v);
    }

    state_.velocities = new_velocities;
    state_.positions = new_positions;
    current_time_ += remaining_step;

    // REDRAW LINES
    if (is_curly_) {
        UpdateCurlPositions();
    } else {
        auto positions_ptr = make_unique<PositionArray>();
        for (int i = 0; i < state_.positions.size(); i++) {
            positions_ptr->push_back(state_.positions[i]);
        }
        curve_polyline_->UpdatePositions(std::move(positions_ptr));
    }
}

 private:
     
     glm::vec3 sphere_center_ = {0.f, 0.f, 0.f};
     float sphere_radius_ = 1.0f;
    int num_joints_ = 100;      // for discretization of hair strand
    bool is_curly_;
    float curl_freq_ = 0.5f;        // period of a curl of hair
    int normal_samples_ = 10;   // number of subdivisions between joints for making curly hair
    float curl_radius_ = 0.025f;
    float radii_;
    glm::vec3 root_pos_;
    glm::vec3 root_normal_;
    float length_;

    std::unique_ptr<IntegratorBase<FTLSystem, ParticleState> > integrator_;
    ParticleState state_;
    FTLSystem system_;
    float step_size_;
    float current_time_;
    std::shared_ptr<VertexObject> sphere_mesh_;
    std::shared_ptr<ShaderProgram> shader_;
    std::vector<SceneNode *> sphere_node_ptrs_;
    std::shared_ptr<VertexObject> curve_polyline_;
    std::shared_ptr<ShaderProgram> polyline_shader_;
    bool prev_released_;
    bool prev_released_2_;

    std::pair<std::vector<glm::vec3>, std::vector<glm::vec3>> GetNormals(std::vector<glm::vec3> positions, glm::vec3 root_normal)
    {
        // Returns a pair where the first element is a vector of all of the normals and the second is all the binormals
        // for use in rendering curly hair

        std::vector<glm::vec3> joint_normals = {};
        std::vector<glm::vec3> joint_binormals = {};
        std::vector<glm::vec3> all_normals = {}; // joint_normals plus the subdivisions' normals
        std::vector<glm::vec3> all_binormals = {}; // joint_binormals plus the subdivisions' binormals

        // Calculate normals and binormals for the joints
        joint_normals.push_back(root_normal);
        joint_binormals.push_back(glm::normalize(glm::cross(root_normal, positions[1]-positions[0])));
        for (int i = 1; i < positions.size(); i++) {
            glm::vec3 prev_norm = joint_normals[i - 1];
            glm::vec3 first_perp_norm = glm::perp(prev_norm, positions[i] - positions[i - 1]);
            glm::vec3 this_norm = glm::vec3(0.f);
            glm::vec3 this_binorm = glm::vec3(0.f);
            if (i < positions.size() - 1) {
                glm::vec3 second_perp_norm = glm::perp(prev_norm, positions[i + 1] - positions[i]);
                this_norm += glm::normalize((first_perp_norm + second_perp_norm) / 2.f);
                this_binorm += glm::normalize(glm::cross(this_norm, positions[i + 1] - positions[i]));
            } else {
                this_norm += glm::normalize(first_perp_norm);
                this_binorm += glm::normalize(glm::cross(this_norm, positions[i] - positions[i - 1]));
            }
            joint_normals.push_back(this_norm);
            joint_binormals.push_back(this_binorm);
        }

        // Now interpolate for the subdivisions
        for (int i = 0; i < num_joints_; i++) {
            // first add the joint normals and binormals
            all_normals.push_back(joint_normals[i]);
            all_binormals.push_back(joint_binormals[i]);
            glm::vec3 hair_dir = glm::vec3(0.f);
            if (i == num_joints_ - 1) {
                hair_dir += positions[i] - positions[i - 1];
            } else {
                hair_dir += positions[i + 1] - positions[i];
            }

            // now interpolate for normal_samples_ # of times
            if (i < num_joints_ - 1) {
                for (int j = 1; j < normal_samples_; j++) {
                    float weight1 = normal_samples_ - j;
                    float weight2 = j;
                    glm::vec3 interpolated_norm = glm::normalize(weight1 * joint_normals[i] + weight2 * joint_normals[i + 1]);
                    all_normals.push_back(interpolated_norm);
                    all_binormals.push_back(glm::normalize(glm::cross(interpolated_norm, hair_dir)));
                }
            }
        }
        return std::pair<std::vector<glm::vec3>, std::vector<glm::vec3>>(all_normals, all_binormals);
    }

    void UpdateCurlPositions() {
        auto normals_pair = GetNormals(state_.positions, root_normal_);
        std::vector<glm::vec3> normals = normals_pair.first;
        std::vector<glm::vec3> binormals = normals_pair.second;
        auto curl_positions = make_unique<PositionArray>();

        int t = 0;
        for (int i = 0; i < state_.positions.size(); i ++) {

            glm::vec3 pos = state_.positions[i] + float(cos(curl_freq_ * t)) * normals[i] * curl_radius_ + float(sin(curl_freq_ * t)) * binormals[i] * curl_radius_;
            curl_positions->push_back(pos);
            t++;

            if (i < state_.positions.size() - 1) {
                for (int j = 1; j < normal_samples_; j++) {

                    glm::vec3 dir = state_.positions[i + 1] - state_.positions[i];
                    glm::vec3 pos = state_.positions[i] + dir * float(j) / float(normal_samples_) + float(cos(curl_freq_ * t)) * normals[i] * curl_radius_ + float(sin(curl_freq_ * t)) * binormals[i] * curl_radius_;
                    curl_positions->push_back(pos);
                    t++;

                }
            }
        }
        curve_polyline_->UpdatePositions(std::move(curl_positions));
    }

};  // namespace GLOO
}
