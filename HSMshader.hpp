#include <GL/glew.h>
#define NO_SDL_GLEXT
#include <SDL_opengl.h>
#include "HSMmisc.hpp"

struct SHADER //this gets its own file asap.
{
    //loading will be a function, the shader will be a key here.
    GLuint vshader, fshader, gshader;
    GLuint shaderprogram;
    bool useGshader;

    public:
    SHADER();
    SHADER(const char *vertshader, const char *fragshader, const char *geoshader, bool geobit);
    ~SHADER();


};
