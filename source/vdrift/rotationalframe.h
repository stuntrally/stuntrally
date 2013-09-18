#pragma once
#include "dbl.h"
#include "quaternion.h"
#include "mathvector.h"
#include "matrix3.h"

//#define NSV
//#define MODIFIEDVERLET
#define SUVAT
//#define EULER
//Samuel R. Buss second order angular momentum(and energy) preserving integrator
//#define SECOND_ORDER


class ROTATIONALFRAME
{
private:
	//primary
	QUATERNION<Dbl> orientation;
	MATHVECTOR<Dbl,3> angular_momentum;
	MATHVECTOR<Dbl,3> torque;
	
	//secondary
	MATHVECTOR<Dbl,3> old_torque; //this is only state information for the verlet-like integrators
	MATRIX3 <Dbl> orientmat; ///< 3x3 orientation matrix generated during inertia tensor rotation to worldspace and cached here
	MATRIX3 <Dbl> world_inverse_inertia_tensor; ///< inverse inertia tensor in worldspace, cached here
	MATRIX3 <Dbl> world_inertia_tensor;
	MATHVECTOR<Dbl,3> angular_velocity; ///< calculated from angular_momentum, cached here
	
	// constants
	MATRIX3 <Dbl> inverse_inertia_tensor; //used for calculations every frame
	MATRIX3 <Dbl> inertia_tensor; //used for the GetInertia function only
	
	//housekeeping
	bool have_old_torque;
	int integration_step;
	
	void RecalculateSecondary()
	{
		old_torque = torque;
		have_old_torque = true;
		orientation.GetMatrix3(orientmat);
		world_inverse_inertia_tensor = orientmat.Transpose().Multiply(inverse_inertia_tensor).Multiply(orientmat);
		world_inertia_tensor = orientmat.Transpose().Multiply(inertia_tensor).Multiply(orientmat);
		angular_velocity = GetAngularVelocityFromMomentum(angular_momentum);
	}
	
	///this call depends on having orientmat and world_inverse_inertia_tensor calculated
	MATHVECTOR<Dbl,3> GetAngularVelocityFromMomentum(const MATHVECTOR<Dbl,3> & moment) const
	{
		return world_inverse_inertia_tensor.Multiply(moment);
	}
	
	QUATERNION<Dbl> GetSpinFromMomentum(const MATHVECTOR<Dbl,3> & ang_moment) const
	{
		const MATHVECTOR<Dbl,3> ang_vel = GetAngularVelocityFromMomentum(ang_moment);
		QUATERNION<Dbl> qav = QUATERNION<Dbl> (ang_vel[0], ang_vel[1], ang_vel[2], 0);
		return (qav * orientation) * 0.5;
	}
	
public:
	ROTATIONALFRAME() : have_old_torque(false),integration_step(0) {}
	
	void SetInertia(const MATRIX3 <Dbl> & inertia)
	{
		//inertia.DebugPrint(std::cout);
		MATHVECTOR<Dbl,3> angvel_old = GetAngularVelocityFromMomentum(angular_momentum);
		inertia_tensor = inertia;
		inverse_inertia_tensor = inertia_tensor.Inverse();
		world_inverse_inertia_tensor = orientmat.Transpose().Multiply(inverse_inertia_tensor).Multiply(orientmat);
		world_inertia_tensor = orientmat.Transpose().Multiply(inertia_tensor).Multiply(orientmat);
		angular_momentum = world_inertia_tensor.Multiply(angvel_old);
		angular_velocity = GetAngularVelocityFromMomentum(angular_momentum);
	}
	
	const MATRIX3 <Dbl> & GetInertia() const
	{
		return world_inertia_tensor;
	}
	const MATRIX3 <Dbl> & GetInertiaConst() const
	{
		return inertia_tensor;
	}
	
	void SetOrientation(const QUATERNION<Dbl> & neworient)
	{
		orientation = neworient;
	}
	
	void SetAngularVelocity(const MATHVECTOR<Dbl,3> & newangvel)
	{
		angular_momentum = world_inertia_tensor.Multiply(newangvel);
		angular_velocity = newangvel;
	}
	
	const MATHVECTOR<Dbl,3> GetAngularVelocity() const
	{
		return angular_velocity;
	}
	
	///this is a modified velocity verlet integration method that relies on a two-step calculation.
	/// both steps must be executed each frame and forces can only be set between steps 1 and 2
	void Integrate1(const Dbl & dt)
	{
		assert(integration_step == 0);
		
		assert (have_old_torque); //you'll get an assert problem unless you call SetInitialTorque at the beginning of the simulation
		
	#ifdef MODIFIEDVERLET
		orientation = orientation + GetSpinFromMomentum(angular_momentum + old_torque*dt*0.5)*dt;
		orientation.Normalize();
		angular_momentum = angular_momentum + old_torque * dt * 0.5;
		RecalculateSecondary();
	#endif
		
		integration_step++;
	}
	
	///this is a modified velocity verlet integration method that relies on a two-step calculation.
	/// both steps must be executed each frame and forces must be set between steps 1 and 2
	void Integrate2(const Dbl & dt)
	{
		assert(integration_step == 1);
	#ifdef MODIFIEDVERLET
		angular_momentum = angular_momentum + torque * dt * 0.5;
	#endif
		
	#ifdef NSV
		//simple NSV integration
		angular_momentum = angular_momentum + torque * dt;
		orientation = orientation + GetSpinFromMomentum(angular_momentum)*dt;
		orientation.Normalize();
	#endif
		
	#ifdef EULER
		orientation = orientation + GetSpinFromMomentum(angular_momentum)*dt;
		orientation.Normalize();
		angular_momentum = angular_momentum + torque * dt;
	#endif
		
	#ifdef SUVAT
		//orientation = orientation + GetSpinFromMomentum(angular_momentum)*dt + GetSpinFromMomentum(torque)*dt*dt*0.5;
		orientation = orientation + GetSpinFromMomentum(angular_momentum + torque*dt*0.5)*dt;
		orientation.Normalize();
		angular_momentum = angular_momentum + torque * dt;
	#endif
		
	#ifdef SECOND_ORDER
		MATHVECTOR<Dbl,3> ang_acc = 
			world_inverse_inertia_tensor.Multiply(torque - angular_velocity.cross(angular_momentum));
		MATHVECTOR<Dbl,3> avg_rot = 
			angular_velocity + ang_acc * dt/2.0 + ang_acc.cross(angular_velocity) * dt * dt/12.0;
		QUATERNION<Dbl> dq = 
			QUATERNION<Dbl>(avg_rot[0], avg_rot[1], avg_rot[2], 0) * orientation * 0.5 * dt;
		orientation = orientation + dq;
		orientation.Normalize();
	#endif
		 // update angular velocity, inertia
		RecalculateSecondary();
		
		integration_step = 0;
		torque.Set(0.0);
	}
	
    ///this must only be called between integrate1 and integrate2 steps
	const MATHVECTOR<Dbl,3> GetLockUpTorque(const Dbl dt) const
	{
	#ifdef MODIFIEDVERLET
	    return -angular_momentum * 2 / dt;
	#else
        return -angular_momentum / dt;
	#endif
	}
	
	///this must only be called between integrate1 and integrate2 steps
	void ApplyTorque(const MATHVECTOR<Dbl,3> & t)
	{
		assert(integration_step == 1);
		torque = torque + t;
	}
	
	void SetTorque(const MATHVECTOR<Dbl,3> & t)
	{
		assert(integration_step == 1);
		torque = t;
	}
	
	const MATHVECTOR<Dbl,3> & GetTorque()
	{
		return old_torque;
	}
	
	///this must be called once at sim start to set the initial torque present
	void SetInitialTorque(const MATHVECTOR<Dbl,3> & t)
	{
		assert(integration_step == 0);
		
		old_torque = t;
		have_old_torque = true;
	}

	const QUATERNION<Dbl> & GetOrientation() const
	{
		return orientation;
	}
};
