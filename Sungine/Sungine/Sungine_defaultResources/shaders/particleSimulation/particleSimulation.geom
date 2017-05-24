#version 410 core

#define POSITIONS 0
#define VELOCITIES 1
#define FORCES 2
#define ELAPSED_TIMES 3
#define LIFE_TIMES 4
#define COLORS 5
#define SIZES 6

layout(std140, column_major) uniform;
layout(points) in;
layout(points) out;
layout(max_vertices = 30) out;

//uniform :
uniform float DeltaTime;

//inputs :
in vec3 positions[]; 
//in vec3 velocities[];
//in vec3 forces[];
//in float elapsedTimes[];
//in float lifeTimes[];
//in vec4 colors[];
//in vec2 sizes[];

//outputs :
out vec3 outPositions; 
//out vec3 outVelocities;
//out vec3 outForces;
//out float outElapsedTimes;
//out float outLifeTimes;
//out vec4 outColors;
//out vec2 outSizes;

void main()
{
	//float fixedMass = 0.1;

	////update time :
	//outElapsedTimes = elapsedTimes[0] + DeltaTime;

	////forces update :
	//outVelocities =  velocities[0] +  (DeltaTime / fixedMass)*vec3(0.0,0.1,0.0);
	outPositions = vec3(10,9,8);// positions[0] + DeltaTime*velocities[0];
	
	////todo :
	//outColors = colors[0];
	//outSizes = sizes[0];

	////remove ?
	//outForces = forces[0];
	//outLifeTimes = lifeTimes[0];

	EmitVertex();
	EndPrimitive();
}
