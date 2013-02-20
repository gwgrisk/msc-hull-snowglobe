
// TODO: 
// Improvement:
// The idea of IGeometry is that each class that implements this interface will be
// responsible for procedurally generating vertices, indices, normals etc.
//
// A class that contains an IGeometry object will instantiate the IGeometry object
// with various args in order to tailor the geometry to their needs.  The IGeometry
// is then moved over to the gfx card (via VBO).  At that point, the geometry data,
// allocated on the heap, will no longer be needed.  
// 
// The improvement would allow the programmer to discard the allocated heap, 
// but maintain geometry state vars e.g. vert count etc.


#pragma once
class CustomVertex;

// interface/contract for procedurally generated geometry objects
class IGeometry
{
protected:
	enum CoOrds		{X, Y, Z};
	enum TexCoOrds	{U, V};
	enum Colours	{R, G, B, A};

protected:
	virtual ~IGeometry(){}	

public:
	virtual bool GenerateData(){ return false; }
	virtual const int VertCount() const = 0;
	virtual const int IndexCount() const	{return 0;}
	virtual const int TriCount() const		{return 0;}

	virtual CustomVertex** Vertices() =0;
	// virtual DWORD* Indices() { return NULL; }
	virtual unsigned short* Indices() { return NULL; }
};