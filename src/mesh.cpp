#include "mesh.h"
#include <cassert>
#include "includes.h"
#include "shader.h"
#include "extra/textparser.h"
#include "utils.h"
#include <sys/stat.h>

Mesh::Mesh()
{
	vertices_vbo_id = 0;
	normals_vbo_id = 0;
	uvs_vbo_id = 0;
	colors_vbo_id = 0;
}

Mesh::Mesh( const Mesh& m )
{
	vertices = m.vertices;
	normals = m.normals;
	uvs = m.uvs;
	colors = m.colors;
}

Mesh::~Mesh()
{
	if(vertices_vbo_id) glDeleteBuffers(1, &vertices_vbo_id);
	if(normals_vbo_id) glDeleteBuffers(1, &normals_vbo_id);
	if(uvs_vbo_id) glDeleteBuffers(1, &uvs_vbo_id);
	if(colors_vbo_id) glDeleteBuffers(1, &colors_vbo_id);
}

void Mesh::clear()
{
	vertices.clear();
	normals.clear();
	uvs.clear();
	colors.clear();
}


void Mesh::render(int primitive)
{
	assert(vertices.size() && "No vertices in this mesh");

    glEnableClientState(GL_VERTEX_ARRAY);

	if(vertices_vbo_id)
	{
		glBindBuffer( GL_ARRAY_BUFFER, vertices_vbo_id );
		glVertexPointer(3, GL_FLOAT, 0, NULL );
	}
	else
		glVertexPointer(3, GL_FLOAT, 0, &vertices[0] );

	if (normals.size())
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		if(normals_vbo_id)
		{
			glBindBuffer( GL_ARRAY_BUFFER, normals_vbo_id );
			glNormalPointer(GL_FLOAT, 0, NULL );
		}
		else
			glNormalPointer(GL_FLOAT, 0, &normals[0] );
	}

	if (uvs.size())
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if(uvs_vbo_id)
		{
			glBindBuffer( GL_ARRAY_BUFFER, uvs_vbo_id );
			glTexCoordPointer(2, GL_FLOAT, 0, NULL );
		}
		else
			glTexCoordPointer(2, GL_FLOAT, 0, &uvs[0] );
	}

	if (colors.size())
	{
		glEnableClientState(GL_COLOR_ARRAY);
		if(colors_vbo_id)
		{
			glBindBuffer( GL_ARRAY_BUFFER, colors_vbo_id );
			glColorPointer(4, GL_FLOAT, 0, NULL );
		}
		else
			glColorPointer(4, GL_FLOAT, 0, &colors[0] );
	}
    
	glDrawArrays(primitive, 0, (GLsizei)vertices.size() );
	glDisableClientState(GL_VERTEX_ARRAY);

	if (normals.size())
		glDisableClientState(GL_NORMAL_ARRAY);
	if (uvs.size())
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if (colors.size())
		glDisableClientState(GL_COLOR_ARRAY);
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void Mesh::render( int primitive, Shader* sh )
{
	if(!sh || !sh->compiled)
        return render(primitive);
    
	assert(vertices.size() && "No vertices in this mesh");

    int vertex_location = sh->getAttribLocation("a_vertex");
	assert(vertex_location != -1 && "No a_vertex found in shader");

	if(vertex_location == -1)
		return;

    glEnableVertexAttribArray(vertex_location);
	if(vertices_vbo_id)
	{
		glBindBuffer( GL_ARRAY_BUFFER, vertices_vbo_id );
		glVertexAttribPointer(vertex_location, 3, GL_FLOAT, GL_FALSE, 0, NULL );
	}
	else
		glVertexAttribPointer(vertex_location, 3, GL_FLOAT, GL_FALSE, 0, &vertices[0] );

	int normal_location = -1;
    if (normals.size())
    {
        normal_location = sh->getAttribLocation("a_normal");
        if(normal_location != -1)
        {
            glEnableVertexAttribArray(normal_location);
			if(normals_vbo_id)
			{
				glBindBuffer( GL_ARRAY_BUFFER, normals_vbo_id );
	            glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, 0, NULL );
			}
			else
	            glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, 0, &normals[0] );
        }
    }
    
	int uv_location = -1;
	if (uvs.size())
	{
        uv_location = sh->getAttribLocation("a_uv");
        if(uv_location != -1)
        {
            glEnableVertexAttribArray(uv_location);
			if(uvs_vbo_id)
			{
				glBindBuffer( GL_ARRAY_BUFFER, uvs_vbo_id );
	            glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, 0, NULL );
			}
			else
	            glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, 0, &uvs[0] );
        }
    }
    
	int color_location = -1;
	if (colors.size())
	{
        color_location = sh->getAttribLocation("a_color");
        if(color_location != -1)
        {
            glEnableVertexAttribArray(color_location);
			if(colors_vbo_id)
			{
				glBindBuffer( GL_ARRAY_BUFFER, colors_vbo_id );
	            glVertexAttribPointer(color_location, 4, GL_FLOAT, GL_FALSE, 0, NULL );
			}
			else
	            glVertexAttribPointer(color_location, 4, GL_FLOAT, GL_FALSE, 0, &colors[0] );
        }
    }
    
	glDrawArrays(primitive, 0, (GLsizei)vertices.size() );

	glDisableVertexAttribArray( vertex_location );
	if(normal_location != -1) glDisableVertexAttribArray( normal_location );
	if(uv_location != -1) glDisableVertexAttribArray( uv_location );
	if(color_location != -1) glDisableVertexAttribArray( color_location );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void Mesh::uploadToVRAM()
{
	if (glGenBuffers == 0)
	{
		std::cout << "Error: your graphics cards dont support VBOs. Sorry." << std::endl;
		exit(0);
	}

	//delete old
	if(vertices_vbo_id) glDeleteBuffers(1, &vertices_vbo_id);
	if(normals_vbo_id) glDeleteBuffers(1, &normals_vbo_id);
	if(uvs_vbo_id) glDeleteBuffers(1, &uvs_vbo_id);
	if(colors_vbo_id) glDeleteBuffers(1, &colors_vbo_id);

	glGenBuffers( 1, &vertices_vbo_id ); //generate one handler (id)
	glBindBuffer( GL_ARRAY_BUFFER, vertices_vbo_id ); //bind the handler
	glBufferData( GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(float), &vertices[0], GL_STATIC_DRAW ); //upload data

	if(normals.size())
	{
		glGenBuffers( 1, &normals_vbo_id); //generate one handler (id)
		glBindBuffer( GL_ARRAY_BUFFER, normals_vbo_id ); //bind the handler
		glBufferData( GL_ARRAY_BUFFER, normals.size() * 3 * sizeof(float), &normals[0], GL_STATIC_DRAW ); //upload data
	}

	if(uvs.size())
	{
		glGenBuffers( 1, &uvs_vbo_id); //generate one handler (id)
		glBindBuffer( GL_ARRAY_BUFFER, uvs_vbo_id ); //bind the handler
		glBufferData( GL_ARRAY_BUFFER, uvs.size() * 2 * sizeof(float), &uvs[0], GL_STATIC_DRAW ); //upload data
	}

	if(colors.size())
	{
		glGenBuffers( 1, &colors_vbo_id); //generate one handler (id)
		glBindBuffer( GL_ARRAY_BUFFER, colors_vbo_id ); //bind the handler
		glBufferData( GL_ARRAY_BUFFER, colors.size() * 4 * sizeof(float), &colors[0], GL_STATIC_DRAW ); //upload data
	}

}

void Mesh::createQuad(float center_x, float center_y, float w, float h, bool flip_uvs )
{
	vertices.clear();
	normals.clear();
	uvs.clear();
	colors.clear();

	//create six vertices (3 for upperleft triangle and 3 for lowerright)

	vertices.push_back( Vector3( center_x + w*0.5f, center_y + h*0.5f, 0.0f ) );
	vertices.push_back( Vector3( center_x - w*0.5f, center_y - h*0.5f, 0.0f ) );
	vertices.push_back( Vector3( center_x + w*0.5f, center_y - h*0.5f, 0.0f ) );
	vertices.push_back( Vector3( center_x - w*0.5f, center_y + h*0.5f, 0.0f ) );
	vertices.push_back( Vector3( center_x - w*0.5f, center_y - h*0.5f, 0.0f ) );
	vertices.push_back( Vector3( center_x + w*0.5f, center_y + h*0.5f, 0.0f ) );

	//texture coordinates
	uvs.push_back( Vector2(1.0f,flip_uvs ? 0.0f : 1.0f) );
	uvs.push_back( Vector2(0.0f,flip_uvs ? 1.0f : 0.0f) );
	uvs.push_back( Vector2(1.0f,flip_uvs ? 1.0f : 0.0f) );
	uvs.push_back( Vector2(0.0f,flip_uvs ? 0.0f : 1.0f) );
	uvs.push_back( Vector2(0.0f,flip_uvs ? 1.0f : 0.0f) );
	uvs.push_back( Vector2(1.0f,flip_uvs ? 0.0f : 1.0f) );

	//all of them have the same normal
	normals.push_back( Vector3(0.0f,0.0f,1.0f) );
	normals.push_back( Vector3(0.0f,0.0f,1.0f) );
	normals.push_back( Vector3(0.0f,0.0f,1.0f) );
	normals.push_back( Vector3(0.0f,0.0f,1.0f) );
	normals.push_back( Vector3(0.0f,0.0f,1.0f) );
	normals.push_back( Vector3(0.0f,0.0f,1.0f) );
}


void Mesh::createPlane(float size)
{
	vertices.clear();
	normals.clear();
	uvs.clear();
	colors.clear();

	//create six vertices (3 for upperleft triangle and 3 for lowerright)

	vertices.push_back( Vector3(size,0,size) );
	vertices.push_back( Vector3(size,0,-size) );
	vertices.push_back( Vector3(-size,0,-size) );
	vertices.push_back( Vector3(-size,0,size) );
	vertices.push_back( Vector3(size,0,size) );
	vertices.push_back( Vector3(-size,0,-size) );

	//all of them have the same normal
	normals.push_back( Vector3(0,1,0) );
	normals.push_back( Vector3(0,1,0) );
	normals.push_back( Vector3(0,1,0) );
	normals.push_back( Vector3(0,1,0) );
	normals.push_back( Vector3(0,1,0) );
	normals.push_back( Vector3(0,1,0) );

	//texture coordinates
	uvs.push_back( Vector2(1,1) );
	uvs.push_back( Vector2(1,0) );
	uvs.push_back( Vector2(0,0) );
	uvs.push_back( Vector2(0,1) );
	uvs.push_back( Vector2(1,1) );
	uvs.push_back( Vector2(0,0) );
}


bool Mesh::loadASE( const char* filename){
    long time1 = getTime();

	//check if binary exists:
	struct stat buffer;
	std::string binfilename = ".bin";
	binfilename = filename + binfilename;
	if(stat (binfilename.c_str(), &buffer) == 0){
		loadBIN(binfilename.c_str());
	}else{
        std::vector<Vector3> all_vertex;
        std::vector<Vector2> all_uvs;
        TextParser t;
        if(!t.create(filename)) {
            std::cout << "File not found" << std::endl;
            return false;
        }

        t.seek("*MESH_NUMVERTEX");
        int num_vertex = t.getint();
        t.seek("*MESH_NUMFACES");
        int num_faces = t.getint();

        all_vertex.resize(num_vertex);
        for(int i = 0 ; i < all_vertex.size() ; ++i){
            t.seek("*MESH_VERTEX");
            t.getint();
            float x = t.getfloat(), y= t.getfloat(), z=t.getfloat();
            Vector3 v( z, y, x );
            //std::cout<<x<<" "<<y<<" "<<z<<'\n';
            all_vertex[i] = v;
        }

        vertices.resize(num_faces*3);
        for(int i = 0 ; i < vertices.size() ;){
            t.seek("*MESH_FACE");
            t.getword();t.getword();
            int A = t.getint();
            t.getword();
            int B = t.getint();
            t.getword();
            int C = t.getint();

            vertices[i++]= all_vertex[C];
            vertices[i++]= all_vertex[B];
            vertices[i++]= all_vertex[A];
        }

        normals.resize(num_faces);
        for(int i = 0 ; i < normals.size() ; ++i) {
            t.seek("*MESH_FACENORMAL");
            t.getint();
            float x = t.getfloat(), y = t.getfloat(), z = - t.getfloat();
            normals[i] = Vector3(x, y, z);
        }

        colors.resize(vertices.size());
        t.reset();
        for(int i = 0 ; i < colors.size() ; ++i){
            t.seek("*MESH_VERTEXNORMAL");
            t.getint();
            float r = t.getfloat(), g = t.getfloat(), b = t.getfloat();
            colors[i] = Vector4(b,g,r,1);
        }

        t.reset();
        t.seek("*MESH_NUMTVERTEX");
        int num_uvvertex = t.getint();
        all_uvs.resize(num_uvvertex);
        for(int i = 0 ; i < num_uvvertex ; ++i){
            t.seek("*MESH_TVERT");
            t.getint();
            float u = t.getfloat(), v = t.getfloat();
            Vector2 uv( u, v );
            all_uvs[i] = uv;
        }

        t.seek("*MESH_NUMTVFACES");
        int num_uvs = t.getint();
        uvs.resize(num_uvs * 3);
        for(int i = 0 ; i < num_uvs * 3;){
            t.seek("*MESH_TFACE");
            t.getint();
            int A = t.getint(), B = t.getint(), C = t.getint();
            uvs[i++] = all_uvs[C];
            uvs[i++] = all_uvs[B];
            uvs[i++] = all_uvs[A];
        }
    }
    long time2 = getTime();
    printf("Time elapsed: %f ms\n",(time2-time1));

    storeBIN(binfilename.c_str());
    return true;
}

bool Mesh::loadBIN(const char *filename) {
	long time1 = getTime();
	FILE* file = fopen(filename, "rb");
	if(file == NULL){
		std::cout << "File not found" << std::endl;
		return false;
	}
	unsigned long N_vertices, N_normals, N_uvs, N_colors;
	fread(&N_vertices, sizeof(unsigned long), 1, file);
	fread(&N_normals, sizeof(unsigned long), 1, file);
	fread(&N_uvs, sizeof(unsigned long), 1, file);
	fread(&N_colors, sizeof(unsigned long), 1, file);

	vertices.resize(N_vertices);
	normals.resize(N_normals);
	uvs.resize(N_uvs);
	colors.resize(N_colors);

	fread(&vertices[0], sizeof(Vector3), N_vertices, file);
	fread(&normals[0], sizeof(Vector3), N_normals, file);
	fread(&uvs[0], sizeof(Vector2), N_uvs, file);
	fread(&colors[0], sizeof(Vector4), N_colors, file);

	long time2 = getTime();
	printf("Time elapsed: %li ms\n",(time2-time1));
    return true;
}

bool Mesh::storeBIN(const char *filename) {
	std::cout << "Storing Mesh in .bin format. . ." << std::endl;
	FILE* file = fopen(filename, "wb");
	if(file == NULL){
		std::cout << "An error ocurred while trying to open the file" << std::endl;
		return false;
	}

	unsigned long N_vertices = vertices.size(),
			N_normals = normals.size(),
			N_uvs = uvs.size(),
			N_colors = colors.size();

	fwrite(&N_vertices, sizeof(unsigned long), 1, file);
	fwrite(&N_normals, sizeof(unsigned long), 1, file);
	fwrite(&N_uvs, sizeof(unsigned long), 1, file);
	fwrite(&N_colors, sizeof(unsigned long), 1, file);

	fwrite(&vertices[0], sizeof(Vector3), N_vertices, file);
	fwrite(&normals[0], sizeof(Vector3), N_normals, file);
	fwrite(&uvs[0], sizeof(Vector2), N_uvs, file);
	fwrite(&colors[0], sizeof(Vector4), N_colors, file);

    fclose(file);
	return true;
}

void Mesh::debugVerticesAsColor() {
	colors.clear();
	colors.resize(vertices.size());

	for(int i=0; i<vertices.size(); i++){
		colors[i] = Vector4(vertices[i], 0.0);
	}
}

void Mesh::debugNormalsAsColor() {
	colors.clear();
	colors.resize(normals.size());

	for(int i=0; i<normals.size(); i++){
		colors[i] = Vector4(normals[i], 0.0);
	}
}