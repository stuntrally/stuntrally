#include "pch.h"
#include "roadstrip.h"

bool ROADSTRIP::ReadFrom(std::istream & openfile, std::ostream & error_output)
{
	patches.clear();

	assert(openfile);

	//number of patches in this road strip
	int num;
	openfile >> num;

	//add all road patches to this strip
	int badcount = 0;
	for (int i = 0; i < num; ++i)
	{
		BEZIER * prevbezier = NULL;
		if (!patches.empty())
			prevbezier = &patches.back().GetPatch();

		patches.push_back(ROADPATCH());
		patches.back().GetPatch().ReadFrom(openfile);

		if (prevbezier)
			prevbezier->Attach(patches.back().GetPatch());

		if (patches.back().GetPatch().CheckForProblems())
		{
			badcount++;
			patches.pop_back();
		}
	}

	if (badcount > 0)
		error_output << "Rejected " << badcount << " bezier patch(es) from roadstrip due to errors" << std::endl;

	//close the roadstrip
	if (patches.size() > 2)
	{
		//only close it if it ends near where it starts
		if (((patches.back().GetPatch().GetFL() - patches.front().GetPatch().GetBL()).Magnitude() < 0.1) &&
		    ((patches.back().GetPatch().GetFR() - patches.front().GetPatch().GetBR()).Magnitude() < 0.1))
		{
			patches.back().GetPatch().Attach(patches.front().GetPatch());
			closed = true;
		}
	}

	GenerateSpacePartitioning();

	return true;
}

void ROADSTRIP::GenerateSpacePartitioning()
{
	aabb_part.Clear();

	for (std::list <ROADPATCH>::iterator i = patches.begin(); i != patches.end(); ++i)
	{
		ROADPATCH * rp = &(*i);
		aabb_part.Add(rp, i->GetPatch().GetAABB());
	}

	aabb_part.Optimize();
}

bool ROADSTRIP::Collide(const MATHVECTOR<float,3> & origin, const MATHVECTOR<float,3> & direction, float seglen, MATHVECTOR<float,3> & outtri, const BEZIER * & colpatch, MATHVECTOR<float,3> & normal) const
{
	std::list <ROADPATCH *> candidates;
	aabb_part.Query(AABB<float>::RAY(origin, direction, seglen), candidates);
	bool col = false;
	for (std::list <ROADPATCH *>::iterator i = candidates.begin(); i != candidates.end(); ++i)
	{
		MATHVECTOR<float,3> coltri, colnorm;
		if ((*i)->Collide(origin, direction, seglen, coltri, colnorm))
		{
			if (!col || (coltri-origin).Magnitude() < (outtri-origin).Magnitude())
			{
				outtri = coltri;
				normal = colnorm;
				colpatch = &(*i)->GetPatch();
			}

			col = true;
		}
	}

	return col;
}

void ROADSTRIP::Reverse()
{
	patches.reverse();

	for (std::list <ROADPATCH>::iterator i = patches.begin(); i != patches.end(); ++i)
	{
		i->GetPatch().Reverse();
		i->GetPatch().ResetDistFromStart();
	}

	//fix pointers to next patches for race placement
	for (std::list <ROADPATCH>::iterator i = patches.begin(); i != patches.end(); ++i)
	{
		std::list <ROADPATCH>::iterator n = i;
		++n;
		BEZIER * nextpatchptr = NULL;
		if (n != patches.end())
		{
			nextpatchptr = &(n->GetPatch());
			i->GetPatch().Attach(*nextpatchptr);
		}
		else
		{
			i->GetPatch().ResetNextPatch();
			i->GetPatch().Attach(patches.front().GetPatch());
		}
	}
}

/*void ROADSTRIP::CreateRacingLine(
	SCENENODE * parentnode, 
	TEXTURE_GL & racingline_texture,
	std::ostream & error_output)
{
	for (std::list <ROADPATCH>::iterator i = patches.begin(); i != patches.end(); ++i)
	{
		std::list <ROADPATCH>::iterator n = i;
		++n;
		ROADPATCH * nextpatch(NULL);
		if (n != patches.end())
		{
			nextpatch = &(*n);
		}
		i->AddRacinglineScenenode(parentnode, nextpatch, racingline_texture, error_output);
	}
}
*/