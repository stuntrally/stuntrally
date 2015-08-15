#pragma once
#include "aabb.h"
#include "mathvector.h"

//#include <list>
//#include <iostream>
//#include <map>

template <typename DATATYPE>
class AABB_SPACE_PARTITIONING_NODE
{
	private:
		typedef std::vector <std::pair <DATATYPE, AABB <float> > > objectlist_type;
		objectlist_type objects;
		typedef std::vector <AABB_SPACE_PARTITIONING_NODE> childrenlist_type;
		childrenlist_type children;
		AABB <float> bbox;
		
		const AABB <float> & GetBBOX() const {return bbox;}
		
		///recursively send all objects and all childrens' objects to the target node, clearing out everything else
		void CollapseTo(AABB_SPACE_PARTITIONING_NODE & collapse_target)
		{
			if (this != &collapse_target)
			{
				for (typename objectlist_type::iterator i = objects.begin(); i != objects.end(); ++i)
				{
					collapse_target.Add(i->first, i->second);
				}
				objects.clear();
			}
			
			for (typename childrenlist_type::iterator i = children.begin(); i != children.end(); ++i)
			{
				i->CollapseTo(collapse_target);
			}
			children.clear();
		}
		
		///requires the DATATYPE class implements operator< and operator==
		void RemoveDuplicateObjects()
		{
			//TODO:  re-enable this code and make it compile
			//objects.sort();
			//objects.unique();
		}
		
		///intelligently add new child nodes and parse objects to them, recursively
		void DistributeObjectsToChildren(const int level)
		{
			const unsigned int ideal_objects_per_node(1);
			const unsigned int ideal_children_per_node(2);
			const bool verbose(false);
	
			//enforce the rules:  i don't want to start with any children,
			// so tell all children to send me their objects, then delete them
			for (typename childrenlist_type::iterator i = children.begin(); i != children.end(); ++i)
			{
				i->CollapseTo(*this);
			}
			children.clear();
	
			//only continue if we have more objects than we need
			if (objects.size() <= ideal_objects_per_node)
				return;
	
			children.resize(ideal_children_per_node);
	
			//determine the average center position of all objects
			MATHVECTOR<float,3> avgcenter;
			int numobj = objects.size();
			float incamount = 1.0 / numobj;
			for (typename objectlist_type::iterator i = objects.begin(); i != objects.end(); ++i)
			{
				avgcenter = avgcenter + i->second.GetCenter() * incamount;
			}
	
			//find axis of maximum change, so we know where to split
			MATHVECTOR<float,3> axismask;
			axismask.Set(1,0,0);
			MATHVECTOR<float,3> bboxsize = bbox.GetSize();
			if (bboxsize[0] > bboxsize[1] && bboxsize[0] > bboxsize[2])
			{
				axismask.Set(1,0,0);
			}
			else if (bboxsize[1] > bboxsize[0] && bboxsize[1] > bboxsize[2])
			{
				axismask.Set(0,1,0);
			}
			else if (bboxsize[2] > bboxsize[1] && bboxsize[2] > bboxsize[0])
			{
				axismask.Set(0,0,1);
			}
	
			//cout << level << endl;
	
			//distribute objects to each child
			float avgcentercoord = avgcenter.dot(axismask);
			int distributor(0);
			for (typename objectlist_type::iterator i = objects.begin(); i != objects.end(); ++i)
			{
				float objcentercoord = i->second.GetCenter().dot(axismask);
		
				if (objcentercoord > avgcentercoord)
					children.front().Add(i->first, i->second);
				else if (objcentercoord < avgcentercoord)
					children.back().Add(i->first, i->second);
				else
				{
			//cout << "unusual case #" << distributor << endl;
			
			//distribute children that sit right on our average center in an even way
					if (distributor % 2 == 0)
						children.front().Add(i->first, i->second);
					else
						children.back().Add(i->first, i->second);
					distributor++;
				}
			}
	
			//we've given away all of our objects; clear them out
			objects.clear();
	
			//count objects that belong to our children
			int child1obj = children.front().objects.size();
			int child2obj = children.back().objects.size();
	
			if (verbose) std::cout << "Objects: " << objects.size() << ", Child nodes: " << children.size() << " L obj: " << child1obj << " R obj: " << child2obj << std::endl;
	
			//if one child doesn't have any objects, then delete both children and take back their objects
			if (child1obj == 0 || child2obj == 0)
			{
				for (typename childrenlist_type::iterator i = children.begin(); i != children.end(); ++i)
				{
					for (typename objectlist_type::iterator n = i->objects.begin(); n != i->objects.end(); ++n)
					{
						Add(n->first, n->second);
					}
				}
		
				children.clear();
			}
	
			for (typename childrenlist_type::iterator i = children.begin(); i != children.end(); ++i)
				i->DistributeObjectsToChildren(level + 1);
		}
	
	public:
		void DebugPrint(int level, int & objectcount, bool verbose, std::ostream & o)
		{
			if (verbose)
			{
				for (int i = 0; i < level; ++i)
					o << "-";
		
				o << "objects: " << objects.size() << ", child nodes: " << children.size() << ", aabb: ";
				bbox.DebugPrint(o);
			}
	
			objectcount += objects.size();
	
			for (typename childrenlist_type::iterator i = children.begin(); i != children.end(); ++i)
				i->DebugPrint(level+1, objectcount, verbose, o);
	
			if (level == 0)
			{
				if (verbose)
					o << "================" << std::endl;
				o << "TOTAL OBJECTS: " << objectcount << std::endl;
			}
		}
		
		void Optimize()
		{
			CollapseTo(*this);

			RemoveDuplicateObjects();

			DistributeObjectsToChildren(0);
		}
		
		void Add(DATATYPE & object, const AABB <float> & newaabb)
		{
			objects.push_back(std::pair <DATATYPE, AABB <float> > (object, newaabb));
			if (objects.size() == 1) //don't combine if this is the first object, otherwise the AABB would be forced to include (0,0,0)
				bbox = newaabb;
			else
				bbox.CombineWith(newaabb);
		}
		
		///a slow delete that only requires the object
		void Delete(DATATYPE & object)
		{
			typename std::list <typename objectlist_type::iterator> todel;
	
			//if we've got objects, test them
			for (typename objectlist_type::iterator i = objects.begin(); i != objects.end(); ++i)
			{
				if (i->first == object)
				{
					todel.push_back(i);
				}
			}
	
			//do any deletions
			for (typename std::list <typename objectlist_type::iterator>::iterator i = todel.begin(); i != todel.end(); ++i)
			{
				objects.erase(*i);
			}
	
			//if we have children, pass it on
			for (typename childrenlist_type::iterator i = children.begin(); i != children.end(); ++i)
			{
				i->Delete(object);
			}
		}
		
		///a faster delete that uses the supplied AABB to find the object
		void Delete(DATATYPE & object, const AABB <float> & objaabb)
		{
			typename std::list <typename objectlist_type::iterator> todel;
	
			//if we've got objects, test them
			for (typename objectlist_type::iterator i = objects.begin(); i != objects.end(); ++i)
			{
				if (i->first == object)
				{
					todel.push_back(i);
				}
			}
	
			//do any deletions
			for (typename std::list <typename objectlist_type::iterator>::iterator i = todel.begin(); i != todel.end(); ++i)
			{
				objects.erase(*i);
			}
	
			//if we have children, pass it on
			for (typename childrenlist_type::iterator i = children.begin(); i != children.end(); ++i)
			{
				if (i->GetBBOX().Intersect(objaabb))
					i->Delete(object, objaabb);
			}
		}
		
		///run a query for objects that collide with the given shape
		template <typename T, typename U>
		void Query(const T & shape, U &outputlist) const
		{
			//if we've got objects, test them
			for (typename objectlist_type::const_iterator i = objects.begin(); i != objects.end(); ++i)
			{
				if (i->second.Intersect(shape))
				{
					outputlist.push_back(i->first);
				}
			}
	
			//if we have children, test them
			for (typename childrenlist_type::const_iterator i = children.begin(); i != children.end(); ++i)
			{
				if (i->GetBBOX().Intersect(shape))
				{
					//our child intersects with the segment, dispatch a query
					i->Query(shape, outputlist);
				}
			}
		}
		
		bool Empty() const {return (objects.empty() && children.empty());}
		void Clear() {objects.clear(); children.clear();}
		
		///traverse the entire tree putting pointers to all DATATYPE objects into the given outputlist
		void GetContainedObjects(std::list <DATATYPE *> & outputlist)
		{
			//if we've got objects, add them
			for (typename objectlist_type::iterator i = objects.begin(); i != objects.end(); ++i)
			{
				outputlist.push_back(&i->first);
			}
	
			//if we have children, add them
			for (typename childrenlist_type::iterator i = children.begin(); i != children.end(); ++i)
			{
				i->GetContainedObjects(outputlist);
			}
		}
};
