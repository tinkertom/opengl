#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <ctime>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GL_LOG_FILE "gl.log"

bool restart_gl_log();
bool gl_log(const char* msg, ...);
bool gl_log_err(const char* msg, ...);
void glfw_error_callback(int err, const char* msg);
char* read_file_to_str(const char* path);
GLuint compile_shader(const char* src, GLenum type);
GLuint create_program(GLuint vs, GLuint fs);

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

    glEnable(GL_DEPTH_TEST | GL_CULL_FACE);
    glDepthFunc (GL_LESS);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    GLfloat vertices[] = {
         0.0f,  0.5f,
         0.5f, -0.5f,
        -0.5f, -0.5f,
    };
    GLfloat colors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    };
    GLuint vbo_ver = 0;
    glGenBuffers(1, &vbo_ver);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_ver);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const void*)0);
    GLuint vbo_col = 0;
    glGenBuffers(1, &vbo_col);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_col);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);
    
    char* vs_src = read_file_to_str("src/shaders/vert.glsl");
    char* fs_src = read_file_to_str("src/shaders/frag.glsl");
    if (!vs_src || !fs_src) {
        fprintf(stderr, "[ERROR] could not read file to string\n");
        glfwDestroyWindow(win);
        glfwTerminate();
    }
    GLuint vs = compile_shader(vs_src, GL_VERTEX_SHADER);
    if (!vs) {
        return 0;
    }
    GLuint fs = compile_shader(fs_src, GL_FRAGMENT_SHADER);
    if (!fs) {
        return 0;
    }
    free(vs_src);
    free(fs_src);
    GLuint prog = create_program(vs, fs);
    if (!prog) {
        return 0;
    }
    glUseProgram(prog);

    glm::vec4 trans_vec(1.0f, 1.0f, 0.0f, 1.0f);
    glm::mat4 trans_mat(1.0f);
    trans_mat = glm::translate(trans_mat, glm::vec3(1.0f, 1.0f, 0.0f));
    GLint uniform = glGetUniformLocation(prog, "u_transform");
    glUniformMatrix4fv(uniform, 1.0, 0.0f, 1.0f, 1.0f);
    
    while (!glfwWindowShouldClose(win)) {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(prog);
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

GLuint compile_shader(const char* src, GLenum type)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    int compile_status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    if (!compile_status){
        int len;
        const int max_len = 1024;
        char log[max_len];
        glGetShaderInfoLog(shader, max_len, &len, log);
        gl_log_err("[ERROR] could not compile shader - compile message:\n%s\n", log);
        //return 0;
    }

    return shader;
}

GLuint create_program(GLuint vs, GLuint fs)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int link_status;
    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    if (!link_status) {
        int len;
        const int max_len = 1024;
        char log[max_len];
        glGetProgramInfoLog(program, max_len, &len, log);
        //gl_log_err("[ERROR] could not link program - link message:\n%s\n", log);
    }

    return program;
}
