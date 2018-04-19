#version 330

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in float fogFactor;

// Input: Water Related
in float waterDepth;	// water depth (positive for underwater, negative for the shore)

// Uniform: The Texture
uniform sampler2D texture0;
uniform sampler2D textureBed;
uniform sampler2D textureShore;

// Uniform: Lights
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;
uniform mat4 matrixView;
uniform vec3 fogColour;



// Output Variable (sent down through the Pipeline)
out vec4 outColor;

// Water-realted
uniform vec3 waterColor;

struct POINT
{	int on;
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};
uniform POINT lightPoint1, lightPoint2, lightPoint3, lightPoint4;

vec4 compPoint(vec3 materialDiffuse, vec3 materialSpecular, float materialShininess, POINT light)
{
	vec4 result = vec4(0, 0, 0, 1);

	// diffuse
	vec3 L = normalize(matrixView * vec4(light.position, 1) - position).xyz;
	float NdotL = dot(L, normal.xyz);
	if (NdotL > 0)
		result += vec4(light.diffuse * materialDiffuse, 1) * NdotL;

	// specular
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal.xyz);
	float RdotV = dot(R, V);
	if (NdotL > 0 && RdotV > 0)
		result += vec4(light.specular * materialSpecular * pow(RdotV, materialShininess), 1);

	// attenuation
	float dist = length(matrixView * vec4(light.position, 1) - position);
	float att = 1 / (dist * dist) / 0.1;

	return result * att;
}


void main(void) 
{
	outColor = color;
	//outColor *= texture(texture0, texCoord0.st);

	if (lightPoint1.on == 1){
		outColor += compPoint(materialDiffuse, materialSpecular, materialShininess, lightPoint1);
	}
	if (lightPoint2.on == 1){
		outColor += compPoint(materialDiffuse, materialSpecular, materialShininess, lightPoint2);
	}
	if (lightPoint3.on == 1){
		outColor += compPoint(materialDiffuse, materialSpecular, materialShininess, lightPoint3);
	}
	if (lightPoint4.on == 1){
		outColor += compPoint(materialDiffuse, materialSpecular, materialShininess, lightPoint4);
	}

	// shoreline multitexturing
	float isAboveWater = clamp(-waterDepth, 0, 1);
	outColor *= mix(texture(textureBed, texCoord0), texture(textureShore, texCoord0), isAboveWater);

	//outColor = mix(vec4(waterColor, 1), outColor, fogFactor);
	outColor = mix(vec4(fogColour, 1), outColor, fogFactor);
}
