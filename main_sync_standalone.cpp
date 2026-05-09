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
/* main_sync_standalone.cpp — Qt-free CLI entry point for brf_sync.
 * Replaces main_sync.cpp which depends on QCoreApplication/QCommandLineParser.
 * Zero external dependencies beyond the C++ standard library. */

#include <iostream>
#include <string>
#include <cstring>
#include <filesystem>

#include "brfData.h"
#include "brfJson_sync.h"

namespace fs = std::filesystem;

// Stub for tuneColor (used in brfMesh.cpp)
unsigned int tuneColor(unsigned int col, int c, int h, int s, int b) {
    return col;
}

static void printUsage(const char* progName) {
    std::cout << "BRF Synchronizer v2.0 (standalone)\n"
              << "Usage:\n"
              << "  " << progName << " export   <source.brf> <dest_folder>    Export BRF to folder (JSON + OBJ/SMD)\n"
              << "  " << progName << " import   <source_folder> <dest.brf>    Import folder back to BRF\n"
              << "  " << progName << " info     <source.brf> <dest_folder>    Export metadata only (no OBJ/SMD)\n"
              << "  " << progName << " --help                                 Show this help\n"
              << "  " << progName << " --version                              Show version\n"
              << std::endl;
}

static int doExport(const std::string& source, const std::string& destination) {
    std::cout << "Exporting " << source << " to " << destination << std::endl;

    BrfData data;
    if (!data.Load(source.c_str())) {
        std::cerr << "Failed to load BRF file: " << source << std::endl;
        return 1;
    }

    fs::create_directories(destination);

    BrfJsonSync converter;
    if (!converter.exportToFolder(data, destination)) {
        std::cerr << "Export failed." << std::endl;
        return 1;
    }

    std::cout << "Export completed successfully." << std::endl;
    return 0;
}

static int doImport(const std::string& source, const std::string& destination) {
    std::cout << "Importing from " << source << " to " << destination << std::endl;

    BrfJsonSync converter;
    BrfData data;

    if (!converter.importFromFolder(source, data)) {
        std::cerr << "Import failed." << std::endl;
        return 1;
    }

    if (!data.Save(destination.c_str())) {
        std::cerr << "Failed to save BRF file: " << destination << std::endl;
        return 1;
    }

    std::cout << "Import completed successfully." << std::endl;
    return 0;
}

static int doInfo(const std::string& source, const std::string& destination) {
    std::cout << "Generating metadata for " << source << std::endl;

    BrfData data;
    if (!data.Load(source.c_str())) {
        std::cerr << "Failed to load BRF file: " << source << std::endl;
        return 1;
    }

    fs::create_directories(destination);

    BrfJsonSync converter;
    if (!converter.exportMetadataOnly(data, destination)) {
        std::cerr << "Metadata export failed." << std::endl;
        return 1;
    }

    std::cout << "Info export completed. "
              << data.mesh.size() << " meshes, "
              << data.material.size() << " materials, "
              << data.texture.size() << " textures, "
              << data.body.size() << " bodies."
              << std::endl;
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string command = argv[1];

    if (command == "--help" || command == "-h") {
        printUsage(argv[0]);
        return 0;
    }

    if (command == "--version" || command == "-v") {
        std::cout << "BRF Synchronizer v2.0 (standalone, Qt-free)" << std::endl;
        return 0;
    }

    if (argc < 4) {
        std::cerr << "Error: command '" << command << "' requires <source> and <destination> arguments." << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    std::string source = argv[2];
    std::string destination = argv[3];

    if (command == "export") {
        return doExport(source, destination);
    } else if (command == "import") {
        return doImport(source, destination);
    } else if (command == "info") {
        return doInfo(source, destination);
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
        printUsage(argv[0]);
        return 1;
    }
}

