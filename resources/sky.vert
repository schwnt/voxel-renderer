#version 460 core
layout (location = 0) in vec3 pos;

out vec3 frag_tex;

uniform mat4 transform;

void main() {
    frag_tex = pos;
    gl_Position = vec4(transform * vec4(pos, 1.0)).xyww;
}  