#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <ctime>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GL_LOG_FILE "gl.log"

bool restart_gl_log();
bool gl_log(const char* msg, ...);
bool gl_log_err(const char* msg, ...);
void glfw_error_callback(int err, const char* msg);
char* read_file_to_str(const char* path);

int main()
{
    restart_gl_log();
    glfwSetErrorCallback(&glfw_error_callback);
    if (!glfwInit()) {
        fprintf(stderr, "[GLFW ERROR] could not initialize GLFW3\n");
        return 0;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    int win_w = 640, win_h = 380;
    const char* title = "opengl";
    GLFWwindow* win = glfwCreateWindow(win_w, win_h, title, nullptr, nullptr);
    if (!win) {
        fprintf(stderr, "[GLFW ERROR] could not create GLFW window\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(win);
    glewExperimental = GL_TRUE;
    if (int glew_err = glewInit() != GLEW_OK) {
        gl_log_err("[GLEW ERROR] could not initialize GLEW\n");
        gl_log_err("[GLEW ERROR] %s\n", glewGetErrorString(glew_err));
        glfwDestroyWindow(win);
        glfwTerminate();
        return 0;
    }

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LESS);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    GLfloat vertices[] = {
         0.0f,  0.5f,
         0.5f, -0.5f,
        -0.5f, -0.5f,
    };
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const void*)0);

    char* vs_src = read_file_to_str("src/shaders/vert.glsl");
    char* fs_src = read_file_to_str("src/shaders/frag.glsl");
    if (!vs_src || !fs_src) {
        fprintf(stderr, "[ERROR] could not read file to string\n");
        glfwDestroyWindow(win);
        glfwTerminate();
    }
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vs, 1, &vs_src, nullptr);
    glShaderSource(fs, 1, &fs_src, nullptr);
    free(vs_src);
    free(fs_src);
    glCompileShader(vs);
    glCompileShader(fs);
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glUseProgram(program);
    GLint uniform = glGetUniformLocation(program, "u_col");
    glUniform4f(uniform, 1.0, 0.0f, 1.0f, 1.0f);
    
    while (!glfwWindowShouldClose(win)) {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(vao);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwPollEvents();
        glfwSwapBuffers(win);
    }
    
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}

bool restart_gl_log()
{
    FILE* file = fopen(GL_LOG_FILE, "w");
    if (!file) {
        fprintf(stderr, "[ERROR] could open GL_LOG_FILE \"%s\" for writing\n", GL_LOG_FILE);
        return false;
    }

    time_t now = time(NULL);
    char* date = ctime(&now);
    fprintf(file, "GL_LOG_FILE: %s - local time: %s\n", GL_LOG_FILE, date);

    fclose(file);
    return true;
}

bool gl_log(const char* msg, ...)
{
    FILE* file = fopen(GL_LOG_FILE, "a");
    if (!file) {
        fprintf(stderr, "[ERROR] could open GL_LOG_FILE \"%s\" for appending\n", GL_LOG_FILE);
        return false;
    }

    va_list args;

    va_start(args, msg);
    vfprintf(file, msg, args);
    va_end(args);

    fclose(file);
    return true;
}

bool gl_log_err(const char* msg, ...)
{
    FILE* file = fopen(GL_LOG_FILE, "a");
    if (!file) {
        fprintf(stderr, "[ERROR] could open GL_LOG_FILE \"%s\" for appending\n", GL_LOG_FILE);
        return false;
    }

    va_list args;

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    
    va_start(args, msg);
    vfprintf(file, msg, args);
    va_end(args);

    fclose(file);
    return true;
}

void glfw_error_callback(int err, const char* msg)
{
    gl_log_err("[GLFW ERROR] error code: %d - message:\n%s\n", err, msg);
}

char* read_file_to_str(const char* path)
{
    FILE* file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "[ERROR] could not open file \"%s\" for reading\n", path);
        return nullptr;
    }
    
    unsigned int len = 64, pos = 0;
    char* buf = (char*)malloc(sizeof(char) * 64);
    if (!buf) {
        fprintf(stderr, "[ERROR] could not allocate memory\n");
        fclose(file);
        return nullptr;
    }
    do {
        if (pos == len) {
            len *= 2;
            buf = (char*)realloc(buf, sizeof(char) * len);
            if (!buf) {
                fprintf(stderr, "[ERROR] could not reallocate memory\n");
                fclose(file);
                return nullptr;
            }
        }
        buf[pos] = fgetc(file);
        ++pos;
    } while (buf[pos - 1] != EOF);
    buf[pos - 1] = '\0';

    fclose(file);
    return buf;
}
