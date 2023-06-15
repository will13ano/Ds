//[]---------------------------------------------------------------[]
//|                                                                 |
//| Copyright (C) 2022 Paulo Pagliosa.                              |
//|                                                                 |
//| This software is provided 'as-is', without any express or       |
//| implied warranty. In no event will the authors be held liable   |
//| for any damages arising from the use of this software.          |
//|                                                                 |
//| Permission is granted to anyone to use this software for any    |
//| purpose, including commercial applications, and to alter it and |
//| redistribute it freely, subject to the following restrictions:  |
//|                                                                 |
//| 1. The origin of this software must not be misrepresented; you  |
//| must not claim that you wrote the original software. If you use |
//| this software in a product, an acknowledgment in the product    |
//| documentation would be appreciated but is not required.         |
//|                                                                 |
//| 2. Altered source versions must be plainly marked as such, and  |
//| must not be misrepresented as being the original software.      |
//|                                                                 |
//| 3. This notice may not be removed or altered from any source    |
//| distribution.                                                   |
//|                                                                 |
//[]---------------------------------------------------------------[]
//
// OVERVIEW: MainWindow.cpp
// ========
// Source file for cg template window.
//
// Author: Paulo Pagliosa
// Last revision: 07/11/2022

#include "MainWindow.h"
#define STRINGFY(s) "#version400\n"#s //é um macro -gera uma string cujo conteudo é o que ta em "" 
static const char* fs=STRINGFY(
  in vec4 color;//entrada do shader -> se ela é entrada, é saída do shader anterior
  out vec4 fragColor;//saída do shader
  void main()
  {
    fragColor=color;
  }
);//shader de fragemento - define a cor do fragmento sendo processado - fragmento é candidato a pixel
struct uint3
{
  unsigned int x;
  unsigned int y;
  unsigned int z;

}; // int3

namespace cube
{ // begin namespace cube

  static const cg::vec4f v[]
  {
    {-0.5f, -0.5f, -0.5f, 1}, // 0
    {+0.5f, -0.5f, -0.5f, 1}, // 1
    {+0.5f, +0.5f, -0.5f, 1}, // 2
    {-0.5f, +0.5f, -0.5f, 1}, // 3
    {-0.5f, -0.5f, +0.5f, 1}, // 4
    {+0.5f, -0.5f, +0.5f, 1}, // 5
    {+0.5f, +0.5f, +0.5f, 1}, // 6
    {-0.5f, +0.5f, +0.5f, 1}  // 7
  };

  static const cg::Color c[]
  {
    cg::Color{0.0f, 0.0f, 0.0f}, // 0
    cg::Color{1.0f, 0.0f, 0.0f}, // 1
    cg::Color{1.0f, 1.0f, 0.0f}, // 2
    cg::Color{0.0f, 1.0f, 0.0f}, // 3
    cg::Color{0.0f, 0.0f, 1.0f}, // 4
    cg::Color{1.0f, 0.0f, 1.0f}, // 5
    cg::Color{1.0f, 1.0f, 1.0f}, // 6
    cg::Color{0.0f, 1.0f, 1.0f}  // 7
  };

  static const uint3 t[]
  {
    {0, 3, 1}, {1, 3, 2}, // back
    {4, 5, 7}, {5, 6, 7}, // front
    {0, 4, 3}, {4, 7, 3}, // left
    {1, 2, 5}, {5, 2, 6}, // right
    {0, 1, 4}, {1, 5, 4}, // bottom
    {7, 6, 3}, {6, 2, 3}  // top
  };

} // end namespace cube
//shader de vertice tem q determinar glposition (coordenadas de recorte x,y,z e w)
//rasterizacao determina todos os pixels do triangulo, para cada um deles roda o shader de fragmento
//fragmento pertencente a face da frente vira pixel
//nosso shader de fragmento vai definir a cor e passar p frente
//agora o shader de vertice
static const char* vs=STRINGFY(
  layout(location = 0) in vec4 vertex;
  layout(location = 1) in vec4 vertexColor;
  uniform mat4 transf;
  out vec4 color;

  void main()
  {
    gl_Position = transf * vertex;
    color = vertexColor;
  }
  // uniform vec4 vertices[36]=
  // {
  //   vec4(-0.5f, -0.5f, -0.5f,1),
  //   vec4(-0.5f,0.5f,-0.5f,1),
  //   vec4(0.5f, 0.5f, -0.5f, 1),
  //   vec4(-0.5f, -0.5f, -0.5f,1),
  //   vec4(0.5f, -0.5f, -0.5f, 1),
  //   vec4(0.5f, 0.5f, -0.5f, 1),
  //   //TODO: fazer mais 30 vezes p completar os 36
  // }
  // uniform vec4 colors[36]=
  // {
  //   vec4(0,0,1,1),
  //   vec4(0,1,1,1),
  //   vec4(1, 1, 1, 1),
  //   //continua ate fazer os 36
  // }
  // uniform mat4 transf;

  // out vec4 color;

  // void main()
  // {
  //   gl_Position = transf* vertices[gl_VertexID];
  //   color = colors[gl_VertexID];
  // }
);

/////////////////////////////////////////////////////////////////////
//
// MainWindow implementation
// ==========
MainWindow::MainWindow(int width, int height):
  //Base{"Ds template", width, height}
  Base{"RGB cube", width, height}
  _program{"Cube Drawer"}
{
  // Put your code here. Example:
  /*_lineColor = cg::Color::red;
  _meshColor = cg::Color::blue;
  _radius = 1;
  _speed = 0.01f;*/
}

template <typename T>
inline int
size(int count)
{
  return count * sizeof(T);
}

void
MainWindow::initialize()
{
  // Put your OpenGL initialization code here. Example:
  Base::initialize();
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0f, 1.0f);
  glGenVertexArrays(1,&_vao);//(numero de vaos que quero criar, ponteiro que contem os lugares onde vao os vertex array gerados)
  // vao = Dados para o inicio da execução do pipeline
  glBindVertexArray(_vao);

  glGenBuffers(3, _buffers);
  glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
  glBufferData(GL_ARRAY_BUFFER, size<cg::vec4f>(8), cube::v, GL_STATIC_DRAW);
  // glVertexAttribPointer(idBuffer, intervalo, TIPO, , ,)
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  //Envia para varialvel com a location de numero 0 no vs
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, _buffers[1]);
  glBufferData(GL_ARRAY_BUFFER, size<cg::Color>(8), cube::c, GL_STATIC_DRAW);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffers[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
    size<uint3>(12),
    cube::t,
    GL_STATIC_DRAW
  );

  _program.setShaders(vs, fs).use();//vs, fs -> shader de vertice, shader de fragmento
}

void
MainWindow::update()
{
  // Put your scene update code here. Example:
  /*static float time{};

  if (_animate)
    _radius = 1 + cosf(_speed * (time += deltaTime()) * 0.5f);*/
}

void
//MainWindow::renderScene()
MainWindow::render()
{
  // Put your scene rendering code here. 
  clear(cg::Color::darkGray);//limpa a tela e coloca darkgray de cor de fundo
  _program.setUniformMat4("transf", _transf);

  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
  // glDrawArrays(GL_TRIANGLES, 0,36);
  //funcao de desenho da openGl( , 0 ->contador offset - pula tantos e vai até->,numero de vertices) -cada triangulo fornece 3 vertices, vai repetir vertice msm
  //36 pq sao 12 triangulos p fazer um cubo
}

/*bool
MainWindow::keyInputEvent(int key, int action, int mods)
{
  // Put your key event handler code here. Example:
  if (action != GLFW_RELEASE && mods == GLFW_MOD_ALT)
    switch (key)
    {
      case GLFW_KEY_P:
        _animate ^= true;
        return true;
    }
  return Base::keyInputEvent(key, action, mods);
}*/

void
MainWindow::gui()
{
  using namespace cg;
  // Put your gui code here. Example:
  ImGui::SetNextWindowSize({360, 180});//-> tamanho da janela 360*180
  ImGui::Begin("Controls");

  ImGui::SetNextWindowSize(ImVec2(240, 240), ImGuiCond_FirstUseEver);
  ImGui::Begin("Inspector");
  ImGui::DragFloat3("Rotation", (float*)&_rotation, 1.0f, 0, 0, "%.2f");
  
  transf = mat4f::rotation(quatf::eulerAngles(_rotation), vec3f::null());

  _program.setUniformMat4("transf", transf);
  ImGui::End();

  // ImGui::Begin("Template GUI");
  // ImGui::ColorEdit3("Line Color", (float*)&_lineColor);
  // ImGui::ColorEdit3("Mesh Color", (float*)&_meshColor);
  // ImGui::Separator();
  // ImGui::Checkbox("Animate", &_animate);
  // ImGui::SliderFloat("Speed", &_speed, 0.001f, 0.01f);
  // ImGui::Checkbox("Show Ground", &_showGround);
  // ImGui::Separator();
  // ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
  // deltaTime(),
  // ImGui::GetIO().Framerate);
  // ImGui::End();//->abre janela que nao tem nada, so tem titulo
}

void
MainWindow::terminate()
{
  glDeleteVertexArrays(1, &_vao);
}
