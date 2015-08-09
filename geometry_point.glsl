#version 330 core

layout(points) in;
layout(points, max_vertices = 1) out;

in VertexData {
	vec4 color;
} vertexIn[];
out float alpha;
out VertexData {
	vec4 color;
} vertexOut;

void main() {
	gl_Position = gl_in[0].gl_Position;
	alpha = 1.0;
	vertexOut.color = vertexIn[0].color;
	EmitVertex();
	EndPrimitive();
}
