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
/* ioOBJ_sync.cpp — Qt-free OBJ import/export for brf_sync standalone build.
 * Replaces ioOBJ.cpp which uses QFile, QString, QPair, QMap.
 * Uses only C stdlib (FILE*, snprintf) and STL (std::map, std::pair, std::string). */

#include <vector>
#include <map>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <utility>

#include <vcg/space/box3.h>
#include <vcg/space/point3.h>
#include <vcg/space/point2.h>

using namespace std;
using namespace vcg;

#include "brfMesh.h"
#include "brfBody.h"
#include "brfSkeleton.h"

// defined in vcgmesh.cpp
void setGotNormals(bool b);
void setGotMaterialName(bool b);
void setGotTexture(bool b);
void setGotColor(bool b);

// ============================================================
// OBJ EXPORT
// ============================================================

static int cv_sync = 0, cp_sync = 0;

bool BrfMesh::SaveOBJ(char* fn, int nframe) const {
    FILE* f = fopen(fn, "w");
    if (!f) return false;

    cv_sync = 0;
    cp_sync = 0;

    fprintf(f, "#\n# Exported by OpenBRF -- marco tarini\n#\n");
    fprintf(f, "s %s\n", name);
    fprintf(f, "newmtl %s\nKa 0.8 0.8 0.8\nKd 0.2 0.2 0.2\nusemtl %s\n", material, material);

    // Build unique position+color map
    typedef std::pair<int, unsigned int> PosColPair;
    std::map<PosColPair, uint32_t> unique_pos;

    for (unsigned int i = 0; i < vert.size(); i++) {
        PosColPair key = {vert[i].index, vert[i].col};
        if (unique_pos.find(key) == unique_pos.end())
            unique_pos[key] = 0;
    }

    int np = 0;
    uint32_t index = 0;
    for (auto& kv : unique_pos) {
        int cur_pos_idx = kv.first.first;
        unsigned int cur_col = kv.first.second;
        kv.second = index++;

        fprintf(f, "v %f %f %f",
            -frame[nframe].pos[cur_pos_idx].X(),
             frame[nframe].pos[cur_pos_idx].Y(),
             frame[nframe].pos[cur_pos_idx].Z());

        if (hasVertexColor) {
            fprintf(f, "  %f %f %f  %f",
                ((cur_col >> (8*2)) & 0xFF) / 255.f,
                ((cur_col >> (8*1)) & 0xFF) / 255.f,
                ((cur_col >> (8*0)) & 0xFF) / 255.f,
                ((cur_col >> (8*3)) & 0xFF) / 255.f);
        }

        fprintf(f, "\n");
        np++;
    }

    int nv = 0;
    for (unsigned int i = 0; i < vert.size(); i++) {
        fprintf(f, "vn %f %f %f\n",
            -frame[nframe].norm[i].X(),
             frame[nframe].norm[i].Y(),
             frame[nframe].norm[i].Z());
        fprintf(f, "vt %f %f\n",
             vert[i].ta.X(),
             1.0f - vert[i].ta.Y());
        nv++;
    }

    for (unsigned int i = 0; i < face.size(); i++) {
        fprintf(f, "f");
        for (int w = 0; w < 3; w++) {
            PosColPair key = {vert[face[i].index[w]].index, vert[face[i].index[w]].col};
            auto uniquePosIdx = unique_pos[key];
            fprintf(f, " %d/%d/%d",
                uniquePosIdx + 1 + cp_sync,
                face[i].index[w] + 1 + cv_sync,
                face[i].index[w] + 1 + cv_sync);
        }
        fprintf(f, "\n");
    }

    cp_sync += np;
    cv_sync += nv;

    fclose(f);
    return true;
}

// ============================================================
// OBJ IMPORT
// ============================================================

class Triple_sync {
public:
    int a, b, c;
    Triple_sync(int a0, int b0, int c0) { a = a0; b = b0; c = c0; }
    bool operator<(const Triple_sync& t) const {
        if (a < t.a) return true;
        if (a > t.a) return false;
        if (b < t.b) return true;
        if (b > t.b) return false;
        return (c < t.c);
    }
};

bool BrfMesh::LoadOBJ(char* fn) {
    name[0] = 0;
    frame.resize(1);
    FILE* f = fopen(fn, "r");
    flags = 0;
    material[0] = 0;
    if (!f) return false;

    std::vector<Point3f> norm;
    std::vector<Point2f> ta;
    std::map<Triple_sync, int> map;
    std::vector<uint32_t> posColor;

    setGotColor(false);
    setGotNormals(false);
    setGotMaterialName(false);
    setGotTexture(false);

    bool norms = false;

    char st[512];
    while (fgets(st, 511, f) != NULL) {
        // Trim trailing whitespace
        int len = (int)strlen(st);
        while (len > 0 && (st[len-1] == '\n' || st[len-1] == '\r' || st[len-1] == ' '))
            st[--len] = 0;

        if (strncmp(st, "usemtl ", 7) == 0) {
            sscanf(st, "usemtl %s", material);
            setGotMaterialName(true);
        }
        else if (strncmp(st, "v ", 2) == 0) {
            Point3f p;
            float r, g, b_col, a_col = 1.f;
            int found = sscanf(st, "v %f %f %f %f %f %f %f",
                &(p[0]), &(p[1]), &(p[2]), &r, &g, &b_col, &a_col);
            p[0] = -p[0];
            frame[0].pos.push_back(p);

            uint32_t cur_col = 0xFFFFFFFF;
            if (found >= 6) {
                cur_col = (((uint32_t)(b_col * 255u) & 0xFF) << (8*0)) |
                          (((uint32_t)(g     * 255u) & 0xFF) << (8*1)) |
                          (((uint32_t)(r     * 255u) & 0xFF) << (8*2)) |
                          (((uint32_t)(a_col * 255u) & 0xFF) << (8*3));
                setGotColor(true);
            }
            posColor.push_back(cur_col);
        }
        else if (strncmp(st, "vn ", 3) == 0) {
            Point3f p;
            sscanf(st, "vn %f %f %f", &(p[0]), &(p[1]), &(p[2]));
            p[0] = -p[0];
            norm.push_back(p);
        }
        else if (strncmp(st, "vt ", 3) == 0) {
            Point2f p;
            sscanf(st, "vt %f %f", &(p[0]), &(p[1]));
            p[1] = 1 - p[1];
            ta.push_back(p);
        }
        else if (strncmp(st, "s ", 2) == 0) {
            // smoothing group / mesh name — ignored in sync
        }
        else if (strncmp(st, "f ", 2) == 0) {
            char parts[6][100];
            int res = sscanf(st, "f %s %s %s %s %s %s",
                parts[0], parts[1], parts[2], parts[3], parts[4], parts[5]);
            int remap[6];
            for (int w = 0; w < res; w++) {
                int n = (int)strlen(parts[w]);
                int rr[3] = {0, 0, 0};
                int rri = 0;
                for (int i = 0; i < n; i++) {
                    char c = parts[w][i];
                    if (c == '/') { if (rri < 2) rri++; }
                    else rr[rri] = rr[rri] * 10 + (c - '0');
                }
                Triple_sync t(rr[0]-1, rr[1]-1, rr[2]-1);

                if (map.find(t) == map.end()) {
                    BrfVert v;
                    v.index = t.a;
                    if (t.b >= 0 && t.b < (int)ta.size()) {
                        v.ta = v.tb = ta[t.b];
                        setGotTexture(true);
                    }
                    if (t.c >= 0 && t.c < (int)norm.size()) {
                        v.__norm = norm[t.c];
                        norms = true;
                        setGotNormals(true);
                    }
                    if (t.a >= 0 && t.a < (int)posColor.size())
                        v.col = posColor[t.a];
                    else
                        v.col = 0xFFFFFFFF;
                    vert.push_back(v);
                    remap[w] = map[t] = (int)vert.size() - 1;
                } else {
                    remap[w] = map[t];
                }
            }
            for (int w = 2; w < res; w++) {
                BrfFace bf;
                bf.index[0] = remap[0];
                bf.index[1] = remap[w-1];
                bf.index[2] = remap[w];
                face.push_back(bf);
            }
        }
    }

    frame[0].norm.resize(vert.size());
    for (unsigned int i = 0; i < vert.size(); i++)
        frame[0].norm[i] = vert[i].__norm;

    fclose(f);

    if (!norms) {
        UnifyVert(false);
        ComputeNormals();
    }

    AfterLoad();
    return true;
}

