#include <GL/glew.h>
#define NO_SDL_GLEXT
#include <SDL_opengl.h>
//#include <stdlib.h>
#include "HSMmisc.hpp"
//#include <vector>

class ShipRoom
{
    public:
    std::vector<GLfloat> floor;  //for now, just the verts on floor, deckn is just another vert
    std::vector<GLfloat> floorUV;
    GLuint floorvao;
    GLuint floorvbo;
    GLuint floorUVvbo;
    std::vector<GLfloat> wall; //these get built in a seperate step
    std::vector<GLfloat> wallUV;
    GLuint wallvao;
    GLuint wallvbo;
    GLuint wallUVvbo;
    std::vector<GLfloat> ceiling; //also seperate step
    std::vector<GLfloat> ceilingUV;
    GLuint ceilingvao;
    GLuint ceilingvbo;
    GLuint ceilingUVvbo;
    unsigned char roomnum;
    unsigned char roomtype;
    unsigned char globalnum;
    unsigned char decknum;
    unsigned int vertcount;

    bool drawroom();
    bool drawfloor();
    bool drawwall();
    bool drawceiling();
    bool buildroomv();
    bool buildfloorv();
    bool buildwallv();
    bool buildceilingv();
};

struct ShipRoomType
{
    ShipRoom *rooms[256];
    //unsigned int roomtype;
    //unsigned int roomcount;
    //unsigned int roomtype;

};

struct ShipDeck
{
    ShipRoomType roomtypes[256];  //
    //unsigned char decknumber;
   // unsigned int roomcount;
};

struct Ship //change later to class with actual organization this time!!!
{
    ShipDeck decks[57];  //Ship.deck[0].rooms[0].verts[0]
    unsigned char streamlined;
    unsigned char armorlevel;
    unsigned int deckcount;
    bool drawall();
    bool buildall();

};
//Ship1.decks[decknum].roomtypes[typeid].rooms[roomid].verts[]
//convert this to work with my struct and shit
