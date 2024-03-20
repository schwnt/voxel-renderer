#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 color_pos;
layout (location = 2) in float tex_id;
layout (location = 3) in uint norm_id;
layout (location = 4) in int tint_id;

flat out vec3 frag_norm;
out vec3 frag_tex;
flat out float sky_illumination;
out vec2 frag_tint;
flat out int tint_index;

uniform mat4 transform;
uniform vec3 skylight;

const vec2 tex_table[4] = vec2[4](vec2(0.0,0.0),vec2(1.0,0.0),vec2(0.0,1.0),vec2(1.0,1.0));

const vec3 norm_table[6] =  vec3[6]( vec3(0.0,0.0,1.0),vec3(0.0,0.0,-1.0),vec3(-1.0,0.0,0.0),
									  vec3(1.0,0.0,0.0),vec3(0.0,1.0,0.0),vec3(0.0,-1.0,0.0));

const float spec = 0.7;
const float ambient = 0.25;

void main() {
  vec4 position = transform * vec4(pos, 1);
  gl_Position = position;

  tint_index = tint_id;
  frag_tint = color_pos;

  frag_norm = norm_table[norm_id];
  frag_tex = vec3(tex_table[gl_VertexID % 4],tex_id);

  float sky_strength = 0.3+ dot(skylight, vec3(0.0,-1.0,0.0));
  sky_strength = clamp(sky_strength, 0.0,1.0);
  float spec_factor = clamp(dot(-skylight,frag_norm) + 0.35,0.0,1.0) * spec;

  sky_illumination = ambient + sky_strength * (1.0-spec) + sky_strength * spec_factor;
}

