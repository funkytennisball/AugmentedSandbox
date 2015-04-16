#version 330

in layout(location=0) vec3 position;
in layout(location=1) vec3 waterheight;
in layout(location=2) vec2 texCoord;

out vec2 texCoordV;
out float positionyV;
out vec3 waterheightV;
flat out int stateV;

uniform	mat4 mvp;
uniform mat4 scale;
uniform int state;


void main()
{
	texCoordV = texCoord;
	positionyV = position.y;
	stateV = state;
	waterheightV = waterheight;

	vec3 pos = position;
	mat4 nscale = scale;
	if(state ==1){
		const float wRange0 = 0.025;
		vec3 wh = vec3(0.0, waterheight.y, 0.0);
		
		if(waterheight.y>wRange0 ){
			pos = pos + wh;
		}
		
	}
	
	gl_Position = mvp * nscale * vec4(pos, 1.0);
}