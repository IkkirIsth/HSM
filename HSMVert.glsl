
#version 330 core


in vec3 vertex;
in vec2 vertUV;

uniform mat4 MVP = mat4(1);
//uniform mat4 viewmat;
//uniform mat4 modelmat;
//uniform mat3 mv3mat;
//uniform vec3 LightPosWorld;
out vec2 fragUV;

void main()
{
    gl_Position = MVP * vec4(vertex, 1);
    fragUV = vertUV;
}
