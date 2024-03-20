#version 460 core

layout (location = 0) out vec4 color;

in vec3 frag_tex;

uniform samplerCube skybox0;
uniform samplerCube skybox1;
uniform samplerCube skybox2;
uniform samplerCube skybox3;

uniform vec4 image_factors;

void main() {    
  vec3 c= vec3(0.0,0.0,0.0);
  if (image_factors[0] > 0) c += image_factors[0] * texture(skybox0, frag_tex).rgb;
  if (image_factors[1] > 0) c += image_factors[1] * texture(skybox1, frag_tex).rgb;
  if (image_factors[2] > 0) c += image_factors[2] * texture(skybox2, frag_tex).rgb;
  if (image_factors[3] > 0) c += image_factors[3] * texture(skybox3, frag_tex).rgb;

  color = vec4(c,1.0);
}