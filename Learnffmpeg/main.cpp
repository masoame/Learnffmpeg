#include"learnffmpeg.h"
#include"learnopengl.h"

int main() {

    glfwInit();
    GLFWwindow* window = glfwCreateWindow(1920/2, 1024/2, "LearnOpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);

    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        printf("Failed to initialize OpenGL context\n");
        return -1;
    }

    printf("Loaded OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));


}
