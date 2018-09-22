/*
Hazeron Ship Museum for use with .ship files (and for use with SODS, by Ikkir, not officially associated with Shores of Hazeron)

This is a program I made to learn GL 3.3 aka slightly more modern openGL, and some shader stuff, presented AS IS.
Featured loading in and displaying .ship files in 3d, with various test geometry to display possible room configurations in a proof-of-concept for ship redesigns.
Also featured exporting ships to .obj files and writing captured images to an FBO and exporting to .png


Requirements: SDL (pretty sure this one was SDL 2.0), SDL_image, GL, glm, zlib (for png stuff) and libpng, glew

*/


#include <vector>
#include <GL/glew.h>
#define NO_SDL_GLEXT

#include <SDL.h>
//#include <SDL_keycode.h>
#include <SDL_opengl.h>
#include "SDL_image.h"
#include "objloader.hpp"
#include "tangentspace.hpp"
#include "vboindexer.hpp"
#include "HSMshader.hpp"
#include "HSMship.hpp"

#include <glm/glm.hpp>
//#include <glm/gtc/matrix_projection.hpp>
//#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include "pngfuncs.h"
#include <glm/gtx/transform.hpp>

#define DECKHEIGHTM 20

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;
GLfloat rotmod = 0;
//
//The surfaces
//we need to change the screen to an SDL_Window.
SDL_Window *mainwindow = NULL;
SDL_Event event;

SDL_GLContext maincontext;
//SDL_Surface *testsurf;
//SDL_Texture *testtext;
//make controls: cam and shit
glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}

glm::vec3 position = glm::vec3( 4, 3, -3 );
// Initial horizontal angle : toward -Z
float horizontalAngle = 1.5716f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 0.02f; // 3 units / second
float mouseSpeed = 0.02f;
bool keyspressed[512];

std::vector <GLfloat> decklines;

Ship Ship1;
//textures
const int NUMTEXTURES = 1;
GLint Textures[NUMTEXTURES];

SHADER *HSMshader;


void initlines()
{
    for (int d = 0; d < 57; d++)
    {
        decklines.push_back(80.0f*5);
        decklines.push_back(80.0f*5);
        decklines.push_back((float)d*DECKHEIGHTM);
        decklines.push_back(80.0f*5);
        decklines.push_back(-80.0f*5);
        decklines.push_back((float)d*DECKHEIGHTM);
        decklines.push_back(-80.0f*5);
        decklines.push_back(-80.0f*5);
        decklines.push_back((float)d*DECKHEIGHTM);
        decklines.push_back(-80.0f*5);
        decklines.push_back(80.0f*5);
        decklines.push_back((float)d*DECKHEIGHTM);
        decklines.push_back(80.0f*5);
        decklines.push_back(80.0f*5);
        decklines.push_back((float)d*DECKHEIGHTM);
    }
}

struct DECKSVO
{
    GLuint decksvao;
    GLuint decksvbo;
    bool initdecks();

} decksvo;

bool DECKSVO::initdecks()
{
    //GLuint vao2;
    //glGenVertexArrays(1, &vao2);
    glGenVertexArrays(1, &decksvao);


    glBindVertexArray(decksvao);
    //GLuint vbo2;
    //glGenBuffers(1, &vbo2);
    glGenBuffers(1, &decksvbo);
    glBindBuffer(GL_ARRAY_BUFFER, decksvbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*decklines.size(), &decklines[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    return true;
}


void initkeys()
{
    for (int i = 0; i < 512; i++)
    {
        keyspressed[i]=0;
    }
}

void computeMatricesFromInputs()
{

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = SDL_GetTicks();

	// Compute time difference between current and last frame
	double currentTime = SDL_GetTicks();
	float deltaTime = float(currentTime - lastTime);
	//deltaTime = 1;

	// Get mouse position
	static int xpos = (SCREEN_WIDTH/2);
	static int ypos = (SCREEN_HEIGHT/2);
	SDL_GetMouseState(&xpos, &ypos);
	if (xpos == 0 && ypos == 0)
	{
	    xpos = SCREEN_WIDTH/2;
	    ypos = SCREEN_HEIGHT/2;
	}
	printf("XPOS %i", xpos);
	printf("  YPOS %i \n", ypos);

	// Reset mouse position for next frame
	SDL_WarpMouseInWindow(mainwindow, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);

	// Compute new orientation
	horizontalAngle += mouseSpeed * float(  SCREEN_WIDTH/2 - xpos );
	verticalAngle   += mouseSpeed * float( SCREEN_HEIGHT/2 - ypos );
	printf("%f", horizontalAngle);
	printf(" %f \n", verticalAngle);

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
	//glm::vec3 direction(0.0f, 0.0f, 0.0f);
	//std::ofstream myfile;
       // myfile.open("angles.txt");
       // myfile << direction.x << '\n';
       // myfile << direction.y << '\n';
       // myfile << direction.z << '\n';
       // myfile.close();
    //exit(7);
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f/2.0f),
		0,
		cos(horizontalAngle - 3.14f/2.0f)
	);

	// Up vector
	glm::vec3 up = glm::cross( right, direction );
	//int keysarray;
	//Uint8 *keystate = SDL_GetKeyboardState(&keysarray);
	//printf("%i", keysarray);
	//printf("%i", SDL_GetTicks());

    //if (keystate[SDLK_w])
    //keystate[SDLK_w] = true;
    if(keyspressed[SDLK_w])
    {
        position += direction * deltaTime * speed;
        //printf("%i", SDL_GetTicks());
    }
	if (keyspressed[SDLK_s])
    {
        position -= direction * deltaTime * speed;
    }

	if (keyspressed[SDLK_a])
    {
        position += right * deltaTime * speed;
    }

	if (keyspressed[SDLK_d])
    {
        position -= right * deltaTime * speed;
    }
    float FoV = initialFoV;
    //int count = SDL_PeepEvents(event, SDL_GETEVENT, SDL_EVENTMASK(SDL_MOUSEBUTTONDOWN));
        FoV = initialFoV - 5 * event.wheel.y;


	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 2000.0f);
	// Camera matrix
	ViewMatrix   =    //= glm::lookAt(
						//		position,           // Camera is here
					//			/*position+*/direction, // and looks here : at the same position, plus "direction"
					//			up                  // Head is up (set to 0,-1,0 to look upside-down)
					//	   );
                    glm::lookAt(
								position, // Camera is at (4,3,-3), in World Space
								position + direction, // and looks at the origin
								up  // Head is up (set to 0,-1,0 to look upside-down)
						   );
    printf("Position %f, %f, %f \n", position.x, position.y, position.z);
	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}



bool init()
{
    //Initialize all SDL subsystems
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 ) //remember, sdl_init_everything too
    {
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,8);
    //Set up the screen
//    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );
    mainwindow = SDL_CreateWindow("Test1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL| SDL_WINDOW_SHOWN);
   // mainrenderer = SDL_CreateRenderer(mainwindow, -1, SDL_RENDERER_ACCELERATED);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,8);

    /* Turn on double buffering with a 24bit Z buffer.
     * You may need to change this to 16 or 32 for your system */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    maincontext = SDL_GL_CreateContext(mainwindow);

    //If there was an error in setting up the screen
    if( mainwindow == NULL )
    {
        return 1;
    }

    //Set the window caption
//    SDL_WM_SetCaption( "Foo says \"Hello!\"", NULL );

    //If everything initialized fine
    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit())
    {
        exit(1);
    };
    SDL_GL_SetSwapInterval(1);
    return true;
}

bool initgl()
{
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); //still works
    glClearStencil(0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glOrtho(0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, 1.0, -1.0); //deprecated

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    //glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glClearStencil(0x0);
    return 1;
}

GLuint load_image(char *file)
{
    //SDL_Surface *tex = SDL_LoadBMP(file);
    SDL_Surface *tex = IMG_Load(file);
    GLuint texture;

    //printf("Status:  Loading image ");
    //printf(file);
    //printf("... ");

    if(tex)
    {
        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &texture);
        //glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->w, tex->h,
        0, GL_RGBA, GL_UNSIGNED_BYTE, tex->pixels);
//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex[i]->w, tex[i]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex[i]->pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        //glBindTexture(GL_TEXTURE_2D, NULL);
        printf("OK\n");
        SDL_FreeSurface(tex);

    }
    else
    {
        printf("Failed\nQuitting...");
        SDL_Quit();
        exit(-1);
    }
    //destroy texture thingy too to avoid memleak

    return texture;
}

bool load_files()
{
    Textures[0] = load_image((char*)"TxTileTest.png");
    if (Textures[0] == -1)
    {
        exit(8);
    }
    //Texture2 = load_image((char*)"normal.png");
    //if (Texture2 == -1)
    //{
    //    exit(8);
    //}
    return true;
}

void clean_up()
{
    //Free the surfaces
//    SDL_FreeSurface( background );
 //   SDL_FreeSurface( foo );

    //Quit SDL
    SDL_Quit();
}

//
GLuint LoadShaders(const char * vertshader, const char * fragshader, const char * geoshader, int geobit)
{
    int IsCompiled_VS, IsCompiled_FS; //error checks
    //int IsLinked; // more error checks
    int maxLength = 10000; //file loading?
    char *vertexInfoLog;
    char *fragmentInfoLog = 0;
    //char *shaderProgramInfoLog; //log dumps from errors
    GLchar *vertexsource, *fragmentsource, *geometrysource; //load into these from file
    GLuint vertexshader, fragmentshader, geometryshader;
    GLuint shaderprogram;

    vertexsource = filetobuf((char*)vertshader);
    if (!vertexsource)
        exit(9999);
    fragmentsource = filetobuf((char*)fragshader);
    if (!fragmentsource)
        exit(9910);
    geometrysource = filetobuf((char*)geoshader);
    if (!geometrysource)
        exit(9911);

    vertexshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);
    glCompileShader(vertexshader);

    glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &IsCompiled_VS);
    if(IsCompiled_VS == FALSE)
    {
        glGetShaderiv(vertexshader, GL_INFO_LOG_LENGTH, &maxLength);
        vertexInfoLog = (char *)malloc(maxLength);
        glGetShaderInfoLog(vertexshader, maxLength, &maxLength, vertexInfoLog);
        std::ofstream myfile;
        myfile.open("output.txt");
        myfile << fragmentInfoLog;
        myfile << maxLength;
        myfile << vertexsource;
        myfile.close();
        free(vertexInfoLog);
        exit(51);
    }

    fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);

    glCompileShader(fragmentshader);
    glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &IsCompiled_FS);
    if(IsCompiled_FS == FALSE)
    {

        glGetShaderiv(fragmentshader, GL_INFO_LOG_LENGTH, &maxLength);
        fragmentInfoLog = (char *)malloc(maxLength);
        glGetShaderInfoLog(fragmentshader, maxLength, &maxLength, fragmentInfoLog);
        std::ofstream myfile;
        myfile.open("output.txt");
        myfile << fragmentInfoLog;
        //myfile << maxLength;
        myfile.close();
        free(fragmentInfoLog);
        exit(52);
    }

    if(geobit)
    {
        geometryshader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryshader, 1, (const GLchar**)&geometrysource, 0);
        glCompileShader(geometryshader);
        glGetShaderiv(geometryshader, GL_COMPILE_STATUS, &IsCompiled_FS);
        if(IsCompiled_FS == FALSE)
        {

            glGetShaderiv(geometryshader, GL_INFO_LOG_LENGTH, &maxLength);
            fragmentInfoLog = (char *)malloc(maxLength);
            glGetShaderInfoLog(geometryshader, maxLength, &maxLength, fragmentInfoLog);
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
    //start shader linking
    shaderprogram = glCreateProgram();

    glAttachShader(shaderprogram, vertexshader);
    if(geobit)
        glAttachShader(shaderprogram, geometryshader);
    glAttachShader(shaderprogram, fragmentshader);
    //glBindAttribLocation(shaderprogram, 0, "vertex");
    //glBindAttribLocation(shaderprogram, 1, "modelUV");
    glLinkProgram(shaderprogram);
    //glUseProgram(shaderprogram);

    return shaderprogram;
}

//

enum {L_EOF = 1, BADROOM = 2};

long int loadFindID(unsigned char sh_ID[4], FILE *ship, int &err)
{
    long int idpos;
    if (feof(ship))
    {
        err = L_EOF;
        return 0;
    }
    int i = 0;
    unsigned char f_test = sh_ID[i];
    unsigned char f_num = 0x00;
    bool found = false;
    //bool c_test = false;

    while (!feof(ship))
    {
        if (found)
            break;
        f_num= fgetc(ship);
        if (f_num == f_test) //true for this value
        {
            i++;
            if (i >=4) // found
                found = true;
            else
                f_test = sh_ID[i];
        }
        else //not true, then we need to back to square one and keep testing
        {
            i = 0;
            f_test = sh_ID[i];
            if (f_num == f_test)
            {
                i++;
                f_test = sh_ID[i];
            }
        }
    }
    if (feof(ship))
        err = L_EOF;
    idpos = ftell(ship)-1;
    //if (found)
    //    fprintf(stdout, "Found %li", idpos);
    //else
    //    fprintf(stdout, "Error EOF %li", idpos);
    return idpos;
}

struct xyvert
{
    unsigned char x;
    unsigned char y;
};
xyvert *loadgetroom(unsigned char *info, int &ncorner, int &err, FILE *ship);
ShipRoom *loadsetroom(unsigned char info[5], int ncorner, xyvert corners[]);

bool buildwalls(ShipRoom *thisroom);
bool buildroof(ShipRoom *thisroom);
bool buildfloor(ShipRoom *thisroom, int ncorner, xyvert corners);

bool loaddesign() //probably need to add a char string here later, so i can pass in the addy. for now, use default
{
    std::ofstream myfile;
        myfile.open("LoadErrorLog.txt");
        myfile << "Going in!!!";


    //cleardesign();  //k, we load into Ship1
    FILE *shipload = NULL;
    shipload=fopen("Charond.ship", "rb");
    if (!shipload)
    {
        myfile << "Error opening file!!! '\n'";
        return false;
    }

    //check file length?
    long fsize = 0;
    {
        fseek(shipload, 0, SEEK_END);
        fsize = ftell(shipload);
        if (fsize < 40)
        {
            fclose(shipload);
            myfile << "Error Break";
            myfile.close();
            return false; // not a valid ship file:: too small
        }
        fseek(shipload, 0, SEEK_SET);
    }
    unsigned char sl_ID[4];
    for (int i = 0; i < 4; i++)
        sl_ID[i] = fgetc(shipload);
    fseek(shipload, 8, SEEK_CUR);
    //int junk = 0;
    Ship1.streamlined = fgetc(shipload);
    Ship1.armorlevel = fgetc(shipload);
    //if (mainship.streamlined>1)
    //    mainship.streamlined = 1;
    //if (mainship.armorlevel >10)
    //    mainship.armorlevel = 10;

    //need to find the characters and get the position: if theres an error, abort
    //func: long int loadFindID(char[4], FILE*, &err)
    //returns position: if eof found, sets error
    int err = 0;
    long int filepos = 0;
    filepos = loadFindID(sl_ID, shipload, err);
    if (err == L_EOF)
    {
        fclose(shipload);
        myfile << "Error Break 2";
        myfile.close();
        return false;
    }
    int g_rooms = 0;
    unsigned char f_g1;
    unsigned char f_g2;
    fseek (shipload, filepos - 5, SEEK_SET);
    f_g1 = fgetc (shipload);
    f_g2 = fgetc (shipload);

    //fprintf(stdout, " Pos1 %i Pos2 %i", f_g1, f_g2);
    g_rooms = (f_g1*0x100)+f_g2;
    //fprintf(stdout, "\n TotalRooms %i", g_rooms);

    if (g_rooms == 0)  //we assign g_rooms to the ship later, it could have bad rooms!
        return false; //abort, no rooms error
    fseek(shipload, 4, SEEK_CUR); //go past the room id and start reading in rooms.
    xyvert *r_verts;
    unsigned char *r_info = new unsigned char[5];
    //int err;
    int r_corner;

    for (int i = 0; i < g_rooms; i++)
    {
        r_verts = loadgetroom(r_info, r_corner, err, shipload);
        if (err == L_EOF)
            break;
        //fprintf(stderr, "\n %i %i %i %i ", r_info[3], r_info[4], r_info[2], r_corner);
        if (err == 0)
        { //heres the meat of the setup
            //I need to set shit right: decks[57].types[256].rooms[256]
            //Ship1.decks[decknum].roomtypes[typeid].rooms[roomid].verts[]
            //deckrooms[r_info[3]][r_info[4]][r_info[2]] = loadsetroom(r_info, r_corner, r_verts);
            //std::vector <GLfloat> *verts
            Ship1.decks[r_info[2]].roomtypes[r_info[3]].rooms[r_info[4]] = loadsetroom(r_info, r_corner, r_verts);
        }
        if (i < g_rooms -1)
            fseek(shipload, 4, SEEK_CUR);
    }
    if (err == L_EOF)
    {
        fclose(shipload);
        myfile << "Error break 3";
        myfile.close();
        return false;
    }

    for (int t = 0; t < 22; t++)
    {
        for (int r = 0; r < 256; r++)
        {
            for (int d = 0; d < 57; d++)
            {
               // finishroom2(deckrooms[t][r][d]);
            }
        }
    }

    delete[] r_info;
    if (feof(shipload))
    {
        fclose(shipload);
        myfile << "Error break 4";
        myfile.close();
        return true;
    }

    //if we arent done yet, we need to start placing equipment in the proper spots.
    //next 4 = equipunitsnum.  if eof, or 0, fail
    unsigned char sl_eq[4];
    //long int cpos = ftell(shipload);
    //fprintf(stdout, "\n CPOS %li", cpos);
    for (int i = 0; i < 4; i++)
    {
        sl_eq[i] = fgetc(shipload);
    }
    if (feof(shipload))
    {
        fclose(shipload);
        myfile << "Error break 5";
        myfile.close();
        return true;
    }
    long t_eq = (sl_eq[0]*0x1000000)+(sl_eq[1]*0x10000)+(sl_eq[2]*0x100)+sl_eq[3];
    if (t_eq == 0)
    {
        fclose(shipload);
        myfile << "Error break 6";
        myfile.close();
        return true;
    }


    //fprintf(stdout, "\n TEQ %li", t_eq);
    // we have n equipment to load: make a function to start grabbing, then assign if theres no errors.
    // should probably uss the addequip function simply after ripping the proper data.
    //unsigned char e_x;
    //unsigned char e_y;
    //unsigned char e_deck;
    //unsigned char e_type;
    //int e_rID;
    //long int e_num;
    //short int modtype;


    for (long int i = 0; i < t_eq && i < 65536; i++)
    {
//        loadgetequip(e_x, e_y, e_deck, e_type, e_rID, modtype, e_num, err, shipload);
        if (err == L_EOF)
            break;
        if (err == 0)
        {
//            currentdeck = e_deck;
//            equiptype = e_type;
//            currentrot = 0;
//            AddEquip2(e_x*5, 800-e_y*5, currentdeck, 0, modtype);
        }
    }
//    currentdeck = 28;
//    equiptype = 0x00;
//    currentrot = 0;

    fclose(shipload);
    myfile << "Coming out!!!";
    myfile.close();
    return true;


}
/*
void loadgetequip(unsigned char &l_x, unsigned char &l_y, unsigned char &l_deck, unsigned char &l_type, int &l_rID, short int &l_emod, long int &l_enum, int &err, FILE *ship)
{
    err = 0;
    l_deck = fgetc(ship);
    l_deck += 28;
    l_type = fgetc(ship);
    fseek(ship, 1, SEEK_CUR); //skip next, then more stuff
    unsigned char eqnum[4];
    for (int i = 0; i < 4; i++)
    {
        eqnum[i]= fgetc(ship);
    }
    //fprintf(stdout, "\n %i %i", l_deck, l_type);
    //fprintf(stdout, "\n %i %i %i %i", eqnum[0], eqnum[1], eqnum[2], eqnum[3]);
    l_enum = (eqnum[0]*0x1000000)+(eqnum[1]*0x10000)+(eqnum[2]*0x100)+eqnum[3];
    fseek(ship, 4, SEEK_CUR);
    l_x = fgetc(ship);
    l_y = fgetc(ship);
    l_x += 80;
    l_y += 80;
    unsigned char r1;
    unsigned char r2;
    r1 = fgetc(ship);
    r2 = fgetc(ship);
    l_rID = (r1*0x100)+r2;
    //these next two are mod stuff, i might have to get these later
    unsigned char m1;
    unsigned char m2;
    m1 = fgetc(ship);
    m2 = fgetc(ship);
    short int modtype;
    modtype = m1*0x100 + m2;
    //fseek(ship, 2, SEEK_CUR);
    if (l_deck > 56 || l_x > 160 || l_y > 160)
        err = BADROOM;
    //type tests
    if (l_type != MAN_U &&
        l_type != POW_U &&
        l_type != SHI_U &&
        l_type != SEN_U &&
        l_type != FTL_U &&
        l_type != WEA_U &&
        l_type != LIF_U &&
        l_type != MED_U &&
        l_type != TRA_U &&
        l_type != BED_U &&
        l_type != BED2_U &&
        !(l_type >= BED_U_UR && l_type <= BED_U_DR) &&
        !(l_type >= BED2_U_UR && l_type <= BED2_U_DR) &&
        l_type != DOOR_H &&
        l_type != DOOR_C &&
        l_type != DOOR_P &&
        l_type != DOORL_H &&
        l_type != DOORL_C &&
        l_type != DOORL_P &&
        l_type != HATCH_H &&
        !(l_type >= HATCH_H_UR &&  l_type <= HATCH_H_DR) &&
        l_type != HATCH_C &&
        !(l_type >= HATCH_C_UR &&  l_type <= HATCH_C_DR) &&
        l_type != HATCH_P &&
        !(l_type >= HATCH_P_UR &&  l_type <= HATCH_P_DR) &&
        l_type != OPENING_I &&
        l_type != OPENING_E &&
        l_type != RAIL &&
        l_type != PIVOT &&
        l_type != EMIT_C &&
        l_type != EMIT_W &&
        l_type != TABLE_G &&
        l_type != V_A_T &&
        l_type != V_A_F1 &&
        l_type != V_A_F2 &&
        l_type != V_G_M &&
        l_type != V_G_S &&
        l_type != V_G_A &&
        //l_type != GUN_T_CAN &&
        l_type != T_SIDE_R &&
        !(l_type >= T_SIDE_UR &&  l_type <= T_SIDE_DR) &&
        //l_type != GUN_S_CAN &&
        l_type != T_TOP &&
        l_type != T_BOT &&
        l_type != CAP_C &&
        l_type != HEL_C &&
        l_type != ENG_C &&
        l_type != SHI_C &&
        l_type != SEN_C &&
        l_type != NAV_C &&
        l_type != WEA_C &&
        l_type != POW_C &&
        l_type != MED_C &&
        l_type != TRA_C)
        err = BADROOM;


//enum  {MAN_U = 0x0A, POW_U = 0x0E, SHI_U = 0x12, SEN_U = 0x10, FTL_U = 0x18, WEA_U = 0x17, LIF_U = 0x20, MED_U = 0x0B,
//       TRA_U = 0x14, BED_U = 0x41, BED2_U = 0x02, DOOR_H = 0x32, DOOR_C = 0x31, DOOR_P = 0x05, DOORL_H = 0x03, DOORL_C = 0x36,
//       DOORL_P = 0x35, HATCH_H = 0x34, HATCH_C = 0x33, HATCH_P = 0x08, OPENING_I = 0x37, OPENING_E = 0x09, RAIL = 0x38,
//       PIVOT = 0x46, EMIT_C = 0x62, EMIT_W = 0x63, TABLE_G = 0x00, V_A_T = 0x00, V_A_F1 = 0x00, V_A_F2 = 0x00, V_G_M = 0x00,
//       V_G_S = 0x00, V_G_A = 0x00, GUN_T_CAN = 0x00, GUN_S_CAN = 0x00, CAP_C = 0x49, HEL_C = 0x0C, ENG_C = 0x06, SHI_C = 0x11,
//       SEN_C = 0x0F, NAV_C = 0x01, WEA_C = 0x07, POW_C = 0x0D, MED_C = 0x04, TRA_C = 0x13, DUMMY = 0xFF };

    if (feof(ship))
        err = L_EOF;



    return;
}
*/
//
/*
void testrender()
{
    //make a simple vao, etc, and test rendering

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearStencil(0);
    //GLuint shaderprogram = LoadShaders("HSMVert.glsl", "HSMFrag.glsl", "HSMGeo.glsl",0);
    //GLuint shaderprogram = LoadShaders("normal.vert", "normal.frag", "normal.geo");
    GLuint shaderprogram = HSMshader->shaderprogram;
    GLuint vao;
    glGenVertexArrays(1, &vao);
    //glBindVertexArray(vao);
    std::vector <GLfloat> *vpointer;
    std::vector <GLfloat> *vpointerUV;

    GLuint vao2;
    glGenVertexArrays(1, &vao2);
    //glBindVertexArray(vao2);
    GLuint vao3;
    glGenVertexArrays(1, &vao3);

    GLuint vao4;
    glGenVertexArrays(1, &vao4);
    std::vector <GLfloat> *vpointer2;
    std::vector <GLfloat> *vpointer2UV;

    GLuint vao5;
    glGenVertexArrays(1, &vao5);
    std::vector <GLfloat> *vpointer3;



    unsigned int vcount;
   // float lookatthis[3];
   // int lookcount = 0;

    for (int n = 0; n < 57; n++)
    {
        for (int m = 0; m < 256; m++)
        {
            for (int o = 0; o < 256; o++)
            {
                if (Ship1.decks[n].roomtypes[m].rooms[o])
                {
                    vpointer = &Ship1.decks[n].roomtypes[m].rooms[o]->floor;
                    vpointerUV = &Ship1.decks[n].roomtypes[m].rooms[o]->floorUV;
                    vpointer2 = &Ship1.decks[n].roomtypes[m].rooms[o]->wallverts;
                    vpointer2UV = &Ship1.decks[n].roomtypes[m].rooms[o]->wallvertsUV;
                    vpointer3 = &Ship1.decks[n].roomtypes[m].rooms[o]->ceilingverts;
                    vcount = Ship1.decks[n].roomtypes[m].rooms[o]->vertcount;
                    //for (int i = 0; i < thisship->decks[n].roomtypes[m].rooms[o]->vertcount * 3 ; i++)
                        //myfile << thisship->decks[n].roomtypes[m].rooms[o]->floor[i] << "\n";

                }
            }
        }
    }
    glBindVertexArray(vao);
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo); //bind to 1st buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vpointer->size(), &vpointer->at(0), GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    GLuint vboUV;
    glGenBuffers(1, &vboUV);
    glBindBuffer(GL_ARRAY_BUFFER, vboUV); //bind to UV buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vpointerUV->size(), &vpointerUV->at(0), GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);


    glBindVertexArray(vao2);
    GLuint vbo2;
    glGenBuffers(1, &vbo2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*decklines.size(), &decklines[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    //change vao3 to draw the walls
    glBindVertexArray(vao3);
    GLuint vbo3;
    glGenBuffers(1, &vbo3);
    std::vector <GLfloat> derpderp;
    {
        derpderp.push_back(-1.0f);
        derpderp.push_back(-1.0f);
        derpderp.push_back(-1.0f);

        derpderp.push_back(-1.0f);
        derpderp.push_back(-1.0f);
        derpderp.push_back(1.0f);

        derpderp.push_back(-1.0f);
        derpderp.push_back(1.0f);
        derpderp.push_back(1.0f);

        derpderp.push_back(1.0f);
        derpderp.push_back(1.0f);
        derpderp.push_back(-1.0f);
        //-1.0f,-1.0f,-1.0f,
		//-1.0f,-1.0f, 1.0f,
		//-1.0f, 1.0f, 1.0f,
		// 1.0f, 1.0f,-1.0f,
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo3);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*derpderp.size(), &derpderp[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(vao4);
    GLuint vbo4;
    glGenBuffers(1, &vbo4);
    glBindBuffer(GL_ARRAY_BUFFER, vbo4);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vpointer2->size(), &vpointer2->at(0), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    GLuint vbo4UV;
    glGenBuffers(1, &vbo4UV);
    glBindBuffer(GL_ARRAY_BUFFER, vbo4UV); //bind to UV buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vpointer2UV->size(), &vpointer2UV->at(0), GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(vao5);
    GLuint vbo5;
    glGenBuffers(1, &vbo5);
    glBindBuffer(GL_ARRAY_BUFFER, vbo5);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vpointer3->size(), &vpointer3->at(0), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    //
    GLint mMVPLocation = 0;
    GLint mModelMatrixLocation = 0;
    GLint mViewMatrixLocation = 0;
    GLint mModelView3x3MatrixLocation = 0;
   // GLint mLightPositionLocation = 0;
   // GLint mSamplerTextureLocation = 0;
   // GLint mSamplerNormalLocation = 0;


    //mSamplerNormalLocation = glGetUniformLocation(shaderprogram, "normalsampler");
    //if(mSamplerNormalLocation == -1)
    //{
        //exit(99);
    //}
    GLint mSamplerTextureLocation = glGetUniformLocation(shaderprogram, "texsampler");
    if(mSamplerTextureLocation == -1)
    {
        exit(100);
    }

    GLint mTextDraw = glGetUniformLocation(shaderprogram, "usetext");
    if(mTextDraw == -1)
    {
        exit(100);
    }

    mMVPLocation = glGetUniformLocation(shaderprogram, "MVP");
    if(mMVPLocation == -1)
    {
        exit(4);
    }
    mModelMatrixLocation = glGetUniformLocation(shaderprogram, "modelmat");
    if(mModelMatrixLocation  == -1)
    {
       // exit(6);
    }
    mViewMatrixLocation = glGetUniformLocation(shaderprogram, "viewmat");
    if(mViewMatrixLocation == -1)
    {
       // exit(6);
    }
    mModelView3x3MatrixLocation = glGetUniformLocation(shaderprogram, "mv3mat");
    if(mModelView3x3MatrixLocation == -1)
    {
       // exit(6);
    }
    //mLightPositionLocation = glGetUniformLocation(shaderprogram, "LightPosWorld");
    //if(mLightPositionLocation == -1)
    //{
       // exit(6);
    //}
    //
    //

    glUseProgram(shaderprogram);
    //GLchar derpput[22];
    //glGetActiveAttrib(shaderprogram,0,20,0,0,0,derpput);
    //std::ofstream myfile;
    //myfile.open("output.txt");
    //myfile << derpput;
    //myfile.close();

    glm::mat4 mProjectionMatrix;
    glm::mat4 mModelViewMatrix;
    glm::mat4 mMVP;

    mProjectionMatrix = //glm::perspective<GLfloat>(45.0f, 1.0f, .1f, 100000.0f);
    //glm::ortho<GLfloat>(0.0f, 800.0f, 600.0f, 0.0f, -5.0f, 5000.0f);
    glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
   // mProjectionMatrix = getProjectionMatrix();
   //glm::mat4 mViewMatrix =// glm::lookAt(glm::vec3(0, 0, 10),glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
   //glm::mat4(1);
   //glm::lookAt(
	//							glm::vec3(40,30,-30), // Camera is at (4,3,-3), in World Space
	//							glm::vec3(0,0,0), // and looks at the origin
	//							glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
	//					   );
   //glm::mat4 mViewMatrix = glm::lookAt(glm::vec3(0, -10, 0),glm::vec3(lookatthis[0],lookatthis[1],lookatthis[2]), glm::vec3(0, 1, 0));
    glm::mat4 mViewMatrix = getViewMatrix();
    glm::mat4 mModelMatrix = glm::mat4(1.0);
//   glm::mat4 mModelMatrix = glm::mat4(1.0);
    //mModelMatrix= glm::rotate<GLfloat>(mModelMatrix, rotmod, 0, 1, 1);
    //mModelMatrix= glm::rotate<GLfloat>(mModelMatrix, rotmod, 1, 0, 0);
    //mModelMatrix= glm::rotate<GLfloat>(mModelMatrix, rotmod, 0, 0, 1);

    mModelViewMatrix = mViewMatrix * mModelMatrix;
    //mModelViewMatrix = glm::rotate<GLfloat>(mModelViewMatrix, rotmod, 1, 0, 0);
    //mModelViewMatrix = glm::rotate<GLfloat>(mModelViewMatrix, rotmod, 1, 0, 0);
    glm::mat3 mModelView3x3Matrix = glm::mat3(mModelViewMatrix);
    mMVP = mProjectionMatrix * mViewMatrix * mModelMatrix;
    //mMVP = glm::rotate<GLfloat>(mMVP, rotmod, 1, 0, 0);

    //glm::vec3 mLightPosition = glm::vec3(1);

    GLint mMVPLocation = 0;
    GLint mModelMatrixLocation = 0;
    GLint mViewMatrixLocation = 0;
    GLint mModelView3x3MatrixLocation = 0;
    GLint mLightPositionLocation = 0;

uniform mat4 MVP;
uniform mat4 modelmat;
uniform mat4 viewmat;
uniform mat3 mv3mat;
uniform vec3 LightPosWorld = vec3(1);

    glUniformMatrix4fv(mProjectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(mProjectionMatrix)); //projmat
    glUniformMatrix4fv(mMVPLocation, 1, GL_FALSE, glm::value_ptr(mMVP)); //MVP
    glUniformMatrix4fv(mModelMatrixLocation, 1, GL_FALSE, glm::value_ptr(mModelViewMatrix)); //modelmat
    glUniformMatrix4fv(mViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(mViewMatrix)); //viewmat
    glUniformMatrix3fv(mModelView3x3MatrixLocation, 1, GL_FALSE, glm::value_ptr(mModelView3x3Matrix)); //mv3x3mat
    //glUniformMatrix3fv(mLightPositionLocation, 1, GL_FALSE, glm::value_ptr(mLightPosition));

    glUniform1i(mTextDraw, 0);

    //make matrices, grab the first vert, and look at it. that way, we can actually see
    //where the hell it is.
    //we also need to set up window, etc.
    //glBindVertexArray(vao);
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, decklines.size());
   // glDrawArrays(GL_TRIANGLES, 0, vpointer->size()/3);
    //glBindVertexArray(vao2);
    //glDrawArrays(GL_LINE_LOOP, 0, decklines.size()/3);
    //glBindVertexArray(vao);
    //glDrawArrays(GL_TRIANGLE_FAN, 0, vpointer->size()/3);

    glBindVertexArray(vao2);
    glDrawArrays(GL_LINE_LOOP, 0, decklines.size()/3);

    //glDrawArrays(GL_TRIANGLES, 0, 12*3);
    //glBindVertexArray(vao3);
    //glDrawArrays(GL_TRIANGLES, 0, 4*3);

    glUniform1i(mTextDraw, 1);
    glBindVertexArray(vao4);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vpointer2->size()/3);

    //glBindVertexArray(vao5);
    //glDrawArrays(GL_TRIANGLE_FAN, 0, vpointer3->size()/3);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Textures[0]);
    glUniform1i(mSamplerTextureLocation, 0);
    glUniform1i(mTextDraw, 1);

    Ship1.drawall();

    {

        glDepthMask(GL_FALSE);
        glEnable(GL_STENCIL_TEST);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // disable writing to color buffer
        glStencilFunc(GL_ALWAYS, 0x1, 0x1);
        glStencilOp(GL_KEEP, GL_INVERT, GL_INVERT);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, vpointer->size()/3);

        // PASS 2: draw color buffer
        // Draw again the exact same polygon to color buffer where the stencil
        // value is only odd number(1). The even(0) area will be descarded.
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);    // enable writing to color buffer
        glStencilFunc(GL_EQUAL, 0x1, 0x1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
        glDepthMask(GL_TRUE);


        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, vpointer->size()/3);

        glDisable(GL_STENCIL_TEST);
        glDepthMask(GL_TRUE);
    }


    glEnable(GL_STENCIL_TEST);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // disable writing to color buffer
        glStencilFunc(GL_ALWAYS, 0x1, 0x1);
        glStencilOp(GL_KEEP, GL_INVERT, GL_INVERT);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, vpointer->size()/3);

        // PASS 2: draw color buffer
        // Draw again the exact same polygon to color buffer where the stencil
        // value is only odd number(1). The even(0) area will be descarded.
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);    // enable writing to color buffer
        glStencilFunc(GL_EQUAL, 0x1, 0x1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, vpointer->size()/3);

        glDisable(GL_STENCIL_TEST);



    SDL_GL_SwapWindow(mainwindow);

    glUseProgram(0); //cleanup
    glDisableVertexAttribArray(0);
//   glDisableVertexAttribArray(1);
//    glDetachShader(shaderprogram, vertexshader);
//    glDetachShader(shaderprogram, fragmentshader);
    //glDeleteProgram(shaderprogram);
//    glDeleteShader(vertexshader);
//    glDeleteShader(fragmentshader);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &vboUV);
    glDeleteBuffers(1, &vbo2);
    glDeleteBuffers(1, &vbo3);
    glDeleteBuffers(1, &vbo4);
    //glDeleteBuffers(1, &uvbuffer);
    //glDeleteBuffers(1, &elementbuffer);

    //free(vertexsource);
    //free(fragmentsource);

    return;
} */

void testrender2(SHADER *shaderprog)
{
    //this is after categorizing things so its not just one big clusterfuck
    //shader(x/y/z) already declared, no need to load shaders, we just use it.
    //make a simple vao, etc, and test rendering

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearStencil(0);
    GLuint shaderprogram = shaderprog->shaderprogram;

    GLint mMVPLocation = 0;
    GLint mModelMatrixLocation = 0;
    GLint mViewMatrixLocation = 0;
    GLint mModelView3x3MatrixLocation = 0;

  //these should be part of shader, no?
  //or at least a subset of it. array of uniform objects with char 'names' ideally
  //right now, use as is because get it working asap
    GLint mSamplerTextureLocation = glGetUniformLocation(shaderprog->shaderprogram, "texsampler");
    if(mSamplerTextureLocation == -1)
    {
        exit(100);
    }

    GLint mTextDraw = glGetUniformLocation(shaderprog->shaderprogram, "usetext");
    if(mTextDraw == -1)
    {
        exit(100);
    }

    mMVPLocation = glGetUniformLocation(shaderprog->shaderprogram, "MVP");
    if(mMVPLocation == -1)
    {
        exit(4);
    }
    mModelMatrixLocation = glGetUniformLocation(shaderprog->shaderprogram, "modelmat");
    if(mModelMatrixLocation  == -1)
    {
       // exit(6);
    }
    mViewMatrixLocation = glGetUniformLocation(shaderprog->shaderprogram, "viewmat");
    if(mViewMatrixLocation == -1)
    {
       // exit(6);
    }
    mModelView3x3MatrixLocation = glGetUniformLocation(shaderprog->shaderprogram, "mv3mat");
    if(mModelView3x3MatrixLocation == -1)
    {
       // exit(6);
    }
    glUseProgram(shaderprog->shaderprogram);

    glm::mat4 mProjectionMatrix;
    glm::mat4 mModelViewMatrix;
    glm::mat4 mMVP;

    mProjectionMatrix = //glm::perspective<GLfloat>(45.0f, 1.0f, .1f, 100000.0f);
   glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
   glm::mat4 mViewMatrix = getViewMatrix();
    glm::mat4 mModelMatrix = glm::mat4(1.0);

    mModelViewMatrix = mViewMatrix * mModelMatrix;

    glm::mat3 mModelView3x3Matrix = glm::mat3(mModelViewMatrix);
    mMVP = mProjectionMatrix * mViewMatrix * mModelMatrix;
    glUniformMatrix4fv(mMVPLocation, 1, GL_FALSE, glm::value_ptr(mMVP)); //MVP
    glUniformMatrix4fv(mModelMatrixLocation, 1, GL_FALSE, glm::value_ptr(mModelViewMatrix)); //modelmat
    glUniformMatrix4fv(mViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(mViewMatrix)); //viewmat
    glUniformMatrix3fv(mModelView3x3MatrixLocation, 1, GL_FALSE, glm::value_ptr(mModelView3x3Matrix)); //mv3x3mat

    glUniform1i(mTextDraw, 0);

    glBindVertexArray(decksvo.decksvao);
    glDrawArrays(GL_LINE_LOOP, 0, decklines.size()/3);

    glUniform1i(mTextDraw, 1);

    Ship1.drawall();
    //ship.draw
    //now, for rendering. each array set is a function: use array or elements or whatever, then do
    //this or that or whatev.
    //first, build arrays: we already do this with the ship loader.
    //then, you need to make vaos for each section of each room, with their vbos as well
    //so, make arrays, then make vao for floor, wall, and roof
    //then, drawroof and drawfloor have to use stencil and triangle fan
    //while drawwalls uses trianglestrip for the basic wall
    //possibly another method for other walls I experiment with, I shall
    //find out!
    //TODO: flesh out vaos, vbos, and room drawing methods
    //and make functions to draw all rooms
    //for that matter, make shit into proper class!
/*
    {

        glDepthMask(GL_FALSE);
        glEnable(GL_STENCIL_TEST);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // disable writing to color buffer
        glStencilFunc(GL_ALWAYS, 0x1, 0x1);
        glStencilOp(GL_KEEP, GL_INVERT, GL_INVERT);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, vpointer->size()/3);

        // PASS 2: draw color buffer
        // Draw again the exact same polygon to color buffer where the stencil
        // value is only odd number(1). The even(0) area will be descarded.
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);    // enable writing to color buffer
        glStencilFunc(GL_EQUAL, 0x1, 0x1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
        glDepthMask(GL_TRUE);


        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, vpointer->size()/3);

        glDisable(GL_STENCIL_TEST);
        glDepthMask(GL_TRUE);
    }
*/

    SDL_GL_SwapWindow(mainwindow);

    glUseProgram(0); //cleanup

    return;

}

bool setupship(Ship *thisship)
{
    for (int n = 0; n < 57; n++)
    {
        thisship->armorlevel = 0;
        thisship->deckcount = 57;  //do i need this?
        thisship->streamlined = 0;
        for (int m = 0; m < 256; m++)
        {
            for (int o = 0; o < 256; o++)
            {
                thisship->decks[n].roomtypes[m].rooms[o] = NULL;
            }
        }
    }
    return true;
}

bool buildwalls(ShipRoom *thisroom)
{
    //if verts has size 0, exit
    if (thisroom->floor.size()<9)
        return 0;
    //triangle strip, s, up, over-s, up-over, etc.
    GLfloat xUV = 0.0f;
    GLfloat yUV = 0.0f;
    for (unsigned int i = 0; i < thisroom->floor.size()/3; i++) //4 has 12
    {
        thisroom->wall.push_back(thisroom->floor[i*3]);
        thisroom->wall.push_back(thisroom->floor[i*3+1]);
        thisroom->wall.push_back(thisroom->floor[i*3+2]);
        thisroom->wallUV.push_back(xUV);
        thisroom->wallUV.push_back(yUV);

        thisroom->wall.push_back(thisroom->floor[i*3]);
        thisroom->wall.push_back(thisroom->floor[i*3+1]);
        thisroom->wall.push_back(thisroom->floor[i*3+2]+DECKHEIGHTM);
        thisroom->wallUV.push_back(xUV);
        thisroom->wallUV.push_back(yUV+1.0f);

        if (xUV <= 0.1f)
            xUV = 1.0f;
        else
            xUV = 0.0f;
    }
    thisroom->wall.push_back(thisroom->floor[0]);
    thisroom->wall.push_back(thisroom->floor[1]);
    thisroom->wall.push_back(thisroom->floor[2]);
    thisroom->wallUV.push_back(xUV);
    thisroom->wallUV.push_back(yUV);

    thisroom->wall.push_back(thisroom->floor[0]);
    thisroom->wall.push_back(thisroom->floor[1]);
    thisroom->wall.push_back(thisroom->floor[2]+DECKHEIGHTM);
    thisroom->wallUV.push_back(xUV);
    thisroom->wallUV.push_back(yUV+1.0f);
    return true;
}

bool buildroof(ShipRoom *thisroom)
{
    printf("%i \n", thisroom->vertcount);
    for (unsigned int i = 0; i < thisroom->vertcount; i++)
    {
        //push values onto the verts from xyverts corners[] using ncorner as count
        //eg: push x[0][1][2]  for xyz
        //AddVLINK(corners[i].x*5, 800-corners[i].y*5, nroom);
        thisroom->ceiling.push_back(thisroom->floor.at(3*i));
        thisroom->ceiling.push_back(thisroom->floor.at(3*i+1));
        thisroom->ceiling.push_back(thisroom->floor.at(3*i+2)+DECKHEIGHTM);

        thisroom->ceilingUV.push_back(thisroom->floorUV.at(2*i));
        thisroom->ceilingUV.push_back(thisroom->floorUV.at(2*i+1));
    }

    return true;
}

bool buildfloor(ShipRoom *thisroom, int ncorner, xyvert corners[])
{
    std::ofstream myfile;
    myfile.open("UVS.txt");
    for (int i = 0; i < ncorner; i++)
    {
        //push values onto the verts from xyverts corners[] using ncorner as count
        //eg: push x[0][1][2]  for xyz
        //AddVLINK(corners[i].x*5, 800-corners[i].y*5, nroom);
        thisroom->floor.push_back(400-(GLfloat)corners[i].x*5);
        thisroom->floor.push_back(400-(GLfloat)corners[i].y*5);
        thisroom->floor.push_back((GLfloat)thisroom->decknum*DECKHEIGHTM);
        thisroom->vertcount ++;
        thisroom->floorUV.push_back((GLfloat)corners[i].x/2);
        thisroom->floorUV.push_back((GLfloat)corners[i].y/2);

        myfile << thisroom->floorUV[2*i] << ' ' <<thisroom->floorUV[2*i+1] << '\n';

    }
    myfile.close();
    return true;
}


//loadshader: struct that holds the global shader shit.

struct VAO
{

} vaos;
//



bool printoutship(Ship *thisship)
{
    std::ofstream myfile;
        myfile.open("Shipprintout.txt");

    myfile << thisship->armorlevel;
    myfile << thisship->deckcount;  //do i need this?
    myfile << thisship->streamlined << "\n";
    for (int n = 0; n < 57; n++)
    {
        for (int m = 0; m < 256; m++)
        {
            for (int o = 0; o < 256; o++)
            {
                if (thisship->decks[n].roomtypes[m].rooms[o])
                {
                    for (unsigned int i = 0; i < thisship->decks[n].roomtypes[m].rooms[o]->vertcount * 3 ; i++)
                        myfile << thisship->decks[n].roomtypes[m].rooms[o]->floor[i] << "\n";

                }
            }
        }
    }
    myfile.close();
    return true;

}

ShipRoom *loadsetroom(unsigned char info[5], int ncorner, xyvert corners[])
{
    if (ncorner == 0 || corners == NULL)
        return NULL;
    //rooms_g[globalnum-1]
    unsigned char g1 = info[0];
    unsigned char g2 = info[1];

    int gnum = (g1*0x100)+g2;
//    rooms_g[gnum-1] = true;
    ShipRoom *nroom = new ShipRoom; //turn into std::vector <GLfloat> cause im makin verts //problem, everything else gets set here too!!!
    //nroom->f_legal = 1; //roomnumber roomtype globalnum vertcount decknum
    nroom->decknum = info[2];
    nroom->globalnum = gnum;
    nroom->roomnum = info[4];
    nroom->roomtype = info[3];
    nroom->vertcount = 0;
    //nroom->lverts = NULL;  verts is a vector, default constructor sets verts.size() to 0
    //currentdeck = nroom->decknum;


    buildfloor(nroom, ncorner, corners);
    buildwalls(nroom);
    buildroof(nroom);
    return nroom;




}

xyvert *loadgetroom(unsigned char *info, int &ncorner, int &err, FILE *ship)
{// we start this at a good id, so just start loading in.
    err = 0;
    if (!ship)
    {
        err = L_EOF;
        return NULL;
    }
    for (int i = 0; i < 5; i++)
    {
        info[i] = fgetc(ship);
    }
    unsigned char c1;
    unsigned char c2;
    c1 = fgetc(ship);
    c2 = fgetc(ship);
    ncorner = (c1*0x100)+c2;

    xyvert *averts;
    averts = new xyvert[ncorner];
    for (int i = 0; i < ncorner; i++)
    {
        averts[i].x = fgetc(ship);
        averts[i].y = fgetc(ship);
        averts[i].x += 80;
        averts[i].y += 80;
    }
    if (feof(ship))
    {
        err = L_EOF;
        delete[] averts;
        return NULL;
    }
    for (int i = 0; i < ncorner; i++)
    {
        if (averts[i].x > 160 || averts[i].y > 160)
        {
            err = BADROOM;
            delete[]averts;
            return NULL;
        }
    }
    //now we do various tests.  first, test info members: only one we are concerned about is roomtype really.
    //but we need to convert the decknumber into one we can use.
    info[2] += 28;
    if (info[3] > 21 ||
        info[3] == 0 ||
        info[3] == 12 ||
        ncorner == 0 ||
        ((info[0]*0x100)+info[1]) == 0)
        {
            err = BADROOM;
            delete[]averts;
            return NULL;
        }


    return averts;
}



int main( int argc, char* args[] )
{
    //Quit flag
    bool quit = false;

    //Initialize
    if( init() == false )
    {
        return 3;
    }
    initgl();
    //Load the files
    if( load_files() == false )
    {
        return 2;
    }
//    bool res = loadOBJ("cylinder.obj", vertices, uvs, normals);

   SDL_GL_SwapWindow(mainwindow);
   setupship(&Ship1);
    loaddesign();
    printoutship(&Ship1);

    //While the user hasn't quit
    initkeys();
    initlines();
    computeMatricesFromInputs();

    Ship1.buildall();
    decksvo.initdecks();
    HSMshader = new SHADER("HSMVert.glsl", "HSMFrag.glsl", "HSMGeo.glsl",0);
    while( quit == false )
    {
        //While there's events to handle
        //SDL_PumpEvents();
        computeMatricesFromInputs();
        while( SDL_PollEvent( &event ) )
        {

            //If the user has Xed out the window
            if( event.type == SDL_QUIT )
            {
                //Quit the program
                quit = true;
            }
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    quit = true;
                keyspressed[event.key.keysym.sym] = true;
                //keyspressed[SDLK_w] = true;
                //printf("%i", SDLK_w);
            }
            if (event.type == SDL_KEYUP)
            {
                keyspressed[event.key.keysym.sym] = 0;
            }
        }


            //testrender();
            testrender2(HSMshader);

        SDL_Delay(5);
    }
    //Free the surfaces and quit SDL
    clean_up();

    return 0;
}
