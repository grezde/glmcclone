#include "game.hpp"
#include "renderer.hpp"
#include <cstring>
#include <glm/ext/vector_int3.hpp>
#include <stb_image.h>
#include <stb_image_write.h>

Game* Game::instance = nullptr;

void Game::processInput(f32 dt) {
    static constexpr float cameraAngleSpeed = 2.0f;
    static constexpr float cameraSpeed = 8.0f;
    static bool prevEscape = false;
    static glm::vec2 lastMousePos = glm::vec2(0, 0);
    static bool lastMouseFirst = true;

    glm::vec3 movement = {0, 0, 0};
    if(window::isPressed(GLFW_KEY_W))
        movement += glm::vec3(0, 0, 1);
    if(window::isPressed(GLFW_KEY_A))
        movement += glm::vec3(-1, 0, 0);
    if(window::isPressed(GLFW_KEY_S))
        movement += glm::vec3(0, 0, -1);
    if(window::isPressed(GLFW_KEY_D))
        movement += glm::vec3(1, 0, 0);
    if(window::isPressed(GLFW_KEY_UP))
        movement += glm::vec3(0, 1, 0);
    if(window::isPressed(GLFW_KEY_DOWN))
        movement += glm::vec3(0, -1, 0);
    bool escape = window::isPressed(GLFW_KEY_ESCAPE);
    if(escape && !prevEscape)
        window::toggleCursor();
    prevEscape = escape;

    glm::vec2& cameraAngle = Game::instance->renderer.cameraAngle;
    glm::vec3& cameraPos = Game::instance->renderer.cameraPos;

    glm::vec3 movementReal = movement.z * glm::vec3(std::cos(cameraAngle.x), 0, std::sin(cameraAngle.x))
            + movement.y * glm::vec3(0, 1, 0)
            + movement.x * glm::vec3(-std::sin(cameraAngle.x), 0, std::cos(cameraAngle.x));
    movementReal *= dt * cameraSpeed;
    cameraPos += movementReal;

    if(window::disabledCursor) {
        glm::vec2 mc = window::mousePos - lastMousePos;
        if(lastMouseFirst && (mc.x != 0.0f || mc.y != 0.0f)) {
            lastMouseFirst = false;
            return;
        }
        mc.x /= window::width;
        mc.y /= window::height;
        cameraAngle += glm::vec2(mc.x, mc.y) * cameraAngleSpeed;
        constexpr float PI = 3.14159268f;
        if(cameraAngle.y > 0.5f*PI*0.99f)
            cameraAngle.y = 0.5f*PI*0.99f;
        if(cameraAngle.y < -0.5f*PI*0.99f)
            cameraAngle.y = -0.5f*PI*0.99f;
        if(cameraAngle.x > PI)
            cameraAngle.x -= 2.0*PI;
        if(cameraAngle.x < -PI)
            cameraAngle.x += 2.0*PI;
    }
    lastMousePos = window::mousePos;
}

vector<BlockRenderProps> Chunk::props = {
    // 0 is the missing texture
    // air
    { 0, 0, 0, 0, 0, 0, 0 },
    // cobblestone 
    { 1, 1, 1, 1, 1, 1, 127 },
    // dirt
    { 2, 2, 2, 2, 2, 2, 127 },
    // grass
    { 3, 3, 3, 3, 4, 2, 127 },
    // sand
    { 5, 5, 5, 5, 5, 5, 127 },
    // log
    { 6, 6, 6, 6, 7, 7, 127 },
    // planks
    { 8, 8, 8, 8, 8, 8, 127 },
    // stone
    { 9, 9, 9, 9, 9, 9, 127 },
};

enum Direction: u32 {
    EAST = 0,
    SOUTH = 1,
    WEST = 2,
    NORTH = 3,
    UP = 4,
    DOWN = 5,
    DIRECTION_COUNT = 6
};

Direction directionOpposite[DIRECTION_COUNT+1] = {
    WEST, NORTH, EAST, SOUTH,
    DOWN, UP, DIRECTION_COUNT
};

glm::ivec3 directionVector[DIRECTION_COUNT+1] = {
    { 1, 0, 0},
    { 0, 0, 1},
    {-1, 0, 0},
    { 0, 0,-1},
    { 0, 1, 0},
    { 0,-1, 0},
    { 0, 0, 0}
};

bool Chunk::inBounds(glm::ivec3 inChunkCoords) {
    return !(
        inChunkCoords.x < 0 || inChunkCoords.x >= (i32)CHUNKSIZE || 
        inChunkCoords.y < 0 || inChunkCoords.y >= (i32)CHUNKSIZE ||
        inChunkCoords.z < 0 || inChunkCoords.z >= (i32)CHUNKSIZE
    );
}

u32 Chunk::indexOf(glm::ivec3 inChunkCoords) {
    return inChunkCoords.x + inChunkCoords.z*CHUNKSIZE + inChunkCoords.y*CHUNKSIZE*CHUNKSIZE;
}

SimpleVertex cubeFaces[DIRECTION_COUNT*4] = {
    { { 1, 0, 0 }, {1,1,1}, { 1, 0 } },
    { { 1, 1, 0 }, {1,1,1}, { 1, 1 } },
    { { 1, 1, 1 }, {1,1,1}, { 0, 1 } },
    { { 1, 0, 1 }, {1,1,1}, { 0, 0 } },

    { { 0, 0, 1 }, {1,1,1}, { 0, 0 } },
    { { 1, 0, 1 }, {1,1,1}, { 1, 0 } },
    { { 1, 1, 1 }, {1,1,1}, { 1, 1 } },
    { { 0, 1, 1 }, {1,1,1}, { 0, 1 } },

    { { 0, 0, 0 }, {1,1,1}, { 0, 0 } },
    { { 0, 0, 1 }, {1,1,1}, { 1, 0 } },
    { { 0, 1, 1 }, {1,1,1}, { 1, 1 } },
    { { 0, 1, 0 }, {1,1,1}, { 0, 1 } },

    { { 0, 0, 0 }, {1,1,1}, { 0, 0 } },
    { { 0, 1, 0 }, {1,1,1}, { 0, 1 } },
    { { 1, 1, 0 }, {1,1,1}, { 1, 1 } },
    { { 1, 0, 0 }, {1,1,1}, { 1, 0 } },

    { { 0, 1, 0 }, {1,1,1}, { 0, 0 } },
    { { 0, 1, 1 }, {1,1,1}, { 0, 1 } },
    { { 1, 1, 1 }, {1,1,1}, { 1, 1 } },
    { { 1, 1, 0 }, {1,1,1}, { 1, 0 } },

    { { 0, 0, 0 }, {1,1,1}, { 0, 0 } },
    { { 1, 0, 0 }, {1,1,1}, { 1, 0 } },
    { { 1, 0, 1 }, {1,1,1}, { 1, 1 } },
    { { 0, 0, 1 }, {1,1,1}, { 0, 1 } },
};

void Chunk::makeSimpleMesh(SimpleMesh& mesh) {
    for(u32 x=0; x<CHUNKSIZE; x++)
    for(u32 z=0; z<CHUNKSIZE; z++)
    for(u32 y=0; y<CHUNKSIZE; y++) {
        u32 i = indexOf({x, y, z});
        f32 r = (f32)rand()/(f32)RAND_MAX;
        if(r > (f32)y/(f32)CHUNKSIZE)
            blocks[i] = rand()%6+1;
        else
            blocks[i] = 0;
    };

    //mesh.vertices, mesh.indices;
    for(u32 x=0; x<CHUNKSIZE; x++)
    for(u32 z=0; z<CHUNKSIZE; z++)
    for(u32 y=0; y<CHUNKSIZE; y++) {
        glm::ivec3 pos = {x, y, z};
        u32 i = indexOf(pos);
        if(blocks[i] == 0)
            continue;

        // pseudo code:
        // for direction //
            // check if there is a block in this chunk in that direction
            // if not, render that face
            // get solidity of block in that direction
            // if solidity is 0, render that face
            // otherwise skip

        for(u32 dir=0; dir<DIRECTION_COUNT; dir++) {
            glm::ivec3 dv = directionVector[dir];
            glm::ivec3 blockToCheck = pos + dv;
            if(inBounds(blockToCheck)) {
                // TODO: implement solidity
                if(blocks[indexOf(blockToCheck)] != 0)
                    continue;
            }
            //; //draw face 
            //glm::vec3 facepos = glm::vec3(pos) + glm::vec3(0.5f, 0.5f, 0.5f) + glm::vec3(dv)*0.5f;
            u8 textureID = props[blocks[i]].faces[dir];
            for(u32 faceVertex=0; faceVertex<4; faceVertex++) {
                SimpleVertex vertex = cubeFaces[dir*4+faceVertex];
                vertex.texCoords = glm::vec2(textureID%8, 7-textureID/8)/8.0f + vertex.texCoords/8.0f;
                vertex.position += pos;
                mesh.vertices.push_back(vertex);
            }

            // Ambient oclusion demo
            // TODO: the ambient oclusion sucks
            // for each vertex we check the block on the sides on color it less the more neighbours it has
            for(u32 faceVertex=0; faceVertex<4; faceVertex++) {
                SimpleVertex& vertex = mesh.vertices[mesh.vertices.size()-4+faceVertex];
                glm::vec3 g = vertex.position - glm::vec3(pos);
                g = glm::vec3(-1, -1, -1) + 2.0f*g;
                // g gives us one of the 8 corners, 1or -1 for each axis
                u32 neighbours = 0;
                glm::vec3 ns[7] = { g, {g.x, g.y, 0}, {g.x, 0, g.z}, {0, g.y, g.z}, {g.x, 0, 0}, {0,g.y,0}, {0,0,g.z} };
                for(u32 nsi=0; nsi<7; nsi++) {
                    glm::ivec3 nsint = ns[nsi];
                    //cout << "{" << nsint.x << " " << nsint.y << " " << nsint.z << "} ";
                    nsint += pos;
                    if(inBounds(nsint)) {
                        if(blocks[indexOf(nsint)] != 0)
                            neighbours++;
                    }
                }
                vertex.color *= 1.0f - neighbours * 0.07f;
            }

            mesh.indices.push_back(mesh.vertices.size()-4 + 0);
            mesh.indices.push_back(mesh.vertices.size()-4 + 1);
            mesh.indices.push_back(mesh.vertices.size()-4 + 3);
            mesh.indices.push_back(mesh.vertices.size()-4 + 3);
            mesh.indices.push_back(mesh.vertices.size()-4 + 1);
            mesh.indices.push_back(mesh.vertices.size()-4 + 2);
        }

    };
}

void World::init() {

}

void World::draw() {
    
}

void Game::init() {
    printFPS = false;
    window::init(800, 600, "Hello warld");
    renderer.init(800, 600, "Hello warld");
    
    renderer.makeAtlas();
    testWorld.init();
    renderer.meshes.push_back(SimpleMesh());
    testChunk.makeSimpleMesh(renderer.meshes.back());
    renderer.meshes.back().makeObjects();
}

void Game::run() {
    glfwSetTime(0);
    f32 ptime = 0.0;
    f32 fpst = 0.0f;
    u32 fpsc = 0;

    while(!glfwWindowShouldClose(window::window)) {

        f32 time = glfwGetTime();
        f32 dt = time - ptime;
        ptime = time;
        fpst += dt;
        fpsc++;
        if(fpst > 1.0f) {
            if(printFPS)
                cout << fpsc/fpst << " FPS\n";
            fpsc = 0;
            fpst = 0.0f;
        }

        processInput(dt);
        renderer.render();
        glfwPollEvents();
    }

}

void Game::destory() {
    renderer.destroy();
}

