#version 120
#pragma debug(on)

varying float vertexDistance;
varying vec3 normal, lightDir, eyeVec;

uniform sampler2D Texture0; //diffuse
uniform sampler2D Texture1; //tcmask
uniform sampler2D Texture2; //normal map
uniform vec4 teamcolour;
uniform int tcmask, normalmap;
uniform int fogEnabled;

void main(void)
{
	vec4 mask, colour;
	vec4 light = (gl_FrontLightModelProduct.sceneColor * gl_FrontMaterial.ambient) + (gl_LightSource[0].ambient * gl_FrontMaterial.ambient);
	vec3 N;
	vec3 L;
	
	//normal map implementation
	if (normalmap == 1)
	{
		vec3 bump = texture2D(Texture2, gl_TexCoord[0].st).xyz;
		bump = (bump * 2.0) - 1;
		
		// object space
		{
			L = normalize(lightDir);
			bump = bump.xzy;
			bump = gl_NormalMatrix * bump;
		}
		N = normalize(bump);
	}
	else //no normalmap
	{
		N = normalize(normal);
		L = normalize(lightDir);
	}

	float lambertTerm = dot(N, L);
	if (lambertTerm > 0.0)
	{
		light += gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * lambertTerm;
		vec3 E = normalize(eyeVec);
		vec3 R = reflect(-L, N);
		float specular = pow(max(dot(R, E), 0.0), gl_FrontMaterial.shininess);
		light += gl_LightSource[0].specular * gl_FrontMaterial.specular * specular;
	}

	// Get color from texture unit 0
	colour = texture2D(Texture0, gl_TexCoord[0].st);

	if (tcmask == 1)
	{
		// Get tcmask information from texture unit 1
		mask = texture2D(Texture1, gl_TexCoord[0].st);
	
		// Apply color using grain merge with tcmask
		gl_FragColor = (colour + (teamcolour - 0.5) * mask.a) * light * gl_Color;
	}
	else
	{
		gl_FragColor = colour * light * gl_Color;
	}

	if (fogEnabled > 0)
	{
		// Calculate linear fog
		float fogFactor = (gl_Fog.end - vertexDistance) / (gl_Fog.end - gl_Fog.start);
		fogFactor = clamp(fogFactor, 0.0, 1.0);
	
		// Return fragment color
		gl_FragColor = mix(gl_Fog.color, gl_FragColor, fogFactor);
	}
}
