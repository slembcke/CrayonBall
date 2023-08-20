#include <stdio.h>
#include <string.h>

#ifdef __APPLE__
#include "OpenGL/gl.h"
#include "OpenGL/glu.h"
#else
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#endif

#include "chipmunk.h"

#include "photon.h"

static const GLfloat circleVAR[] = {
	 0.0000,  1.0000,
	 0.2588,  0.9659,
	 0.5000,  0.8660,
	 0.7071,  0.7071,
	 0.8660,  0.5000,
	 0.9659,  0.2588,
	 1.0000,  0.0000,
	 0.9659, -0.2588,
	 0.8660, -0.5000,
	 0.7071, -0.7071,
	 0.5000, -0.8660,
	 0.2588, -0.9659,
	 0.0000, -1.0000,
	-0.2588, -0.9659,
	-0.5000, -0.8660,
	-0.7071, -0.7071,
	-0.8660, -0.5000,
	-0.9659, -0.2588,
	-1.0000, -0.0000,
	-0.9659,  0.2588,
	-0.8660,  0.5000,
	-0.7071,  0.7071,
	-0.5000,  0.8660,
	-0.2588,  0.9659,
	 0.0000,  1.0000,
	 0.0f, 0.0f, // For an extra line to see the rotation.
};
static const int circleVAR_count = sizeof(circleVAR)/sizeof(GLfloat)/2;

static void
drawCircleShape(cpBody *body, cpCircleShape *circle)
{
	glVertexPointer(2, GL_FLOAT, 0, circleVAR);

	glPushMatrix(); {
		cpVect center = cpvadd(body->p, cpvrotate(circle->c, body->rot));
		glTranslatef(center.x, center.y, 0.0f);
		glRotatef(body->a*180.0/M_PI, 0.0f, 0.0f, 1.0f);
		glScalef(circle->r, circle->r, 1.0f);
		
//		glColor_from_pointer(circle);
//		glDrawArrays(GL_TRIANGLE_FAN, 0, circleVAR_count - 1);
		
//		glColor3f(LINE_COLOR);
		glDrawArrays(GL_LINE_STRIP, 0, circleVAR_count);
	} glPopMatrix();
}

static const GLfloat pillVAR[] = {
	 0.0000,  1.0000,
	 0.2588,  0.9659,
	 0.5000,  0.8660,
	 0.7071,  0.7071,
	 0.8660,  0.5000,
	 0.9659,  0.2588,
	 1.0000,  0.0000,
	 0.9659, -0.2588,
	 0.8660, -0.5000,
	 0.7071, -0.7071,
	 0.5000, -0.8660,
	 0.2588, -0.9659,
	 0.0000, -1.0000,

	 0.0000, -1.0000,
	-0.2588, -0.9659,
	-0.5000, -0.8660,
	-0.7071, -0.7071,
	-0.8660, -0.5000,
	-0.9659, -0.2588,
	-1.0000, -0.0000,
	-0.9659,  0.2588,
	-0.8660,  0.5000,
	-0.7071,  0.7071,
	-0.5000,  0.8660,
	-0.2588,  0.9659,
	 0.0000,  1.0000,
};
static const int pillVAR_count = sizeof(pillVAR)/sizeof(GLfloat)/2;

static void
drawSegmentShape(cpBody *body, cpSegmentShape *seg)
{
	cpVect a = cpvadd(body->p, cpvrotate(seg->a, body->rot));
	cpVect b = cpvadd(body->p, cpvrotate(seg->b, body->rot));
	
	if(seg->r){
		cpVect delta = cpvsub(b, a);
		cpFloat len = cpvlength(delta)/seg->r;
		
		GLfloat VAR[pillVAR_count*2];
		memcpy(VAR, pillVAR, sizeof(pillVAR));
		
		for(int i=0, half=pillVAR_count; i<half; i+=2)
			VAR[i] += len;
			
		glVertexPointer(2, GL_FLOAT, 0, VAR);
		glPushMatrix(); {
			GLfloat x = a.x;
			GLfloat y = a.y;
			GLfloat cos = delta.x/len;
			GLfloat sin = delta.y/len;

			const GLfloat matrix[] = {
				 cos,  sin, 0.0f, 0.0f,
				-sin,  cos, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 1.0f,
					 x,    y, 0.0f, 1.0f,
			};
			
			glMultMatrixf(matrix);
				
//			glColor_from_pointer(seg);
//			glDrawArrays(GL_TRIANGLE_FAN, 0, pillVAR_count);
			
//			glColor3f(LINE_COLOR);
			glDrawArrays(GL_LINE_LOOP, 0, pillVAR_count);
		} glPopMatrix();
	} else {
//		glColor3f(LINE_COLOR);
		glBegin(GL_LINES); {
			glVertex2f(a.x, a.y);
			glVertex2f(b.x, b.y);
		} glEnd();
	}
}

static void
drawPolyShape(cpBody *body, cpPolyShape *poly)
{
	int count = count=poly->numVerts;
	GLfloat VAR[count*2];
	glVertexPointer(2, GL_FLOAT, 0, VAR);

	cpVect *verts = poly->verts;
	for(int i=0; i<count; i++){
		cpVect v = cpvadd(body->p, cpvrotate(verts[i], body->rot));
		VAR[2*i    ] = v.x;
		VAR[2*i + 1] = v.y;
	}

//	glColor_from_pointer(poly);
//	glDrawArrays(GL_TRIANGLE_FAN, 0, count);
	
//	glColor3f(LINE_COLOR);
	glDrawArrays(GL_LINE_LOOP, 0, count);
}

static void
drawObject(void *ptr, void *unused)
{
	cpShape *shape = (cpShape *)ptr;
	cpBody *body = shape->body;
	
	switch(shape->klass->type){
		case CP_CIRCLE_SHAPE:
			drawCircleShape(body, (cpCircleShape *)shape);
			break;
		case CP_SEGMENT_SHAPE:
			drawSegmentShape(body, (cpSegmentShape *)shape);
			break;
		case CP_POLY_SHAPE:
			drawPolyShape(body, (cpPolyShape *)shape);
			break;
		default:
			printf("Bad enumeration in drawObject().\n");
	}
}

static const GLfloat springVAR[] = {
	0.00f, 0.0f,
	0.20f, 0.0f,
	0.25f, 3.0f,
	0.30f,-6.0f,
	0.35f, 6.0f,
	0.40f,-6.0f,
	0.45f, 6.0f,
	0.50f,-6.0f,
	0.55f, 6.0f,
	0.60f,-6.0f,
	0.65f, 6.0f,
	0.70f,-3.0f,
	0.75f, 6.0f,
	0.80f, 0.0f,
	1.00f, 0.0f,
};
static const int springVAR_count = sizeof(springVAR)/sizeof(GLfloat)/2;

//static void
//drawSpring(cpDampedSpring *spring, cpBody *body_a, cpBody *body_b)
//{
//	cpVect a = cpvadd(body_a->p, cpvrotate(spring->anchr1, body_a->rot));
//	cpVect b = cpvadd(body_b->p, cpvrotate(spring->anchr2, body_b->rot));
//
//	glPointSize(5.0f);
//	glBegin(GL_POINTS); {
//		glVertex2f(a.x, a.y);
//		glVertex2f(b.x, b.y);
//	} glEnd();
//
//	cpVect delta = cpvsub(b, a);
//
//	glVertexPointer(2, GL_FLOAT, 0, springVAR);
//	glPushMatrix(); {
//		GLfloat x = a.x;
//		GLfloat y = a.y;
//		GLfloat cos = delta.x;
//		GLfloat sin = delta.y;
//		GLfloat s = 1.0f/cpvlength(delta);
//
//		const GLfloat matrix[] = {
//			 cos,  sin, 0.0f, 0.0f,
//			-sin*s,  cos*s, 0.0f, 0.0f,
//			0.0f, 0.0f, 1.0f, 1.0f,
//				 x,    y, 0.0f, 1.0f,
//		};
//		
//		glMultMatrixf(matrix);
//		glDrawArrays(GL_LINE_STRIP, 0, springVAR_count);
//	} glPopMatrix();
//}

//static void
//drawConstraint(cpConstraint *constraint)
//{
//	cpBody *body_a = constraint->a;
//	cpBody *body_b = constraint->b;
//
//	const cpConstraintClass *klass = constraint->klass;
//	if(klass == &cpPinJointClass){
//		cpPinJoint *joint = (cpPinJoint *)constraint;
//	
//		cpVect a = cpvadd(body_a->p, cpvrotate(joint->anchr1, body_a->rot));
//		cpVect b = cpvadd(body_b->p, cpvrotate(joint->anchr2, body_b->rot));
//
//		glPointSize(5.0f);
//		glBegin(GL_POINTS); {
//			glVertex2f(a.x, a.y);
//			glVertex2f(b.x, b.y);
//		} glEnd();
//
//		glBegin(GL_LINES); {
//			glVertex2f(a.x, a.y);
//			glVertex2f(b.x, b.y);
//		} glEnd();
//	} else if(klass == &cpSlideJointClass){
//		cpSlideJoint *joint = (cpSlideJoint *)constraint;
//	
//		cpVect a = cpvadd(body_a->p, cpvrotate(joint->anchr1, body_a->rot));
//		cpVect b = cpvadd(body_b->p, cpvrotate(joint->anchr2, body_b->rot));
//
//		glPointSize(5.0f);
//		glBegin(GL_POINTS); {
//			glVertex2f(a.x, a.y);
//			glVertex2f(b.x, b.y);
//		} glEnd();
//
//		glBegin(GL_LINES); {
//			glVertex2f(a.x, a.y);
//			glVertex2f(b.x, b.y);
//		} glEnd();
//	} else if(klass == &cpPivotJointClass){
//		cpPivotJoint *joint = (cpPivotJoint *)constraint;
//	
//		cpVect a = cpvadd(body_a->p, cpvrotate(joint->anchr1, body_a->rot));
//		cpVect b = cpvadd(body_b->p, cpvrotate(joint->anchr2, body_b->rot));
//
//		glPointSize(10.0f);
//		glBegin(GL_POINTS); {
//			glVertex2f(a.x, a.y);
//			glVertex2f(b.x, b.y);
//		} glEnd();
//	} else if(klass == &cpGrooveJointClass){
//		printf("Cannot draw constraint\n");
//	} else if(klass == &cpDampedSpringClass){
//		drawSpring((cpDampedSpring *)constraint, body_a, body_b);
//	} else if(klass == &cpBreakableJointClass){
//		cpBreakableJoint *breakable = (cpBreakableJoint *)constraint;
//		drawConstraint(breakable->delegate);
//	} else {
//		printf("Cannot draw constraint\n");
//	}
//}

static void
drawBB(void *ptr, void *unused)
{
	cpShape *shape = (cpShape *)ptr;

	glBegin(GL_LINE_LOOP); {
		glVertex2f(shape->bb.l, shape->bb.b);
		glVertex2f(shape->bb.l, shape->bb.t);
		glVertex2f(shape->bb.r, shape->bb.t);
		glVertex2f(shape->bb.r, shape->bb.b);
	} glEnd();
}

static void
drawCollisions(void *ptr, void *data)
{
	cpArbiter *arb = (cpArbiter *)ptr;
	for(int i=0; i<arb->numContacts; i++){
		cpVect v = arb->contacts[i].p;
		glVertex2f(v.x, v.y);
	}
}

void drawSpace(cpSpace *space)
{
	glMatrixMode(GL_MODELVIEW);
//	glLineWidth(1.0f);
//	if(options->drawBBs){
//		glColor3f(0.6, 1.0, 0.6);
//		cpSpaceHashEach(space->activeShapes, &drawBB, NULL);
//		cpSpaceHashEach(space->staticShapes, &drawBB, NULL);
//	}

//	glLineWidth(options->lineThickness);
//	if(options->drawShapes){
		cpSpaceHashEach(space->activeShapes, &drawObject, NULL);
		cpSpaceHashEach(space->staticShapes, &drawObject, NULL);
//	}
	
//	cpArray *constraints = space->constraints;
//
//	glColor3f(0.5f, 1.0f, 0.5f);
//	for(int i=0, count = constraints->num; i<count; i++){
//		drawConstraint(constraints->arr[i]);
//	}
	
//	if(options->bodyPointSize){
//		cpArray *bodies = space->bodies;
//
//		glPointSize(options->bodyPointSize);
//		glBegin(GL_POINTS); {
//			glColor3f(LINE_COLOR);
//			for(int i=0, count = bodies->num; i<count; i++){
//				cpBody *body = (cpBody *)bodies->arr[i];
//				glVertex2f(body->p.x, body->p.y);
//			}
//		} glEnd();
//	}

//	if(options->collisionPointSize){
//		glPointSize(options->collisionPointSize);
//		glBegin(GL_POINTS); {
//			glColor3f(COLLISION_COLOR);
//			cpArrayEach(space->arbiters, &drawCollisions, NULL);
//		} glEnd();
//	}
}
