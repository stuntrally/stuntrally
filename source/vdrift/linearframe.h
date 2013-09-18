#pragma once
#include "dbl.h"
#include "mathvector.h"


//#define EULER
//#define NSV
//#define MODIFIEDVERLET
#define SUVAT


class LINEARFRAME
{
private:
	//primary
	MATHVECTOR<Dbl,3> position;
	MATHVECTOR<Dbl,3> momentum;
	MATHVECTOR<Dbl,3> force;
	
	//secondary
	MATHVECTOR<Dbl,3> old_force; //this is only necessary state information for the verlet-like integrators
	
	// constants
	Dbl inverse_mass;
	
	//housekeeping
	bool have_old_force;
	int integration_step;
	
	void RecalculateSecondary()
	{
		old_force = force;
		have_old_force = true;
	}
	
	MATHVECTOR<Dbl,3> GetVelocityFromMomentum(const MATHVECTOR<Dbl,3> & moment) const
	{
		return moment*inverse_mass;
	}
	
public:
	LINEARFRAME() : inverse_mass(1.0), have_old_force(false), integration_step(0)
	{	}
	
	void SetMass(const Dbl & mass)
	{
		inverse_mass = 1.0 / mass;
	}
	
	const Dbl GetMass() const
	{
		return 1.0 / inverse_mass;
	}
	
	void SetPosition(const MATHVECTOR<Dbl,3> & newpos)
	{
		position = newpos;
	}
	
	void SetVelocity(const MATHVECTOR<Dbl,3> & velocity)
	{
		momentum = velocity / inverse_mass;
	}
	
	///this is a modified velocity verlet integration method that relies on a two-step calculation.
	/// both steps must be executed each frame and forces can only be set between steps 1 and 2
	void Integrate1(const Dbl & dt)
	{
		assert(integration_step == 0);
		
		assert (have_old_force); //you'll get an assert problem unless you call SetInitialForce at the beginning of the simulation
		
	#ifdef MODIFIEDVERLET
		position = position + momentum*inverse_mass*dt + old_force*inverse_mass*dt*dt*0.5;
		momentum = momentum + old_force * dt * 0.5;
	#endif
		
		integration_step++;
	}
	
	///this is a modified velocity verlet integration method that relies on a two-step calculation.
	/// both steps must be executed each frame and forces must be set between steps 1 and 2
	void Integrate2(const Dbl & dt)
	{
		assert(integration_step == 1);
		
	#ifdef MODIFIEDVERLET
		momentum = momentum + force * dt * 0.5;
	#endif
		
	#ifdef NSV
		momentum = momentum + force * dt;
		position = position + momentum*inverse_mass*dt;
	#endif
		
	#ifdef EULER
		position = position + momentum*inverse_mass*dt;
		momentum = momentum + force * dt;
	#endif
		
	#ifdef SUVAT
		position = position + momentum*inverse_mass*dt + force*inverse_mass*dt*dt*0.5;
		momentum = momentum + force * dt;
	#endif
		
		RecalculateSecondary();
		
		integration_step = 0;
		force.Set(0.0);
	}
	
	///this must only be called between integrate1 and integrate2 steps
	void ApplyForce(const MATHVECTOR<Dbl,3> & f)
	{
		assert(integration_step == 1);
		force = force + f;
	}
	
	void SetForce(const MATHVECTOR<Dbl,3> & f)
	{
		assert(integration_step == 1);
		force = f;
	}
	
	///this must be called once at sim start to set the initial force present
	void SetInitialForce(const MATHVECTOR<Dbl,3> & newforce)
	{
		assert(integration_step == 0);
		
		old_force = newforce;
		have_old_force = true;
	}

	const MATHVECTOR<Dbl,3> & GetPosition() const
	{
		return position;
	}
	
	const MATHVECTOR<Dbl,3> GetVelocity() const
	{
		return GetVelocityFromMomentum(momentum);
	}
	
	const MATHVECTOR<Dbl,3> & GetForce() const
	{
		return old_force;
	}
};
