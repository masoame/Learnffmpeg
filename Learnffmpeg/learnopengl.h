#pragma once
#include"common.h"

class LearnOpengl
{
public:
	LearnOpengl(const char* title, int width = 0, int height = 0);
	bool GLFWStartWindow();

private:

	GLFWwindow* glfw_win = nullptr;

	int monitors_count = 0;
	GLFWmonitor** monitors = nullptr;
};