#include "triangle_complex.h"

TriangleComplex::TriangleComplex() {
	//Create the global vertex list from scratch
	// + The 0th vertex is always a NULL vertex
	global_vertex_list = new VertexList;
	global_vertex_list->push_back(NULL);

	//Initialize all the other variables
	initialize();
}

TriangleComplex::TriangleComplex(VertexList* global_vertex_list) {
	//Get the global vertex list from the function input
	this->global_vertex_list = global_vertex_list;

	//Initialize all the other variables
	initialize();
}

TriangleComplex::~TriangleComplex() {
	free_data();
}

//General use functions
int TriangleComplex::LoadFromFile(const char* filename) {
	return true;
}

int TriangleComplex::WriteToFile(const char* filename) {
	//Some simple error checking
	if(global_vertex_list->size() <= 1)
		return false;

	//Open the file for writing
	FILE* handle = fopen(filename, "w");
	if(!handle)
		return false;

	//Write the vertex list to a file
	fprintf(handle, "<vertexlist>\n");

	for(unsigned int i=0; i<global_vertex_list->size(); i++) {
		//Skip the first vertex, it is always null
		if(i == 0)
			continue;

		double x = (*global_vertex_list)[i]->x;
		double y = (*global_vertex_list)[i]->y;

		fprintf(handle, "\t<vertex index=\"%u\" x=\"%f\" y=\"%f\"/>\n", i, x, y);
	}
	fprintf(handle, "</vertexlist>\n\n");

	//Write the triangle list to a file
	if(WriteToFile(handle) == false) {
		fclose(handle);
		return false;
	}

	//Close up and return
	fclose(handle);
	return true;
}

int TriangleComplex::WriteToFile(FILE* handle) {
	if(triangle_list->size() == 0)
		return true;

	fprintf(handle, "<trianglelist>\n");

	for(unsigned int i=0; i<triangle_list->size(); i++) {
		unsigned int n1 = (*triangle_list)[i]->GetVertexIndex(0);
		unsigned int n2 = (*triangle_list)[i]->GetVertexIndex(1);
		unsigned int n3 = (*triangle_list)[i]->GetVertexIndex(2);

		fprintf(handle, "\t<triangle n1=\"%u\" n2=\"%u\" n3=\"%u\"/>\n", n1, n2, n3);
	}

	fprintf(handle, "</trianglelist>\n\n");

	return true;
}

//Data management functions
unsigned int TriangleComplex::GetTriangleCount() {
	return triangle_list->size();
}

Triangle* TriangleComplex::GetTriangle(unsigned int tindex) {
	return (*triangle_list)[tindex];
}

unsigned int TriangleComplex::GetVertexCount() {
	return vertex_list->size();
}

unsigned int TriangleComplex::GetVertexIndex(unsigned int vertex) {
	//Safety check
	if(vertex >= GetVertexCount())
		return 0;

	return (*vertex_list)[vertex];
}

Vector2d* TriangleComplex::GetVertex(unsigned int vertex) {
	//Safety check
	if(vertex >= GetVertexCount())
		return NULL;

	unsigned int vindex = GetVertexIndex(vertex);
	return (*global_vertex_list)[vindex];
}

int TriangleComplex::SetVertexIndex(unsigned int vertex, unsigned int vindex) {
	//Safety check
	if(vertex >= GetVertexCount())
		return false;

	(*vertex_list)[vertex] = vindex;
	return true;
}

int TriangleComplex::AppendVertexIndex(unsigned int vindex) {
	vertex_list->push_back(vindex);

	return true;
}

//Meshing functions
int TriangleComplex::RunTriangleMesher() {
	if(GetVertexCount() > MAXIMUM_MESH_SIZE) {
		//Split up mesh via kd-tree
	}

	else {
		//Run a simple triangle mesher
		if(basic_triangle_mesher() == false)
			return false;
	}

	return true;
}

int TriangleComplex::RunDelaunayFlips() {
	return true;
}


//Debugging functions
int TriangleComplex::write_svg(const char* filename, double w, double h) {
	//Simple error check
	if(global_vertex_list->size() <= 1)
		return false;

	//Open the file for writing
	FILE* handle = fopen(filename, "w");
	if(!handle)
		return false;

	//Write a background rectangle
	fprintf(handle, "<svg viewBox=\"0 0 %f %f\" version=\"1.1\">\n", w, h);
	fprintf(handle, "<rect x=\"%f\" y=\"%f\" width=\"%f\" height=\"%f\" fill=\"white\"/>\n", 0.0, 0.0, w, h);

	//Write all the triangle/vertex data
	if(write_svg(handle, w, h) == false) {
		fclose(handle);
		return false;
	}

	//Close the file and return
	fprintf(handle, "</svg>\n");
	fclose(handle);
	return true;
}

int TriangleComplex::write_svg(FILE* handle, double w, double h) {
	//Write the triangles/edges
	for(unsigned int i=0; i<triangle_list->size(); i++)
		(*triangle_list)[i]->write_svg(handle);

	//Write the vertices
	for(unsigned int i=0; i<global_vertex_list->size(); i++) {
		//Skip the first vertex, it is null
		if(i == 0)
			continue;

		double cx = (*global_vertex_list)[i]->x;
		double cy = (*global_vertex_list)[i]->y;
		double r = 3.0;

		fprintf(handle, "<circle cx=\"%f\" cy=\"%f\" r=\"%f\" fill=\"blue\"/>\n", cx, cy, r);
	}

	return true;
}

int TriangleComplex::generate_random_vertex_list(int num, double xmin, double xmax) {
	for(int i=0; i<num; i++) {
		Vector2d* new_vector = new Vector2d;
		new_vector->set(get_rand(xmin, xmax), get_rand(xmin, xmax));

		global_vertex_list->push_back(new_vector);
		AppendVertexIndex(i+1);
	}

	return true;
}


//Internal use functions
int TriangleComplex::initialize() {
	vertex_list = new vector<unsigned int>;
	vertex_list->clear();

	triangle_list = new TriangleList;
	triangle_list->clear();

	//Clear some lists
	incomplete_vertices.clear();
	incomplete_vertices_adjacent_triangles.clear();
	incomplete_edges.clear();

	return true;
}

int TriangleComplex::free_data() {
	//Delete the vertex list
	vertex_list->clear();
	delete vertex_list;

	//Delete the triangle list
	if(triangle_list) {
		for(unsigned int i=0; i<triangle_list->size(); i++)
			if((*triangle_list)[i] != NULL)
				delete (*triangle_list)[i];

		triangle_list->clear();
		delete triangle_list;
	}

	//Clear some lists
	incomplete_vertices.clear();
	incomplete_vertices_adjacent_triangles.clear();
	incomplete_edges.clear();

	return true;
}

int TriangleComplex::basic_triangle_mesher() {
	//Safety test
	if(GetVertexCount() < 4) {
		printf("Error: Not enough vertices\n");
		return false;
	}

	//If necessary create a seed triangle
	if(create_seed_triangle() == false) {
		printf("Error: Could not create seed triangle\n");
		return false;
	}

	//Figure out the list of incomplete vertices and edges
	if(compute_incomplete_vertices_and_edges() == false) {
		printf("Error: Could not compute incomplete vertices and edges\n");
		return false;
	}

	//The meshing algorithm
	int done = false;
	while(!done) {
		//Assert that we are done finding new triangles
		done = true;

		//Search through the list of all the triangles and try to make a new one
		int new_triangle_added = false;


		if(new_triangle_added)
			done = false;
	}

	return true;
}

int TriangleComplex::create_seed_triangle() {
	//Safety check
	if(GetVertexCount() < 4)
		return false;

	if(triangle_list->size() == 0) {
		//Sort all the vertices by how close they are to the center
		Vector2d center(0.0, 0.0);

		for(unsigned int i=0; i<GetVertexCount(); i++) {
			Vector2d* v = GetVertex(i);

			if(v != NULL)
				center += ((*v) / double(GetVertexCount()));
		}

		//Quick-sort on vertex_list
		for(unsigned int i=0; i<GetVertexCount(); i++) {
			unsigned int center_closest_vertex = i;

			for(unsigned int j=i+1; j<GetVertexCount(); j++) {
				Vector2d* vc = GetVertex(center_closest_vertex);
				Vector2d* vj = GetVertex(j);

				if(vc == NULL)
					center_closest_vertex = j;

				else if(vj != NULL && vj->distance2(center) < vc->distance2(center))
					center_closest_vertex = j;
			}

			unsigned int vindex = GetVertexIndex(i);
			SetVertexIndex(i, GetVertexIndex(center_closest_vertex));
			SetVertexIndex(center_closest_vertex, vindex);
		}

		//Safety check
		if(GetVertexIndex(0) == 0 || GetVertexIndex(1) == 0)
			return false;


		//Create a triangle
		Triangle* tri = new Triangle(global_vertex_list);

		tri->SetVertex(0, GetVertexIndex(0));
		tri->SetVertex(1, GetVertexIndex(1));

		//Try to complete the triangle
		for(unsigned int i=2; i<GetVertexCount(); i++) {
			//Try to add a new vertex
			tri->SetVertex(2, GetVertexIndex(i));

			//If the triangle is degenerate reset the vertex to the null vertex
			if(tri->OrientVertices() == false)
				tri->SetVertex(2, 0);

			//Make sure that none of the other vertices are inside this triangle
			else {
				int found_point_inside = false;
				for(unsigned int j=2; j<GetVertexCount(); j++) {
					if(j == i)
						continue;

					Vector2d* pt = GetVertex(j);
					if(pt != NULL && tri->TestPointInside(*pt, false) == true) {
						found_point_inside = true;
						break;
					}
				}

				//This triangle was no good after all, so reset the vertex we tested to null
				if(found_point_inside)
					tri->SetVertex(2, 0);

				//We found a good triangle
				else
					break;
			}
		}

		//If we found a good triangle
		if(tri->GetVertexCount() == 3)
			triangle_list->push_back(tri);

		//Otherwise if we failed
		else {
			delete tri;
			return false;
		}
	}

	return true;
}

int TriangleComplex::compute_incomplete_vertices_and_edges() {
	//Safety checking
	if(GetVertexCount() < 4)
		return false;

	//Clear the lists
	incomplete_vertices.clear();
	incomplete_vertices_adjacent_triangles.clear();
	incomplete_edges.clear();

	//All vertices are considered incomplete for now
	incomplete_vertices = *vertex_list;
	incomplete_vertices_adjacent_triangles.resize(GetVertexCount());

	for(unsigned int i=0; i<GetTriangleCount(); i++) {
		Triangle* tri = GetTriangle(i);

		//Go through the vertices of this triangle
		for(int j=0; j<3; j++) {
			//Manage the incomplete edge list
			TriangleEdge tej(i, j);
			tej.GetVertices(tri);

			vector<TriangleEdge>::iterator find_result;
			find_result = find(incomplete_edges.begin(), incomplete_edges.end(), tej);

			//If edge j was not in the incomplete edge list append it
			if(find_result == incomplete_edges.end())
				incomplete_edges.push_back(tej);

			//Otherwise we know this edge is complete so remove it
			else
				incomplete_edges.erase(find_result);

			//Manage the incomplete vertices list
			unsigned int vindex = tri->GetVertexIndex(j);

			for(unsigned int k=0; k<incomplete_vertices.size(); k++) {
				if(incomplete_vertices[k] == vindex) {
					incomplete_vertices_adjacent_triangles[k].push_back(tri);
					break;
				}
			}
		}
	}

	//Screen out the complete vertices from the incomplete ones
	int removed_vertex = false;
	for(unsigned int i=0; i<incomplete_vertices.size(); i++) {
		if(removed_vertex == true) {
			i--;
			removed_vertex = false;
		}

		unsigned int vindex = incomplete_vertices[i];
		TriangleList tri_list = incomplete_vertices_adjacent_triangles[i];

		if(is_vertex_complete(vindex, tri_list)) {
			incomplete_vertices.erase(incomplete_vertices.begin() + i);
			incomplete_vertices_adjacent_triangles.erase(incomplete_vertices_adjacent_triangles.begin() + i);
			removed_vertex = true;
		}
	}

	return true;
}

int TriangleComplex::is_vertex_complete(unsigned int vindex, TriangleList adjacent_triangles) {
	//This function assumes that all the triangles are oriented ccw, and
	//basically checks to see if the vertex is completely surrounded by triangles

	//Safety check for the null vertex
	// + returning true means the algorithm will overlook this vertex from now on
	if(vindex == 0)
		return true;

	//A vertex needs to be surrounded by at least three triangles to be complete
	if(adjacent_triangles.size() < 3)
		return false;

	//Try to form a cycle of edges around the vertex
	for(unsigned int i=0; i<adjacent_triangles.size(); i++) {
		unsigned int next_vertex = adjacent_triangles[i]->GetNextVertex(vindex);

		int found = false;
		for(unsigned int j=0; j<adjacent_triangles.size(); j++) {
			//There's no need to check a triangle against itself
			if(i == j)
				continue;

			if(next_vertex == adjacent_triangles[j]->GetPrevVertex(vindex)) {
				found = true;
				break;
			}
		}

		//We've determined that this vertex is not surrounded by triangles
		if(found == false)
			return false;
	}

	//We were able to cycle around the vertex with adjacent triangle edges
	return true;
}