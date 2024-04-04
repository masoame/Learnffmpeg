#include"learnffmpeg.h"
#include"learnopengl.h"

int main(int argc, char* args[])
{
	LearnVideo LV;
	 err = LV.open("C:\\Users\\observer\\Downloads\\baiduyun\\kona.mp4");
	LV.init_decode();
	return 0;
}