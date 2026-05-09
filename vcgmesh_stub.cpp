/*
 * Copyright (C) 2026 markpryk
 *
 * This file is part of brf_sync, a standalone version of OpenBRF/openbrf-redux.
 *
 * brf_sync is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * brf_sync is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/* vcgmesh_stub.cpp — Minimal stubs for vcgmesh functions needed by the sync tool.
 * Provides setGotNormals/etc and BrfBodyPart::MakeQuadDominant stub. */

#include <vector>
#include <vcg/space/box3.h>
#include <vcg/space/point3.h>
#include <vcg/space/point2.h>
using namespace vcg;
using namespace std;

#include "brfBody.h"
#include "brfMesh.h"
#include "vcgmesh.h"

// These are called by ioOBJ_sync.cpp during LoadOBJ
static int lastMask_stub = 0;
static bool lastMaterial_stub = false;

void setGotColor(bool b)     { (void)b; }
void setGotTexture(bool b)   { (void)b; }
void setGotMaterialName(bool b) { lastMaterial_stub = b; }
void setGotNormals(bool b)   { (void)b; }

// BrfBodyPart::MakeQuadDominant needs the full VCG mesh infrastructure.
// In the sync tool we never call it, but the linker needs a symbol.
void BrfBodyPart::MakeQuadDominant() {}

// VcgMesh stubs — the sync tool doesn't use PLY/STL/DAE import/export
VcgMesh::VcgMesh() {}

bool VcgMesh::load(char* /*filename*/) { return false; }
bool VcgMesh::save(char* /*filename*/) { return false; }
int VcgMesh::simplify(int /*percFaces*/) { return 0; }
void VcgMesh::add(const BrfSkeleton& /*s*/) {}
void VcgMesh::add(const BrfMesh& /*b*/, int /*fi*/) {}
BrfMesh VcgMesh::toBrfMesh() { return BrfMesh(); }
bool VcgMesh::modifyBrfSkeleton(BrfSkeleton& /*s*/) { return false; }
void VcgMesh::moveBoneInSkelMesh(int /*nb*/, vcg::Point3f /*d*/) {}
bool VcgMesh::gotColor() { return false; }
bool VcgMesh::gotTexture() { return false; }
bool VcgMesh::gotNormals() { return false; }
bool VcgMesh::gotMaterialName() { return lastMaterial_stub; }
const char* VcgMesh::lastErrString() { return ""; }

