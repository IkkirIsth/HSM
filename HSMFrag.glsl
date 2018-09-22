
#version 330


in vec2 fragUV;
out vec4 fragcolor;

uniform int usetext = 1;
uniform sampler2D texsampler;


void main()
{
    vec4 MaterialDiffuseColor = texture(texsampler, fragUV);
    if (usetext == 1)
    {
        fragcolor = MaterialDiffuseColor;
    }
    else
    {
        fragcolor = vec4(1.0);
    }



}
