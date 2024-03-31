#include"learnopengl.h"


LearnOpengl::LearnOpengl(const char* title,int width, int height)
{
	//初始化glfw
	if (glfwInit() == GLFW_FALSE) return;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	//获得显示器信息
	monitors = glfwGetMonitors(&monitors_count);

	//创建窗口
	if (width && height)
		glfw_win = glfwCreateWindow(width, height, title, nullptr, nullptr);
	else
	{
		int xpos, ypos, width, height;
		glfwGetMonitorWorkarea(monitors[0], &xpos, &ypos, &width, &height);
		glfw_win = glfwCreateWindow(width/2, height/2, title,nullptr , nullptr);
	}

	glfwMakeContextCurrent(glfw_win);

	//初始化glad需要在绑定本地线程之后进行
	if (!gladLoadGL(glfwGetProcAddress))

	glfwSwapInterval(1);

}

bool LearnOpengl::GLFWStartWindow()
{
	while (!glfwWindowShouldClose(glfw_win))
	{
		glfwSwapBuffers(glfw_win);
		glfwPollEvents();
	}
	glfwDestroyWindow(glfw_win);
	glfwTerminate();
	return true;
}





