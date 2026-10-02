/*
 * =====================================================================
 *   MAZE ESCAPE 3D  -  Computer Graphics Mini Project (OpenGL + GLUT)
 * =====================================================================
 *
 *  Goal : Collect all the glowing orbs to unlock the exit portal,
 *         then reach the portal before the timer runs out.
 *         Every level the maze gets bigger.
 *
 *  Controls
 *    W / Up arrow       move forward
 *    S / Down arrow     move backward
 *    A / D, Left/Right  turn left / right
 *    Q / E              strafe left / right
 *    M                  toggle minimap
 *    P                  pause / resume
 *    R                  restart current level
 *    ENTER              start / next level / retry
 *    ESC                quit
 *
 *  CG concepts used
 *    - Procedural maze generation (recursive backtracker on a grid)
 *    - 3D viewing: perspective projection + first-person camera (gluLookAt)
 *    - Geometric transformations (translate / rotate / scale) for animation
 *    - Procedural texture mapping (brick walls, stone floor) with mipmaps
 *    - Lighting: a "torch" light that moves with the player + coloured
 *      point light on the exit portal, material emission for glowing objects
 *    - Fog for depth cueing / atmosphere
 *    - Display lists for fast static geometry
 *    - Collision detection with wall sliding
 *    - 2D overlay (orthographic projection) for HUD, menus and minimap
 *      with fog-of-war, alpha blending for translucent panels
 *
 *  Build
 *    Linux   : g++ maze_escape_3d.cpp -o maze -lGL -lGLU -lglut
 *    Windows : (Code::Blocks / MinGW with freeglut)
 *              g++ maze_escape_3d.cpp -o maze -lfreeglut -lopengl32 -lglu32
 * =====================================================================
 */

#include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cctype>
#include <vector>
using namespace std;

/* ------------------------------------------------------------------ */
/*  Constants                                                          */
/* ------------------------------------------------------------------ */
const float CELL       = 2.0f;   // size of one grid cell in world units
const float WALL_H     = 2.2f;   // wall height
const float EYE_H      = 1.1f;   // camera height
const float PLAYER_R   = 0.3f;   // player collision radius
const float MOVE_SPEED = 3.2f;   // units per second
const float TURN_SPEED = 2.2f;   // radians per second
const float PI         = 3.14159265f;

const GLfloat FOG_COLOR[] = { 0.02f, 0.02f, 0.05f, 1.0f };

/* ------------------------------------------------------------------ */
/*  Game state                                                         */
/* ------------------------------------------------------------------ */
enum State { MENU, PLAYING, PAUSED, LEVEL_DONE, GAME_OVER };
State state = MENU;

int winW = 1024, winH = 700;

vector< vector<int> >  maze;   // 1 = wall, 0 = floor   (indexed [z][x])
vector< vector<bool> > seen;   // cells revealed on the minimap
int W, H;                      // grid size
int exitX, exitZ;              // exit cell

struct Orb { float x, z; bool taken; };

vector<Orb> orbs;
int orbsTaken = 0;

float px, pz, angle;           // player position and facing angle
float bobPhase = 0.0f;         // head-bob animation
bool  moving   = false;

int   level          = 1;
int   score          = 0;
int   levelStartScore = 0;
int   levelBonus     = 0;
float timeLeft       = 0.0f;

bool keys[256];
bool spec[256];
bool showMap = true;

char  msg[128] = "";
float msgTimer = 0.0f;

GLuint wallTex, floorTex, mazeList = 0;
int    lastTick   = 0;
float  globalTime = 0.0f;

/* ------------------------------------------------------------------ */
/*  Helpers                                                            */
/* ------------------------------------------------------------------ */
void setMsg(const char *s)
{
    strncpy(msg, s, sizeof(msg) - 1);
    msg[sizeof(msg) - 1] = '\0';
    msgTimer = 2.5f;
}


bool cellWall(int x, int z)
{
    if (x < 0 || z < 0 || x >= W || z >= H) return true;
    return maze[z][x] == 1;
}

bool isWall(float x, float z)
{
    return cellWall((int)floor(x / CELL), (int)floor(z / CELL));
}

// Player is a square of radius PLAYER_R: test its four corners
bool blocked(float x, float z)
{
    return isWall(x - PLAYER_R, z - PLAYER_R) || isWall(x + PLAYER_R, z - PLAYER_R) ||
           isWall(x - PLAYER_R, z + PLAYER_R) || isWall(x + PLAYER_R, z + PLAYER_R);
}

float cellCenter(int i) { return (i + 0.5f) * CELL; }

bool allOrbsTaken() { return orbsTaken == (int)orbs.size(); }

/* ------------------------------------------------------------------ */
/*  Procedural textures                                                */
/* ------------------------------------------------------------------ */
void uploadTexture(GLuint &id, unsigned char *data, int size)
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, size, size, GL_RGB, GL_UNSIGNED_BYTE, data);
}

void makeTextures()
{
    const int S = 128;
    static unsigned char img[S][S][3];

    // Brick wall: rows of 16px, bricks 32px wide, alternate rows offset
    for (int y = 0; y < S; y++)
        for (int x = 0; x < S; x++) {
            int  row    = y / 16;
            int  off    = (row % 2) * 16;
            int  bx     = (x + off) % 32;
            bool mortar = (y % 16) < 2 || bx < 2;
            int  n      = rand() % 25;
            if (mortar) {
                img[y][x][0] = img[y][x][1] = img[y][x][2] = (unsigned char)(90 + n);
            } else {
                int shade = ((row * 7 + ((x + off) / 32) * 13) % 5) * 8; // per-brick tint
                img[y][x][0] = (unsigned char)(130 + shade + n);
                img[y][x][1] = (unsigned char)(55 + shade / 2 + n / 2);
                img[y][x][2] = (unsigned char)(40 + n / 2);
            }
        }
    uploadTexture(wallTex, &img[0][0][0], S);

    // Stone floor: 64px tiles with grout, checker tint, noise
    for (int y = 0; y < S; y++)
        for (int x = 0; x < S; x++) {
            int  tx    = x % 64, ty = y % 64;
            bool grout = tx < 3 || ty < 3;
            int  n     = rand() % 30;
            int  tint  = (((x / 64) + (y / 64)) % 2) ? 15 : 0;
            unsigned char v = (unsigned char)(grout ? 40 + n / 2 : 95 + tint + n);
            img[y][x][0] = v;
            img[y][x][1] = v;
            img[y][x][2] = (unsigned char)(v + (grout ? 0 : 10));
        }
    uploadTexture(floorTex, &img[0][0][0], S);
}

/* ------------------------------------------------------------------ */
/*  Maze generation (recursive backtracker, iterative with a stack)    */
/* ------------------------------------------------------------------ */
void generateMaze(int cellsW, int cellsH)
{
    W = 2 * cellsW + 1;
    H = 2 * cellsH + 1;
    maze.assign(H, vector<int>(W, 1));

    const int dx[4] = { 2, -2, 0, 0 };
    const int dz[4] = { 0, 0, 2, -2 };

    vector< pair<int, int> > stack;
    maze[1][1] = 0;
    stack.push_back(make_pair(1, 1));

    while (!stack.empty()) {
        int x = stack.back().first;
        int z = stack.back().second;

        int choices[4], count = 0;
        for (int d = 0; d < 4; d++) {
            int nx = x + dx[d], nz = z + dz[d];
            if (nx > 0 && nz > 0 && nx < W - 1 && nz < H - 1 && maze[nz][nx] == 1)
                choices[count++] = d;
        }

        if (count == 0) { stack.pop_back(); continue; }

        int d  = choices[rand() % count];
        int nx = x + dx[d], nz = z + dz[d];
        maze[z + dz[d] / 2][x + dx[d] / 2] = 0;   // knock down wall between
        maze[nz][nx] = 0;
        stack.push_back(make_pair(nx, nz));
    }

    // Remove a few extra walls to create loops (multiple routes)
    int loops = (cellsW * cellsH) / 6;
    for (int tries = 0; loops > 0 && tries < 2000; tries++) {
        int x = 1 + rand() % (W - 2);
        int z = 1 + rand() % (H - 2);
        if (maze[z][x] != 1 || (x + z) % 2 == 0) continue;
        bool horiz = (x % 2 == 0) && maze[z][x - 1] == 0 && maze[z][x + 1] == 0;
        bool vert  = (z % 2 == 0) && maze[z - 1][x] == 0 && maze[z + 1][x] == 0;
        if (horiz || vert) { maze[z][x] = 0; loops--; }
    }

    exitX = W - 2;
    exitZ = H - 2;
}

void placeOrbs(int count)
{
    vector< pair<int, int> > cells;
    for (int z = 1; z < H - 1; z += 2)
        for (int x = 1; x < W - 1; x += 2) {
            if (x == 1 && z == 1) continue;              // start
            if (x == exitX && z == exitZ) continue;      // exit
            cells.push_back(make_pair(x, z));
        }

    // Fisher-Yates shuffle
    for (int i = (int)cells.size() - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        pair<int, int> t = cells[i]; cells[i] = cells[j]; cells[j] = t;
    }

    orbs.clear();
    for (int i = 0; i < count && i < (int)cells.size(); i++) {
        Orb o = { cellCenter(cells[i].first), cellCenter(cells[i].second), false };
        orbs.push_back(o);
    }
    orbsTaken = 0;
}

/* ------------------------------------------------------------------ */
/*  Geometry: a subdivided, textured quad (better per-vertex lighting) */
/* ------------------------------------------------------------------ */
void face(float ox, float oy, float oz,
          float ux, float uy, float uz,
          float vx, float vy, float vz,
          float nx, float ny, float nz, int n)
{
    glNormal3f(nx, ny, nz);
    glBegin(GL_QUADS);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            float s[4] = { (float)i / n, (float)(i + 1) / n, (float)(i + 1) / n, (float)i / n };
            float t[4] = { (float)j / n, (float)j / n, (float)(j + 1) / n, (float)(j + 1) / n };
            for (int k = 0; k < 4; k++) {
                glTexCoord2f(s[k], t[k]);
                glVertex3f(ox + ux * s[k] + vx * t[k],
                           oy + uy * s[k] + vy * t[k],
                           oz + uz * s[k] + vz * t[k]);
            }
        }
    glEnd();
}

// Compile the whole static maze into a display list (drawn every frame)
void buildMazeList()
{
    if (mazeList) glDeleteLists(mazeList, 1);
    mazeList = glGenLists(1);
    glNewList(mazeList, GL_COMPILE);

    glColor3f(1, 1, 1);
    glEnable(GL_TEXTURE_2D);

    // Walls: only draw faces that touch a floor cell
    glBindTexture(GL_TEXTURE_2D, wallTex);
    for (int z = 0; z < H; z++)
        for (int x = 0; x < W; x++) {
            if (maze[z][x] != 1) continue;
            float x0 = x * CELL, x1 = x0 + CELL;
            float z0 = z * CELL, z1 = z0 + CELL;
            if (!cellWall(x, z - 1)) face(x0, 0, z0,  CELL, 0, 0,  0, WALL_H, 0,  0, 0, -1, 4);
            if (!cellWall(x, z + 1)) face(x1, 0, z1, -CELL, 0, 0,  0, WALL_H, 0,  0, 0,  1, 4);
            if (!cellWall(x - 1, z)) face(x0, 0, z1,  0, 0, -CELL, 0, WALL_H, 0, -1, 0,  0, 4);
            if (!cellWall(x + 1, z)) face(x1, 0, z0,  0, 0, CELL,  0, WALL_H, 0,  1, 0,  0, 4);
        }

    // Floor
    glBindTexture(GL_TEXTURE_2D, floorTex);
    for (int z = 0; z < H; z++)
        for (int x = 0; x < W; x++)
            if (maze[z][x] == 0)
                face(x * CELL, 0, z * CELL, CELL, 0, 0, 0, 0, CELL, 0, 1, 0, 3);

    // Ceiling (untextured, dark)
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.18f, 0.17f, 0.20f);
    for (int z = 0; z < H; z++)
        for (int x = 0; x < W; x++)
            if (maze[z][x] == 0)
                face(x * CELL, WALL_H, z * CELL, CELL, 0, 0, 0, 0, CELL, 0, -1, 0, 3);

    glEndList();
}

/* ------------------------------------------------------------------ */
/*  Level setup                                                        */
/* ------------------------------------------------------------------ */
void startLevel()
{
    int cells = 4 + level;
    if (cells > 14) cells = 14;

    generateMaze(cells, cells);
    placeOrbs(3 + level);

    px = cellCenter(1);
    pz = cellCenter(1);
    angle = (maze[1][2] == 0) ? 0.0f : PI / 2;   // face the open corridor

    timeLeft = 45.0f + cells * cells * 1.2f;
    seen.assign(H, vector<bool>(W, false));
    buildMazeList();

    memset(keys, 0, sizeof(keys));
    memset(spec, 0, sizeof(spec));

    char buf[64];
    sprintf(buf, "Level %d - collect %d orbs!", level, (int)orbs.size());
    setMsg(buf);
}

void markSeen()
{
    int gi = (int)(px / CELL), gj = (int)(pz / CELL);
    for (int dj = -1; dj <= 1; dj++)
        for (int di = -1; di <= 1; di++) {
            int x = gi + di, z = gj + dj;
            if (x >= 0 && z >= 0 && x < W && z < H) seen[z][x] = true;
        }
}

/* ------------------------------------------------------------------ */
/*  Update loop (~60 fps)                                              */
/* ------------------------------------------------------------------ */
void update(int)
{
    int   now = glutGet(GLUT_ELAPSED_TIME);
    float dt  = (now - lastTick) / 1000.0f;
    lastTick  = now;
    if (dt > 0.05f) dt = 0.05f;

    globalTime += dt;
    if (msgTimer > 0) msgTimer -= dt;

    if (state == MENU) angle += dt * 0.4f;          // slow look-around on menu

    if (state == PLAYING) {
        timeLeft -= dt;
        if (timeLeft <= 0) { timeLeft = 0; state = GAME_OVER; }

        float mv = 0, st = 0, tn = 0;
        if (keys['w'] || spec[GLUT_KEY_UP])    mv += 1;
        if (keys['s'] || spec[GLUT_KEY_DOWN])  mv -= 1;
        if (keys['a'] || spec[GLUT_KEY_LEFT])  tn -= 1;
        if (keys['d'] || spec[GLUT_KEY_RIGHT]) tn += 1;
        if (keys['q']) st -= 1;
        if (keys['e']) st += 1;

        angle += tn * TURN_SPEED * dt;

        // forward = (cos a, sin a), right = (-sin a, cos a)
        float ca = cos(angle), sa = sin(angle);
        float vx = ca * mv - sa * st;
        float vz = sa * mv + ca * st;
        float len = sqrt(vx * vx + vz * vz);
        moving = len > 0.01f;

        if (moving) {
            vx /= len; vz /= len;
            float nx = px + vx * MOVE_SPEED * dt;
            float nz = pz + vz * MOVE_SPEED * dt;
            // Test each axis separately so the player slides along walls
            if (!blocked(nx, pz)) px = nx;
            if (!blocked(px, nz)) pz = nz;
            bobPhase += dt * 9.0f;
        }

        markSeen();

        // Orb pickup
        for (size_t i = 0; i < orbs.size(); i++) {
            if (orbs[i].taken) continue;
            float dx = orbs[i].x - px, dz = orbs[i].z - pz;
            if (dx * dx + dz * dz < 0.7f * 0.7f) {
                orbs[i].taken = true;
                orbsTaken++;
                score += 100;
                if (allOrbsTaken()) setMsg("All orbs collected! The exit portal is OPEN");
                else                setMsg("Orb collected!");
            }
        }

        // Exit portal
        float ex = cellCenter(exitX) - px, ez = cellCenter(exitZ) - pz;
        if (ex * ex + ez * ez < 0.8f * 0.8f) {
            if (allOrbsTaken()) {
                levelBonus = (int)timeLeft * 10;
                score += levelBonus;
                state = LEVEL_DONE;
            } else if (msgTimer <= 0) {
                setMsg("The portal is locked - collect all orbs first!");
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

/* ------------------------------------------------------------------ */
/*  3D objects                                                         */
/* ------------------------------------------------------------------ */
void setEmission(float r, float g, float b)
{
    GLfloat e[] = { r, g, b, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, e);
}

void drawOrbs()
{
    glDisable(GL_TEXTURE_2D);
    for (size_t i = 0; i < orbs.size(); i++) {
        if (orbs[i].taken) continue;
        float bob = 0.15f * sin(globalTime * 3.0f + i);

        glPushMatrix();
        glTranslatef(orbs[i].x, 1.0f + bob, orbs[i].z);
        glRotatef(globalTime * 90.0f, 0, 1, 0);

        setEmission(0.7f, 0.55f, 0.05f);
        glColor3f(1.0f, 0.85f, 0.2f);
        glutSolidSphere(0.18, 16, 16);

        setEmission(0.35f, 0.25f, 0.0f);
        glRotatef(60, 1, 0, 0);
        glutSolidTorus(0.03, 0.32, 8, 24);
        glPopMatrix();
    }
    setEmission(0, 0, 0);
}

void drawExit()
{
    bool  open = allOrbsTaken();
    float ex = cellCenter(exitX), ez = cellCenter(exitZ);
    float r = open ? 0.15f : 0.9f, g = open ? 0.9f : 0.1f, b = 0.15f;

    glDisable(GL_TEXTURE_2D);

    // Two spinning rings
    glPushMatrix();
    glTranslatef(ex, 1.0f, ez);
    glRotatef(globalTime * 60.0f, 0, 1, 0);
    setEmission(r * 0.7f, g * 0.7f, b * 0.7f);
    glColor3f(r, g, b);
    glutSolidTorus(0.07, 0.75, 12, 36);
    glRotatef(90, 0, 1, 0);
    glRotatef(globalTime * 40.0f, 0, 0, 1);
    glutSolidTorus(0.04, 0.6, 10, 30);
    glPopMatrix();

    // Pulsing core
    glPushMatrix();
    glTranslatef(ex, 1.0f, ez);
    float s = 0.22f + 0.06f * sin(globalTime * 5.0f);
    glScalef(s, s, s);
    setEmission(r, g, b);
    glutSolidSphere(1.0, 16, 16);
    glPopMatrix();

    // Glowing floor pad
    setEmission(r * 0.5f, g * 0.5f, b * 0.5f);
    glNormal3f(0, 1, 0);
    glBegin(GL_QUADS);
    glVertex3f(ex - 0.8f, 0.02f, ez - 0.8f);
    glVertex3f(ex + 0.8f, 0.02f, ez - 0.8f);
    glVertex3f(ex + 0.8f, 0.02f, ez + 0.8f);
    glVertex3f(ex - 0.8f, 0.02f, ez + 0.8f);
    glEnd();

    setEmission(0, 0, 0);
}

/* ------------------------------------------------------------------ */
/*  2D overlay: HUD, minimap, menus                                    */
/* ------------------------------------------------------------------ */
void begin2D()
{
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);
    glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void end2D()
{
    glDisable(GL_BLEND);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
}

void rect(float x0, float y0, float x1, float y1)
{
    glBegin(GL_QUADS);
    glVertex2f(x0, y0); glVertex2f(x1, y0); glVertex2f(x1, y1); glVertex2f(x0, y1);
    glEnd();
}

void drawText(float x, float y, const char *s, void *font = GLUT_BITMAP_HELVETICA_18)
{
    glRasterPos2f(x, y);
    for (const char *c = s; *c; c++) glutBitmapCharacter(font, *c);
}

void drawCentered(float y, const char *s, void *font = GLUT_BITMAP_HELVETICA_18)
{
    int w = glutBitmapLength(font, (const unsigned char *)s);
    drawText((winW - w) / 2.0f, y, s, font);
}

void dimScreen(float alpha)
{
    glColor4f(0, 0, 0, alpha);
    rect(0, 0, (float)winW, (float)winH);
}

void drawMinimap()
{
    float cs = 200.0f / (W > H ? W : H);
    float ox = winW - W * cs - 15;
    float oy = winH - 15.0f;

    glColor4f(0, 0, 0, 0.55f);
    rect(ox - 5, oy - H * cs - 5, ox + W * cs + 5, oy + 5);

    // Explored cells only (fog of war)
    glBegin(GL_QUADS);
    for (int j = 0; j < H; j++)
        for (int i = 0; i < W; i++) {
            if (!seen[j][i]) continue;
            if (maze[j][i] == 1) glColor4f(0.55f, 0.30f, 0.22f, 0.95f);
            else                 glColor4f(0.85f, 0.85f, 0.80f, 0.85f);
            float x = ox + i * cs, y = oy - (j + 1) * cs;
            glVertex2f(x, y); glVertex2f(x + cs, y);
            glVertex2f(x + cs, y + cs); glVertex2f(x, y + cs);
        }
    glEnd();

    // Orbs (only if their cell has been seen)
    glColor4f(1.0f, 0.85f, 0.1f, 1.0f);
    for (size_t i = 0; i < orbs.size(); i++) {
        if (orbs[i].taken) continue;
        int gi = (int)(orbs[i].x / CELL), gj = (int)(orbs[i].z / CELL);
        if (!seen[gj][gi]) continue;
        float x = ox + orbs[i].x / CELL * cs, y = oy - orbs[i].z / CELL * cs;
        rect(x - cs * 0.3f, y - cs * 0.3f, x + cs * 0.3f, y + cs * 0.3f);
    }

    // Exit is always shown so the player knows where to head
    if (allOrbsTaken()) glColor4f(0.1f, 1.0f, 0.2f, 1.0f);
    else                glColor4f(1.0f, 0.15f, 0.15f, 1.0f);
    {
        float x = ox + exitX * cs, y = oy - (exitZ + 1) * cs;
        rect(x, y, x + cs, y + cs);
    }

    // Player arrow (world z grows downward on the map)
    float mx = ox + px / CELL * cs, my = oy - pz / CELL * cs;
    float dx = cos(angle), dy = -sin(angle);
    float qx = -dy, qy = dx;   // perpendicular
    glColor4f(0.2f, 0.6f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(mx + dx * cs * 0.8f, my + dy * cs * 0.8f);
    glVertex2f(mx - dx * cs * 0.5f + qx * cs * 0.45f, my - dy * cs * 0.5f + qy * cs * 0.45f);
    glVertex2f(mx - dx * cs * 0.5f - qx * cs * 0.45f, my - dy * cs * 0.5f - qy * cs * 0.45f);
    glEnd();
}

void drawHUD()
{
    char buf[128];
    begin2D();

    if (state != MENU) {
        // Stats panel
        glColor4f(0, 0, 0, 0.5f);
        rect(10, winH - 120.0f, 240, winH - 10.0f);

        glColor3f(1, 1, 1);
        sprintf(buf, "Level  %d", level);                         drawText(22, winH - 38.0f, buf);
        sprintf(buf, "Orbs   %d / %d", orbsTaken, (int)orbs.size()); drawText(22, winH - 62.0f, buf);
        if (timeLeft < 10) glColor3f(1.0f, 0.3f, 0.3f);
        sprintf(buf, "Time   %d s", (int)ceil(timeLeft));           drawText(22, winH - 86.0f, buf);
        glColor3f(1, 1, 1);
        sprintf(buf, "Score  %d", score);                         drawText(22, winH - 110.0f, buf);

        if (showMap) drawMinimap();

        // Crosshair
        glColor4f(1, 1, 1, 0.6f);
        rect(winW / 2.0f - 1, winH / 2.0f - 8, winW / 2.0f + 1, winH / 2.0f + 8);
        rect(winW / 2.0f - 8, winH / 2.0f - 1, winW / 2.0f + 8, winH / 2.0f + 1);

        // Toast message
        if (msgTimer > 0 && state == PLAYING) {
            int w = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char *)msg);
            glColor4f(0, 0, 0, 0.6f);
            rect((winW - w) / 2.0f - 15, 40, (winW + w) / 2.0f + 15, 75);
            glColor3f(1.0f, 0.95f, 0.6f);
            drawCentered(52, msg);
        }
    }

    float cy = winH / 2.0f;
    bool  blink = ((int)(globalTime * 2)) % 2 == 0;

    switch (state) {
    case MENU:
        dimScreen(0.55f);
        glColor3f(1.0f, 0.8f, 0.2f);
        drawCentered(cy + 160, "M A Z E   E S C A P E   3 D", GLUT_BITMAP_TIMES_ROMAN_24);
        glColor3f(0.9f, 0.9f, 0.9f);
        drawCentered(cy + 110, "Collect all the glowing orbs to unlock the exit portal.");
        drawCentered(cy + 85,  "Reach the portal before the timer runs out!");
        glColor3f(0.7f, 0.85f, 1.0f);
        drawCentered(cy + 35,  "W / S  or  Up / Down      move", GLUT_BITMAP_HELVETICA_12);
        drawCentered(cy + 15,  "A / D  or  Left / Right    turn", GLUT_BITMAP_HELVETICA_12);
        drawCentered(cy - 5,   "Q / E                      strafe", GLUT_BITMAP_HELVETICA_12);
        drawCentered(cy - 25,  "M  minimap    P  pause    R  restart level    ESC  quit", GLUT_BITMAP_HELVETICA_12);
        if (blink) { glColor3f(0.3f, 1.0f, 0.4f); drawCentered(cy - 90, "Press ENTER to start"); }
        break;

    case PAUSED:
        dimScreen(0.5f);
        glColor3f(1, 1, 1);
        drawCentered(cy + 10, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);
        drawCentered(cy - 25, "Press P to resume");
        break;

    case LEVEL_DONE:
        dimScreen(0.55f);
        glColor3f(0.3f, 1.0f, 0.4f);
        drawCentered(cy + 50, "LEVEL COMPLETE!", GLUT_BITMAP_TIMES_ROMAN_24);
        glColor3f(1, 1, 1);
        sprintf(buf, "Time bonus: %d", levelBonus);  drawCentered(cy + 10, buf);
        sprintf(buf, "Total score: %d", score);      drawCentered(cy - 15, buf);
        if (blink) drawCentered(cy - 60, "Press ENTER for the next level");
        break;

    case GAME_OVER:
        dimScreen(0.6f);
        glColor3f(1.0f, 0.3f, 0.3f);
        drawCentered(cy + 50, "TIME'S UP!", GLUT_BITMAP_TIMES_ROMAN_24);
        glColor3f(1, 1, 1);
        sprintf(buf, "You reached level %d with %d points", level, score);
        drawCentered(cy + 10, buf);
        if (blink) drawCentered(cy - 40, "Press ENTER to play again");
        break;

    default:
        break;
    }

    end2D();
}

/* ------------------------------------------------------------------ */
/*  Rendering                                                          */
/* ------------------------------------------------------------------ */
void display()
{
    glClearColor(FOG_COLOR[0], FOG_COLOR[1], FOG_COLOR[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70.0, (double)winW / winH, 0.05, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Torch light: positioned in eye space so it follows the camera
    GLfloat headPos[] = { 0, 0, 0, 1 };
    glLightfv(GL_LIGHT0, GL_POSITION, headPos);

    float bob  = (moving && state == PLAYING) ? sin(bobPhase) * 0.05f : 0.0f;
    float eyeY = EYE_H + bob;
    gluLookAt(px, eyeY, pz,
              px + cos(angle), eyeY, pz + sin(angle),
              0, 1, 0);

    // Portal light (world space): red when locked, green when open
    bool    open    = allOrbsTaken();
    GLfloat exPos[] = { cellCenter(exitX), 1.0f, cellCenter(exitZ), 1.0f };
    GLfloat exCol[] = { open ? 0.2f : 1.0f, open ? 1.0f : 0.15f, 0.2f, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, exPos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, exCol);

    glCallList(mazeList);
    drawOrbs();
    drawExit();

    drawHUD();
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    winW = w;
    winH = h > 0 ? h : 1;
    glViewport(0, 0, winW, winH);
}

/* ------------------------------------------------------------------ */
/*  Input                                                              */
/* ------------------------------------------------------------------ */
void keyDown(unsigned char k, int, int)
{
    k = (unsigned char)tolower(k);
    keys[k] = true;

    if (k == 27) exit(0);

    if (k == 13) {                                   // ENTER
        if (state == MENU || state == GAME_OVER) {
            level = 1; score = 0; levelStartScore = 0;
            startLevel(); state = PLAYING;
        } else if (state == LEVEL_DONE) {
            level++; levelStartScore = score;
            startLevel(); state = PLAYING;
        }
    }

    if (k == 'p') {
        if      (state == PLAYING) state = PAUSED;
        else if (state == PAUSED)  state = PLAYING;
    }

    if (k == 'm') showMap = !showMap;

    if (k == 'r' && (state == PLAYING || state == PAUSED)) {
        score = levelStartScore;
        startLevel();
        state = PLAYING;
    }
}

void keyUp(unsigned char k, int, int)  { keys[(unsigned char)tolower(k)] = false; }
void specDown(int k, int, int)         { if (k >= 0 && k < 256) spec[k] = true;  }
void specUp(int k, int, int)           { if (k >= 0 && k < 256) spec[k] = false; }

/* ------------------------------------------------------------------ */
/*  Initialisation                                                     */
/* ------------------------------------------------------------------ */
void initGL()
{
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    // Lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat globalAmb[] = { 0.12f, 0.12f, 0.14f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);

    GLfloat torch[] = { 1.0f, 0.9f, 0.7f, 1.0f };
    GLfloat black[] = { 0, 0, 0, 1 };
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, torch);
    glLightfv(GL_LIGHT0, GL_SPECULAR, black);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,  0.4f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION,    0.12f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.06f);

    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, black);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION,  0.5f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION,    0.2f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.3f);

    // Fog
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_EXP2);
    glFogfv(GL_FOG_COLOR, FOG_COLOR);
    glFogf(GL_FOG_DENSITY, 0.12f);
    glHint(GL_FOG_HINT, GL_NICEST);

    makeTextures();
}

int main(int argc, char **argv)
{
    srand((unsigned)time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("Maze Escape 3D - Computer Graphics Mini Project");

    initGL();
    startLevel();          // builds a maze to show behind the menu

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(specDown);
    glutSpecialUpFunc(specUp);
    glutIgnoreKeyRepeat(1);

    lastTick = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
