#include "PsComponentMapper.h"
#include "sprk_parasolid.h"

PsComponentMapper::PsComponentMapper(HPS::Component cadModel)
{
	traverseBodyCompo(cadModel);
}

PsComponentMapper::~PsComponentMapper()
{
}

void PsComponentMapper::traverseBodyCompo(const HPS::Component in_comp)
{
	HPS::ComponentArray subCompArr = in_comp.GetSubcomponents();

	for (int i = 0; i < subCompArr.size(); i++)
	{
		HPS::Component comp = subCompArr[i];
		HPS::Component::ComponentType compType = comp.GetComponentType();

		if (HPS::Component::ComponentType::ParasolidTopoBody == compType)
		{
			HPS::Parasolid::Component psComp = (HPS::Parasolid::Component)comp;
			int psEntity = psComp.GetParasolidEntity();
			m_psBodyMap[psEntity] = comp;

			return;
		}
		traverseBodyCompo(comp);
	}
}

HPS::Component PsComponentMapper::GetPsCompo(const int body, const int entity)
{
	if (0 == m_psBodyMap.count(body))
		return HPS::Component();

	HPS::Component bodyComp = m_psBodyMap[body];

	if (0 == entity)
		return bodyComp;

	if (0 == m_psBodyEntityMap.count(body))
	{
		// Create entity map if it isn't there
		std::map<int, HPS::Component> psEntityMap;

		HPS::ComponentArray faceSubs = bodyComp.GetAllSubcomponents(HPS::Component::ComponentType::ParasolidTopoFace);
		HPS::ComponentArray edgeSubs = bodyComp.GetAllSubcomponents(HPS::Component::ComponentType::ParasolidTopoEdge);

		for (int i = 0; i < faceSubs.size(); i++)
		{
			HPS::Parasolid::Component psComp = (HPS::Parasolid::Component)faceSubs[i];
			int psEntity = psComp.GetParasolidEntity();
			psEntityMap[psEntity] = psComp;
		}
		for (int i = 0; i < edgeSubs.size(); i++)
		{
			HPS::Parasolid::Component psComp = (HPS::Parasolid::Component)edgeSubs[i];
			int psEntity = psComp.GetParasolidEntity();
			psEntityMap[psEntity] = psComp;
		}
		m_psBodyEntityMap[body] = psEntityMap;

	}

	auto psEntityMap = m_psBodyEntityMap[body];

	if (psEntityMap.count(entity))
		return psEntityMap[entity];

	return HPS::Component();
}

void PsComponentMapper::DeleteBodyMap(const int body)
{
	if (1 == m_psBodyMap.count(body))
	{
		if (1 == m_psBodyEntityMap.count(body))
			m_psBodyEntityMap.erase(body);

		m_psBodyMap.erase(body);
	}
}

void PsComponentMapper::InitBodyMap(int body, HPS::Component bodyComp)
{
	DeleteBodyMap(body);
	m_psBodyMap[body] = bodyComp;
}