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
/* brfJson_sync.cpp — Qt-free JSON serialization for brf_sync standalone.
 * Produces identical JSON output to brfJson.cpp (Qt version).
 * Uses nlohmann/json + std::filesystem + FILE*. */

#include "brfJson_sync.h"
#include "ioSMD.h"
#include <fstream>
#include <filesystem>
#include <cstdio>
#include <cstring>
#include <iostream>

namespace fs = std::filesystem;

BrfJsonSync::BrfJsonSync() {}

// ============================================================
// EXPORT (Full — with OBJ/SMD file output)
// ============================================================

bool BrfJsonSync::exportToFolder(const BrfData& data, const std::string& folderPath) {
    json root;
    json meshes = json::array();
    json materials = json::array();
    json textures = json::array();
    json shaders = json::array();
    json skeletons = json::array();
    json animations = json::array();
    json bodies = json::array();

    for (const auto& mesh : data.mesh)
        meshes.push_back(serializeMesh(mesh, folderPath, true));

    for (const auto& mat : data.material)
        materials.push_back(serializeMaterial(mat));

    for (const auto& tex : data.texture)
        textures.push_back(serializeTexture(tex));

    for (const auto& sh : data.shader)
        shaders.push_back(serializeShader(sh));

    for (const auto& skel : data.skeleton)
        skeletons.push_back(serializeSkeleton(skel, folderPath, true));

    for (const auto& anim : data.animation)
        animations.push_back(serializeAnimation(anim, data, folderPath, true));

    for (const auto& body : data.body)
        bodies.push_back(serializeBody(body, folderPath, true));

    root["meshes"] = meshes;
    root["materials"] = materials;
    root["textures"] = textures;
    root["shaders"] = shaders;
    root["skeletons"] = skeletons;
    root["animations"] = animations;
    root["bodies"] = bodies;
    root["version"] = "1.0";

    // Write data.json
    std::string jsonPath = folderPath + "/data.json";
    std::ofstream ofs(jsonPath);
    if (!ofs.is_open()) {
        std::cerr << "Could not write data.json" << std::endl;
        return false;
    }
    ofs << root.dump(4);  // Pretty-print with 4-space indent (matches Qt default)
    ofs.close();

    return true;
}

// ============================================================
// EXPORT (Metadata only — no OBJ/SMD files)
// ============================================================

bool BrfJsonSync::exportMetadataOnly(const BrfData& data, const std::string& folderPath) {
    json root;
    json meshes = json::array();
    json materials = json::array();
    json textures = json::array();
    json shaders = json::array();
    json skeletons = json::array();
    json animations = json::array();
    json bodies = json::array();

    for (const auto& mesh : data.mesh)
        meshes.push_back(serializeMesh(mesh, folderPath, false));

    for (const auto& mat : data.material)
        materials.push_back(serializeMaterial(mat));

    for (const auto& tex : data.texture)
        textures.push_back(serializeTexture(tex));

    for (const auto& sh : data.shader)
        shaders.push_back(serializeShader(sh));

    for (const auto& skel : data.skeleton)
        skeletons.push_back(serializeSkeleton(skel, folderPath, false));

    for (const auto& anim : data.animation)
        animations.push_back(serializeAnimation(anim, data, folderPath, false));

    for (const auto& body : data.body)
        bodies.push_back(serializeBody(body, folderPath, false));

    root["meshes"] = meshes;
    root["materials"] = materials;
    root["textures"] = textures;
    root["shaders"] = shaders;
    root["skeletons"] = skeletons;
    root["animations"] = animations;
    root["bodies"] = bodies;
    root["version"] = "1.0";

    fs::create_directories(folderPath);

    std::string jsonPath = folderPath + "/data.json";
    std::ofstream ofs(jsonPath);
    if (!ofs.is_open()) {
        std::cerr << "Could not write data.json" << std::endl;
        return false;
    }
    ofs << root.dump(4);
    ofs.close();

    return true;
}

// ============================================================
// IMPORT
// ============================================================

bool BrfJsonSync::importFromFolder(const std::string& folderPath, BrfData& data) {
    std::string jsonPath = folderPath + "/data.json";
    std::ifstream ifs(jsonPath);
    if (!ifs.is_open()) {
        std::cerr << "Could not read data.json" << std::endl;
        return false;
    }

    json root;
    try {
        root = json::parse(ifs);
    } catch (const json::parse_error& e) {
        std::cerr << "Invalid JSON in data.json: " << e.what() << std::endl;
        return false;
    }

    // Import Textures
    if (root.contains("textures")) {
        for (const auto& val : root["textures"])
            data.texture.push_back(deserializeTexture(val));
    }

    // Import Shaders
    if (root.contains("shaders")) {
        for (const auto& val : root["shaders"])
            data.shader.push_back(deserializeShader(val));
    }

    // Import Materials
    if (root.contains("materials")) {
        for (const auto& val : root["materials"])
            data.material.push_back(deserializeMaterial(val));
    }

    // Import Meshes
    if (root.contains("meshes")) {
        for (const auto& val : root["meshes"])
            data.mesh.push_back(deserializeMesh(val, folderPath));
    }

    // Import Skeletons
    if (root.contains("skeletons")) {
        for (const auto& val : root["skeletons"])
            data.skeleton.push_back(deserializeSkeleton(val, folderPath));
    }

    // Import Animations
    if (root.contains("animations")) {
        for (const auto& val : root["animations"])
            data.animation.push_back(deserializeAnimation(val, folderPath, data));
    }

    // Import Bodies
    if (root.contains("bodies")) {
        for (const auto& val : root["bodies"])
            data.body.push_back(deserializeBody(val, folderPath));
    }

    return true;
}

// ============================================================
// SERIALIZERS — produce identical JSON keys to Qt version
// ============================================================

json BrfJsonSync::serializeMesh(const BrfMesh& mesh, const std::string& outputDir, bool writeObj) {
    json obj;
    obj["name"] = std::string(mesh.name);
    obj["material"] = std::string(mesh.material);
    obj["flags"] = (int64_t)mesh.flags;
    obj["lod_level"] = mesh.lodLevel;

    std::string filename = std::string(mesh.name) + ".obj";
    std::string relPath = "Meshes/" + filename;
    obj["source"] = relPath;

    if (writeObj) {
        std::string fullPath = outputDir + "/" + relPath;
        fs::create_directories(fs::path(fullPath).parent_path());

        // SaveOBJ expects char* (non-const in legacy API)
        std::string pathStr = fullPath;
        char* pathBuf = const_cast<char*>(pathStr.c_str());
        if (!mesh.SaveOBJ(pathBuf, 0)) {
            std::cerr << "Failed to export mesh: " << mesh.name << std::endl;
        }
    }

    return obj;
}

json BrfJsonSync::serializeMaterial(const BrfMaterial& mat) {
    json obj;
    obj["name"] = std::string(mat.name);
    obj["flags"] = (int64_t)mat.flags;
    obj["shader"] = std::string(mat.shader);
    obj["diffuseA"] = std::string(mat.diffuseA);
    obj["diffuseB"] = std::string(mat.diffuseB);
    obj["bump"] = std::string(mat.bump);
    obj["enviro"] = std::string(mat.enviro);
    obj["spec"] = std::string(mat.spec);
    obj["specular_value"] = mat.specular;

    json color = json::array();
    color.push_back(mat.r);
    color.push_back(mat.g);
    color.push_back(mat.b);
    obj["color_rgb"] = color;

    return obj;
}

json BrfJsonSync::serializeTexture(const BrfTexture& tex) {
    json obj;
    obj["name"] = std::string(tex.name);
    obj["flags"] = (int64_t)tex.flags;
    return obj;
}

json BrfJsonSync::serializeShader(const BrfShader& val) {
    json obj;
    obj["name"] = std::string(val.name);
    obj["technique"] = std::string(val.technique);
    obj["fallback"] = std::string(val.fallback);
    obj["flags"] = (int64_t)val.flags;
    obj["requirements"] = (int64_t)val.requirements;

    json opts = json::array();
    for (const auto& opt : val.opt) {
        json o;
        o["map"] = opt.map;
        o["colorOp"] = (int64_t)opt.colorOp;
        o["alphaOp"] = (int64_t)opt.alphaOp;
        o["flags"] = (int64_t)opt.flags;
        opts.push_back(o);
    }
    obj["options"] = opts;

    return obj;
}

json BrfJsonSync::serializeSkeleton(const BrfSkeleton& skel,
                                    const std::string& outputDir, bool writeSmd) {
    json obj;
    obj["name"] = std::string(skel.name);
    obj["flags"] = (int64_t)skel.flags;

    std::string filename = std::string(skel.name) + ".smd";
    std::string relPath = "Skeletons/" + filename;
    obj["source"] = relPath;

    if (writeSmd) {
        std::string fullPath = outputDir + "/" + relPath;
        fs::create_directories(fs::path(fullPath).parent_path());

        BrfMesh dummyMesh;
        skel.BuildDefaultMesh(dummyMesh);

        if (ioSMD::Export(fullPath.c_str(), dummyMesh, skel, 0) != 0) {
            std::cerr << "Failed to export skeleton: " << skel.name << std::endl;
        }
    }

    return obj;
}

json BrfJsonSync::serializeAnimation(const BrfAnimation& anim,
                                     const BrfData& context,
                                     const std::string& outputDir, bool writeSmd) {
    json obj;
    obj["name"] = std::string(anim.name);

    std::string filename = std::string(anim.name) + ".smd";
    std::string relPath = "Animations/" + filename;
    obj["source"] = relPath;

    if (writeSmd) {
        std::string fullPath = outputDir + "/" + relPath;
        fs::create_directories(fs::path(fullPath).parent_path());

        if (context.skeleton.size() > 0) {
            const BrfSkeleton& skel = context.skeleton[0];
            if (ioSMD::Export(fullPath.c_str(), anim, skel) != 0) {
                std::cerr << "Failed to export animation: " << anim.name << std::endl;
            }
        } else {
            std::cerr << "Cannot export animation " << anim.name
                      << " without a skeleton." << std::endl;
        }
    }

    return obj;
}

json BrfJsonSync::serializeBody(const BrfBody& body, const std::string& outputDir,
                                bool writeObj) {
    json obj;
    obj["name"] = std::string(body.name);

    // Check if any part is a manifold (mesh collider)
    bool hasManifold = false;
    for (const auto& part : body.part) {
        if (part.type == BrfBodyPart::MANIFOLD) {
            hasManifold = true;
            break;
        }
    }

    // Only export OBJ source if we have manifold geometry
    if (hasManifold) {
        std::string filename = std::string(body.name) + ".obj";
        std::string relPath = "Collisions/" + filename;
        obj["source"] = relPath;

        if (writeObj) {
            std::string fullPath = outputDir + "/" + relPath;
            fs::create_directories(fs::path(fullPath).parent_path());

            if (!body.ExportOBJ(fullPath.c_str())) {
                std::cerr << "Failed to export body: " << body.name << std::endl;
            }
        }
    }

    // Export Primitives Data
    json primitives = json::array();
    for (const auto& part : body.part) {
        json prim;
        prim["flags"] = (int64_t)part.flags;

        if (part.type == BrfBodyPart::SPHERE) {
            prim["type"] = "sphere";
            prim["radius"] = part.radius;
            json center;
            center["x"] = part.center[0];
            center["y"] = part.center[1];
            center["z"] = part.center[2];
            prim["center"] = center;
            primitives.push_back(prim);

        } else if (part.type == BrfBodyPart::CAPSULE) {
            prim["type"] = "capsule";
            prim["radius"] = part.radius;
            json p1;
            p1["x"] = part.center[0];
            p1["y"] = part.center[1];
            p1["z"] = part.center[2];
            prim["p1"] = p1;

            json p2;
            p2["x"] = part.dir[0];
            p2["y"] = part.dir[1];
            p2["z"] = part.dir[2];
            prim["p2"] = p2;
            primitives.push_back(prim);

        } else if (part.type == BrfBodyPart::MANIFOLD) {
            prim["type"] = "manifold";
            prim["orientation"] = part.ori;

            json vertices = json::array();
            for (const auto& v : part.pos) {
                json vert;
                vert["x"] = v[0];
                vert["y"] = v[1];
                vert["z"] = v[2];
                vertices.push_back(vert);
            }
            prim["vertices"] = vertices;

            json faces = json::array();
            for (const auto& f : part.face) {
                json faceIndices = json::array();
                for (int idx : f) {
                    faceIndices.push_back(idx);
                }
                faces.push_back(faceIndices);
            }
            prim["faces"] = faces;
            primitives.push_back(prim);

        } else if (part.type == BrfBodyPart::FACE) {
            prim["type"] = "face";

            json vertices = json::array();
            for (const auto& v : part.pos) {
                json vert;
                vert["x"] = v[0];
                vert["y"] = v[1];
                vert["z"] = v[2];
                vertices.push_back(vert);
            }
            prim["vertices"] = vertices;
            primitives.push_back(prim);
        }
    }
    if (!primitives.empty()) {
        obj["primitives"] = primitives;
    }

    return obj;
}

// ============================================================
// DESERIALIZERS
// ============================================================

static void safeCopy(char* dest, const std::string& src, size_t maxLen = 255) {
    if (src.length() < maxLen)
        strcpy(dest, src.c_str());
}

BrfMesh BrfJsonSync::deserializeMesh(const json& j, const std::string& inputDir) {
    BrfMesh m;
    std::string source = j.value("source", "");
    if (!source.empty()) {
        std::string fullPath = inputDir + "/" + source;
        char* pathBuf = const_cast<char*>(fullPath.c_str());
        if (!m.LoadOBJ(pathBuf)) {
            std::cerr << "Failed to import mesh OBJ: " << source << std::endl;
        }
    }

    // LoadOBJ clears the name, so set it AFTER loading
    safeCopy(m.name, j.value("name", ""));
    safeCopy(m.material, j.value("material", ""));
    m.flags = (unsigned int)j.value("flags", (int64_t)0);

    return m;
}

BrfMaterial BrfJsonSync::deserializeMaterial(const json& j) {
    BrfMaterial m;
    safeCopy(m.name, j.value("name", ""));
    m.flags = (unsigned int)j.value("flags", (int64_t)0);
    safeCopy(m.shader, j.value("shader", ""));
    safeCopy(m.diffuseA, j.value("diffuseA", ""));
    safeCopy(m.diffuseB, j.value("diffuseB", ""));
    safeCopy(m.bump, j.value("bump", ""));
    safeCopy(m.enviro, j.value("enviro", ""));
    safeCopy(m.spec, j.value("spec", ""));

    m.specular = j.value("specular_value", 0.0);

    if (j.contains("color_rgb") && j["color_rgb"].is_array() && j["color_rgb"].size() >= 3) {
        m.r = j["color_rgb"][0].get<double>();
        m.g = j["color_rgb"][1].get<double>();
        m.b = j["color_rgb"][2].get<double>();
    }

    return m;
}

BrfTexture BrfJsonSync::deserializeTexture(const json& j) {
    BrfTexture t;
    safeCopy(t.name, j.value("name", ""));
    t.flags = (unsigned int)j.value("flags", (int64_t)0);
    return t;
}

BrfShader BrfJsonSync::deserializeShader(const json& j) {
    BrfShader s;
    safeCopy(s.name, j.value("name", ""));
    safeCopy(s.technique, j.value("technique", ""));
    safeCopy(s.fallback, j.value("fallback", ""));
    s.flags = (unsigned int)j.value("flags", (int64_t)0);
    s.requirements = (unsigned int)j.value("requirements", (int64_t)0);

    if (j.contains("options")) {
        for (const auto& val : j["options"]) {
            BrfShaderOpt opt;
            opt.map = val.value("map", 0);
            opt.colorOp = (unsigned int)val.value("colorOp", (int64_t)0);
            opt.alphaOp = (unsigned int)val.value("alphaOp", (int64_t)0);
            opt.flags = (unsigned int)val.value("flags", (int64_t)0);
            s.opt.push_back(opt);
        }
    }

    return s;
}

BrfSkeleton BrfJsonSync::deserializeSkeleton(const json& j, const std::string& inputDir) {
    BrfSkeleton s;

    std::string source = j.value("source", "");
    if (!source.empty()) {
        std::string fullPath = inputDir + "/" + source;
        BrfMesh dummy;
        if (ioSMD::Import(fullPath.c_str(), dummy, s) != 0) {
            std::cerr << "Failed to import skeleton SMD: " << source << std::endl;
        }
    }

    safeCopy(s.name, j.value("name", ""));
    s.flags = (unsigned int)j.value("flags", (int64_t)0);

    return s;
}

BrfAnimation BrfJsonSync::deserializeAnimation(const json& j, const std::string& inputDir,
                                                const BrfData& context) {
    BrfAnimation a;

    std::string source = j.value("source", "");
    if (!source.empty()) {
        std::string fullPath = inputDir + "/" + source;
        if (context.skeleton.size() > 0) {
            BrfSkeleton skelCopy = context.skeleton[0];
            if (ioSMD::Import(fullPath.c_str(), a, skelCopy) != 0) {
                std::cerr << "Failed to import animation SMD: " << source << std::endl;
            }
        } else {
            std::cerr << "Cannot import animation " << j.value("name", "?")
                      << " because no skeleton is present." << std::endl;
        }
    }

    safeCopy(a.name, j.value("name", ""));
    return a;
}

BrfBody BrfJsonSync::deserializeBody(const json& j, const std::string& inputDir) {
    BrfBody b;

    std::string source = j.value("source", "");
    if (!source.empty()) {
        std::string fullPath = inputDir + "/" + source;
        if (!b.ImportOBJ(fullPath.c_str())) {
            std::cerr << "Failed to import body OBJ: " << source << std::endl;
        }
    }

    safeCopy(b.name, j.value("name", ""));
    return b;
}

