#ifndef _RIGIDBODY_H
#define _RIGIDBODY_H

#include "linearframe.h"
#include "rotationalframe.h"
#include "mathvector.h"
#include "quaternion.h"
#include "joeserialize.h"
#include "macros.h"

template <typename T>
class RIGIDBODY
{
friend class joeserialize::Serializer;
private:
	LINEARFRAME <T> linear;
	ROTATIONALFRAME <T> rotation;
	
public:
	//access to linear frame
	void SetInitialForce(const MATHVECTOR <T, 3> & force) {linear.SetInitialForce(force);}
	void SetForce(const MATHVECTOR <T, 3> & force) {linear.SetForce(force);}
	void SetMass(const T & mass) {linear.SetMass(mass);}
	const T GetMass() const {return linear.GetMass();}
	void SetPosition(const MATHVECTOR <T, 3> & position) {linear.SetPosition(position);}
	const MATHVECTOR <T, 3> & GetPosition() const {return linear.GetPosition();}
	void SetVelocity(const MATHVECTOR <T, 3> & velocity) {linear.SetVelocity(velocity);}
	const MATHVECTOR <T, 3> GetVelocity() const {return linear.GetVelocity();}
	const MATHVECTOR <T, 3> GetVelocity(const MATHVECTOR <T, 3> & offset) {return linear.GetVelocity() + rotation.GetAngularVelocity().cross(offset);}
	const MATHVECTOR <T, 3> & GetForce() const {return linear.GetForce();}
	
	//access to rotational frame
	void SetInitialTorque(const MATHVECTOR <T, 3> & torque) {rotation.SetInitialTorque(torque);}
	void SetTorque(const MATHVECTOR <T, 3> & torque) {rotation.SetTorque(torque);}
	void SetInertia(const MATRIX3 <T> & inertia) {rotation.SetInertia(inertia);}
	const MATRIX3 <T> & GetInertia() {return rotation.GetInertia();}
	void SetOrientation(const QUATERNION <T> & orientation) {rotation.SetOrientation(orientation);}
	const QUATERNION <T> & GetOrientation() const {return rotation.GetOrientation();}
	void SetAngularVelocity(const MATHVECTOR <T, 3> & newangvel) {rotation.SetAngularVelocity(newangvel);}
	const MATHVECTOR <T, 3> GetAngularVelocity() const {return rotation.GetAngularVelocity();}
	
	//acessing both linear and rotational frames
	void Integrate1(const T & dt) {linear.Integrate1(dt);rotation.Integrate1(dt);}
	void Integrate2(const T & dt) {linear.Integrate2(dt);rotation.Integrate2(dt);}
	const MATHVECTOR <T, 3> TransformLocalToWorld(const MATHVECTOR <T, 3> & localpoint) const
	{
		MATHVECTOR <T, 3> output(localpoint);
		GetOrientation().RotateVector(output);
		output = output + GetPosition();
		return output;
	}
	const MATHVECTOR <T, 3> TransformWorldToLocal(const MATHVECTOR <T, 3> & worldpoint) const
	{
		MATHVECTOR <T, 3> output(worldpoint);
		output = output - GetPosition();
		(-GetOrientation()).RotateVector(output);
		return output;
	}
	
	// apply force in world space
	void ApplyForce(const MATHVECTOR <T, 3> & force)
	{
		linear.ApplyForce(force);
	}
	
	// apply force at offset from center of mass in world space
	void ApplyForce(const MATHVECTOR <T, 3> & force, const MATHVECTOR <T, 3> & offset)
	{
		linear.ApplyForce(force);
		rotation.ApplyTorque(offset.cross(force));
	}
	
	// apply torque in world space
	void ApplyTorque(const MATHVECTOR <T, 3> & torque)
	{
		rotation.ApplyTorque(torque);
	}
	
	bool Serialize(joeserialize::Serializer & s)
	{
		_SERIALIZE_(s,linear);
		_SERIALIZE_(s,rotation);
		return true;
	}
};

#endif
