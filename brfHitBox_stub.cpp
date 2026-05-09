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
/* brfHitBox_stub.cpp — Stub for hitbox XML functions when building without Qt.
 * The sync tool doesn't need skeleton_data.xml parsing. */

#include <vector>
#include <vcg/space/box3.h>
#include <vcg/space/point3.h>
#include <vcg/space/point2.h>
using namespace vcg;

#include "brfBody.h"
#include "brfData.h"

int BrfData::LoadHitBoxesFromXml(const char* /*filename*/) {
    return -1; // Not supported in standalone build
}

int BrfData::SaveHitBoxesToXml(const char* /*fin*/, const char* /*fout*/) {
    return -1; // Not supported in standalone build
}

char* BrfData::LastHitBoxesLoadSaveError(const char* st, const char* subst1,
                                          const char* subst2, const char* subst3) {
    (void)st; (void)subst1; (void)subst2; (void)subst3;
    return nullptr;
}

