#version 330

in layout(location=0) vec3 position;
in layout(location=1) vec3 normals;
in layout(location=2) vec2 texCoord;

out vec2 texCoordV;
out float positionyV;

uniform	mat4 mvp;
uniform mat4 scale;

void main()
{
	texCoordV = texCoord;
	positionyV = position.y;
	gl_Position = mvp * scale * vec4(position, 1.0);
}