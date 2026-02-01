#version 310 es
precision highp float;

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{    
    vec4 textureColour = texture(texture_diffuse1, TexCoords);
    if(textureColour.a < 0.5) {
        discard;
    }
    FragColor = textureColour;
}