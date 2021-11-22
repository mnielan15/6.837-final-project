#ifndef INTEGRATOR_BASE_H_
#define INTEGRATOR_BASE_H_

#include "ParticleSystemBase.hpp"

namespace GLOO {
template <class TSystem, class TState>
class IntegratorBase {
 public:
  virtual ~IntegratorBase() {
  }

  virtual TState Integrate(const TSystem& system,
                           const TState& state,
                           float start_time,
                           float dt) const = 0;
};
}  // namespace GLOO

#endif
