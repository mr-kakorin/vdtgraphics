#include <iostream>
#include <vdtgraphics/api/opengl/opengl.h>
#include <GLFW/glfw3.h>
#include <vdtgraphics/graphics.h>
#include <sstream>
#include <vdtmath/math.h>
#include <vdtgraphics/meshes/quad.h>

using namespace std;
using namespace graphics;
using namespace math;

void render_loop();
API* api = nullptr;
Renderer2D* renderer2d = nullptr;
Texture* batmanTexture = nullptr;

int nbFrames = 0;
double lastTime = 0;
void showFPS(GLFWwindow* pWindow)
{
    // Measure speed
    double currentTime = glfwGetTime();
    double delta = currentTime - lastTime;
    nbFrames++;
    if (delta >= 1.0) { // If last cout was more than 1 sec ago
        cout << 1000.0 / double(nbFrames) << endl;

        int fps = double(nbFrames) / delta;

        std::stringstream ss;
        ss << "vdtgraphics" << " " << "1.0" << " [" << fps << " FPS]";

        glfwSetWindowTitle(pWindow, ss.str().c_str());

        nbFrames = 0;
        lastTime = currentTime;
    }
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    // load GL
    //gladLoadGL();

    api = API::Factory::get();
    api->startup();
    api->enableAlpha(true);
    renderer2d = api->createRenderer2D();
    Image batmanImg;
    Image::load("batman_logo.png", batmanImg);
    batmanImg.flipVertically();
    batmanTexture = api->createTexture(batmanImg);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        // display the FPS
        showFPS(window);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        // render logic
        render_loop();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    batmanImg.free();

    api->shutdown();
    glfwTerminate();
    return 0;
}

void render_loop()
{
    renderer2d->clear(Color::Blue);

    renderer2d->drawTexture(batmanTexture, {});
    renderer2d->drawRect(Color::Red, { -.3f, -.3f }, { .2f, .4f });
    renderer2d->drawCircle(Color::Green, {}, .4f);

    renderer2d->render();
    
}