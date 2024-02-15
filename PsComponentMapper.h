#pragma once
#include <map>
#include "sprk.h"

class PsComponentMapper
{
public:
	PsComponentMapper(HPS::Component cadModel);
	~PsComponentMapper();

private:
	std::map<int, HPS::Component> m_psBodyMap;
	std::map<int, std::map<int, HPS::Component>> m_psBodyEntityMap;

	void traverseBodyCompo(const HPS::Component in_comp);
public:
	HPS::Component GetPsCompo(const int body, const int entity = 0);
	void DeleteBodyMap(const int body);
	void InitBodyMap(int body, HPS::Component bodyComp);
};

