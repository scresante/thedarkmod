/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
******************************************************************************/

/*
===============================================================================

	Trace model vs. polygonal model collision detection.

===============================================================================
*/

#include "precompiled.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile("$Id$");

#include "CollisionModel_local.h"

#define CM_FILE_EXT			"cm"
#define CM_FILEID			"CM"
#define CM_FILEVERSION		"1.00"


/*
===============================================================================

Writing of collision model file

===============================================================================
*/

void CM_GetNodeBounds( idBounds *bounds, cm_node_t *node );
int CM_GetNodeContents( cm_node_t *node );


/*
================
idCollisionModelManagerLocal::WriteNodes
================
*/
void idCollisionModelManagerLocal::WriteNodes( idFile *fp, cm_node_t *node ) {
	fp->WriteFloatString( "\t( %d %f )\n", node->planeType, node->planeDist );
	if ( node->planeType != -1 ) {
		WriteNodes( fp, node->children[0] );
		WriteNodes( fp, node->children[1] );
	}
}

/*
================
idCollisionModelManagerLocal::CountPolygonMemory
================
*/
int idCollisionModelManagerLocal::CountPolygonMemory( cm_node_t *node ) const {
	cm_polygonRef_t *pref;
	cm_polygon_t *p;
	int memory;

	memory = 0;
	for ( pref = node->polygons; pref; pref = pref->next ) {
		p = pref->p;
		if ( p->checkcount == checkCount ) {
			continue;
		}
		p->checkcount = checkCount;

		memory += sizeof( cm_polygon_t ) + ( p->numEdges - 1 ) * sizeof( p->edges[0] );
	}
	if ( node->planeType != -1 ) {
		memory += CountPolygonMemory( node->children[0] );
		memory += CountPolygonMemory( node->children[1] );
	}
	return memory;
}

/*
================
idCollisionModelManagerLocal::WritePolygons
================
*/
void idCollisionModelManagerLocal::WritePolygons( idFile *fp, cm_node_t *node ) {
	cm_polygonRef_t *pref;
	cm_polygon_t *p;
	int i;

	for ( pref = node->polygons; pref; pref = pref->next ) {
		p = pref->p;
		if ( p->checkcount == checkCount ) {
			continue;
		}
		p->checkcount = checkCount;
		fp->WriteFloatString( "\t%d (", p->numEdges );
		for ( i = 0; i < p->numEdges; i++ ) {
			fp->WriteFloatString( " %d", p->edges[i] );
		}
		fp->WriteFloatString( " ) ( %f %f %f ) %f", p->plane.Normal()[0], p->plane.Normal()[1], p->plane.Normal()[2], p->plane.Dist() );
		fp->WriteFloatString( " ( %f %f %f )", p->bounds[0][0], p->bounds[0][1], p->bounds[0][2] );
		fp->WriteFloatString( " ( %f %f %f )", p->bounds[1][0], p->bounds[1][1], p->bounds[1][2] );
		fp->WriteFloatString( " \"%s\"\n", p->material->GetName() );
	}
	if ( node->planeType != -1 ) {
		WritePolygons( fp, node->children[0] );
		WritePolygons( fp, node->children[1] );
	}
}

/*
================
idCollisionModelManagerLocal::CountBrushMemory
================
*/
int idCollisionModelManagerLocal::CountBrushMemory( cm_node_t *node ) const {
	cm_brushRef_t *bref;
	cm_brush_t *b;
	int memory;

	memory = 0;
	for ( bref = node->brushes; bref; bref = bref->next ) {
		b = bref->b;
		if ( b->checkcount == checkCount ) {
			continue;
		}
		b->checkcount = checkCount;

		memory += sizeof( cm_brush_t ) + ( b->numPlanes - 1 ) * sizeof( b->planes[0] );
	}
	if ( node->planeType != -1 ) {
		memory += CountBrushMemory( node->children[0] );
		memory += CountBrushMemory( node->children[1] );
	}
	return memory;
}

/*
================
idCollisionModelManagerLocal::WriteBrushes
================
*/
void idCollisionModelManagerLocal::WriteBrushes( idFile *fp, cm_node_t *node ) {
	cm_brushRef_t *bref;
	cm_brush_t *b;
	int i;

	for ( bref = node->brushes; bref; bref = bref->next ) {
		b = bref->b;
		if ( b->checkcount == checkCount ) {
			continue;
		}
		b->checkcount = checkCount;
		fp->WriteFloatString( "\t%d {\n", b->numPlanes );
		for ( i = 0; i < b->numPlanes; i++ ) {
			fp->WriteFloatString( "\t\t( %f %f %f ) %f\n", b->planes[i].Normal()[0], b->planes[i].Normal()[1], b->planes[i].Normal()[2], b->planes[i].Dist() );
		}
		fp->WriteFloatString( "\t} ( %f %f %f )", b->bounds[0][0], b->bounds[0][1], b->bounds[0][2] );
		fp->WriteFloatString( " ( %f %f %f ) \"%s\"\n", b->bounds[1][0], b->bounds[1][1], b->bounds[1][2], StringFromContents( b->contents ) );
	}
	if ( node->planeType != -1 ) {
		WriteBrushes( fp, node->children[0] );
		WriteBrushes( fp, node->children[1] );
	}
}

/*
================
idCollisionModelManagerLocal::WriteCollisionModel
================
*/
void idCollisionModelManagerLocal::WriteCollisionModel( idFile *fp, cm_model_t *model ) {
	int i, polygonMemory, brushMemory;

	fp->WriteFloatString( "collisionModel \"%s\" {\n", model->name.c_str() );
	// vertices
	fp->WriteFloatString( "\tvertices { /* numVertices = */ %d\n", model->numVertices );
	for ( i = 0; i < model->numVertices; i++ ) {
		fp->WriteFloatString( "\t/* %d */ ( %f %f %f )\n", i, model->vertices[i].p[0], model->vertices[i].p[1], model->vertices[i].p[2] );
	}
	fp->WriteFloatString( "\t}\n" );
	// edges
	fp->WriteFloatString( "\tedges { /* numEdges = */ %d\n", model->numEdges );
	for ( i = 0; i < model->numEdges; i++ ) {
		fp->WriteFloatString( "\t/* %d */ ( %d %d ) %d %d\n", i, model->edges[i].vertexNum[0], model->edges[i].vertexNum[1], model->edges[i].internal, model->edges[i].numUsers );
	}
	fp->WriteFloatString( "\t}\n" );
	// nodes
	fp->WriteFloatString( "\tnodes {\n" );
	WriteNodes( fp, model->node );
	fp->WriteFloatString( "\t}\n" );
	// polygons
	checkCount++;
	polygonMemory = CountPolygonMemory( model->node );
	fp->WriteFloatString( "\tpolygons /* polygonMemory = */ %d {\n", polygonMemory );
	checkCount++;
	WritePolygons( fp, model->node );
	fp->WriteFloatString( "\t}\n" );
	// brushes
	checkCount++;
	brushMemory = CountBrushMemory( model->node );
	fp->WriteFloatString( "\tbrushes /* brushMemory = */ %d {\n", brushMemory );
	checkCount++;
	WriteBrushes( fp, model->node );
	fp->WriteFloatString( "\t}\n" );
	// closing brace
	fp->WriteFloatString( "}\n" );
}

/*
================
idCollisionModelManagerLocal::WriteCollisionModelsToFile
================
*/
void idCollisionModelManagerLocal::WriteCollisionModelsToFile( const char *filename, int firstModel, int lastModel, unsigned int mapFileCRC ) {
	int i;
	idFile *fp;
	idStr name;

	name = filename;
	name.SetFileExtension( CM_FILE_EXT );

	common->Printf( "writing %s\n", name.c_str() );
	fp = fileSystem->OpenFileWrite( name, "fs_devpath", "" );
	if ( !fp ) {
		common->Warning( "idCollisionModelManagerLocal::WriteCollisionModelsToFile: Error opening file %s", name.c_str() );
		return;
	}

	// write file id and version
	fp->WriteFloatString( "%s \"%s\"\n\n", CM_FILEID, CM_FILEVERSION );
	// write the map file crc
	fp->WriteFloatString( "%u\n\n", mapFileCRC );

	// write the collision models
	for ( i = firstModel; i < lastModel; i++ ) {
		WriteCollisionModel( fp, models[ i ] );
	}

	fileSystem->CloseFile( fp );
}

/*
================
idCollisionModelManagerLocal::WriteCollisionModelForMapEntity
================
*/
bool idCollisionModelManagerLocal::WriteCollisionModelForMapEntity( const idMapEntity *mapEnt, const char *filename, const bool testTraceModel ) {
	idFile *fp;
	idStr name;
	cm_model_t *model;

	SetupHash();
	model = CollisionModelForMapEntity( mapEnt );
	model->name = filename;

	name = filename;
	name.SetFileExtension( CM_FILE_EXT );

	common->Printf( "writing %s\n", name.c_str() );
	fp = fileSystem->OpenFileWrite( name, "fs_devpath", "" );
	if ( !fp ) {
		common->Printf( "idCollisionModelManagerLocal::WriteCollisionModelForMapEntity: Error opening file %s\n", name.c_str() );
		FreeModel( model );
		return false;
	}

	// write file id and version
	fp->WriteFloatString( "%s \"%s\"\n\n", CM_FILEID, CM_FILEVERSION );
	// write the map file crc
	fp->WriteFloatString( "%u\n\n", 0 );

	// write the collision model
	WriteCollisionModel( fp, model );

	fileSystem->CloseFile( fp );

	if ( testTraceModel ) {
		idTraceModel trm;
		TrmFromModel( model, trm );
	}

	FreeModel( model );

	return true;
}


/*
===============================================================================

Loading of collision model file

===============================================================================
*/

/*
================
idCollisionModelManagerLocal::ParseVertices
================
*/
void idCollisionModelManagerLocal::ParseVertices( idLexer *src, cm_model_t *model ) {
	int i;

	src->ExpectTokenString( "{" );
	model->numVertices = src->ParseInt();
	model->maxVertices = model->numVertices;
	model->vertices = (cm_vertex_t *) Mem_Alloc( model->maxVertices * sizeof( cm_vertex_t ) );
	for ( i = 0; i < model->numVertices; i++ ) {
		src->Parse1DMatrix( 3, model->vertices[i].p.ToFloatPtr() );
		model->vertices[i].side = 0;
		model->vertices[i].sideSet = 0;
		model->vertices[i].checkcount = 0;
	}
	src->ExpectTokenString( "}" );
}

/*
================
idCollisionModelManagerLocal::ParseEdges
================
*/
void idCollisionModelManagerLocal::ParseEdges( idLexer *src, cm_model_t *model ) {
	int i;

	src->ExpectTokenString( "{" );
	model->numEdges = src->ParseInt();
	model->maxEdges = model->numEdges;
	model->edges = (cm_edge_t *) Mem_Alloc( model->maxEdges * sizeof( cm_edge_t ) );
	for ( i = 0; i < model->numEdges; i++ ) {
		src->ExpectTokenString( "(" );
		model->edges[i].vertexNum[0] = src->ParseInt();
		model->edges[i].vertexNum[1] = src->ParseInt();
		src->ExpectTokenString( ")" );
		model->edges[i].side = 0;
		model->edges[i].sideSet = 0;
		model->edges[i].internal = src->ParseInt();
		model->edges[i].numUsers = src->ParseInt();
		model->edges[i].normal = vec3_origin;
		model->edges[i].checkcount = 0;
		model->numInternalEdges += model->edges[i].internal;
	}
	src->ExpectTokenString( "}" );
}

/*
================
idCollisionModelManagerLocal::ParseNodes
================
*/
cm_node_t *idCollisionModelManagerLocal::ParseNodes( idLexer *src, cm_model_t *model, cm_node_t *parent ) {
	cm_node_t *node;

	model->numNodes++;
	node = AllocNode( model, model->numNodes < NODE_BLOCK_SIZE_SMALL ? NODE_BLOCK_SIZE_SMALL : NODE_BLOCK_SIZE_LARGE );
	node->brushes = NULL;
	node->polygons = NULL;
	node->parent = parent;
	src->ExpectTokenString( "(" );
	node->planeType = src->ParseInt();
	node->planeDist = src->ParseFloat();
	src->ExpectTokenString( ")" );
	if ( node->planeType != -1 ) {
		node->children[0] = ParseNodes( src, model, node );
		node->children[1] = ParseNodes( src, model, node );
	}
	return node;
}

/*
================
idCollisionModelManagerLocal::ParsePolygons
================
*/
void idCollisionModelManagerLocal::ParsePolygons( idLexer *src, cm_model_t *model ) {
	cm_polygon_t *p;
	int i, numEdges;
	idVec3 normal;
	idToken token;

	if ( src->CheckTokenType( TT_NUMBER, 0, &token ) ) {
		model->polygonBlock = (cm_polygonBlock_t *) Mem_Alloc( sizeof( cm_polygonBlock_t ) + token.GetIntValue() );
		model->polygonBlock->bytesRemaining = token.GetIntValue();
		model->polygonBlock->next = ( (byte *) model->polygonBlock ) + sizeof( cm_polygonBlock_t );
	}

	src->ExpectTokenString( "{" );
	while ( !src->CheckTokenString( "}" ) ) {
		// parse polygon
		numEdges = src->ParseInt();
		p = AllocPolygon( model, numEdges );
		p->numEdges = numEdges;
		src->ExpectTokenString( "(" );
		for ( i = 0; i < p->numEdges; i++ ) {
			p->edges[i] = src->ParseInt();
		}
		src->ExpectTokenString( ")" );
		src->Parse1DMatrix( 3, normal.ToFloatPtr() );
		p->plane.SetNormal( normal );
		p->plane.SetDist( src->ParseFloat() );
		src->Parse1DMatrix( 3, p->bounds[0].ToFloatPtr() );
		src->Parse1DMatrix( 3, p->bounds[1].ToFloatPtr() );
		src->ExpectTokenType( TT_STRING, 0, &token );
		// get material
		p->material = declManager->FindMaterial( token );
		p->contents = p->material->GetContentFlags();
		p->checkcount = 0;
		// filter polygon into tree
		R_FilterPolygonIntoTree( model, model->node, NULL, p );
	}
}

/*
================
idCollisionModelManagerLocal::ParseBrushes
================
*/
void idCollisionModelManagerLocal::ParseBrushes( idLexer *src, cm_model_t *model ) {
	cm_brush_t *b;
	int i, numPlanes;
	idVec3 normal;
	idToken token;

	if ( src->CheckTokenType( TT_NUMBER, 0, &token ) ) {
		model->brushBlock = (cm_brushBlock_t *) Mem_Alloc( sizeof( cm_brushBlock_t ) + token.GetIntValue() );
		model->brushBlock->bytesRemaining = token.GetIntValue();
		model->brushBlock->next = ( (byte *) model->brushBlock ) + sizeof( cm_brushBlock_t );
	}

	src->ExpectTokenString( "{" );
	while ( !src->CheckTokenString( "}" ) ) {
		// parse brush
		numPlanes = src->ParseInt();
		b = AllocBrush( model, numPlanes );
		b->numPlanes = numPlanes;
		src->ExpectTokenString( "{" );
		for ( i = 0; i < b->numPlanes; i++ ) {
			src->Parse1DMatrix( 3, normal.ToFloatPtr() );
			b->planes[i].SetNormal( normal );
			b->planes[i].SetDist( src->ParseFloat() );
		}
		src->ExpectTokenString( "}" );
		src->Parse1DMatrix( 3, b->bounds[0].ToFloatPtr() );
		src->Parse1DMatrix( 3, b->bounds[1].ToFloatPtr() );
		src->ReadToken( &token );
		if ( token.type == TT_NUMBER ) {
			b->contents = token.GetIntValue();		// old .cm files use a single integer
		} else {
			b->contents = ContentsFromString( token );
		}
		b->checkcount = 0;
		b->primitiveNum = 0;
		// filter brush into tree
		R_FilterBrushIntoTree( model, model->node, NULL, b );
	}
}

/*
================
idCollisionModelManagerLocal::ParseCollisionModel
================
*/
bool idCollisionModelManagerLocal::ParseCollisionModel( idLexer *src ) {
	cm_model_t *model;
	idToken token;

	if ( numModels >= MAX_SUBMODELS ) {
		common->Error( "LoadModel: no free slots" );
		return false;
	}
	model = AllocModel();
	models[numModels ] = model;
	numModels++;
	// parse the file
	src->ExpectTokenType( TT_STRING, 0, &token );
	model->name = token;
	src->ExpectTokenString( "{" );
	while ( !src->CheckTokenString( "}" ) ) {

		src->ReadToken( &token );

		if ( token == "vertices" ) {
			ParseVertices( src, model );
			continue;
		}

		if ( token == "edges" ) {
			ParseEdges( src, model );
			continue;
		}

		if ( token == "nodes" ) {
			src->ExpectTokenString( "{" );
			model->node = ParseNodes( src, model, NULL );
			src->ExpectTokenString( "}" );
			continue;
		}

		if ( token == "polygons" ) {
			ParsePolygons( src, model );
			continue;
		}

		if ( token == "brushes" ) {
			ParseBrushes( src, model );
			continue;
		}

		src->Error( "ParseCollisionModel: bad token \"%s\"", token.c_str() );
	}
	// calculate edge normals
	checkCount++;
	CalculateEdgeNormals( model, model->node );
	// get model bounds from brush and polygon bounds
	CM_GetNodeBounds( &model->bounds, model->node );
	// get model contents
	model->contents = CM_GetNodeContents( model->node );
	// total memory used by this model
	model->usedMemory = model->numVertices * sizeof(cm_vertex_t) +
						model->numEdges * sizeof(cm_edge_t) +
						model->polygonMemory +
						model->brushMemory +
						model->numNodes * sizeof(cm_node_t) +
						model->numPolygonRefs * sizeof(cm_polygonRef_t) +
						model->numBrushRefs * sizeof(cm_brushRef_t);

	return true;
}

/*
================
idCollisionModelManagerLocal::LoadCollisionModelFile
================
*/
bool idCollisionModelManagerLocal::LoadCollisionModelFile( const char *name, const unsigned int mapFileCRC ) {
	idStr fileName;
	idToken token;
	idLexer *src;
	unsigned int crc;

	// load it
	fileName = name;
	fileName.SetFileExtension( CM_FILE_EXT );
	src = new idLexer( fileName );
	src->SetFlags( LEXFL_NOSTRINGCONCAT | LEXFL_NODOLLARPRECOMPILE );
	if ( !src->IsLoaded() ) {
		delete src;
		return false;
	}

	if ( !src->ExpectTokenString( CM_FILEID ) ) {
		common->Warning( "%s is not an CM file.", fileName.c_str() );
		delete src;
		return false;
	}

	if ( !src->ReadToken( &token ) || token != CM_FILEVERSION ) {
		common->Warning( "%s has version %s instead of %s", fileName.c_str(), token.c_str(), CM_FILEVERSION );
		delete src;
		return false;
	}

	if ( !src->ExpectTokenType( TT_NUMBER, TT_INTEGER, &token ) ) {
		common->Warning( "%s has no map file CRC", fileName.c_str() );
		delete src;
		return false;
	}

    crc = token.GetUnsignedIntValue();
	if ( mapFileCRC && crc != mapFileCRC ) {
		common->Printf( "%s is out of date\n", fileName.c_str() );
		delete src;
		return false;
	}

	// parse the file
	while ( 1 ) {
		if ( !src->ReadToken( &token ) ) {
			break;
		}

		if ( token == "collisionModel" ) {
			if ( !ParseCollisionModel( src ) ) {
				delete src;
				return false;
			}
			continue;
		}

		src->Error( "idCollisionModelManagerLocal::LoadCollisionModelFile: bad token \"%s\"", token.c_str() );
	}

	delete src;

	return true;
}
