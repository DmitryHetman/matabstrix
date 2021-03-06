#include "raw.hpp"

#include "../program.hpp"

#include <cstdio>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Models;

const GLchar *const Raw::attribs[] = {
  "position",
  "normal",
};

const GLchar *const Raw::uniforms[] = {
  "mvp",
  "local_modelview",
};

const std::string Raw::filename(const std::string &name)
{
  return "/models/" + name;
}

Raw::Raw(const Adapter &adapter, const std::string &name)
{
  exe = adapter.load<Program>("normal")->build(
    __attrib_count, attribs,
    __uniform_count, uniforms
  );

  FILE *file = fopen(adapter.filename<Raw>(name).c_str(), "r");

  while (!feof(file))
  {
    std::array<glm::vec3, 3> v;

    for (int i = 0; i < 3; ++i)
      fscanf(file, "%f %f %f", &v[i].x, &v[i].y, &v[i].z);

    add3_with_auto_normals(v, glm::vec3(0.0f, -10.0f, 0.0f));
  }

  fclose(file);

  positions_id = create_array_buffer(GL_ARRAY_BUFFER, 3 * positions.size() * sizeof(GLfloat), positions.data());
  normals_id = create_array_buffer(GL_ARRAY_BUFFER, 3 * normals.size() * sizeof(GLfloat), normals.data());
  id = create_array_buffer(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(GLushort), elements.data());
}

void Raw::draw(__attribute__((unused)) const Scene &scene, const glm::mat4 &mvp, const glm::mat4 &transformation) const
{
  exe->use();

  const glm::mat4 transform = mvp * transformation;
  glUniformMatrix4fv(exe->uniform(Uniform::mvp), 1, GL_FALSE, glm::value_ptr(transform));

  const glm::mat3 local_modelview = glm::transpose(glm::inverse(glm::mat3(transformation)));
  glUniformMatrix3fv(exe->uniform(Uniform::local_modelview), 1, GL_FALSE, glm::value_ptr(local_modelview));

  glEnableVertexAttribArray(position);
  glBindBuffer(GL_ARRAY_BUFFER, positions_id);
  glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const GLvoid*>(0));

  glEnableVertexAttribArray(normal);
  glBindBuffer(GL_ARRAY_BUFFER, normals_id);
  glVertexAttribPointer(normal, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const GLvoid*>(0));

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
  glDrawElements(GL_TRIANGLES, elements.size(), GL_UNSIGNED_SHORT, 0);
}
