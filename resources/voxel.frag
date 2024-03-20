#version 460 core

layout (location = 0) out vec4 color;

in vec3 frag_norm;
in vec3 frag_tex;
in vec2 frag_tint;
flat in int tint_index;
in float sky_illumination;

uniform sampler2DArray tex_array;  

uniform sampler2DArray colormap;

const vec3 fog_color = vec3(1.0,1.0,1.0);

void main() {
	if (tint_index < 0) {
	  color = texture(tex_array, frag_tex);
	} 
	else {
	  vec4 tex_color = texture(tex_array, frag_tex);
	  vec4 tint_color = texture(colormap, vec3(frag_tint,tint_index));
	  color = tint_color * tex_color.r;
	}

	color.rgb *=sky_illumination;
	color.a = 1.0;
} 