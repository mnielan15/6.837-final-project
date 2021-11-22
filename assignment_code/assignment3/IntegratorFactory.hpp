#ifndef INTEGRATOR_FACTORY_H_
#define INTEGRATOR_FACTORY_H_

#include "IntegratorBase.hpp"

#include <stdexcept>

#include "gloo/utils.hpp"

#include "IntegratorType.hpp"
#include "TrapezoidIntegrator.hpp"
#include "PendulumSystem.hpp"

namespace GLOO {
class IntegratorFactory {
 public:
  template <class TSystem, class TState>
  static std::unique_ptr<IntegratorBase<TSystem, TState>> CreateIntegrator(
      IntegratorType type) {
        
        if (type == IntegratorType::Euler){
        //return make_unique<ForwardEulerIntegrator<ParticleSystemSimple, ParticleState>>();
        return make_unique<ForwardEulerIntegrator<TSystem, ParticleState>>();

        }
        else if (type == IntegratorType::Trapezoidal){
           //return make_unique<TrapezoidIntegrator<ParticleSystemSimple, ParticleState>>();
          return make_unique<TrapezoidIntegrator<TSystem, ParticleState>>();
        }
        else{
          return make_unique<RK4Integrator<TSystem, ParticleState>>();
        }
    //return nullptr;
  }
};
}  // namespace GLOO

#endif
