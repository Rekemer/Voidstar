#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices = 4) out;

void main() {
    // Iterate over the input triangles
    for (int i = 0; i < gl_in.length(); ++i) {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();

    // Emit additional vertices to fill the T-junction
    gl_Position = (gl_in[0].gl_Position + gl_in[1].gl_Position) * 0.5;
    EmitVertex();

    gl_Position = (gl_in[0].gl_Position + gl_in[2].gl_Position) * 0.5;
    EmitVertex();

    gl_Position = (gl_in[1].gl_Position + gl_in[2].gl_Position) * 0.5;
    EmitVertex();
    EndPrimitive();
}
