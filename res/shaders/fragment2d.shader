#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;
uniform vec2 texCoordOffset;


void main()
{
	vec4 texel = texture(texture1, TexCoord + texCoordOffset).rgba;
	if (texel.a < 0.9) discard;
	FragColor = texel;
};