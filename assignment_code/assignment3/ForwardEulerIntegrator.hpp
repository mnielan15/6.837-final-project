#ifndef FORWARD_EULER_INTEGRATOR_H_
#define FORWARD_EULER_INTEGRATOR_H_

#include "IntegratorBase.hpp"

namespace GLOO {
template <class TSystem, class TState>
class ForwardEulerIntegrator : public IntegratorBase<TSystem, TState> {

  public:
  TState Integrate(const TSystem& system,
                   const TState& state,
                   float start_time,
                   float dt) const override {
    // TODO: Here we are returning the state at time t (which is NOT what we
    // want). Please replace the line below by the state at time t + dt using
    // forward Euler integration.

    //how do I make this for any system? Is the below inheritted compute time derivative just to test for a particular system? 
    //do I need to make a new class for Trapezoid?
    auto new_state = state + dt * system.ComputeTimeDerivative(state, start_time);
    return new_state;
  }
  
}; 
} // namespace GLOO

#endif
