#define ASSERT(x, str) if (!(x)) {printf(str); exit(1);}

typedef unsigned int uint;
typedef int32_t i32;

uint shader_create_program(char vertex_path[], char fragment_path[]) { 
  // Vertex shader
  FILE* file = fopen(vertex_path, "r");
  ASSERT(file != NULL, "Can't open vertex shader");
  i32 success;

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  char vertex_shader_source[size];
  fread(vertex_shader_source, sizeof(char), size - 1, file);
  vertex_shader_source[size - 1] = '\0';
  fclose(file);

  uint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, (const char * const *) &(const char *) { vertex_shader_source }, NULL);
  glCompileShader(vertex_shader);

  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  ASSERT(success, "Error compiling vertex shader");

  // Fragment shader
  file = fopen(fragment_path, "r");
  ASSERT(file != NULL, "Can't open fragment shader");

  fseek(file, 0, SEEK_END);
  size = ftell(file);
  rewind(file);

  char fragment_shader_source[size];
  fread(fragment_shader_source, sizeof(char), size - 1, file);
  fragment_shader_source[size - 1] = '\0';
  fclose(file);

  uint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, (const char * const *) &(const char *) { fragment_shader_source }, NULL);
  glCompileShader(fragment_shader);

  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  ASSERT(success, "Error compiling fragment shader");

  // Create shader program
  uint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  ASSERT(success, "Error linking shaders");

  glUseProgram(shader_program);

  return shader_program;
}
