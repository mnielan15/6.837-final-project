//#ifndef TRAPEZOID_INTEGRATOR_H_
//#define TRAPEZOID_INTEGRATOR_H_

#include "IntegratorBase.hpp"

namespace GLOO {
template <class TSystem, class TState>
class RK4Integrator : public IntegratorBase<TSystem, TState> {

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
    ParticleState k1 = system.ComputeTimeDerivative(state, start_time);
    ParticleState k2 = system.ComputeTimeDerivative(state+dt*0.5f*k1, start_time + dt*0.5f);
    ParticleState k3 = system.ComputeTimeDerivative(state+dt*0.5f*k2, start_time + dt*0.5f);
    ParticleState k4 = system.ComputeTimeDerivative(state+dt*k3, start_time + dt);

    return state + dt/6.0f * (k1 + 2*k2 + 2*k3 + k4);
  }
  
}; 
} // namespace GLOO

//#endif
