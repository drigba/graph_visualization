#include "framework.h"
#include <time.h>
 
 
// vertex shader in GLSL: It is a Raw string (C++11) since it contains new line characters
const char* const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers
 
	uniform mat4 MVP;			// uniform variable, the Model-View-Projection transformation matrix
	layout(location = 0) in vec2 vp;	// Varying input: vp = vertex position is expected in attrib array 0
	layout(location = 1) in vec3 vertexUV;
	out vec3 texCoord;
	void main() {
		texCoord = vertexUV;
		gl_Position = vec4(vp.x, vp.y, 0, 1) * MVP;		// transform vp from modeling space to normalized device space
	}
)";
 
// fragment shader in GLSL
const char* const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers
	
	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel
	in vec3 texCoord;
 
 
	void main() {
		//int i = Mandelbrot(texCoord * 3 - vec2(2, 1.5));
		//float x = ((texCoord.x-1)/50)-1;
		//float y = ((texCoord.y-1)/50)-1;
 
		outColor = vec4(texCoord,  1); 
		//outColor = vec4((i % 5)/5.0f, (i % 11) / 11.0f, (i % 31) / 31.0f, 1);
		//outColor = vec4(texCoord.x, texCoord.y,texCoord.x,  1); 
 
 
	}
)";
 
GPUProgram gpuProgram; // vertex and fragment shaders
 
// Initialization, create an OpenGL context
 
 
 
 
class Vertex {
public:
	vec2 velocity;
	vec2 pos;
	vec2 vel2;
	void move() {
		pos = velocity;
		velocity = vec2(0, 0);
 
	}
	vec2 diff(Vertex v)
	{
		return vec2(0, 0);
	}
};
 
 
float veve(vec2 k)
{
	float pre = 1.0f + powf(k.x, 2.0f) + powf(k.y, 2.0f);
	float vev = sqrtf(pre);
 
	return vev;
 
}
 
 
 
 
float Lorentz(vec2 k, vec2 l) {
	float f = k.x * l.x + k.y * l.y - veve(k) * veve(l);
 
	return  f;
 
}
 
 
float distance(vec2 k, vec2 l)
{
	
	float p =-1* (Lorentz(k, l));
	if (p < 1.0001f)
		p = 1.0001f;
	
	float f = acoshf(p);
 
	return f;
 
}
 
 
 
const int nv = 100;
const int satu = 5;
const int vert = 50;
const int ne = vert * (vert - 1) * satu / 100;
float mass = 1.0f;
float dt = 0.03f;
float c1 = 0.4f;
float c2 = 0.4f;
float c3 = 0.5f;
 
float mu = 0.2f;
bool pres = false;
bool init = true;
vec2 point(vec2 p, vec2 v, float dist) {
 
	vec2 ret = p * coshf( dist) + v * sinhf( dist);
	return ret;
}
 
vec2 vector(vec2 to, vec2 from, float dist)
{
	vec2 ret = (to - from * coshf(dist)) / sinhf(dist);
	return ret;
 
}
vec2 point_from_vector(vec2 to, vec2 from, float dist1, float dist2) {
	vec2 q = vector(to, from, 2*dist1);
	vec2 ret = point(from, q, 2*dist2);
	return ret;
}
class Graph {
 
 
public:
	vec2 bot;
	Vertex vertices[vert];
	unsigned int vao_m[vert];
	unsigned int vao_e[ne];
	int edges2[ne][2];
	vec2 cp[nv];
	vec2 edges[2* ne];
	unsigned int vbo[2];
	unsigned int c_vbo[2];
 
	bool adjacent(int i, int j)
	{
		
		for (int k = 0; k < ne; ++k)
		{
			if ((edges2[k][0] == i && edges2[k][1] == j) || (edges2[k][1] == i && edges2[k][0] == j))
			{
				return true;
			}
 
		}
		return false;
 
	}
 
 
	void calcForce() {
		for (int i = 0; i < vert; ++i)
		{
			vec2 tmp[vert-1];
			int f = 0;
			for (int j = 0; j < vert; ++j)
			{
				
				if (i != j)
				{
					
					float d = distance(vertices[i].pos, vertices[j].pos);
 
						vec2 v = vector(vertices[i].pos, vertices[j].pos, d);
 
						float str = c1 / (d * d);
						if (adjacent(i, j))
						{
 
								v = v * -1;
								str =log10f(d / c1);
 
 
						}
						str = str * mu * dt / mass;
						vec2 p = point(vertices[i].pos, v, 2 * str);
 
 
						tmp[f] = p;
	
 
						++f;
				}
				
				
			}
			
			vec2 tmp2 = vertices[i].vel2;
			for (int j = 0; j < vert-1; ++j)
			{
				float d1 = distance(vertices[i].pos, tmp2);
				if (j != 0)
				{
						vec2 v1 = (tmp2 - vertices[i].pos * coshf(d1)) / sinhf(d1);
						vec2 p1 = vertices[i].pos * coshf(1.5 * d1) + v1 * sinhf(1.5 * d1);
						tmp2 = p1;
 
				}
				d1 = distance(vertices[i].pos, tmp2);
				float d2 = distance(tmp2, tmp[j]);
				vec2 v2 = (tmp2 - tmp[j] * coshf(d2)) / sinhf(d2);
				vec2 p_k = tmp[j] * coshf(d2 / 2) + v2 * sinhf(d2 / 2);
				float d3 = distance(p_k, vertices[i].pos);
 
				vec2 v1 = (p_k - vertices[i].pos * coshf(d3)) / sinhf(d3);
				vec2 p1 = vertices[i].pos * coshf(2 * d3) + v1 * sinhf(2 * d3);
				vertices[i].vel2 = p1;
				tmp2 = p_k;
			}
			
			vertices[i].velocity = tmp2;
		}
		for (int i = 0; i < vert; ++i)
		{
			vertices[i].move();
		}
 
	}
 
	void heuristic() {
 
 
		for (int i = 0; i < vert; ++i)
		{
			
			for (int j = 0; j < vert; j++)
			{
				float f_d;
				if (i != j)
				{
					float d = distance(vertices[i].pos, vertices[j].pos);
					vec2 v = vector(vertices[i].pos, vertices[j].pos,  d);
					if (adjacent(i, j))
					{
						if (d > 0.3) {
							v = v * -1;
							
						}
							f_d = d * 0.03;
 
					}
					else
					{
						if (d < 1.5)
							f_d = d * 0.003;
						else {
							f_d = 0;
						}
 
					}
 
 
 
					vec2 p = point(vertices[i].pos, v, f_d);
					vertices[i].pos = p;
 
 
				}
 
			}
			}
			
 
			
 
 
 
		
	}
 
	void create() 
	{
 
		bot = (0, 0);
		srand(4);
		for (int i = 0; i < vert; ++i)
		{
			
			float r_x = -2.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (4)));
			float r_y = -2.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (4)));
			float den = veve(vec2(r_x, r_y));
			float r_x2 = r_x / den;
			float r_y2 = r_y / den;
			vertices[i].pos= vec2(r_x2, r_y2);
		}
 
		for (int i = 0; i < ne; ++i) {
			edges2[i][0] = rand() % vert;
			edges2[i][1] = rand() % vert;
		}
			glGenVertexArrays(vert, &vao_m[0]);	
			glGenVertexArrays(ne, &vao_e[0]);	
 
			glGenBuffers(2, &vbo[0]);	
 
			glGenBuffers(2, &c_vbo[0]);
 
 
			
			
	}
	void drawvert() {
		
		for (int i = 0; i < vert; ++i)
		{
			vec3 c;
			float egy = i / (vert - 1.0f);
			if(i % 4 == 0)
				c = vec3(1.0f, 1.0f, 0.0f) * (1.0f - egy) + vec3(0.0f, 1.0f, 1.0f) * egy;
			if (i % 4 == 1)
				c = vec3(0.0f, 1.0f, 1.0f) * (1.0f - egy) + vec3(1.0f, 0.0f, 1.0f) * egy;
			if (i % 4 == 2)
				c = vec3(1.0f, 1.0f, 0.0f) * (1.0f - egy) + vec3(1.0f, 0.0f, 1.1f) * egy;
			if (i % 4 == 3)
				c = vec3(1.0f, 0.0f, 1.0f) * (1.0f - egy) + vec3(1.0f,0.0f,0.1f) * egy;
			glBindVertexArray(vao_m[i]);		
			vec3 tmp[nv];
 
			const int radius = 0.05f;
			for (int j = 0; j < nv; ++j)
			{
				float fi = j * 2 * M_PI / nv;
				float x = cosf(fi) * 0.03 + vertices[i].pos.x;
				float y = sinf(fi) * 0.03 + vertices[i].pos.y;
				vec2 fg = vec2(x, y);
				float div = veve(fg);
 
				tmp[j] = c;
				cp[j] = vec2(x / div, y / div);
			}
			glBindBuffer(GL_ARRAY_BUFFER, c_vbo[0]);
			glBufferData(GL_ARRAY_BUFFER, 	// Copy to GPU target
				sizeof(vec2) * nv,  // # bytes
				cp,	      	// address
				GL_DYNAMIC_DRAW);	// we do not change later
			glEnableVertexAttribArray(0);  // AttribArray 0
			glVertexAttribPointer(0,       // vbo -> AttribArray 0
				2, GL_FLOAT, GL_FALSE, // two floats/attrib, not fixed-point
				0, NULL); 		     // stride, offset: tightly packed
 
			glBindBuffer(GL_ARRAY_BUFFER, c_vbo[1]);
			glBufferData(GL_ARRAY_BUFFER, 	// Copy to GPU target
				sizeof(vec3)*nv,  // # bytes
				tmp,	      	// address
				GL_DYNAMIC_DRAW);	// we do not change later
			glEnableVertexAttribArray(1);  // AttribArray 0
			glVertexAttribPointer(1,       // vbo -> AttribArray 0
				3, GL_FLOAT, GL_FALSE, // two floats/attrib, not fixed-point
				0, NULL); 		     // stride, offset: tightly packed
 
 
 
 
			float r_w = veve(vertices[i].pos);
 
			mat4 MVPtransf = mat4(1, 0, 0, 0,    // MVP matrix, 
				0, 1, 0, 0,    // row-major!
				0, 0, 0, 0,
				0, 0, 0, 1);
 
			gpuProgram.setUniform(MVPtransf, "MVP");
			glDrawArrays(GL_TRIANGLE_FAN, 0 /*startIdx*/, nv /*# Elements*/);
 
 
 
 
 
 
		}
 
		
	}
	void drawedge() {
 
		vec3 tmp[2];
		for (int i = 0; i < ne; ++i) {
			float r_w1 = sqrtf(powf(vertices[edges2[i][0]].pos.x, 2.0) + powf(vertices[edges2[i][0]].pos.y, 2.0) + 1.0);
			float r_w2 = sqrtf(powf(vertices[edges2[i][1]].pos.x, 2.0) + powf(vertices[edges2[i][1]].pos.y, 2.0) + 1.0);
			glBindVertexArray(vao_e[i]);
			vec2 t[2];
			t[0] = vertices[edges2[i][0]].pos / r_w1;
			t[1] = vertices[edges2[i][1]].pos / r_w2;
			tmp[0] = vec3(1,1,1);
			tmp[1] = vec3(1,1,1);
			glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
			glBufferData(GL_ARRAY_BUFFER, 	// Copy to GPU target
				sizeof(vec2) * 2,  // # bytes
				t,	      	// address
				GL_STATIC_DRAW);	// we do not change later
			glEnableVertexAttribArray(0);  // AttribArray 0
			glVertexAttribPointer(0,       // vbo -> AttribArray 0
				2, GL_FLOAT, GL_FALSE, // two floats/attrib, not fixed-point
				0, NULL); 		     // stride, offset: tightly packed*/
 
			glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
			glBufferData(GL_ARRAY_BUFFER, 	// Copy to GPU target
				sizeof(vec3) *2,  // # bytes
				tmp,	      	// address
				GL_STATIC_DRAW);	// we do not change later
			glEnableVertexAttribArray(1);  // AttribArray 0
			glVertexAttribPointer(1,       // vbo -> AttribArray 0
				3, GL_FLOAT, GL_FALSE, // two floats/attrib, not fixed-point
				0, NULL); 		     // stride, offset: tightly packed*/
 
 
			 // stride, offset: tightly packed*/
 
 
			mat4 MVPtransf = mat4(1.0f, 0, 0, 0,    // MVP matrix, 
				0, 1.0f, 0, 0,    // row-major!
				0, 0, 0, 0,
				0, 0, 0, 1);
			gpuProgram.setUniform(MVPtransf, "MVP");
 
			glLineWidth(1.5);
			glDrawArrays(GL_LINES, 0 /*startIdx*/, 2/*# Elements*/);
		}
 
		
 
 
	}
 
 
};
 Graph g;
void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);
 
	g.create();
	
	// create program for the GPU
	gpuProgram.create(vertexSource, fragmentSource, "outColor");
}
 
// Window has become invalid: Redraw
void onDisplay() {
	glClearColor(0,0,0,0);     // background color
	glClear(GL_COLOR_BUFFER_BIT); // clear frame buffer
	g.drawedge();
	g.drawvert();
	glutSwapBuffers(); // exchange buffers for double buffering
}
 
// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
	if (init)
	{
		for (int i = 0; i < 20; ++i)
			g.heuristic();
		init = false;
	}
	pres = false;
	for (int i = 0; i < 15; i++)
		g.calcForce();
	glutPostRedisplay();
}
 
// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
}
 
// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {	// pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	// Convert to normalized device space
	float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
	float cY = 1.0f - 2.0f * pY / windowHeight;
	float pit = sqrtf(cX * cX + cY * cY);
 
	printf("Mouse moved to (%3.2f, %3.2f)\n", cX, cY);
	float div = veve(vec2(cX, cY));
	vec2 tmp = vec2(cX/div, cY/div);
	float d = distance(tmp, g.bot);
	vec2 v =-1* vector( g.bot, tmp, d);
	vec2 p1 = point(g.bot, v, d / 3);
	vec2 p2 = point(g.bot, v,  (d / 3)+(d/2));
	for (int i = 0; i < vert; ++i)
	{
		float d2 = distance(g.vertices[i].pos, p1);
		vec2 v2 = vector( p1, g.vertices[i].pos, d2);
		vec2 p_tmp = point(g.vertices[i].pos, v2, 2 * d2);
		float d3 = distance(p_tmp, p2);
		vec2 v3 = vector( p2, p_tmp, d3);
		vec2 p_f = point(p_tmp, v3, 2 * d3);
		g.vertices[i].pos = p_f;
 
	}
	g.bot = tmp;
	glutPostRedisplay();
}
 
// Mouse click event
void onMouse(int button, int state, int pX, int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	// Convert to normalized device space
	float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
	float cY = 1.0f - 2.0f * pY / windowHeight;
 
	char* buttonStat;
	switch (state) {
	case GLUT_DOWN: 
		buttonStat = "pressed";
		g.bot = vec2(cX, cY);
		pres = true;
		break;
	case GLUT_UP:   buttonStat = "released"; break;
	}
 
	switch (button) {
	case GLUT_LEFT_BUTTON:   printf("Left button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);   break;
	case GLUT_MIDDLE_BUTTON: printf("Middle button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY); break;
	case GLUT_RIGHT_BUTTON:  printf("Right button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);  break;
	}
	glutPostRedisplay();
 
}
 
// Idle event indicating that some time elapsed: do animation here
void onIdle() {
 
	long time = glutGet(GLUT_ELAPSED_TIME); // elapsed time since the start of the program
 
 
 
}