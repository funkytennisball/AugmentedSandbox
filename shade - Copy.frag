#version 330

in float positionyV;
in vec2 texCoordV;
out vec4 colorOut;

uniform sampler2D gSampler [3];

void main()
{
	vec4 vTexColor = vec4(0.0);
	float fScale = (positionyV - 100) / (220 - 100);

	const float fRange0 = 0.1f;
	const float fRange1 = 0.55f; 
	const float fRange2 = 0.75f; 

	if(fScale <= fRange2)
	{
		fScale -= fRange1;
		fScale /= (fRange2-fRange1);
		
		float fScale2 = fScale;
		fScale = 1.0-fScale; 

		vTexColor += texture2D(gSampler[1], texCoordV)*fScale;
		vTexColor += texture2D(gSampler[2], texCoordV)*fScale2;
	} else vTexColor = texture2D(gSampler[2], texCoordV);

	colorOut = vTexColor;
}