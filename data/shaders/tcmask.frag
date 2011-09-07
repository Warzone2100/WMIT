uniform sampler2D Texture0;
uniform sampler2D Texture1;
uniform vec4 teamcolour;

void main(void)
{
	vec4 colour, mask;

	// Get color and tcmask information from TIUs 0-1
	colour = texture2D(Texture0, gl_TexCoord[0].st);
	mask  = texture2D(Texture1, gl_TexCoord[0].st);

	// Apply color using "Merge grain" within tcmask
	gl_FragColor = (colour + (teamcolour - 0.5) * mask.a) * gl_Color;
}
