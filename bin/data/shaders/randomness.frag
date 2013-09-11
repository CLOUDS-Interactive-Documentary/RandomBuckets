#version 120
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect pong;
uniform float randSeed;

varying vec2 uv;

float rand(vec2 n)
{
	return 0.5 + 0.5 *
	fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}


void main(void)
{
	float r = rand( uv + vec2( 0., randSeed ) ) * .002;
	r += texture2DRect( pong, uv ).r;
	if(r > 1.) r = 0.;
	
	gl_FragColor = vec4(r,r,r,1.);
}

