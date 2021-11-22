#include "ArcBallCameraNode.hpp"

#include <iostream>

#include <glm/gtx/quaternion.hpp>

#include "gloo/components/CameraComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/InputManager.hpp"
#include "gloo/shaders/SimpleShader.hpp"
#include "gloo/VertexObject.hpp"

namespace GLOO {
ArcBallCameraNode::ArcBallCameraNode(float fov, float aspect, float distance)
    : SceneNode(), fov_(fov), distance_(distance) {
  auto camera = make_unique<CameraComponent>(fov, aspect, 0.1f, 100.f);
  AddComponent(std::move(camera));

  start_position_ = GetTransform().GetPosition();
  start_rotation_ = GetTransform().GetRotation();
  start_distance_ = distance;
}

void ArcBallCameraNode::Update(double delta_time) {
  UpdateViewport();

  auto& input_manager = InputManager::GetInstance();

  static bool prev_released = true;

  if (input_manager.IsMiddleMousePressed()) {
    if (prev_released) {
      mouse_start_click_ = InputManager::GetInstance().GetCursorPosition();
    }
    PlaneTranslation(InputManager::GetInstance().GetCursorPosition());
    prev_released = false;
  } else if (input_manager.IsLeftMousePressed()) {
    if (prev_released) {
      mouse_start_click_ = InputManager::GetInstance().GetCursorPosition();
    }
    ArcBallRotation(InputManager::GetInstance().GetCursorPosition());
    prev_released = false;
  } else if (input_manager.IsRightMousePressed()) {
    if (prev_released) {
      mouse_start_click_ = InputManager::GetInstance().GetCursorPosition();
    }
    DistanceZoom(InputManager::GetInstance().GetCursorPosition());
    prev_released = false;
  } else {
    auto scroll = input_manager.FetchAndResetMouseScroll();
    if (scroll != 0.0) {
      DistanceZoom(-float(scroll) * 0.1f);
    }
    prev_released = true;
    start_position_ = GetTransform().GetPosition();
    start_rotation_ = GetTransform().GetRotation();
    start_distance_ = distance_;
  }

  auto V = make_unique<glm::mat4>(glm::lookAt(
      glm::vec3(0, 0, distance_), glm::vec3(0), glm::vec3(0, 1.f, 0)));
  *V *= glm::toMat4(GetTransform().GetRotation()) *
        glm::translate(glm::mat4(1.f), GetTransform().GetPosition());
  GetComponentPtr<CameraComponent>()->SetViewMatrix(std::move(V));
}

void ArcBallCameraNode::UpdateViewport() {
  glm::ivec2 window_size = InputManager::GetInstance().GetWindowSize();
  float aspect_ratio =
      static_cast<float>(window_size.x) / static_cast<float>(window_size.y);
  GetComponentPtr<CameraComponent>()->SetAspectRatio(aspect_ratio);
}

void ArcBallCameraNode::ArcBallRotation(glm::dvec2 pos) {
  float sx, sy, sz, ex, ey, ez;
  float scale;
  float sl, el;
  float dotprod;
  glm::ivec2 window_size = InputManager::GetInstance().GetWindowSize();

  // find vectors from center of window
  sx = float(mouse_start_click_.x - (window_size.x / 2.f));
  sy = float(mouse_start_click_.y - (window_size.y / 2.f));
  ex = float(pos.x - (window_size.x / 2.f));
  ey = float(pos.y - (window_size.y / 2.f));

  // invert y coordinates (raster versus device coordinates)
  sy = -sy;
  ey = -ey;

  // scale by inverse of size of window and magical sqrt2 factor
  scale = fmin(float(window_size.x), float(window_size.y));

  scale = 1.f / scale;

  sx *= scale;
  sy *= scale;
  ex *= scale;
  ey *= scale;

  // project points to unit circle
  sl = hypot(sx, sy);
  el = hypot(ex, ey);

  if (sl > 1.f) {
    sx /= sl;
    sy /= sl;
    sl = 1.0;
  }
  if (el > 1.f) {
    ex /= el;
    ey /= el;
    el = 1.f;
  }

  // project up to unit sphere - find Z coordinate
  sz = sqrt(1.0f - sl * sl);
  ez = sqrt(1.0f - el * el);

  // rotate (sx,sy,sz) into (ex,ey,ez)

  // compute angle from dot-product of unit vectors (and double it).
  // compute axis from cross product.
  dotprod = sx * ex + sy * ey + sz * ez;

  if (dotprod != 1 && !std::isnan(dotprod)) {
    glm::vec3 axis(sy * ez - ey * sz, sz * ex - ez * sx, sx * ey - ex * sy);
    axis = glm::normalize(axis);
    if (std::isnan(axis.x) || std::isnan(axis.y) || std::isnan(axis.z)) {
      return;
    }

    float angle = 2.0f * acos(dotprod);

    GetTransform().SetRotation(glm::angleAxis(angle, axis) * start_rotation_);
  }
}

void ArcBallCameraNode::PlaneTranslation(glm::dvec2 pos) {
  // compute "distance" of image plane (wrt projection matrix)
  glm::ivec2 window_size = InputManager::GetInstance().GetWindowSize();
  float d = static_cast<float>(window_size.y) / 2.0f /
            tan(fov_ * kPi / 180.0f / 2.0f);

  // compute up plane intersect of clickpoint (wrt fovy)
  float su = float(-mouse_start_click_.y + window_size.y / 2.0f);
  float cu = float(-pos.y + window_size.y / 2.0f);

  // compute right plane intersect of clickpoint (ASSUMED FOVY is 1)
  float sr = float(mouse_start_click_.x - window_size.x / 2.0f);
  float cr = float(pos.x - window_size.x / 2.0f);

  // this maps move
  glm::vec2 move(cr - sr, cu - su);
  move *= -distance_ / d;

  auto rot = glm::toMat4(GetTransform().GetRotation());
  GetTransform().SetPosition(start_position_ -
                             (move.x * glm::vec3(glm::transpose(rot)[0]) +
                              move.y * glm::vec3(glm::transpose(rot)[1])));
}

void ArcBallCameraNode::DistanceZoom(glm::dvec2 pos) {
  glm::ivec2 window_size = InputManager::GetInstance().GetWindowSize();
  DistanceZoom(float((pos.y - mouse_start_click_.y) / window_size.y));
}

void ArcBallCameraNode::DistanceZoom(float delta) {
  distance_ = start_distance_ * exp(delta);
}

}  // namespace GLOO
