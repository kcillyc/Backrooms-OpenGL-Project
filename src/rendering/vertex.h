#pragma once

// Simple 3D point/vector
struct P3 {
    float x;
    float y;
    float z;
};

// Vertex used for both chair and floor: position + texture coords
struct Vtx {
    float x;
    float y;
    float z;
    float u;
    float v;
};
