#version 330

in float positionyV;
in vec2 texCoordV;
in vec3 waterheightV;

flat in int stateV;
out vec4 colorOut;

uniform sampler2D gSampler [6];

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

	if(stateV == 1){
	const float wRange0 = 0.025;
	const float wRange1 = 0.5;
	const float wRange2 = 20;


		if(waterheightV.y > wRange0 && waterheightV.y < wRange1){
			//vTexColor = (0.5+((0.005-waterheightV.y)/0.005) * 0.5)* vTexColor + texture2D(gSampler[3], texCoordV)*(((waterheightV.y)/0.005) * 0.5);
			//vTexColor = 0.5* vTexColor + texture2D(gSampler[3], texCoordV)*0.5;
			vTexColor = (0.5+((wRange1-waterheightV.y)/wRange1) * 0.5)* vTexColor + texture2D(gSampler[3], texCoordV)*(((waterheightV.y)/wRange1) * 0.5);
			//vTexColor = 0.9* vTexColor + texture2D(gSampler[3], texCoordV)*0.1;
		} else if(waterheightV.y > wRange1 && waterheightV.y < wRange2){
			vTexColor = 0.5* vTexColor + texture2D(gSampler[4], texCoordV)*0.5*(waterheightV.y-wRange1)/(wRange2-wRange1) +  + texture2D(gSampler[4], texCoordV)*0.5*(0.5-(waterheightV.y-wRange1)/(wRange2-wRange1));
		} else {
			vTexColor = 0.5* vTexColor + texture2D(gSampler[5], texCoordV)*0.5;
		}
	}
	colorOut = vTexColor;
}