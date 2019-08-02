#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define	ASSERT(x) if (!(x)) __debugbreak();

#define	GLCall(x) GLClearError();\
	x;\
ASSERT(GLLogCall(#x, __FILE__, __LINE__));

static void		GLClearError() {
	while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall( const char* function, const char* file, int line) {
	while (GLenum error = glGetError()) {
		std::cout << "[OpenGL_Error] (" << error << ")" << function << " " << file << " " << ": "<< line << std::endl;
		return false;
	}
	return true;
}

struct ShaderProgramSource {
	std::string VertexSource;
	std::string FragmentSource;
};

static ShaderProgramSource	ParseShader(const std::string& filepath) {
	std::ifstream stream(filepath);

	enum class ShaderType {
		NONE = -1, Vertex = 0, Fragment = 1
	};

	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;
	while (getline(stream, line)) {
		if (line.find("#shader") != std::string::npos) {
			if (line.find("vertex") != std::string::npos)
				type = ShaderType::Vertex;
			else if (line.find("fragment") != std::string::npos)
				type = ShaderType::Fragment;
		}
		else {
			ss[(int)type] << line << '\n';
		}
	}
	return { ss[0].str(), ss[1].str() };
}

static unsigned int	CompileShader(unsigned int type, const std::string& source) {
	unsigned int	id = glCreateShader(type);
	const char		*src = source.c_str();

	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	//TODO :: ERROR handling;
	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int		length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char	*message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") <<" shader!" << std::endl;
		std::cout << message << std::endl;
		glDeleteShader(id);
		return (0);
	}

	return (id);
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return (program);
}

int main(void) {
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/*Init the glew after the context */
	if (glewInit() != GLEW_OK) {
		std::cout << "ERROR" << std::endl;
	}

	float	position[] = {
		-0.5f, -0.5f,
		0.5f, -0.5f,
		0.5f, 0.5f,
		-0.5f, 0.5f
	};

	unsigned int indeces[]{
		0, 1, 2,
		2, 3, 0
	};

	unsigned int buffer;
	GLCall(glGenBuffers(1, &buffer));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
	GLCall(glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), position, GL_STATIC_DRAW));

	GLCall(glEnableVertexAttribArray(0));
	GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));

	unsigned int ibo;
	GLCall(glGenBuffers(1, &ibo));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indeces, GL_STATIC_DRAW));

	ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
	std::cout << "Vertex" << std::endl;
	std::cout << source.VertexSource << std::endl;
	std::cout << "Fragment" << std::endl;
	std::cout << source.FragmentSource << std::endl;
	unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);

	glUseProgram(shader);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glDeleteProgram(shader);
	glfwTerminate();
	return 0;
}