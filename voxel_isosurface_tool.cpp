#include "voxel_isosurface_tool.h"
#include "utility.h"

VoxelIsoSurfaceTool::VoxelIsoSurfaceTool() {
	_iso_scale = 1.0;
}

void VoxelIsoSurfaceTool::set_buffer(Ref<VoxelBuffer> buffer) {
	_buffer = buffer;
}

Ref<VoxelBuffer> VoxelIsoSurfaceTool::get_buffer() const {
	return _buffer;
}

void VoxelIsoSurfaceTool::set_iso_scale(float iso_scale) {
	_iso_scale = iso_scale;
}

float VoxelIsoSurfaceTool::get_iso_scale() const {
	return _iso_scale;
}

void VoxelIsoSurfaceTool::set_offset(Vector3 offset) {
	_offset = offset;
}

Vector3 VoxelIsoSurfaceTool::get_offset() const {
	return _offset;
}

namespace {

inline void do_op(VoxelBuffer &buffer, int x, int y, int z, float d1, VoxelIsoSurfaceTool::Operation op) {

	float res;

	switch (op) {

		case VoxelIsoSurfaceTool::OP_ADD:
			res = MIN(d1, buffer.get_voxel_iso(x, y, z, VoxelBuffer::CHANNEL_ISOLEVEL));
			break;

		case VoxelIsoSurfaceTool::OP_SUBTRACT:
			res = MAX(d1, buffer.get_voxel_iso(x, y, z, VoxelBuffer::CHANNEL_ISOLEVEL));
			break;

		case VoxelIsoSurfaceTool::OP_SET:
			res = d1;
			break;
	}

	buffer.set_voxel_iso(res, x, y, z, VoxelBuffer::CHANNEL_ISOLEVEL);
}

} // namespace

void VoxelIsoSurfaceTool::do_sphere(Vector3 center, real_t radius, Operation op) {

	ERR_FAIL_COND(_buffer.is_null());
	VoxelBuffer &buffer = **_buffer;

	center += _offset;

	for (int z = 0; z < buffer.get_size().z; ++z) {
		for (int x = 0; x < buffer.get_size().x; ++x) {
			for (int y = 0; y < buffer.get_size().y; ++y) {

				float d1 = (center.distance_to(Vector3(x, y, z)) - radius) * _iso_scale;
				do_op(buffer, x, y, z, d1, op);
			}
		}
	}
}

void VoxelIsoSurfaceTool::do_plane(Plane plane, Operation op) {

	ERR_FAIL_COND(_buffer.is_null());
	VoxelBuffer &buffer = **_buffer;

	plane = Plane(plane.center() + _offset, plane.normal);

	for (int z = 0; z < buffer.get_size().z; ++z) {
		for (int x = 0; x < buffer.get_size().x; ++x) {
			for (int y = 0; y < buffer.get_size().y; ++y) {

				float d1 = plane.distance_to(Vector3(x, y, z)) * _iso_scale;
				do_op(buffer, x, y, z, d1, op);
			}
		}
	}
}

namespace {

inline float sdf_cube(Vector3 pos, Vector3 extents) {

	Vector3 d = pos.abs() - extents;
	return min(max(d.x, max(d.y, d.z)), 0.0) + Vector3(max(d.x, 0), max(d.y, 0), max(d.z, 0)).length();
}

} // namespace

void VoxelIsoSurfaceTool::do_cube(Transform transform, Vector3 extents, Operation op) {

	ERR_FAIL_COND(_buffer.is_null());
	VoxelBuffer &buffer = **_buffer;

	transform.origin += _offset;
	Transform inv_transform = transform.affine_inverse();

	for (int z = 0; z < buffer.get_size().z; ++z) {
		for (int x = 0; x < buffer.get_size().x; ++x) {
			for (int y = 0; y < buffer.get_size().y; ++y) {

				Vector3 pos = inv_transform.xform(Vector3(x, y, z));
				do_op(buffer, x, y, z, sdf_cube(pos, extents), op);
			}
		}
	}
}

void VoxelIsoSurfaceTool::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_buffer", "voxel_buffer"), &VoxelIsoSurfaceTool::set_buffer);
	ClassDB::bind_method(D_METHOD("get_buffer"), &VoxelIsoSurfaceTool::get_buffer);

	ClassDB::bind_method(D_METHOD("set_iso_scale", "iso_scale"), &VoxelIsoSurfaceTool::set_iso_scale);
	ClassDB::bind_method(D_METHOD("get_iso_scale"), &VoxelIsoSurfaceTool::get_iso_scale);

	ClassDB::bind_method(D_METHOD("set_offset", "offset"), &VoxelIsoSurfaceTool::set_offset);
	ClassDB::bind_method(D_METHOD("get_offset"), &VoxelIsoSurfaceTool::get_offset);

	ClassDB::bind_method(D_METHOD("do_sphere", "center", "radius", "op"), &VoxelIsoSurfaceTool::do_sphere);
	ClassDB::bind_method(D_METHOD("do_plane", "plane", "op"), &VoxelIsoSurfaceTool::do_plane);
	ClassDB::bind_method(D_METHOD("do_cube", "transform", "extents", "op"), &VoxelIsoSurfaceTool::do_cube);

	BIND_ENUM_CONSTANT(OP_ADD);
	BIND_ENUM_CONSTANT(OP_SUBTRACT);
	BIND_ENUM_CONSTANT(OP_SET);
}