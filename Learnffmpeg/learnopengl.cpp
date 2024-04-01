#include"learnopengl.h"


LearnOpengl::LearnOpengl(const char* title,int width, int height)
{
	//��ʼ��glfw
	if (glfwInit() == GLFW_FALSE) return;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	//�����ʾ����Ϣ
	monitors = glfwGetMonitors(&monitors_count);

	//��������
	if (width && height)
		glfw_win = glfwCreateWindow(width, height, title, nullptr, nullptr);
	else
	{
		int xpos, ypos, width, height;
		glfwGetMonitorWorkarea(monitors[0], &xpos, &ypos, &width, &height);
		glfw_win = glfwCreateWindow(width/2, height/2, title,nullptr , nullptr);
	}

	glfwMakeContextCurrent(glfw_win);

	//��ʼ��glad��Ҫ�ڰ󶨱����߳�֮�����
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





