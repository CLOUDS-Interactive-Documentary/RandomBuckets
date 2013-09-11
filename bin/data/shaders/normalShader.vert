#version 120
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect randomness;
uniform float minY;
uniform float maxY;
uniform vec4 color;

uniform float seedTime;

uniform float maxCubeScale = 20.;

uniform float rExpo = 2.;
uniform float rScale = 1.5;

varying vec4 col;

void main()
{
	
	vec2 uv = gl_MultiTexCoord0.xy;
	
	float r = texture2DRect( randomness, uv ).r;
	
	
	col = color * pow( r * rScale, rExpo );
	
	vec3 cubeCenter = gl_Normal.xyz;
	vec3 vPos = gl_Vertex.xyz;
	float cubeScale = r * maxCubeScale;
	vPos.y *= cubeScale;
	
	//move the cubes up and down if they're hitting the top or bottom
	float cubeTop = cubeCenter.y + cubeScale * .5;
	float cubeBot = cubeCenter.y - cubeScale * .5;
	if( cubeBot < minY )
	{
		cubeCenter.y += distance( minY, cubeBot );
	}
	else if( cubeTop > maxY )
	{
		cubeCenter.y -= distance( maxY, cubeTop );
	}

	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(cubeCenter + vPos, 1.);
	
}

