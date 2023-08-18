/*************************************/
// AS_init.c
void AS_init(void);


/*************************************/
//AS_main.c
extern VALUE rbas_mAS;
extern VALUE rbas_image;
VALUE rbas_loadImage(VALUE self, VALUE filename);

extern int AS_SCREEN_W;
extern int AS_SCREEN_H;

VALUE Start_Aerosol(VALUE self);


/*************************************/
//AS_util.c
void AS_printException(char *message);


/*************************************/
//AS_gl.c
void Init_ASgl(void);


void Init_ASScenegraph(void);

/*************************************/
//AS_event.c
void Init_ASevents(void);


/*************************************/
//AS_texture.c
//typedef struct rbas_texture{
//	int texnum;
//	int target;
//	int w, h;
//	float sx, sy;
//} rbas_texture;
//
//void Init_AStexture(void);


/*************************************/
//AS_sprite.c
//void Init_ASsprite(void);


/*************************************/
//AS_music.c
void Init_ASmusic(void);


/*************************************/
//AS_rigid.c
void Init_ASrigid(void);

/*************************************/
//AS_image.c
void Init_ASimage(void);

typedef struct rbas_color {
	float r,g,b,a;
} rbas_color;

extern VALUE rbas_cColor;
void Init_AScolor(void);
