

// bastardized from the GLFW particles example
// GLFW - http://glfw.sourceforge.net/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <inttypes.h> 


int destx;
int desty;
float xpos, ypos, zpos,angle_x, angle_y, angle_z;
static int partInitOnce = 0;


#ifndef DEGTORAD
#define DEGTORAD 0.034906585039
#endif

// Maximum number of particles
static int MAX_PARTICLES = 2500;

// Life span of a particle (in seconds)
#define LIFE_SPAN 12.0f

// A new particle is born every [BIRTH_INTERVAL] second
static float BIRTH_INTERVAL = (LIFE_SPAN/2500.0);

// Particle size (meters)
#define PARTICLE_SIZE   0.7f

// Gravitational constant (m/s^2)
#define GRAVITY         9.8f

// Base initial velocity (m/s)
#define VELOCITY        8.0f

// Bounce friction (1.0 = no friction, 0.0 = maximum friction)
#define FRICTION        0.75f

// "Fountain" height (m)
#define FOUNTAIN_HEIGHT 3.0f

// Fountain radius (m)
#define FOUNTAIN_RADIUS 1.6f

static const float FOUNTAIN_R2 = (FOUNTAIN_RADIUS+PARTICLE_SIZE/2.0)*(FOUNTAIN_RADIUS+PARTICLE_SIZE/2.0);
// Minimum delta-time for particle phisics (s)
static float MIN_DELTA_T = 0; //(BIRTH_INTERVAL*0.50);
static const float zoom = -20.0; // camz
static const float flength = 100.0;

// Global variable holding the age of the youngest particle
static float min_age = 0.0f;

// This structure holds all state for a single particle
typedef struct {
    float x,y,z;     // Position in space
    float vx,vy,vz;  // Velocity vector
    float r,g,b;     // Color of particle
    float life;      // Life of particle (1.0 = newborn, < 0.0 = dead)
    int   active;    // Tells if this particle is active
}PARTICLE;

// Global vectors holding all particles. We use two vectors for float
// buffering.
static PARTICLE *particles = NULL;


void particlesCleanup ();
static void DrawParticles (TFRAME *frame, float t, float dt);
int displayInput_Particles (TWINAMP *wa, int key, void *data);
int renderParticles (TWINAMP *wa, TFRAME *frame, void *data);
int particlesInit (TFRAME *frame, void *userPtr);
static void particlesResetTime ();


int partPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	return renderParticles(wa, frame, userPtr);
}

int partPageInput (TWINAMP *wa, int key, void *userPtr)
{
	return displayInput_Particles(wa, key, userPtr);
}

int partPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("partPageEnter\n");
	particlesResetTime();
	return 1;
}

void partPageExit (void *userPtr)
{
	//dbprintf("partPageExit\n");
}

int partPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("partPageOpen()\n");
	return particlesInit(frame, userPtr);
}

void partPageClose (void *userPtr)
{
	//dbprintf("partPageClose()\n");
	particlesCleanup();
}

static void InitParticle (PARTICLE *p, float t)
{
    float xy_angle, velocity;

    // Start position of particle is at the fountain blow-out
    p->x = 0.0f;
    p->y = 0.0f;
    p->z = FOUNTAIN_HEIGHT;

    // Start velocity is up (Z)...
    p->vz = 0.7f + (0.3/4096.0) * (float) (rand() & 4095);

    // ...and a randomly chosen X/Y direction
    xy_angle = (2.0*M_PI/4096.0) * (float) (rand() & 4095);
    p->vx = 0.4f * (float) cosf( xy_angle );
    p->vy = 0.4f * (float) sinf( xy_angle );

    // Scale velocity vector according to a time-varying velocity
    velocity = VELOCITY*(0.8f + 0.1f*(float)(sin( 0.5*t )+sin( 1.31*t )));
    p->vx *= velocity;
    p->vy *= velocity;
    p->vz *= velocity;
/*
    // Colour is time-varying
    p->r = 0.7f + 0.3f * (float) sin( 0.34*t + 0.1 );
    p->g = 0.6f + 0.4f * (float) sin( 0.63*t + 1.1 );
    p->b = 0.6f + 0.4f * (float) sin( 0.91*t + 2.1 );
*/
    // The particle is new-born and active
    p->life = 1.0f;
    p->active = 1;
}

static void UpdateParticle (PARTICLE *p, float dt)
{
    // If the particle is not active, we need not do anything
    if (!p->active) return;

    // The particle is getting older...
    p->life = p->life - dt * (1.0f / LIFE_SPAN);

    // Did the particle die?
    if (p->life <= 0.0f ) {
        p->active = 0;
        return;
    }

    // Update particle velocity (apply gravity)
    p->vz = p->vz - GRAVITY * dt;

    // Update particle position
    p->x = p->x + p->vx * dt;
    p->y = p->y + p->vy * dt;
    p->z = p->z + p->vz * dt;

    // Simple collision detection + response
    if (p->vz < 0.0f){
        // Particles should bounce on the fountain (with friction)
        if( (p->x*p->x + p->y*p->y) < FOUNTAIN_R2 && p->z < (FOUNTAIN_HEIGHT + PARTICLE_SIZE/2)){
            p->vz = -FRICTION * p->vz;
            p->z  = FOUNTAIN_HEIGHT + PARTICLE_SIZE/2 + FRICTION * (FOUNTAIN_HEIGHT + PARTICLE_SIZE/2 - p->z);
                        
          // Particles should bounce on the floor (with friction)
        }else if (p->z < PARTICLE_SIZE/2){
            p->vz = -FRICTION * p->vz;
            p->z  = PARTICLE_SIZE/2 + FRICTION * (PARTICLE_SIZE/2 - p->z);
        }
    }
}

static void ParticleEngine (float t, float dt)
{
    int      i;
    float    dt2;

    // Update particles (iterated several times per frame if dt is too
    // large)
    while( dt > 0.0f ) {
        // Calculate delta time for this iteration
        dt2 = dt < MIN_DELTA_T ? dt : MIN_DELTA_T;

        // Update particles
        for( i = 0; i < MAX_PARTICLES; i ++ )
            UpdateParticle( &particles[ i ], dt2 );

        // Increase minimum age
        min_age += dt2;

        // Should we create any new particle(s)?
        while( min_age >= BIRTH_INTERVAL){
            min_age -= BIRTH_INTERVAL;

            // Find a dead particle to replace with a new one
            for (i = 0; i < MAX_PARTICLES; i++){
                if (!particles[i].active){
                    InitParticle(&particles[i], t + min_age);
                    UpdateParticle(&particles[i], min_age);
                    break;
                }
            }
        }
        // Decrease frame delta time
        dt -= dt2;
    }
}

static void DrawParticles (TFRAME *frame, float t, float dt )
{
    PARTICLE  *pptr;
	ParticleEngine(t, dt);

    pptr = particles;
    int sx=0,sy=0,i;
    float x,y,z;
	float x2,y2,z2;
	float tmp;

	float ThetaX = -53.0 * DEGTORAD; //xpos * DEGTORAD;
	float ThetaY = ypos * DEGTORAD;
	float ThetaZ = zpos * DEGTORAD;
	float cosx = cosf(ThetaX);
	float cosy = cosf(ThetaY);
	float cosz = cosf(ThetaZ);
	float sinx = sinf(ThetaX);
	float siny = sinf(ThetaY);
	float sinz = sinf(ThetaZ);
	

    for (i = MAX_PARTICLES; --i;){
        if (pptr->active){
			x = pptr->x;
			z = pptr->z;
			y = pptr->y;
			
  			//RotateX(ThetaX, &y, &z);
  			y2 = (y*cosx) - (z*sinx);
			z = (z*cosx) + (y*sinx);
			y = y2;
				
  			//RotateY(ThetaY, &x, &z);
  			z2 = (z*cosy) - (x*siny);
			x = (x*cosy) + (z*siny);
			z = z2;
				
   			//RotateZ(ThetaZ, &x, &y);
			x2 = (x*cosz) - (y*sinz);
			y = (y*cosz) + (x*sinz);
			x = x2;

			if (z-zoom){
				tmp = flength/(z-zoom);
				sx = (x*tmp)+destx;
				sy = (-y*tmp)+desty;
				lSetPixel(frame, sx, sy, LSP_SET);
			}
        }
        pptr++;
    }
}

static void Draw (TFRAME *frame, float t)
{
    static float t_old = 0.0;
    static float  dt;

    // Calculate frame-to-frame delta time
    dt = t-t_old;
    t_old = t;

    // Rotate camera
    angle_x = 90.0 - 10.0;
    angle_y = 10.0 * sinf(0.3 * t);
    angle_z = 10.0 * t;

#if 1
    // Translate camera
    xpos =  15.0 * sinf( (M_PI/180.0) * angle_z ) + 2.0 * sinf( (M_PI/180.0) * 3.1 * t );
    ypos = -15.0 * cosf( (M_PI/180.0) * angle_z ) + 2.0 * cosf( (M_PI/180.0) * 2.9 * t );
    zpos = 4.0 + 2.0 * cosf( (M_PI/180.0) * 4.9 * t );
#else
    xpos = 20.0 * cos( 0.3 * t );
    ypos = 4.0 + 1.0 * sin( 1.0 * t );
    zpos = 20.0 * sin( 0.3 * t );
#endif
    lClearFrame(frame);
    DrawParticles(frame,t,dt);
	//lRefresh(frame);
}

static __int64 freq, t0_64, t_64;
static float Resolution;


static void particlesDrawScene (TFRAME *frame)
{
	if (particles == NULL)
		return;
	QueryPerformanceCounter((LARGE_INTEGER *)&t_64);
	/*
	unsigned __int64 a = t0_64;
	unsigned __int64 b = t_64;
	printf ("%"PRId64"\n",b-a);
	*/
	Draw(frame, ((float)(t_64 - t0_64))*Resolution);
}

static void particlesResetTime ()
{
	QueryPerformanceCounter((LARGE_INTEGER *)&t0_64);
}

static void particlesReset ()
{
	if (particles == NULL)
		return;

	for (int i = 0; i < MAX_PARTICLES; i++)
		particles[i].active = 0;
	particlesResetTime();
}

void particlesCleanup ()
{
	if (particles)
		free(particles);
	particles = NULL;
	partInitOnce = 0;
}

int particlesInit (TFRAME *frame, void *userPtr)
{
	
	TWINAMP *wa = (TWINAMP*)userPtr;
	if (!wa->config.tparticles)
		wa->config.tparticles = MAX_PARTICLES;

	MAX_PARTICLES = wa->config.tparticles;
	BIRTH_INTERVAL = (LIFE_SPAN/(float)MAX_PARTICLES);
	MIN_DELTA_T = (BIRTH_INTERVAL*0.50);
	particles = (PARTICLE*)calloc(sizeof(PARTICLE), MAX_PARTICLES);
	if (particles == NULL){
		partInitOnce = 0;
		return 0;
	}

 	destx = frame->width/2;
 	desty = (frame->height/2)+8;
	particlesReset();
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
	Resolution = 1.0 / (float)freq;
	partInitOnce = 1;
    return 1;
}

int renderParticles (TWINAMP *wa, TFRAME *frame, void *data)
{
	if (!partInitOnce){
		particlesInit(frame, wa);
		if (!partInitOnce)
			return 0;
	}
	particlesDrawScene(frame);
	return 1;
}

int displayInput_Particles (TWINAMP *wa, int key, void *data)
{
	if (!partInitOnce) return -1;

	if (key == G15_SOFTKEY_4){
		particlesReset();
	}else{
		return -1;
	}
	return 0;
}

