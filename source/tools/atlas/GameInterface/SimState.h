#ifndef SIMSTATE_INCLUDED
#define SIMSTATE_INCLUDED

class CUnit;
#include "maths/Vector3D.h"

class SimState
{
public:
	class Entity
	{
	public:
		static Entity Freeze(CUnit* unit);
		void Thaw();
	private:
		CStrW templateName;
		int unitID;
		std::set<CStr> selections;
		int playerID;
		CVector3D position;
		float angle;
	};

	class Nonentity
	{
	public:
		static Nonentity Freeze(CUnit* unit);
		void Thaw();
	private:
		CStrW actorName;
		int unitID;
		std::set<CStr> selections;
		CVector3D position;
		float angle;
	};
	
	static SimState* Freeze(bool onlyEntities);
	void Thaw();

private:
	bool onlyEntities;
	std::vector<Entity> entities;
	std::vector<Nonentity> nonentities;
};

#endif // SIMSTATE_INCLUDED
