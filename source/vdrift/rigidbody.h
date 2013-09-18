#pragma once
#include "dbl.h"
#include "linearframe.h"
#include "rotationalframe.h"
#include "mathvector.h"
#include "quaternion.h"


class RIGIDBODY
{
private:
	LINEARFRAME linear;
	ROTATIONALFRAME rotation;
	
public:
	// access to linear frame
	void SetInitialForce(const MATHVECTOR<Dbl,3> & force)  {  linear.SetInitialForce(force);  }
	void SetForce(const MATHVECTOR<Dbl,3> & force)     {  linear.SetForce(force);  }
	const MATHVECTOR<Dbl,3> & GetForce() const  {  return linear.GetForce();  }

	void SetMass(const Dbl & mass)    {  linear.SetMass(mass);  }
	const Dbl GetMass() const  {  return linear.GetMass();  }

	void SetPosition(const MATHVECTOR<Dbl,3> & position)  {  linear.SetPosition(position);  }
	const MATHVECTOR<Dbl,3> & GetPosition() const  {  return linear.GetPosition();  }

	void SetVelocity(const MATHVECTOR<Dbl,3> & velocity)  {  linear.SetVelocity(velocity);  }
	const MATHVECTOR<Dbl,3> GetVelocity() const           {  return linear.GetVelocity();  }
	const MATHVECTOR<Dbl,3> GetVelocity(const MATHVECTOR<Dbl,3> & offset)  {  return linear.GetVelocity() + rotation.GetAngularVelocity().cross(offset);  }
	
	// access to rotational frame
	void SetInitialTorque(const MATHVECTOR<Dbl,3> & torque)  {  rotation.SetInitialTorque(torque);  }
	void SetTorque(const MATHVECTOR<Dbl,3> & torque)         {  rotation.SetTorque(torque);  }

	void SetInertia(const MATRIX3 <Dbl> & inertia)  {  rotation.SetInertia(inertia);  }
	const MATRIX3 <Dbl> & GetInertia()       {  return rotation.GetInertia();  }
	const MATRIX3 <Dbl> & GetInertiaConst()  {  return rotation.GetInertiaConst();  }

	void SetOrientation(const QUATERNION<Dbl> & orientation)  {  rotation.SetOrientation(orientation);  }
	const QUATERNION<Dbl> & GetOrientation() const     {  return rotation.GetOrientation();  }

	void SetAngularVelocity(const MATHVECTOR<Dbl,3> & newangvel)  {  rotation.SetAngularVelocity(newangvel);  }
	const MATHVECTOR<Dbl,3> GetAngularVelocity() const     {  return rotation.GetAngularVelocity();  }

	
	// acessing both linear and rotational frames
	void Integrate1(const Dbl & dt)  {  linear.Integrate1(dt);rotation.Integrate1(dt);  }
	void Integrate2(const Dbl & dt)  {  linear.Integrate2(dt);rotation.Integrate2(dt);  }

	const MATHVECTOR<Dbl,3> TransformLocalToWorld(const MATHVECTOR<Dbl,3> & localpoint) const
	{
		MATHVECTOR<Dbl,3> output(localpoint);
		GetOrientation().RotateVector(output);
		output = output + GetPosition();
		return output;
	}
	const MATHVECTOR<Dbl,3> TransformWorldToLocal(const MATHVECTOR<Dbl,3> & worldpoint) const
	{
		MATHVECTOR<Dbl,3> output(worldpoint);
		output = output - GetPosition();
		(-GetOrientation()).RotateVector(output);
		return output;
	}
	
	// apply force in world space
	void ApplyForce(const MATHVECTOR<Dbl,3> & force)
	{
		linear.ApplyForce(force);
	}
	
	// apply force at offset from center of mass in world space
	void ApplyForce(const MATHVECTOR<Dbl,3> & force, const MATHVECTOR<Dbl,3> & offset)
	{
		linear.ApplyForce(force);
		rotation.ApplyTorque(offset.cross(force));
	}
	
	// apply torque in world space
	void ApplyTorque(const MATHVECTOR<Dbl,3> & torque)
	{
		rotation.ApplyTorque(torque);
	}
};
