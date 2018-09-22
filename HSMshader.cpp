
#include "HSMshader.hpp"


SHADER::SHADER()
{

}
//struct that holds vao, vbos.
SHADER::~SHADER()
{
    //clean it up and its usable.  look for proper cleanup techniques
    glUseProgram(0); //just to make sure
    glDeleteProgram(shaderprogram);
    glDeleteShader(vshader);
    glDeleteShader(fshader);
    if (useGshader)
        glDeleteShader(gshader);
}

SHADER::SHADER(const char *vertshader, const char *fragshader, const char *geoshader, bool geobit)
{

    int IsCompiled_VS, IsCompiled_FS; //error checks

    int maxLength = 10000; //file loading?
    char *vertexInfoLog;
    char *fragmentInfoLog = 0;
    //char *shaderProgramInfoLog; //log dumps from errors
    GLchar *vertexsource, *fragmentsource, *geometrysource; //load into these from file

    vertexsource = filetobuf((char*)vertshader);
    if (!vertexsource)
        exit(9999);
    fragmentsource = filetobuf((char*)fragshader);
    if (!fragmentsource)
        exit(9910);
    //exit(4);
    if (geobit) //all geometry shit here
    {
        geometrysource = filetobuf((char*)geoshader);
        if (!geometrysource)
            exit(9911);
        gshader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(gshader, 1, (const GLchar**)&geometrysource, 0);
        glCompileShader(gshader);
        glGetShaderiv(gshader, GL_COMPILE_STATUS, &IsCompiled_FS);
        if(IsCompiled_FS == FALSE)
        {

            glGetShaderiv(gshader, GL_INFO_LOG_LENGTH, &maxLength);
            fragmentInfoLog = (char *)malloc(maxLength);
            glGetShaderInfoLog(gshader, maxLength, &maxLength, fragmentInfoLog);
            //output
            std::ofstream myfile;
            myfile.open("output.txt");
            myfile << fragmentInfoLog;
            //myfile << maxLength;
            myfile.close();
            free(fragmentInfoLog);
            exit(53);
        }
    }
    //exit(4);
    /*
    vertexshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);
    glCompileShader(vertexshader);
    */

    vshader = glCreateShader(GL_VERTEX_SHADER);
    //exit(4);
    glShaderSource(vshader, 1, (const GLchar**)&vertexsource, 0);
    glCompileShader(vshader);
    //exit(4);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &IsCompiled_VS);
    if(IsCompiled_VS == FALSE)
    {
        glGetShaderiv(vshader, GL_INFO_LOG_LENGTH, &maxLength);
        vertexInfoLog = (char *)malloc(maxLength);
        glGetShaderInfoLog(vshader, maxLength, &maxLength, vertexInfoLog);
        std::ofstream myfile;
        myfile.open("output.txt");
        myfile << fragmentInfoLog;
        myfile << maxLength;
        myfile << vertexsource;
        myfile.close();
        free(vertexInfoLog);
        exit(51);
    }

    fshader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fshader, 1, (const GLchar**)&fragmentsource, 0);

    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &IsCompiled_FS);
    if(IsCompiled_FS == FALSE)
    {

        glGetShaderiv(fshader, GL_INFO_LOG_LENGTH, &maxLength);
        fragmentInfoLog = (char *)malloc(maxLength);
        glGetShaderInfoLog(fshader, maxLength, &maxLength, fragmentInfoLog);
        std::ofstream myfile;
        myfile.open("output.txt");
        myfile << fragmentInfoLog;
        //myfile << maxLength;
        myfile.close();
        free(fragmentInfoLog);
        exit(52);
    }

    //start shader linking
    shaderprogram = glCreateProgram();

    glAttachShader(shaderprogram, vshader);
    if(geobit)
        glAttachShader(shaderprogram, gshader);
    glAttachShader(shaderprogram, fshader);
    //glBindAttribLocation(shaderprogram, 0, "vertex");
    //glBindAttribLocation(shaderprogram, 1, "modelUV");
    //int IsLinked; // more error checks
    glLinkProgram(shaderprogram);

    //glUseProgram(shaderprogram);

}
