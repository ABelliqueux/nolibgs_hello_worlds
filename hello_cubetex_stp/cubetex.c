SVECTOR modelCube_mesh[] = {
    {48,48,-48.0},
    {48,-48,-48},
    {-48,-48,-48},
    {-48,48,-48},
    {48,48,48},
    {48,-48,48},
    {-48,-48,48},
    {-48,48,48}
};

SVECTOR modelCube_normal[] = {
    0,-0,-1,0,
    0,0,1,0,
    1,0,-2,0,
    -9,-1,-3,0,
    -1,2,-1,0,
    3,1,2,0,
    0,0,-1,0,
    0,-0,1,0,
    1,-6,3,0,
    -5,-1,9,0,
    -1,2,-1,0,
    2,1,2,0
};

SVECTOR modelCube_uv[] = {
    84,84, 0, 0,
    125,42, 0, 0,
    84,42, 0, 0,
    125,84, 0, 0,
    84,125, 0, 0,
    125,125, 0, 0,
    1,84, 0, 0,
    42,125, 0, 0,
    42,84, 0, 0,
    42,125, 0, 0,
    84,84, 0, 0,
    42,84, 0, 0,
    42,1, 0, 0,
    1,42, 0, 0,
    42,42, 0, 0,
    42,84, 0, 0,
    1,42, 0, 0,
    1,84, 0, 0,
    84,84, 0, 0,
    125,84, 0, 0,
    125,42, 0, 0,
    125,84, 0, 0,
    84,84, 0, 0,
    84,125, 0, 0,
    1,84, 0, 0,
    1,125, 0, 0,
    42,125, 0, 0,
    42,125, 0, 0,
    84,125, 0, 0,
    84,84, 0, 0,
    42,1, 0, 0,
    1,1, 0, 0,
    1,42, 0, 0,
    42,84, 0, 0,
    42,42, 0, 0,
    1,42, 0, 0
};

CVECTOR modelCube_color[] = {
    255,255,255, 0,
    255,255,255, 0,
    255,0,251, 0,
    255,255,255, 0,
    255,5,7, 0,
    255,255,255, 0,
    255,255,255, 0,
    255,255,255, 0,
    4,18,255, 0,
    255,5,7, 0,
    255,255,255, 0,
    255,255,255, 0,
    254,255,23, 0,
    122,255,107, 0,
    255,255,255, 0,
    255,255,255, 0,
    255,255,255, 0,
    254,255,94, 0,
    255,255,255, 0,
    35,255,11, 0,
    255,255,255, 0,
    255,255,255, 0,
    255,255,255, 0,
    255,5,7, 0,
    255,255,255, 0,
    255,5,7, 0,
    255,255,255, 0,
    255,5,7, 0,
    255,255,255, 0,
    255,255,255, 0,
    254,255,23, 0,
    255,255,255, 0,
    122,255,107, 0,
    255,255,255, 0,
    54,65,255, 0,
    255,255,255, 0
};

int modelCube_index[] = {
    0,2,3,
    7,5,4,
    4,1,0,
    5,2,1,
    2,7,3,
    0,7,4,
    0,1,2,
    7,6,5,
    4,5,1,
    5,6,2,
    2,6,7,
    0,3,7
};

TMESH modelCube = {
    modelCube_mesh,
    modelCube_normal,
    modelCube_uv,
    modelCube_color,
    12
};

typedef struct RGB_PIX {
    u_int  R:5, G:5, B:5, STP:1;
    } RGB_PIX;
    
// Some structures to handle TIM files 
typedef struct PIXEL {
    u_long bnum;
    u_short DX, DY;
    u_short W, H;
    RGB_PIX data[]; 
} PIXEL;

typedef struct CLUT {
    u_long bnum;
    u_short DX, DY;
    u_short W, H;
    u_short clut[]; 
} CLUT;

typedef struct TIM_FILE_CLUT{
    u_long ID;
    u_long flag;
    u_long clut;
    PIXEL pixel[];
} TIM_FILE_CLUT;

typedef struct TIM_FILE{
    u_long ID;
    u_long flag;
    PIXEL pixel[];
} TIM_FILE;

extern TIM_FILE _binary_TIM_stpOnBlack_tim_start;
extern TIM_FILE _binary_TIM_stpOnNonBlack_tim_start;
extern TIM_FILE _binary_TIM_stpOn8bpp_tim_start;
