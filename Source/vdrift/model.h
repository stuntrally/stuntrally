#ifndef _MODEL_H
#define _MODEL_H

//#include <string>
//#include <iostream>
//#include <cassert>
//#include <sstream>

#include "vertexarray.h"
#include "mathvector.h"
#include "aabb.h"
#include "joeserialize.h"
#include "macros.h"

template <typename T> class MATRIX4;

///loading data into the mesh vertexarray is implemented by derived classes
class MODEL
{
friend class joeserialize::Serializer;
private:
	bool generatedlistid;
	int listid;
	bool generatedmetrics;
	std::string diffuse_texture;
	std::string normal_texture;
	
	//metrics
	float radius;
	float radiusxz;
	MATHVECTOR <float, 3> bboxmin;
	MATHVECTOR <float, 3> bboxmax;
	
	void RequireMetrics() const
	{
		//Mesh metrics need to be generated before they can be queried
		assert(generatedmetrics);
	}
	
	void RequireListID() const
	{
		//Mesh id needs to be generated
		assert(generatedlistid);
	}
	
	void ClearListID();
	void ClearMetrics() {generatedmetrics = false;}
	
//protected:
public:
	///to be filled by the derived classes
	VERTEXARRAY mesh;

public:
	MODEL() : generatedlistid(false),generatedmetrics(false),radius(0),radiusxz(0) {}
	virtual ~MODEL() {Clear();}
	
	virtual bool Load(const std::string & strFileName, std::ostream & error_output) {return false;}
	virtual bool CanSave() const {return false;}  ///< returns true if the model format is capable of saving to a file
	virtual bool Save(const std::string & strFileName, std::ostream & error_output) const {return false;} ///< optional capability
	
	bool Serialize(joeserialize::Serializer & s)
	{
		_SERIALIZE_(s,mesh);
		
		return true;
	}
	
	bool WriteToFile(const std::string & filepath);
	bool ReadFromFile(const std::string & filepath, std::ostream & error_output, bool generatelistid=true);
	
	void SetDiffuseTexture(const std::string & newdiff) {diffuse_texture = newdiff;}
	void SetNormalTexture(const std::string & newnorm) {normal_texture = newnorm;}
	const std::string GetDiffuseTexture() const {return diffuse_texture;}
	const std::string GetNormalTexture() const {return normal_texture;}
	
	void GenerateListID(std::ostream & error_output);
	void GenerateMeshMetrics();
	void ClearMeshData() {mesh.Clear();}
	
	int GetListID() const {RequireListID(); return listid;}
	
	//"metrics"
	float GetRadius() const {RequireMetrics();return radius + 0.5f;}
	float GetRadiusXZ() const {RequireMetrics();return radiusxz;}
	MATHVECTOR <float, 3> GetCenter() {return (bboxmax + bboxmin) * 0.5;}
	
	bool HaveMeshData() const {return (mesh.GetNumFaces() > 0);}
	bool HaveMeshMetrics() const {return generatedmetrics;}
	bool HaveListID() const {return generatedlistid;}
	
	void Clear() {ClearMeshData();ClearListID();ClearMetrics();}
	
	const VERTEXARRAY & GetVertexArray() const {return mesh;}
	void SetVertexArray(const VERTEXARRAY & newmesh) {Clear(); mesh = newmesh;}
	
	bool Loaded() {return (mesh.GetNumFaces() > 0);}

	void Transform(const MATRIX4 <float> & m) {mesh.Transform(m); GenerateMeshMetrics();}
	
	AABB <float> GetAABB() const
	{
		assert(generatedmetrics);
		AABB <float> output;
		output.SetFromCorners(bboxmin, bboxmax);
		return output;
	}
};

#endif


