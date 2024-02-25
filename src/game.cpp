#include "game.hpp"
#include "renderer.hpp"

Game* Game::instance = nullptr;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    (void) window;
    Game::instance->input.mousePos.x = xpos;
    Game::instance->input.mousePos.y = ypos;
}

void InputHandler::init() {
    GLFWwindow* window = Game::instance->renderer.window;
    glfwSetInputMode(window, GLFW_CURSOR, disabledCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, cursor_position_callback);
}

void InputHandler::toggleCursor() {
    disabledCursor = !disabledCursor;
    glfwSetInputMode(Game::instance->renderer.window, GLFW_CURSOR, disabledCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

bool InputHandler::isPressed(i32 key) {
    return glfwGetKey(Game::instance->renderer.window, key) == GLFW_PRESS;
}

void InputHandler::processInput(f32 dt) {
    static constexpr float cameraAngleSpeed = 2.0f;
    static constexpr float cameraSpeed = 3.0f;
    static bool prevEscape = false;
    static glm::vec2 lastMousePos = glm::vec2(0, 0);
    static bool lastMouseFirst = true;

    glm::vec3 movement = {0, 0, 0};
    if(isPressed(GLFW_KEY_W))
        movement += glm::vec3(0, 0, 1);
    if(isPressed(GLFW_KEY_A))
        movement += glm::vec3(-1, 0, 0);
    if(isPressed(GLFW_KEY_S))
        movement += glm::vec3(0, 0, -1);
    if(isPressed(GLFW_KEY_D))
        movement += glm::vec3(1, 0, 0);
    if(isPressed(GLFW_KEY_UP))
        movement += glm::vec3(0, 1, 0);
    if(isPressed(GLFW_KEY_DOWN))
        movement += glm::vec3(0, -1, 0);
    bool escape = isPressed(GLFW_KEY_ESCAPE);
    if(escape && !prevEscape)
        toggleCursor();
    prevEscape = escape;

    glm::vec2& cameraAngle = Game::instance->renderer.cameraAngle;
    glm::vec3& cameraPos = Game::instance->renderer.cameraPos;

    glm::vec3 movementReal = movement.z * glm::vec3(std::cos(cameraAngle.x), 0, std::sin(cameraAngle.x))
            + movement.y * glm::vec3(0, 1, 0)
            + movement.x * glm::vec3(-std::sin(cameraAngle.x), 0, std::cos(cameraAngle.x));
    movementReal *= dt * cameraSpeed;
    cameraPos += movementReal;

    if(disabledCursor) {
        glm::vec2 mc = mousePos - lastMousePos;
        if(lastMouseFirst && (mc.x != 0.0f || mc.y != 0.0f)) {
            lastMouseFirst = false;
            return;
        }
        mc.x /= Game::instance->renderer.width;
        mc.y /= Game::instance->renderer.height;
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
    lastMousePos = mousePos;
}

void Chunk::makeSimpleMesh(SimpleMesh& mesh) {
    //mesh.vertices, mesh.indices;
    for(u32 x=0; x<CHUNKSIZE; x++)
    for(u32 z=0; z<CHUNKSIZE; z++)
    for(u32 y=0; y<CHUNKSIZE; y++) {
        u32 i = x*CHUNKSIZE*CHUNKSIZE + z*CHUNKSIZE + y;
        // blocks[i]; -> we don't care tho
        u32 cstart = mesh.vertices.size();
        mesh.vertices.push_back({ { x,   y,   z   }, { 1, 1, 1 }, { 0, 0 } }); //0
        mesh.vertices.push_back({ { x+1, y,   z   }, { 1, 1, 1 }, { 1, 0 } }); //1
        mesh.vertices.push_back({ { x,   y+1, z   }, { 1, 1, 1 }, { 0, 1 } }); //2
        mesh.vertices.push_back({ { x+1, y+1, z   }, { 1, 1, 1 }, { 1, 1 } }); //3
        mesh.vertices.push_back({ { x+1, y,   z+1 }, { 1, 1, 1 }, { 0, 0 } }); //4
        mesh.vertices.push_back({ { x+1, y+1, z+1 }, { 1, 1, 1 }, { 0, 1 } }); //5
        mesh.vertices.push_back({ { x,   y,   z+1 }, { 1, 1, 1 }, { 1, 0 } }); //6
        mesh.vertices.push_back({ { x,   y+1, z+1 }, { 1, 1, 1 }, { 1, 1 } }); //7
        mesh.vertices.push_back({ { x+1, y,   z+1 }, { 1, 1, 1 }, { 1, 1 } }); //8 copy of 4
        mesh.vertices.push_back({ { x+1, y+1, z+1 }, { 1, 1, 1 }, { 1, 0 } }); //9 copy of 5
        mesh.vertices.push_back({ { x,   y,   z+1 }, { 1, 1, 1 }, { 0, 1 } }); //10 copy of 6
        mesh.vertices.push_back({ { x,   y+1, z+1 }, { 1, 1, 1 }, { 0, 0 } }); //11 copy of 7

        
        mesh.indices.push_back(cstart+0);
        mesh.indices.push_back(cstart+1);
        mesh.indices.push_back(cstart+2);
        mesh.indices.push_back(cstart+2);
        mesh.indices.push_back(cstart+1);
        mesh.indices.push_back(cstart+3);

        mesh.indices.push_back(cstart+1);
        mesh.indices.push_back(cstart+3);
        mesh.indices.push_back(cstart+4);
        mesh.indices.push_back(cstart+4);
        mesh.indices.push_back(cstart+5);
        mesh.indices.push_back(cstart+3);

        mesh.indices.push_back(cstart+4);
        mesh.indices.push_back(cstart+6);
        mesh.indices.push_back(cstart+7);
        mesh.indices.push_back(cstart+7);
        mesh.indices.push_back(cstart+5);
        mesh.indices.push_back(cstart+4);

        mesh.indices.push_back(cstart+6);
        mesh.indices.push_back(cstart+7);
        mesh.indices.push_back(cstart+0);
        mesh.indices.push_back(cstart+0);
        mesh.indices.push_back(cstart+2);
        mesh.indices.push_back(cstart+7);
        
        mesh.indices.push_back(cstart+6);
        mesh.indices.push_back(cstart+7);
        mesh.indices.push_back(cstart+0);
        mesh.indices.push_back(cstart+0);
        mesh.indices.push_back(cstart+2);
        mesh.indices.push_back(cstart+7);

        mesh.indices.push_back(cstart+2);
        mesh.indices.push_back(cstart+3);
        mesh.indices.push_back(cstart+9);
        mesh.indices.push_back(cstart+2);
        mesh.indices.push_back(cstart+11);
        mesh.indices.push_back(cstart+9);

        mesh.indices.push_back(cstart+0);
        mesh.indices.push_back(cstart+1);
        mesh.indices.push_back(cstart+8);
        mesh.indices.push_back(cstart+0);
        mesh.indices.push_back(cstart+10);
        mesh.indices.push_back(cstart+8);

    }
}

void Game::init() {
    renderer.init(800, 600, "Hello warld");
    input.init();
    renderer.meshes.push_back(SimpleMesh());
    testChunk.makeSimpleMesh(renderer.meshes.back());
    renderer.meshes.back().makeObjects();
}

void Game::run() {
    glfwSetTime(0);
    f32 ptime = 0.0;
    f32 fpst = 0.0f;
    u32 fpsc = 0;

    while(!glfwWindowShouldClose(renderer.window)) {

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

        input.processInput(dt);
        renderer.render(time, dt);
        glfwPollEvents();
    }

}

void Game::destory() {
    renderer.destroy();
}

