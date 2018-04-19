#version 330

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in float fogFactor;				// Fog Factor

// Uniform: The Texture
uniform sampler2D texture0;
uniform vec3 fogColour;			// Fog Colour

// Output Variable (sent down through the Pipeline)
out vec4 outColor;


void main(void) 
{
	outColor = color;
	outColor *= texture(texture0, texCoord0.st);
	outColor = mix(vec4(fogColour, 1), outColor, fogFactor);
}
