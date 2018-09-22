#include "HSMship.hpp"

bool ShipRoom::drawroom()
{
    drawfloor();
    drawwall();
    drawceiling();
    return true;
}

bool ShipRoom::drawfloor()
{

    glDepthMask(GL_FALSE);
    glEnable(GL_STENCIL_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // disable writing to color buffer
    glStencilFunc(GL_ALWAYS, 0x1, 0x1);
    glStencilOp(GL_KEEP, GL_INVERT, GL_INVERT);

    glBindVertexArray(floorvao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, floor.size()/3);

    // PASS 2: draw color buffer
    // Draw again the exact same polygon to color buffer where the stencil
    // value is only odd number(1). The even(0) area will be descarded.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);    // enable writing to color buffer
    glStencilFunc(GL_EQUAL, 0x1, 0x1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
    glDepthMask(GL_TRUE);


    glBindVertexArray(floorvao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, floor.size()/3);

    glDisable(GL_STENCIL_TEST);
    glDepthMask(GL_TRUE);


    return true;
}

bool ShipRoom::drawceiling()
{
    glDepthMask(GL_FALSE);
    glEnable(GL_STENCIL_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // disable writing to color buffer
    glStencilFunc(GL_ALWAYS, 0x1, 0x1);
    glStencilOp(GL_KEEP, GL_INVERT, GL_INVERT);

    glBindVertexArray(ceilingvao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, ceiling.size()/3);

    // PASS 2: draw color buffer
    // Draw again the exact same polygon to color buffer where the stencil
    // value is only odd number(1). The even(0) area will be descarded.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);    // enable writing to color buffer
    glStencilFunc(GL_EQUAL, 0x1, 0x1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
    glDepthMask(GL_TRUE);


    glBindVertexArray(ceilingvao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, ceiling.size()/3);

    glDisable(GL_STENCIL_TEST);
    glDepthMask(GL_TRUE);


    return true;
}

bool ShipRoom::drawwall()
{
    glBindVertexArray(wallvao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, wall.size()/3);

    return true;
}

bool ShipRoom::buildroomv()
{
    buildfloorv();
    buildwallv();
    buildceilingv();
    return true;

}

bool ShipRoom::buildfloorv()
{
    //gluint vao
    //genvertexarrays
    //assume empty, for now.  add stuff later to 'ensure' its empty, or to clear it
    glGenVertexArrays(1, &floorvao);
    glBindVertexArray(floorvao);

    //gen buffers
    glGenBuffers(1, &floorvbo);
    glBindBuffer(GL_ARRAY_BUFFER, floorvbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*floor.size(), &floor.at(0), GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &floorUVvbo);
    glBindBuffer(GL_ARRAY_BUFFER, floorUVvbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*floorUV.size(), &floorUV.at(0), GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    return true;

}

bool ShipRoom::buildwallv()
{

    glGenVertexArrays(1, &wallvao);
    glBindVertexArray(wallvao);
    //gen buffers
    glGenBuffers(1, &wallvbo);
    glBindBuffer(GL_ARRAY_BUFFER, wallvbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*wall.size(), &wall.at(0), GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &wallUVvbo);
    glBindBuffer(GL_ARRAY_BUFFER, wallUVvbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*wallUV.size(), &wallUV.at(0), GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    return true;
}

bool ShipRoom::buildceilingv()
{
//gluint vao
    //genvertexarrays
    //assume empty, for now.  add stuff later to 'ensure' its empty, or to clear it
    glGenVertexArrays(1, &ceilingvao);
    glBindVertexArray(ceilingvao);
    //gen buffers
    glGenBuffers(1, &ceilingvbo);
    glBindBuffer(GL_ARRAY_BUFFER, ceilingvbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*ceiling.size(), &ceiling.at(0), GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &ceilingUVvbo);
    glBindBuffer(GL_ARRAY_BUFFER, ceilingUVvbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*ceilingUV.size(), &ceilingUV.at(0), GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    return true;

}

bool Ship::drawall()
{
    //first, all decks exist
    for (int d = 0; d < 57; d++)
    {
        //second, all roomtypes exist
        for (int t = 0; t < 256; t++)
        {
            //third, not all rooms exist
            for (int r = 0; r < 256; r++)
            {
                if (decks[d].roomtypes[t].rooms[r])
                {
                    decks[d].roomtypes[t].rooms[r]->drawroom();
                }
            }
        }
    }
    return true;
}

bool Ship::buildall()
{
    //first, all decks exist
    for (int d = 0; d < 57; d++)
    {
        //second, all roomtypes exist
        for (int t = 0; t < 256; t++)
        {
            //third, not all rooms exist
            for (int r = 0; r < 256; r++)
            {
                if (decks[d].roomtypes[t].rooms[r])
                {
                    decks[d].roomtypes[t].rooms[r]->buildroomv();
                }
            }
        }
    }
    return true;

}

/*
GLuint vao;
    glGenVertexArrays(1, &vao);


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
*/
