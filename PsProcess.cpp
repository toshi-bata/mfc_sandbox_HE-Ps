#include "stdafx.h"
#include "PsProcess.h"
#include "ps_utilities.h"

//////////////// Lower-level functions ////////////////
// Calculates the dot product of two vectors.
double dotProduct(PK_VECTOR_t a, PK_VECTOR_t b)
{
	double dotProduct = 0.0;
	dotProduct = (a.coord[0] * b.coord[0]) + (a.coord[1] * b.coord[1]) + (a.coord[2] * b.coord[2]);
	return dotProduct;
}

// Calculates the cross product of two vectors.
PK_VECTOR_t findCrossProduct(PK_VECTOR_t a, PK_VECTOR_t b)
{
	PK_VECTOR_t findCrossProduct = { 0 };
	findCrossProduct.coord[0] = (a.coord[1] * b.coord[2]) - (a.coord[2] * b.coord[1]);
	findCrossProduct.coord[1] = (a.coord[2] * b.coord[0]) - (a.coord[0] * b.coord[2]);
	findCrossProduct.coord[2] = (a.coord[0] * b.coord[1]) - (a.coord[1] * b.coord[0]);
	return findCrossProduct;
}

// Calculates the magnitude of the specified vector.
double findMagnitude(PK_VECTOR_t a)
{
	double squaredMagnitude = 0.0;
	for (int i = 0; i < 3; i++)
	{
		squaredMagnitude += a.coord[i] * a.coord[i];
	}
	double mag = sqrt(squaredMagnitude);
	return sqrt(squaredMagnitude);
}

//////////////// Geometric tests ////////////////
// This function determines the angle between two axes of cylindrical surfaces.
double findAngleBetweenAxes(PK_SURF_t surf1, PK_SURF_t surf2)
{
	// Initialisations ////////////////
	double angle = 0;
	PK_VECTOR1_t surf_axis1, surf_axis2;
	///////////////////////////////////

	// In this code example findAngleBetweenAxes is called on a cylinder face. This function could be written to consider other surface types not demonstrated in this
	// code example.
	PK_CYL_sf_t cylinder_sf_1;
	PK_CYL_sf_t cylinder_sf_2;
	PK_CYL_ask(surf1, &cylinder_sf_1);
	PK_CYL_ask(surf2, &cylinder_sf_2);
	surf_axis1 = cylinder_sf_1.basis_set.axis;
	surf_axis2 = cylinder_sf_2.basis_set.axis;

	// Find the angle. See Implementation notes for more detail on this calculation.
	return angle = asin(findMagnitude(findCrossProduct(surf_axis1, surf_axis2)));
}

// This function finds the distance between two axes of cylindrical surfaces.
double findDistanceBetweenAxes(PK_SURF_t surf1, PK_SURF_t surf2)
{
	// Initialisations ////////////////
	double distance = 0;
	PK_VECTOR1_t surf_axis1, surf_axis2, norm_axis_vec;
	PK_VECTOR_t surf_location1 = { 0 }, surf_location2 = { 0 }, dist_vec = { 0 };
	//////////////////////////////////

	// In this code example findDistanceBetweenAxes is called on a cylinder face. This function could be written to consider other surface type not demonstrated in this
	// code example.

	PK_CYL_sf_t cylinder_sf_1;
	PK_CYL_sf_t cylinder_sf_2;
	PK_CYL_ask(surf1, &cylinder_sf_1);
	PK_CYL_ask(surf2, &cylinder_sf_2);
	surf_axis1 = cylinder_sf_1.basis_set.axis;
	surf_axis2 = cylinder_sf_2.basis_set.axis;
	surf_location1 = cylinder_sf_1.basis_set.location;
	surf_location2 = cylinder_sf_2.basis_set.location;

	// Find the distance vector between the cylinders' basis set location points.
	dist_vec.coord[0] = surf_location1.coord[0] - surf_location2.coord[0];
	dist_vec.coord[1] = surf_location1.coord[1] - surf_location2.coord[1];
	dist_vec.coord[2] = surf_location1.coord[2] - surf_location2.coord[2];
	// Normalise vector for one axis.
	PK_VECTOR_normalise(surf_axis1, &norm_axis_vec);
	// Final calculation to find the linear distance. See Implementation notes for more detail on this calculation.
	return distance = findMagnitude(findCrossProduct(dist_vec, norm_axis_vec));
}

// This function finds the angle between two planes
double findAngleBetweenPlanes(PK_PLANE_t plane1, PK_PLANE_t plane2)
{
	// Initialisations ////////////////
	PK_PLANE_sf_t plane_info1;
	PK_PLANE_sf_t plane_info2;
	PK_VECTOR_t surf_normal1 = { 0 }, surf_normal2 = { 0 }, point1 = { 0 }, point2 = { 0 };
	double angle = 0;
	//////////////////////////////////
	PK_PLANE_ask(plane1, &plane_info1);
	PK_PLANE_ask(plane2, &plane_info2);
	surf_normal1 = plane_info1.basis_set.axis;
	surf_normal2 = plane_info2.basis_set.axis;

	// Find the angle between the two surface normals.
	return angle = asin(findMagnitude(findCrossProduct(surf_normal1, surf_normal2)));
}

// This function finds the distance between two planes. See Implementation notes for more detail on this calculation.
/// This function assumes that the planes are coplanar or very nearly coplanar - the calculation in this function requires
/// this condition to be met in order to give reliable results
double findDistanceBetweenPlanes(PK_PLANE_t plane1, PK_PLANE_t plane2)
{
	// Initialisations ////////////////
	PK_PLANE_sf_t plane_info1;
	PK_PLANE_sf_t plane_info2;
	PK_VECTOR1_t norm_axis_vec;
	PK_VECTOR_t point = { 0 }, surf_normal = { 0 }, surf_location_norm2 = { 0 },
		dist_vec = { 0 }, surf_location1 = { 0 }, surf_location2 = { 0 }, surf_normal2 = { 0 };
	double distance = 0;
	///////////////////////////////////

	// Find information about plane1. We need a location on plane1, so use the one from the basis set and call it surf_location1.
	PK_PLANE_ask(plane1, &plane_info1);
	surf_location1 = plane_info1.basis_set.location;
	surf_normal = plane_info1.basis_set.axis;

	PK_PLANE_ask(plane2, &plane_info2);
	surf_location2 = plane_info2.basis_set.location;

	// Find the distance vector between the basis set location points.
	dist_vec.coord[0] = surf_location1.coord[0] - surf_location2.coord[0];
	dist_vec.coord[1] = surf_location1.coord[1] - surf_location2.coord[1];
	dist_vec.coord[2] = surf_location1.coord[2] - surf_location2.coord[2];
	// Normalise vector for one axis.
	PK_VECTOR_normalise(surf_normal, &norm_axis_vec);

	// Final calculation to find the linear distance. See Implementation notes for more detail on this calculation.
	return distance = abs(dotProduct(dist_vec, norm_axis_vec));
}

PsProcess::PsProcess() :
	m_partition(PK_ENTITY_null)
{
}

PsProcess::~PsProcess()
{
}

void PsProcess::Initialize()
{
	PK_ERROR_code_t error_code;

	// Get current partation
	PK_PARTITION_t old_partition;
	error_code = PK_SESSION_ask_curr_partition(&old_partition);

	// Make a new partation and set current
	error_code = PK_PARTITION_create_empty(&m_partition);
	error_code = PK_PARTITION_set_current(m_partition);

	// Delet old partition
	PK_PARTITION_delete_o_t delete_opts;
	PK_PARTITION_delete_o_m(delete_opts);
	delete_opts.delete_non_empty = PK_LOGICAL_true;
	error_code = PK_PARTITION_delete(old_partition, &delete_opts);
}

PK_ASSEMBLY_t PsProcess::findTopAssy()
{
	PK_ERROR_code_t error_code;
	PK_ASSEMBLY_t topAssem = PK_ENTITY_null;

	PK_PARTITION_t partition;
	int n_assy = 0;
	PK_ASSEMBLY_t* assems;
	PK_SESSION_ask_curr_partition(&partition);
	error_code = PK_PARTITION_ask_assemblies(partition, &n_assy, &assems);

	// chile-parent map
	std::map<PK_ASSEMBLY_t, PK_ASSEMBLY_t> child_parent;

	// Search assemblies and regiter to the map
	for (int i = 0; i < n_assy; i++)
	{
		PK_ASSEMBLY_t assem = assems[i];
		PK_CLASS_t ent_class;
		PK_ENTITY_ask_class(assem, &ent_class);

		if (PK_CLASS_assembly == ent_class)
			child_parent.insert(std::make_pair(assem, PK_ENTITY_null));
	}

	for (auto itr = child_parent.begin(); itr != child_parent.end(); ++itr)
	{
		PK_ASSEMBLY_t assem = itr->first;

		int n_inst;
		PK_INSTANCE_t* instances;
		PK_ASSEMBLY_ask_instances(assem, &n_inst, &instances);

		for (int i = 0; i < n_inst; i++)
		{
			PK_INSTANCE_t instance = instances[i];

			PK_INSTANCE_sf_s instance_sf;
			error_code = PK_INSTANCE_ask(instance, &instance_sf);

			// If instance part exists in the map, its pearent becomes current assem 
			PK_CLASS_t ent_class;
			PK_ENTITY_ask_class(instance_sf.part, &ent_class);
			PK_PART_t part = instance_sf.part;

			if (PK_CLASS_assembly == ent_class)
			{
				for (auto itr2 = child_parent.begin(); itr2 != child_parent.end(); ++itr2)
				{
					if (part == itr2->first)
						itr2->second = assem;
				}
			}
		}
	}

	//Assem which doesn't have a parent becomes top assem
	for (auto itr = child_parent.begin(); itr != child_parent.end(); ++itr)
	{
		if (PK_ENTITY_null == itr->second)
		{
			topAssem = itr->first;
			return topAssem;
		}
	}
	return topAssem;
}

bool PsProcess::Save()
{
	PK_ERROR_code_t error_code;

	int n_parts = 0;
	PK_PART_t* parts = NULL;

	int n_assy = 0;
	PK_ASSEMBLY_t* assems;
	PK_PARTITION_t partition;

	PK_SESSION_ask_curr_partition(&partition);
	error_code = PK_PARTITION_ask_assemblies(partition, &n_assy, &assems);

	if (0 < n_assy)
	{
		PK_ASSEMBLY_t topAssy = findTopAssy();

		if (PK_ENTITY_null != topAssy)
		{
			n_parts = 1;
			parts = new PK_PART_t[1];
			parts[0] = topAssy;
		}
	}
	else
	{
		PK_SESSION_ask_parts(&n_parts, &parts);
	}

	if (0 < n_parts)
	{
		PK_PART_transmit_o_t transmit_opts;
		PK_PART_transmit_o_m(transmit_opts);
		transmit_opts.transmit_format = PK_transmit_format_text_c;
		transmit_opts.transmit_version = 320;

		char filePath[256];
		sprintf(filePath, "C:\\temp\\debug.x_t");
		error_code = PK_PART_transmit(n_parts, parts, filePath, &transmit_opts);

		if (PK_ERROR_no_errors == error_code)
			return true;
		else
		{
			return false;
		}
	}

	return false;
}

bool PsProcess::BlendRC(const PsBlendType blendType, const double inBlendR, const double blendC2, const PK_BODY_t body, 
	const int edgeCnt, const PK_EDGE_t* edges, const PK_FACE_t* faces)
{
	// Parasolid session
	PK_ERROR_code_t error_code;

	// Set mark
	PK_PMARK_t mark;
	error_code = PK_MARK_create(&mark);

	int n_blend_edges = 0;
	PK_EDGE_t* blend_edges = NULL;

	switch (blendType)
	{
	case PsBlendType::R:
	{
		PK_EDGE_set_blend_constant_o_t options;
		PK_EDGE_set_blend_constant_o_m(options);
		options.properties.propagate = PK_blend_propagate_yes_c;
		options.properties.ov_cliff_end = PK_blend_ov_cliff_end_yes_c;

		error_code = PK_EDGE_set_blend_constant(edgeCnt, edges, inBlendR / m_dUnit, &options, &n_blend_edges, &blend_edges);
	} break;
	case PsBlendType::C:
	{
		PK_EDGE_set_blend_chamfer_o_t options;
		PK_EDGE_set_blend_chamfer_o_m(options);
		error_code = PK_EDGE_set_blend_chamfer(edgeCnt, edges, inBlendR / m_dUnit, blendC2 / m_dUnit, faces, &options, &n_blend_edges, &blend_edges);
	} break;
	default:
		break;
	}

	PK_BODY_fix_blends_o_t optionsFix;
	PK_BODY_fix_blends_o_m(optionsFix);

	int n_blends = 0;
	PK_FACE_t* blends = 0;
	PK_FACE_array_t* unders = 0;
	int* topols = 0;
	PK_blend_fault_t fault;
	PK_EDGE_t fault_edge = PK_ENTITY_null;
	PK_ENTITY_t fault_topol = PK_ENTITY_null;

	// perform the Blend operation
	error_code = PK_BODY_fix_blends(body, &optionsFix, &n_blends, &blends, &unders, &topols, &fault, &fault_edge, &fault_topol);

#ifdef _DEBUG
	{
		//char  *path = "C:\\temp\\debug_blend.x_t";
		//PK_PART_transmit_o_t transmit_opts;
		//PK_PART_transmit_o_m(transmit_opts);
		//transmit_opts.transmit_format = PK_transmit_format_text_c;

		//error_code = PK_PART_transmit(1, &body, path, &transmit_opts);
	}
#endif

	if (PK_blend_fault_no_fault_c != fault)
	{
		// Undo operation
		PK_MARK_goto_o_t goto_ots;
		PK_MARK_goto_r_t goto_result;
		PK_MARK_goto_o_m(goto_ots);
		error_code = PK_MARK_goto_2(mark, &goto_ots, &goto_result);

		return false;
	}

	return true;
}

bool PsProcess::Hollow(const double thisckness, const PK_BODY_t body, const int faceCnt, const PK_FACE_t* pierceFaces)
{
	PK_ERROR_code_t error_code;

	PK_TOPOL_track_r_t   tracking;
	PK_TOPOL_local_r_t   results;
	PK_BODY_hollow_o_t   hollow_opts;
	PK_BODY_hollow_o_m(hollow_opts);

	if (faceCnt)
	{
		hollow_opts.n_pierce_faces = faceCnt;
		hollow_opts.pierce_faces = pierceFaces;
	}

	error_code = PK_BODY_hollow_2(body, thisckness / m_dUnit, 1.0e-06, &hollow_opts, &tracking, &results);

	if (PK_ERROR_no_errors != error_code)
		return false;

	return true;
}

void PsProcess::setBasisSet(const double* in_offset, const double* in_dir, PK_AXIS2_sf_s& basis_set)
{
	PK_ERROR_code_t error_code;

	PK_VECTOR_t zVect, dir, rotateAxis;
	zVect.coord[0] = 0;
	zVect.coord[1] = 0;
	zVect.coord[2] = 1.0;

	dir.coord[0] = in_dir[0];
	dir.coord[1] = in_dir[1];
	dir.coord[2] = in_dir[2];

	double angle = PK_VECTOR_angleDeg(zVect, dir, rotateAxis);

	PK_VECTOR_t axis;
	axis.coord[0] = 0;
	axis.coord[1] = 0;
	axis.coord[2] = 1.0;

	PK_VECTOR_t xDir;
	xDir.coord[0] = 1.0;
	xDir.coord[1] = 0;
	xDir.coord[2] = 0;

	if (0 == angle) {}
	else if (M_PI == angle) {
		axis.coord[2] = -1;
	}
	else {
		PK_TRANSF_t rotTransf;
		PK_VECTOR_t org;
		org.coord[0] = 0.0;
		org.coord[1] = 0.0;
		org.coord[2] = 0.0;

		PK_TRANSF_create_rotation(org, rotateAxis, angle, &rotTransf);

		error_code = PK_VECTOR_transform(axis, rotTransf, &axis);
		error_code = PK_VECTOR_transform(xDir, rotTransf, &xDir);
	}

	basis_set.location.coord[0] = in_offset[0] / m_dUnit;
	basis_set.location.coord[1] = in_offset[1] / m_dUnit;
	basis_set.location.coord[2] = in_offset[2] / m_dUnit;
	basis_set.axis = axis;
	basis_set.ref_direction = xDir;
}

bool PsProcess::createBlock(const double* in_size, const double* in_offset, const double* in_dir, PK_BODY_t& body)
{
	PK_ERROR_code_t error_code;

	double dX = in_size[0] / m_dUnit;
	double dY = in_size[1] / m_dUnit;
	double dZ = in_size[2] / m_dUnit;

	PK_AXIS2_sf_s basis_set;
	setBasisSet(in_offset, in_dir, basis_set);

	error_code = PK_BODY_create_solid_block(dX, dY, dZ, &basis_set, &body);

	if (PK_ERROR_no_errors != error_code)
		return false;

	return true;
}

bool PsProcess::createCylinder(const double rad, const double height, const double* in_offset, const double* in_dir, PK_BODY_t& body)
{
	PK_ERROR_code_t error_code;

	PK_AXIS2_sf_s basis_set;
	setBasisSet(in_offset, in_dir, basis_set);

	error_code = PK_BODY_create_solid_cyl(rad / m_dUnit, height / m_dUnit, &basis_set, &body);

	if (PK_ERROR_no_errors != error_code)
		return false;

	return true;
}

bool PsProcess::createSpring(const double in_outDia, const double in_L, const double in_wireDia, const double in_hook_ang, PK_BODY_t& body)
{
	PK_ERROR_code_t error_code;

	double dCenterRad = (in_outDia - in_wireDia) / 2.0;
	double dSpringLen = in_L - dCenterRad * 4.0 + in_wireDia;
	double turnNum = floor(dSpringLen / (in_wireDia * 3.0)) - (180.0 - in_hook_ang) / 360.0;

	// Compute helical angle
	double helAng = atan((dSpringLen / (turnNum * 2.0)) / (dCenterRad * 2.0)) / M_PI * 180.0;

	// Create rotate transform for top hook angle
	PK_TRANSF_t roTransf;
	PK_TRANSF_create_rotation({ 0, 0, 0 }, { 0, 0, 1.0 }, -(180.0 - in_hook_ang) / 180.0 * M_PI, &roTransf);

	// Create a bottom fook arc
	PK_CIRCLE_t arc_b1 = PK_ENTITY_null;
	{
		PK_AXIS2_sf_s basis_set;
		basis_set.location.coord[0] = 0;
		basis_set.location.coord[1] = 0;
		basis_set.location.coord[2] = (dCenterRad - in_wireDia / 2.0) / m_dUnit;
		basis_set.axis.coord[0] = -1.0;
		basis_set.axis.coord[1] = 0;
		basis_set.axis.coord[2] = 0;
		basis_set.ref_direction.coord[0] = 0;
		basis_set.ref_direction.coord[1] = 1;
		basis_set.ref_direction.coord[2] = 0;

		PK_CIRCLE_sf_t circle_info;
		circle_info.basis_set = basis_set;
		circle_info.radius = dCenterRad / m_dUnit;
		error_code = PK_CIRCLE_create(&circle_info, &arc_b1);
	}
	PK_INTERVAL_t arc_b1_int = { 0, 270.0 / 180.0 * M_PI };

	// Create a bottom line
	PK_LINE_t line_b;
	{
		PK_LINE_sf_t line_info;
		line_info.basis_set.axis.coord[0] = 0;
		line_info.basis_set.axis.coord[1] = 1.0;
		line_info.basis_set.axis.coord[2] = 0;
		line_info.basis_set.location.coord[0] = 0;
		line_info.basis_set.location.coord[1] = 0;
		line_info.basis_set.location.coord[2] = (dCenterRad * 2.0 - in_wireDia / 2.0) / m_dUnit;
		error_code = PK_LINE_create(&line_info, &line_b);
	}
	PK_INTERVAL_t line_b_int = { 0, dCenterRad / m_dUnit };

	// Create a bottom arc between line and helical curve
	PK_CIRCLE_t arc_b2 = PK_ENTITY_null;
	{
		PK_AXIS2_sf_s basis_set;
		basis_set.location.coord[0] = 0;
		basis_set.location.coord[1] = 0;
		basis_set.location.coord[2] = (dCenterRad * 2.0 - in_wireDia / 2.0) / m_dUnit;
		basis_set.axis.coord[0] = 0;
		basis_set.axis.coord[1] = 0;
		basis_set.axis.coord[2] = 1.0;
		basis_set.ref_direction.coord[0] = 0;
		basis_set.ref_direction.coord[1] = 1.0;
		basis_set.ref_direction.coord[2] = 0;

		PK_CIRCLE_sf_t circle_info;
		circle_info.basis_set = basis_set;
		circle_info.radius = dCenterRad / m_dUnit;
		error_code = PK_CIRCLE_create(&circle_info, &arc_b2);
	}
	PK_INTERVAL_t arc_b2_int = { 0, 90.0 / 180.0 * M_PI };

	// Set up the point standard form structure
	PK_POINT_sf_t point_sf;
	point_sf.position.coord[0] = -dCenterRad / m_dUnit;
	point_sf.position.coord[1] = 0.0;
	point_sf.position.coord[2] = (dCenterRad * 2.0 - in_wireDia / 2.0) / m_dUnit;

	// Create a point
	PK_POINT_t point;
	error_code = PK_POINT_create(&point_sf, &point);

	// Set up the axis standard form
	PK_AXIS1_sf_t axis;
	axis.location.coord[0] = 0.0;
	axis.location.coord[1] = 0.0;
	axis.location.coord[2] = 0.0;
	axis.axis.coord[0] = 0.0;
	axis.axis.coord[1] = 0.0;
	axis.axis.coord[2] = 1.0;

	PK_INTERVAL_t helical_int = { 0.0, ceil(turnNum) };

	PK_HAND_t hand = PK_HAND_right_c;
	double helical_pitch = dSpringLen / turnNum / m_dUnit;
	double spiral_pitch = 0.0;
	double tolerance = 1.0e-5;

	PK_CURVE_t helical;
	error_code = PK_POINT_make_helical_curve(point, &axis, hand, helical_int, helical_pitch, spiral_pitch, tolerance, &helical);

	// Create a top arc between line and helical curve
	PK_CIRCLE_t arc_t1 = PK_ENTITY_null;
	{
		PK_VECTOR_t pos = { -1.0, 0, 0 };
		PK_VECTOR_transform(pos, roTransf, &pos);

		PK_AXIS2_sf_s basis_set;
		basis_set.location.coord[0] = 0;
		basis_set.location.coord[1] = 0;
		basis_set.location.coord[2] = (dCenterRad * 2.0 + dSpringLen - in_wireDia / 2.0) / m_dUnit;
		basis_set.axis.coord[0] = 0;
		basis_set.axis.coord[1] = 0;
		basis_set.axis.coord[2] = 1.0;
		basis_set.ref_direction = pos;

		PK_CIRCLE_sf_t circle_info;
		circle_info.basis_set = basis_set;
		circle_info.radius = dCenterRad / m_dUnit;
		error_code = PK_CIRCLE_create(&circle_info, &arc_t1);
	}
	PK_INTERVAL_t arc_t1_int = { 0, 90.0 / 180.0 * M_PI };

	// Create a top line
	PK_LINE_t line_t;
	{
		PK_VECTOR_t pos = { 0, 1.0, 0 };
		PK_VECTOR_transform(pos, roTransf, &pos);

		PK_LINE_sf_t line_info;
		line_info.basis_set.axis = pos;
		line_info.basis_set.location.coord[0] = 0;
		line_info.basis_set.location.coord[1] = 0;
		line_info.basis_set.location.coord[2] = (dCenterRad * 2.0 + dSpringLen - in_wireDia / 2.0) / m_dUnit;
		error_code = PK_LINE_create(&line_info, &line_t);
	}
	PK_INTERVAL_t line_t_int = { -dCenterRad / m_dUnit, 0 };

	// Create a top hook arc
	PK_CIRCLE_t arc_t = PK_ENTITY_null;
	{
		PK_VECTOR_t pos = { 1.0, 0, 0 };
		PK_VECTOR_transform(pos, roTransf, &pos);

		PK_AXIS2_sf_s basis_set;
		basis_set.location.coord[0] = 0;
		basis_set.location.coord[1] = 0;
		basis_set.location.coord[2] = (dCenterRad * 3.0 + dSpringLen - in_wireDia / 2.0) / m_dUnit;
		basis_set.axis = pos;
		basis_set.ref_direction.coord[0] = 0;
		basis_set.ref_direction.coord[1] = 0;
		basis_set.ref_direction.coord[2] = -1;

		PK_CIRCLE_sf_t circle_info;
		circle_info.basis_set = basis_set;
		circle_info.radius = dCenterRad / m_dUnit;
		error_code = PK_CIRCLE_create(&circle_info, &arc_t);
	}
	PK_INTERVAL_t arc_t_int = { 0, 270.0 / 180.0 * M_PI };

	// Create a wire body from the helical curve
	PK_CURVE_t* curves_array;
	PK_INTERVAL_t* intervals_array;

	curves_array = new PK_CURVE_t[7];
	curves_array[0] = arc_b1;
	curves_array[1] = line_b;
	curves_array[2] = arc_b2;
	curves_array[3] = helical;
	curves_array[4] = arc_t1;
	curves_array[5] = line_t;
	curves_array[6] = arc_t;

	intervals_array = new PK_INTERVAL_t[7];
	intervals_array[0] = arc_b1_int;
	intervals_array[1] = line_b_int;
	intervals_array[2] = arc_b2_int;
	helical_int.value[1] = 1.0 - (ceil(turnNum) - turnNum) / ceil(turnNum);
	intervals_array[3] = helical_int;
	intervals_array[4] = arc_t1_int;
	intervals_array[5] = line_t_int;
	intervals_array[6] = arc_t_int;

	int n_curves = 7;

	PK_CURVE_make_wire_body_o_t make_wire_body_opts;
	PK_CURVE_make_wire_body_o_m(make_wire_body_opts);
	static PK_BODY_t path_body = PK_ENTITY_null;
	int n_edges = 0;
	PK_EDGE_t* new_edges = PK_ENTITY_null;
	int* edge_index = NULL;
	error_code = PK_CURVE_make_wire_body_2(n_curves, curves_array, intervals_array, &make_wire_body_opts, &path_body, &n_edges, &new_edges, &edge_index);

	// Find the vertices of the path body
	{
		int n_vertices;
		PK_VERTEX_t* vertices = PK_ENTITY_null;
		PK_BODY_ask_vertices(path_body, &n_vertices, &vertices);

		// Sweep the body if vertices are returned
		if (n_vertices > 0)
		{
			PK_VECTOR_t pos = { 0, -dCenterRad, (dCenterRad * 2.0 + dSpringLen - in_wireDia / 2.0) };
			PK_VECTOR_transform(pos, roTransf, &pos);

			for (int i = 0; i < n_vertices; i++)
			{
				PK_VERTEX_t vertex = vertices[i];

				PK_POINT_t point;
				PK_VERTEX_ask_point(vertex, &point);

				PK_POINT_sf_s point_sf;
				PK_POINT_ask(point, &point_sf);
				if (m_dTol > abs(point_sf.position.coord[0]) &&
					m_dTol > abs(point_sf.position.coord[1] - dCenterRad / m_dUnit) &&
					m_dTol > abs(point_sf.position.coord[2] - (dCenterRad * 2.0 - in_wireDia / 2.0) / m_dUnit))
				{
					PK_EDGE_t new_edge;
					PK_VERTEX_t new_vertices[2];
					error_code = PK_VERTEX_make_blend(vertex, (in_wireDia + 0.1) / m_dUnit, PK_LOGICAL_false, &new_edge, new_vertices);
				}
				else if (m_dTol > abs(point_sf.position.coord[0] - pos.coord[0] / m_dUnit) &&
					m_dTol > abs(point_sf.position.coord[1] - pos.coord[1] / m_dUnit) &&
					m_dTol > abs(point_sf.position.coord[2] - pos.coord[2] / m_dUnit))
				{
					PK_EDGE_t new_edge;
					PK_VERTEX_t new_vertices[2];
					error_code = PK_VERTEX_make_blend(vertex, (in_wireDia + 0.1) / m_dUnit, PK_LOGICAL_false, &new_edge, new_vertices);
				}
			}
			//Free the memory
			PK_MEMORY_free(vertices);
		}
	}

	// Delete arrays
	delete[] curves_array;
	delete[] intervals_array;

	// Create a circular sheet (profile)
	static PK_BODY_t profile = PK_ENTITY_null;
	{
		// Set up the basis set for the profile
		PK_AXIS2_sf_s basis_set;
		basis_set.location.coord[0] = 0.0;
		basis_set.location.coord[1] = dCenterRad / m_dUnit;
		basis_set.location.coord[2] = (dCenterRad - in_wireDia / 2.0) / m_dUnit;
		basis_set.axis.coord[0] = 0.0;
		basis_set.axis.coord[1] = 0.0;
		basis_set.axis.coord[2] = -1.0;
		basis_set.ref_direction.coord[0] = 1.0;
		basis_set.ref_direction.coord[1] = 0.0;
		basis_set.ref_direction.coord[2] = 0.0;

		error_code = PK_BODY_create_sheet_circle(in_wireDia / m_dUnit, &basis_set, &profile);
	}

	// Set up and initialize the sweep options structure to its default values
	PK_BODY_make_swept_body_2_o_t sweep_opts;
	PK_BODY_make_swept_body_2_o_m(sweep_opts);
	sweep_opts.topology_form = PK_BODY_topology_grid_c;

	// Find the vertices of the path body
	PK_VERTEX_t start_vertex;
	{
		int n_vertices;
		PK_VERTEX_t* vertices = PK_ENTITY_null;
		PK_BODY_ask_vertices(path_body, &n_vertices, &vertices);

		// Sweep the body if vertices are returned
		if (n_vertices > 0)
		{
			start_vertex = vertices[n_vertices - 1];
			for (int i = 0; i < n_vertices; i++)
			{
				PK_VERTEX_t vertex = vertices[i];

				PK_POINT_t point;
				PK_VERTEX_ask_point(vertex, &point);

				PK_POINT_sf_s point_sf;
				PK_POINT_ask(point, &point_sf);

				if (m_dTol > abs(point_sf.position.coord[0]) &&
					m_dTol > abs(point_sf.position.coord[1] - dCenterRad / m_dUnit) &&
					m_dTol > abs(point_sf.position.coord[2] - (dCenterRad - in_wireDia / 2.0) / m_dUnit))
				{
					start_vertex = vertex;
					//Free the memory
					PK_MEMORY_free(vertices);

					break;
				}
			}
		}
	}

	// Ensure the correct start vertex is selected for the sweep and not just the first vertex returned in the array of vertices
	static PK_BODY_tracked_sweep_2_r_t swept_res;
	error_code = PK_BODY_make_swept_body_2(1, &profile, path_body, &start_vertex, &sweep_opts, &swept_res);

	// Check for error and the status
	if ((error_code == PK_ERROR_no_errors) && (swept_res.status.fault == PK_BODY_sweep_ok_c))
	{
		// Assign the relevant return information into the relevant variables so that they can be referred to easily

		// The status
		static PK_BODY_sweep_status_2_r_t  status = swept_res.status;
		// The swept body
		body = swept_res.body;
	}
	// If errors are returned or the status returned is not OK, display a message appropriately to the user in a message box 
	else
	{
		return false;
	}
	return true;
}

void PsProcess::setPartName(const PK_PART_t in_part, const char* in_value)
{
	PK_ERROR_code_t error_code;

	PK_ATTDEF_t attdef;
	error_code = PK_ATTDEF_find("SDL/TYSA_NAME", &attdef);

	PK_ATTRIB_t attribs;
	error_code = PK_ATTRIB_create_empty(in_part, attdef, &attribs);

	error_code = PK_ATTRIB_set_string(attribs, 0, in_value);
}

void PsProcess::setPartColor(const PK_PART_t in_part, const double* in_color)
{
	PK_ERROR_code_t error_code;

	PK_ATTDEF_t system_color;
	PK_ATTDEF_find("SDL/TYSA_COLOUR_2", &system_color);

	PK_ATTRIB_t color_attribute;
	PK_ATTRIB_create_empty(in_part, system_color, &color_attribute);

	error_code = PK_ATTRIB_set_doubles(color_attribute, 0, 3, in_color);
}

bool PsProcess::CreateSolid(const SolidShape solidShape, const double* in_size, const double* in_offset, const double* in_dir, PK_BODY_t& body)
{
	PK_ERROR_code_t error_code;
	
	bool bRet = false;
	switch (solidShape)
	{
	case BLOCK:
	{
		bRet =  createBlock(in_size, in_offset, in_dir, body);
	} break;
	case CYLINDER:
	{
		double rad = in_size[0];
		double height = in_size[1];
		bRet = createCylinder(rad, height, in_offset, in_dir, body);
	} break;
	case SPRING:
	{
		double D = in_size[0];
		double H = in_size[1];
		double wireD = in_size[2];
		double ang = in_size[3];
		bRet = createSpring(D, H, wireD, ang, body);
	} break;
	default:
		break;
	}

	// Set body color
	double color[] = { 128.0/255.0, 128.0/255.0, 128/255.0 };
	setPartColor(body, color);

	return bRet;
}

bool PsProcess::DeleteBody(const PK_BODY_t body)
{
	PK_ERROR_code_t error_code = PK_ENTITY_delete(1, &body);

	if (PK_ERROR_no_errors != error_code)
		return false;

	return true;
}

bool PsProcess::DeleteFace(const int faceCnt, const PK_FACE_t* faces)
{
	PK_ERROR_code_t error_code;

	// Set mark
	PK_PMARK_t mark;
	error_code = PK_MARK_create(&mark);

	PK_FACE_delete_o_t del_ots;
	PK_FACE_delete_o_m(del_ots);
	PK_TOPOL_track_r_t track;
	error_code = PK_FACE_delete_2(faceCnt, faces, &del_ots, &track);

	if (PK_ERROR_no_errors != error_code)
	{
		// Undo operation
		PK_MARK_goto_o_t goto_ots;
		PK_MARK_goto_r_t goto_result;
		PK_MARK_goto_o_m(goto_ots);
		error_code = PK_MARK_goto_2(mark, &goto_ots, &goto_result);

		return false;
	}
	return true;
}

bool PsProcess::Boolean(const PsBoolType boolType, const PK_BODY_t targetBody, const int toolCnt, const PK_BODY_t* toolBodies, int& bodyCnt, PK_BODY_t*& bodies)
{
	PK_ERROR_code_t error_code;

	// Set mark
	PK_PMARK_t mark;
	error_code = PK_MARK_create(&mark);

	PK_BODY_boolean_o_t options;
	PK_TOPOL_track_r_t tracking;
	PK_boolean_r_t results;

	PK_BODY_boolean_o_m(options);

	switch (boolType)
	{
	case PsBoolType::UNITE: options.function = PK_boolean_unite_c; break;
	case PsBoolType::SUBTRACT: options.function = PK_boolean_subtract_c; break;
	case PsBoolType::INTERSECT: options.function = PK_boolean_intersect_c; break;
	default: return false; break;
	}
	options.merge_imprinted = PK_LOGICAL_true;

	// perform the Boolean operation
	error_code = PK_BODY_boolean_2(targetBody, toolCnt, toolBodies, &options, &tracking, &results);

	if (PK_ERROR_no_errors != error_code)
	{
		// Undo operation
		PK_MARK_goto_o_t goto_ots;
		PK_MARK_goto_r_t goto_result;
		PK_MARK_goto_o_m(goto_ots);
		error_code = PK_MARK_goto_2(mark, &goto_ots, &goto_result);

		return false;
	}

#ifdef _DEBUG
	{
		//char  *path = "C:\\temp\\debug_blend.x_t";
		//PK_PART_transmit_o_t transmit_opts;
		//PK_PART_transmit_o_m(transmit_opts);
		//transmit_opts.transmit_format = PK_transmit_format_text_c;

		//error_code = PK_PART_transmit(1, &targetBody, path, &transmit_opts);
	}
#endif

	bodyCnt = results.n_bodies;
	bodies = results.bodies;

	return true;
}

bool PsProcess::FR_BOSS(const PK_BODY_t body, const PK_FACE_t face, std::vector<PK_ENTITY_t>& entityArr)
{
	PK_ERROR_code_t error_code;
	
	int n_edges = 0, edge_counter = 0;
	PK_EDGE_t* concave_edges = NULL, * convex_edges = NULL, * edges = NULL;
	PK_EDGE_ask_convexity_o_t convexity_opts;
	PK_emboss_convexity_t convexity;

	error_code = PK_BODY_ask_edges(body, &n_edges, &edges);
	if (PK_ERROR_no_errors != error_code)
		return false;

	if (0 != n_edges)
	{
		concave_edges = (PK_EDGE_t*)malloc(sizeof(PK_EDGE_t) * n_edges);

		edge_counter = 0;
		// Find all of the concave edges by looping through every edge on the body.
		for (int i = 0; i < n_edges; i++)
		{
			PK_EDGE_ask_convexity_o_m(convexity_opts);
			error_code = PK_EDGE_ask_convexity(edges[i], &convexity_opts, &convexity);
			if ((convexity == PK_EDGE_convexity_concave_c) || (convexity == PK_EDGE_convexity_smooth_ccv_c))
			{
				(concave_edges[edge_counter++] = edges[i]);

			}
		}

		// Free memory.
		PK_MEMORY_free(edges);
	}

	// Call PK_BODY_find_facesets with the concave_edges array and the input_face as the selecting topol.
	PK_BODY_find_facesets_o_t facesets_opts;
	PK_TOPOL_t selecting_topols_faces[1] = { PK_ENTITY_null };
	PK_BODY_find_facesets_r_t results;

	PK_BODY_find_facesets_o_m(facesets_opts);
	facesets_opts.selector = PK_boolean_include_c;
	facesets_opts.n_selecting_topol = 1;
	selecting_topols_faces[0] = face;
	facesets_opts.selecting_topol = selecting_topols_faces;
	error_code = PK_BODY_find_facesets(body, edge_counter, concave_edges, &facesets_opts, &results);
	if (PK_ERROR_no_errors != error_code)
		return false;

	// PK_BODY_find_facesets returns the faces in sets so get A3DEntities of selected faces using PkMapper
	for (int i = 0; i < results.n_selected_facesets; i++)
	{
		// Assign elements from faceset i.
		for (int m = 0; m < results.selected_facesets[i].length; m++)
		{
			PK_FACE_t selectedFace = results.selected_facesets[i].array[m];
			entityArr.push_back(selectedFace);
		}
	}

	return true;
}

bool PsProcess::FR_CONCENTRIC(const PK_BODY_t body, const PK_FACE_t face, std::vector<PK_ENTITY_t>& entityArr)
{
	double tolerance = 1.0e-06;		/// Linear tolerance
	double angular_tolerance = 1.0e-9; /// See implementation notes for reason for setting angular tolerance

	PK_SURF_t input_surf = PK_ENTITY_null, surf = PK_ENTITY_null;
	int n_faces = 0;
	PK_FACE_t* tempFaces = NULL;
	PK_CLASS_t surf_class = PK_ENTITY_null;

	// Find the concentric faces.
	// Find the surface tag of the input face.
	PK_FACE_ask_surf(face, &input_surf);

	// Find every face on the body.
	PK_BODY_ask_faces(body, &n_faces, &tempFaces);

	// Loop through every face and see if they match the criteria to be a cylindrical concentric face.
	// (This loop could easily be adjusted to exclude the input face if required.)
	for (int i = 0; i < n_faces; i++)
	{
		// Ask if the surface is a cylinder. If not, then it cannot be a concentric cylinder.
		PK_FACE_ask_surf(tempFaces[i], &surf);
		PK_ENTITY_ask_class(surf, &surf_class);
		if (surf_class == PK_CLASS_cyl)
		{
			// Angular precision is 1000 times smaller than linear precision. See Implementation notes for details.
			if ((findAngleBetweenAxes(input_surf, surf) <= angular_tolerance) && (findDistanceBetweenAxes(input_surf, surf) <= tolerance))
			{
				entityArr.push_back(tempFaces[i]);
			}
		}
	}

	// Free the memory for faces.
	if (n_faces)
	{
		PK_MEMORY_free(tempFaces);
	}

	return true;
}

bool PsProcess::FR_COPLANAR(const PK_BODY_t body, const PK_FACE_t face, std::vector<PK_ENTITY_t>& entityArr)
{
	double tolerance = 1.0e-06;		/// Linear tolerance
	double angular_tolerance = 1.0e-9; /// See implementation notes for reason for setting angular tolerance

	PK_SURF_t input_surf = PK_ENTITY_null, surf = PK_ENTITY_null;
	int n_faces = 0;
	PK_FACE_t* tempFaces = NULL;
	PK_CLASS_t surf_class = PK_ENTITY_null;

	// Find the coplanar faces.
// Find the tag of the input surf.
	PK_FACE_ask_surf(face, &input_surf);
	// Find every face on the body.
	PK_BODY_ask_faces(body, &n_faces, &tempFaces);

	// Loop through every face and see if they match the criteria to be a coplanar face.
	for (int i = 0; i < n_faces; i++)
	{
		// Ask if the surface is a plane. If not, then input_face and faces[i] cannot be coplanar.
		PK_FACE_ask_surf(tempFaces[i], &surf);
		PK_ENTITY_ask_class(surf, &surf_class);
		if (surf_class == PK_CLASS_plane)
		{
			if ((findAngleBetweenPlanes(input_surf, surf) <= angular_tolerance) && (findDistanceBetweenPlanes(input_surf, surf) <= tolerance))
			{
				entityArr.push_back(tempFaces[i]);
			}
		}
	}

	// Free the memory for faces.
	if (n_faces)
	{
		PK_MEMORY_free(tempFaces);
	}

	return true;
}

bool PsProcess::FR(const PsFRType frType, const PK_FACE_t face, std::vector<PK_ENTITY_t>& pkFaceArr)
{
	PK_ERROR_code_t error_code;
	
	bool bRet;

	// Get body tag
	PK_BODY_t body;
	error_code = PK_FACE_ask_body(face, &body);


	switch (frType)
	{
	case PsFRType::BOSS:
		bRet = FR_BOSS(body, face, pkFaceArr);
		break;
	case PsFRType::CONCENTRIC:
		bRet = FR_CONCENTRIC(body, face, pkFaceArr);
		break;
	case PsFRType::COPLANAR:
		bRet = FR_COPLANAR(body, face, pkFaceArr);
		break;
	default:
		bRet = false;
		break;
	}

	return bRet;
}

bool PsProcess::MirrorBody(const PK_BODY_t in_body, const double* in_location, const double* in_normal, const double isCopy, const double isMerge, PK_BODY_t& mirror_body)
{
	PK_ERROR_code_t error_code;

	PK_VECTOR_t position, normal;
	position.coord[0] = in_location[0] / m_dUnit;
	position.coord[1] = in_location[1] / m_dUnit;
	position.coord[2] = in_location[2] / m_dUnit;

	normal.coord[0] = in_normal[0];
	normal.coord[1] = in_normal[1];
	normal.coord[2] = in_normal[2];

	mirror_body = in_body;
	
	if (isCopy)
	{
		// Use PK_ENTITY_copy_2 to create a new body which is a copy of the original.
		PK_ENTITY_copy_o_t copy_opts;
		PK_ENTITY_track_r_t en_tracking;

		PK_ENTITY_copy_o_m(copy_opts);

		error_code = PK_ENTITY_copy_2(in_body, &copy_opts, &mirror_body, &en_tracking);

		// Set body color
		double color[] = { 128.0 / 255.0, 128.0 / 255.0, 128 / 255.0 };
		setPartColor(mirror_body, color);
	}

	// PK_TRANSF_create_reflection will create a transformation that when applied causes the body to be reflected in the plane defined by the given 'position' and 'normal'.
	PK_TRANSF_t transf;
	error_code = PK_TRANSF_create_reflection(position, normal, &transf);

	// Call PK_BODY_transform_2 to apply the transform to the desired body.
	PK_BODY_transform_o_t transf_opts;
	PK_TOPOL_track_r_t tracking;
	PK_TOPOL_local_r_t local_res;

	PK_BODY_transform_o_m(transf_opts);

	error_code = PK_BODY_transform_2(mirror_body, transf, 1.0e-06, &transf_opts, &tracking, &local_res);

	if (isMerge)
	{
		// Unite bodies
		PK_BODY_boolean_o_t options;
		PK_boolean_r_t results;

		PK_BODY_boolean_o_m(options);
		options.function = PK_boolean_unite_c;
		options.merge_imprinted = PK_LOGICAL_true;

		error_code = PK_BODY_boolean_2(in_body, 1, &mirror_body, &options, &tracking, &results);

		mirror_body = PK_ENTITY_null;
	}

	return true;
}

bool PsProcess::GetPlaneInfo(const PK_FACE_t face, double* position, double* normal)
{
	PK_SURF_t surf;
	PK_CLASS_t surf_class;

	PK_FACE_ask_surf(face, &surf);
	PK_ENTITY_ask_class(surf, &surf_class);

	if (PK_CLASS_plane == surf_class)
	{
		PK_PLANE_sf_t plane_sf;
		PK_PLANE_ask(surf, &plane_sf);
		position[0] = plane_sf.basis_set.location.coord[0] * m_dUnit;
		position[1] = plane_sf.basis_set.location.coord[1] * m_dUnit;
		position[2] = plane_sf.basis_set.location.coord[2] * m_dUnit;
		
		normal[0] = plane_sf.basis_set.axis.coord[0];
		normal[1] = plane_sf.basis_set.axis.coord[1];
		normal[2] = plane_sf.basis_set.axis.coord[2];
	}
	else
		return false;

	return true;
}