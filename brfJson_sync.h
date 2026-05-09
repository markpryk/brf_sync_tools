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
/* brfJson_sync.h — Qt-free JSON serialization for brf_sync standalone.
 * Uses nlohmann/json instead of Qt JSON classes. */

#ifndef BRFJSON_SYNC_H
#define BRFJSON_SYNC_H

#include "brfData.h"
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class BrfJsonSync {
public:
    BrfJsonSync();

    // Main Entry Points (identical API to BrfJson)
    bool exportToFolder(const BrfData& data, const std::string& folderPath);
    bool importFromFolder(const std::string& folderPath, BrfData& data);

    // New: Metadata-only export (no OBJ/SMD files written)
    bool exportMetadataOnly(const BrfData& data, const std::string& folderPath);

private:
    // Serializers
    json serializeMesh(const BrfMesh& mesh, const std::string& outputDir, bool writeObj);
    json serializeMaterial(const BrfMaterial& mat);
    json serializeTexture(const BrfTexture& tex);
    json serializeShader(const BrfShader& shader);
    json serializeSkeleton(const BrfSkeleton& skel, const std::string& outputDir, bool writeSmd);
    json serializeAnimation(const BrfAnimation& anim, const BrfData& context,
                           const std::string& outputDir, bool writeSmd);
    json serializeBody(const BrfBody& body, const std::string& outputDir, bool writeObj);

    // Deserializers
    BrfMesh deserializeMesh(const json& j, const std::string& inputDir);
    BrfMaterial deserializeMaterial(const json& j);
    BrfTexture deserializeTexture(const json& j);
    BrfShader deserializeShader(const json& j);
    BrfSkeleton deserializeSkeleton(const json& j, const std::string& inputDir);
    BrfAnimation deserializeAnimation(const json& j, const std::string& inputDir,
                                      const BrfData& context);
    BrfBody deserializeBody(const json& j, const std::string& inputDir);
};

#endif // BRFJSON_SYNC_H

