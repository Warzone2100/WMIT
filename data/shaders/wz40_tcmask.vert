// Version directive is set by Warzone when loading the shader
// (This shader supports GLSL 1.20 - 1.50 core.)

//#pragma debug(on)

uniform float stretch;
uniform mat4 ModelViewMatrix;
uniform mat4 ModelViewProjectionMatrix;
uniform mat4 NormalMatrix;
uniform int hasTangents; // whether tangents were calculated for model
uniform vec4 lightPosition;

#if (!defined(GL_ES) && (__VERSION__ >= 130)) || (defined(GL_ES) && (__VERSION__ >= 300))
in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexCoord;
in vec4 vertexTangent;
#else
attribute vec4 vertex;
attribute vec3 vertexNormal;
attribute vec2 vertexTexCoord;
attribute vec4 vertexTangent;
#endif

#if (!defined(GL_ES) && (__VERSION__ >= 130)) || (defined(GL_ES) && (__VERSION__ >= 300))
out float vertexDistance;
out vec3 normal, lightDir, halfVec;
out vec2 texCoord;
out mat3 TangentSpaceMatrix;
#else
varying float vertexDistance;
varying vec3 normal, lightDir, halfVec;
varying vec2 texCoord;
varying mat3 TangentSpaceMatrix;
#endif

void main()
{
	// Pass texture coordinates to fragment shader
	texCoord = vertexTexCoord;

	// Lighting we pass to the fragment shader
	// eyeVec runs surface -> eye, as in WZ's tcmask.vert ("normalize(-viewVertex.xyz)").
	vec3 eyeVec = -normalize((ModelViewMatrix * vertex).xyz);
	vec3 n = normalize((NormalMatrix * vec4(vertexNormal, 0.0)).xyz);
	// lightPosition arrives already inverted from QWZM.cpp, which converts WMIT's
	// light POSITION (origin -> light) into the sun DIRECTION vector WZ's shaders
	// expect. Negating here is therefore the same expression WZ uses.
	lightDir = -normalize(lightPosition.xyz);

	if (hasTangents != 0)
	{
		// Building the matrix Tangent Space -> Eye Space with handness
		vec3 t = normalize((NormalMatrix * vertexTangent).xyz);

		// WMIT renders through wz_scale = (-1/128, 1/128, 1/128) (QWZM.cpp), whose
		// negative x makes the model transform a MIRROR - its determinant is
		// negative. Under such a transform cross(n, t) comes out with the opposite
		// handedness to a correctly transformed bitangent, so the frame built here
		// is left-handed relative to the game's and normal-map relief lights
		// inverted along V. WZ's engine has no such mirror, which is why its
		// shaders need no equivalent.
		//
		// determinant() is GLSL 1.50; this shader targets 1.20, hence the explicit
		// triple product.
		mat3 nm = mat3(NormalMatrix);
		float mirror = sign(dot(cross(nm[0], nm[1]), nm[2]));

		vec3 b = cross (n, t) * vertexTangent.w * mirror;

		// Handed to the fragment shader, which applies it - as WZ's tcmask.vert
		// does. Nothing is rotated INTO tangent space here: lightDir, eyeVec and
		// the geometric normal all stay in eye space, so the mapped and unmapped
		// cases are lit in the same space without a second convention.
		TangentSpaceMatrix = mat3(t, b, n); // conventional (T, B, N)
	}

	normal = n;
	halfVec = lightDir + eyeVec; //can be normalized for better quality

	// Implement building stretching to accommodate terrain
	vec4 position = vertex;
	if (vertex.y <= 0.0) // use vertex here directly to help shader compiler optimization
	{
		position.y -= stretch;
	}

	// Translate every vertex according to the Model View and Projection Matrix
	vec4 gposition = ModelViewProjectionMatrix * position;
	gl_Position = gposition;

	// Remember vertex distance
	vertexDistance = gposition.z;
}
