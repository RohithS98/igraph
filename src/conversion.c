/* -*- mode: C -*-  */
/* 
   IGraph library.
   Copyright (C) 2005  Gabor Csardi <csardi@rmki.kfki.hu>
   MTA RMKI, Konkoly-Thege Miklos st. 29-33, Budapest 1121, Hungary
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
   02110-1301 USA

*/

#include "igraph_conversion.h"
#include "igraph_iterators.h"
#include "igraph_interface.h"
#include "igraph_attributes.h"
#include "igraph_constructors.h"
#include "igraph_types_internal.h"
#include "igraph_sparsemat.h"
#include "config.h"

/**
 * \ingroup conversion
 * \function igraph_get_adjacency
 * \brief Returns the adjacency matrix of a graph
 * 
 * </para><para>
 * The result is an incidence matrix, it contains numbers greater
 * than one if there are multiple edges in the graph.
 * \param graph Pointer to the graph to convert
 * \param res Pointer to an initialized matrix object, it will be
 *        resized if needed.
 * \param type Constant giving the type of the adjacency matrix to
 *        create for undirected graphs. It is ignored for directed
 *        graphs. Possible values:
 *        \clist
 *        \cli IGRAPH_GET_ADJACENCY_UPPER 
 *          the upper right triangle of the matrix is used.
 *        \cli IGRAPH_GET_ADJACENCY_LOWER 
 *          the lower left triangle of the matrix is used.
 *        \cli IGRAPH_GET_ADJACENCY_BOTH 
 *          the whole matrix is used, a symmetric matrix is returned.
 *        \endclist
 * \param type eids Logical, if true, then the edges ids plus one 
 *        are stored in the adjacency matrix, instead of the number of 
 *        edges between the two vertices. (The plus one is needed, since
 *        edge ids start from zero, and zero means no edge in this case.)
 * \return Error code:
 *        \c IGRAPH_EINVAL invalid type argument.
 *
 * \sa igraph_get_adjacency_sparse if you want a sparse matrix representation
 *
 * Time complexity: O(|V||V|),
 * |V| is the 
 * number of vertices in the graph.
 */

int igraph_get_adjacency(const igraph_t *graph, igraph_matrix_t *res,
			 igraph_get_adjacency_t type, igraph_bool_t eids) {
  
  igraph_eit_t edgeit;
  long int no_of_nodes=igraph_vcount(graph);
  igraph_bool_t directed=igraph_is_directed(graph);
  int retval=0;
  long int from, to;
  igraph_integer_t ffrom, fto;
  
  IGRAPH_CHECK(igraph_matrix_resize(res, no_of_nodes, no_of_nodes));
  igraph_matrix_null(res);
  IGRAPH_CHECK(igraph_eit_create(graph, igraph_ess_all(0), &edgeit));
  IGRAPH_FINALLY(igraph_eit_destroy, &edgeit);
  
  if (directed) {
    while (!IGRAPH_EIT_END(edgeit)) {
      long int edge=IGRAPH_EIT_GET(edgeit);
      igraph_edge(graph, edge, &ffrom, &fto);
      from=ffrom;
      to=fto;
      if (eids) { 
	MATRIX(*res, from, to) = edge+1;
      } else { 
	MATRIX(*res, from, to) += 1;
      }
      IGRAPH_EIT_NEXT(edgeit);
    }
  } else if (type==IGRAPH_GET_ADJACENCY_UPPER) {
    while (!IGRAPH_EIT_END(edgeit)) {  
      long int edge=IGRAPH_EIT_GET(edgeit);
      igraph_edge(graph, edge, &ffrom, &fto);
      from=ffrom;
      to=fto;
      if (to < from) {
	if (eids) { 
	  MATRIX(*res, to, from) = edge+1;
	} else {
	  MATRIX(*res, to, from) += 1;
	}
      } else {
	if (eids) {
	  MATRIX(*res, from, to) = edge+1;
	} else { 
	  MATRIX(*res, from, to) += 1;    
	}
      }
      IGRAPH_EIT_NEXT(edgeit);
    }
  } else if (type==IGRAPH_GET_ADJACENCY_LOWER) {
    while (!IGRAPH_EIT_END(edgeit)) {
      long int edge=IGRAPH_EIT_GET(edgeit);
      igraph_edge(graph, edge, &ffrom, &fto);
      from=ffrom;
      to=fto;
      if (to < from) {
	if (eids) {
	  MATRIX(*res, from, to) = edge+1;
	} else {
	  MATRIX(*res, from, to) += 1;
	}
      } else {
	if (eids) { 
	  MATRIX(*res, to, from) = edge+1;
	} else {
	  MATRIX(*res, to, from) += 1;
	}
      }
      IGRAPH_EIT_NEXT(edgeit);
    }
  } else if (type==IGRAPH_GET_ADJACENCY_BOTH) {
    while (!IGRAPH_EIT_END(edgeit)) {
      long int edge=IGRAPH_EIT_GET(edgeit);
      igraph_edge(graph, edge, &ffrom, &fto);
      from=ffrom;
      to=fto;
      if (eids) { 
	MATRIX(*res, from, to) = edge+1;
      } else {
	MATRIX(*res, from, to) += 1;
      }
      if (from != to) {
	if (eids) { 
	  MATRIX(*res, to, from) = edge+1;
	} else {
	  MATRIX(*res, to, from) += 1;
	}
      }
      IGRAPH_EIT_NEXT(edgeit);
    }
  } else {
    IGRAPH_ERROR("Invalid type argument", IGRAPH_EINVAL);
  }

  igraph_eit_destroy(&edgeit);
  IGRAPH_FINALLY_CLEAN(1);
  return retval;
}

/**
 * \ingroup conversion
 * \function igraph_get_adjacency_sparse
 * \brief Returns the adjacency matrix of a graph in sparse matrix format
 * 
 * </para><para>
 * The result is an incidence matrix, it contains numbers greater
 * than one if there are multiple edges in the graph.
 * \param graph Pointer to the graph to convert
 * \param res Pointer to an initialized sparse matrix object, it will be
 *        resized if needed.
 * \param type Constant giving the type of the adjacency matrix to
 *        create for undirected graphs. It is ignored for directed
 *        graphs. Possible values:
 *        \clist
 *        \cli IGRAPH_GET_ADJACENCY_UPPER 
 *          the upper right triangle of the matrix is used.
 *        \cli IGRAPH_GET_ADJACENCY_LOWER 
 *          the lower left triangle of the matrix is used.
 *        \cli IGRAPH_GET_ADJACENCY_BOTH 
 *          the whole matrix is used, a symmetric matrix is returned.
 *        \endclist
 * \return Error code:
 *        \c IGRAPH_EINVAL invalid type argument.
 *
 * \sa igraph_get_adjacency if you would like to get a normal matrix
 *   ( \type igraph_matrix_t )
 *
 * Time complexity: O(|V||V|),
 * |V| is the 
 * number of vertices in the graph.
 */

int igraph_get_adjacency_sparse(const igraph_t *graph, igraph_spmatrix_t *res,
			 igraph_get_adjacency_t type) {
  
  igraph_eit_t edgeit;
  long int no_of_nodes=igraph_vcount(graph);
  igraph_bool_t directed=igraph_is_directed(graph);
  int retval=0;
  long int from, to;
  igraph_integer_t ffrom, fto;
  
  igraph_spmatrix_null(res);
  IGRAPH_CHECK(igraph_spmatrix_resize(res, no_of_nodes, no_of_nodes));
  IGRAPH_CHECK(igraph_eit_create(graph, igraph_ess_all(0), &edgeit));
  IGRAPH_FINALLY(igraph_eit_destroy, &edgeit);
  
  if (directed) {
    while (!IGRAPH_EIT_END(edgeit)) {
      igraph_edge(graph, IGRAPH_EIT_GET(edgeit), &ffrom, &fto);
      from=ffrom;
      to=fto;
      igraph_spmatrix_add_e(res, from, to, 1);
      IGRAPH_EIT_NEXT(edgeit);
    }
  } else if (type==IGRAPH_GET_ADJACENCY_UPPER) {
    while (!IGRAPH_EIT_END(edgeit)) {  
      igraph_edge(graph, IGRAPH_EIT_GET(edgeit), &ffrom, &fto);
      from=ffrom;
      to=fto;
      if (to < from) {
        igraph_spmatrix_add_e(res, to, from, 1);
      } else {
        igraph_spmatrix_add_e(res, from, to, 1);
      }
      IGRAPH_EIT_NEXT(edgeit);
    }
  } else if (type==IGRAPH_GET_ADJACENCY_LOWER) {
    while (!IGRAPH_EIT_END(edgeit)) {
      igraph_edge(graph, IGRAPH_EIT_GET(edgeit), &ffrom, &fto);
      from=ffrom;
      to=fto;
      if (to > from) {
        igraph_spmatrix_add_e(res, to, from, 1);
      } else {
        igraph_spmatrix_add_e(res, from, to, 1);
      }
      IGRAPH_EIT_NEXT(edgeit);
    }
  } else if (type==IGRAPH_GET_ADJACENCY_BOTH) {
    while (!IGRAPH_EIT_END(edgeit)) {
      igraph_edge(graph, IGRAPH_EIT_GET(edgeit), &ffrom, &fto);
      from=ffrom;
      to=fto;
      igraph_spmatrix_add_e(res, from, to, 1);
      if (from != to) {
        igraph_spmatrix_add_e(res, to, from, 1);
      }
      IGRAPH_EIT_NEXT(edgeit);
    }
  } else {
    IGRAPH_ERROR("Invalid type argument", IGRAPH_EINVAL);
  }

  igraph_eit_destroy(&edgeit);
  IGRAPH_FINALLY_CLEAN(1);
  return retval;
}

/**
 * \ingroup conversion
 * \function igraph_get_edgelist
 * \brief Returns the list of edges in a graph
 * 
 * </para><para>The order of the edges is given by the edge ids.
 * \param graph Pointer to the graph object
 * \param res Pointer to an initialized vector object, it will be
 *        resized.
 * \param bycol Logical, if true, the edges will be returned
 *        columnwise, eg. the first edge is
 *        <code>res[0]->res[|E|]</code>, the second is
 *        <code>res[1]->res[|E|+1]</code>, etc.
 * \return Error code.
 * 
 * Time complexity: O(|E|), the
 * number of edges in the graph.
 */

int igraph_get_edgelist(const igraph_t *graph, igraph_vector_t *res, igraph_bool_t bycol) {

  igraph_eit_t edgeit;
  long int no_of_edges=igraph_ecount(graph);
  long int vptr=0;
  igraph_integer_t from, to;
  
  IGRAPH_CHECK(igraph_vector_resize(res, no_of_edges*2));
  IGRAPH_CHECK(igraph_eit_create(graph, igraph_ess_all(IGRAPH_EDGEORDER_ID),
				 &edgeit));
  IGRAPH_FINALLY(igraph_eit_destroy, &edgeit);
  
  if (bycol) {
    while (!IGRAPH_EIT_END(edgeit)) {
      igraph_edge(graph, IGRAPH_EIT_GET(edgeit), &from, &to);
      VECTOR(*res)[vptr]=from;
      VECTOR(*res)[vptr+no_of_edges]=to;
      vptr++;
      IGRAPH_EIT_NEXT(edgeit);
    }
  } else {
    while (!IGRAPH_EIT_END(edgeit)) {
      igraph_edge(graph, IGRAPH_EIT_GET(edgeit), &from, &to);
      VECTOR(*res)[vptr++]=from;
      VECTOR(*res)[vptr++]=to;
      IGRAPH_EIT_NEXT(edgeit);
    }
  }
  
  igraph_eit_destroy(&edgeit);
  IGRAPH_FINALLY_CLEAN(1);
  return 0;
}

/**
 * \function igraph_to_directed
 * \brief Convert an undirected graph to a directed one
 * 
 * </para><para>
 * If the supplied graph is directed, this function does nothing.
 * \param graph The graph object to convert.
 * \param mode Constant, specifies the details of how exactly the
 *        conversion is done. Possible values: \c
 *        IGRAPH_TO_DIRECTED_ARBITRARY: the number of edges in the
 *        graph stays the same, an arbitrarily directed edge is
 *        created for each undirected edge; 
 *         \c IGRAPH_TO_DIRECTED_MUTUAL: two directed edges are
 *        created for each undirected edge, one in each direction.
 * \return Error code.
 * 
 * Time complexity: O(|V|+|E|), the number of vertices plus the number
 * of edges.
 */

int igraph_to_directed(igraph_t *graph,
		       igraph_to_directed_t mode) {

  if (mode != IGRAPH_TO_DIRECTED_ARBITRARY &&
      mode != IGRAPH_TO_DIRECTED_MUTUAL) {
    IGRAPH_ERROR("Cannot directed graph, invalid mode", IGRAPH_EINVAL);
  }

  if (igraph_is_directed(graph)) {
    return 0;
  }

  if (mode==IGRAPH_TO_DIRECTED_ARBITRARY) {

    igraph_t newgraph;
    igraph_vector_t edges;
    long int no_of_edges=igraph_ecount(graph);
    long int no_of_nodes=igraph_vcount(graph);
    long int size=no_of_edges*2;
    IGRAPH_VECTOR_INIT_FINALLY(&edges, size);
    IGRAPH_CHECK(igraph_get_edgelist(graph, &edges, 0));

    IGRAPH_CHECK(igraph_create(&newgraph, &edges, no_of_nodes,
			       IGRAPH_DIRECTED));
    IGRAPH_FINALLY(igraph_destroy, &newgraph);
    igraph_vector_destroy(&edges);
    IGRAPH_I_ATTRIBUTE_DESTROY(&newgraph);
    IGRAPH_I_ATTRIBUTE_COPY(&newgraph, graph, 1,1,1);
    IGRAPH_FINALLY_CLEAN(2);
    igraph_destroy(graph);
    *graph=newgraph;

  } else if (mode==IGRAPH_TO_DIRECTED_MUTUAL) {
    
    igraph_t newgraph;
    igraph_vector_t edges;
    igraph_vector_t index;
    long int no_of_edges=igraph_ecount(graph);
    long int no_of_nodes=igraph_vcount(graph);
    long int size=no_of_edges*4;
    long int i;
    IGRAPH_VECTOR_INIT_FINALLY(&edges, 0);
    IGRAPH_CHECK(igraph_vector_reserve(&edges, size));
    IGRAPH_CHECK(igraph_get_edgelist(graph, &edges, 0));
    IGRAPH_CHECK(igraph_vector_resize(&edges, no_of_edges*4));
    IGRAPH_VECTOR_INIT_FINALLY(&index, no_of_edges*2);
    for (i=0; i<no_of_edges; i++) {
      VECTOR(edges)[no_of_edges*2+i*2]  =VECTOR(edges)[i*2+1];
      VECTOR(edges)[no_of_edges*2+i*2+1]=VECTOR(edges)[i*2];
      VECTOR(index)[i] = VECTOR(index)[no_of_edges+i] = i;
    }

    IGRAPH_CHECK(igraph_create(&newgraph, &edges, no_of_nodes,
			       IGRAPH_DIRECTED));
    IGRAPH_FINALLY(igraph_destroy, &newgraph);
    IGRAPH_I_ATTRIBUTE_DESTROY(&newgraph);
    IGRAPH_I_ATTRIBUTE_COPY(&newgraph, graph, 1,1,/*edges=*/0);
    IGRAPH_CHECK(igraph_i_attribute_permute_edges(graph, &newgraph, &index));
    
    igraph_vector_destroy(&index);
    igraph_vector_destroy(&edges);
    igraph_destroy(graph);
    IGRAPH_FINALLY_CLEAN(3);
    *graph=newgraph;
  }
  
  return 0;
}

/**
 * \function igraph_to_undirected
 * \brief Convert a directed graph to an undirected one.
 * 
 * </para><para>
 * If the supplied graph is undirected, this function does nothing.
 * \param graph The graph object to convert.
 * \param mode Constant, specifies the details of how exactly the
 *        conversion is done. Possible values: \c 
 *        IGRAPH_TO_UNDIRECTED_EACH: the number of edges remains
 *        constant, an undirected edge is created for each directed
 *        one, this version might create graphs with multiple edges; 
 *        \c IGRAPH_TO_UNDIRECTED_COLLAPSE: one undirected edge will
 *        be created for each pair of vertices which are connected
 *        with at least one directed edge, no multiple edges will be
 *        created. \c IGRAPH_TO_UNDIRECTED_MUTUAL creates an undirected 
 *        edge for each pair of mutual edges in the directed graph. 
 *        Non-mutual edges are lost. This mode might create multiple
 *        edges.
 * \param edge_comb What to do with the edge attributes. See the igraph 
 *        manual section about attributes for details.
 * \return Error code.
 * 
 * Time complexity: O(|V|+|E|), the number of vertices plus the number
 * of edges. 
 * 
 * \example examples/simple/igraph_to_undirected.c
 */

int igraph_to_undirected(igraph_t *graph,
			 igraph_to_undirected_t mode,
			 const igraph_attribute_combination_t *edge_comb) {

  long int no_of_nodes=igraph_vcount(graph);
  long int no_of_edges=igraph_ecount(graph);
  igraph_vector_t edges;
  igraph_t newgraph;
  igraph_bool_t attr=edge_comb && igraph_has_attribute_table();
  
  if (mode != IGRAPH_TO_UNDIRECTED_EACH &&
      mode != IGRAPH_TO_UNDIRECTED_COLLAPSE && 
      mode != IGRAPH_TO_UNDIRECTED_MUTUAL) {
    IGRAPH_ERROR("Cannot undirect graph, invalid mode", IGRAPH_EINVAL);
  }
  
  if (!igraph_is_directed(graph)) {
    return 0;
  }

  IGRAPH_VECTOR_INIT_FINALLY(&edges, 0);
  
  if (mode==IGRAPH_TO_UNDIRECTED_EACH) {
    igraph_es_t es;
    igraph_eit_t eit;

    IGRAPH_CHECK(igraph_vector_reserve(&edges, no_of_edges*2));
    IGRAPH_CHECK(igraph_es_all(&es, IGRAPH_EDGEORDER_ID));
    IGRAPH_FINALLY(igraph_es_destroy, &es);
    IGRAPH_CHECK(igraph_eit_create(graph, es, &eit));
    IGRAPH_FINALLY(igraph_eit_destroy, &eit);
    
    while (!IGRAPH_EIT_END(eit)) {
      long int edge=IGRAPH_EIT_GET(eit);
      igraph_integer_t from, to;
      igraph_edge(graph, edge, &from, &to);
      IGRAPH_CHECK(igraph_vector_push_back(&edges, from));
      IGRAPH_CHECK(igraph_vector_push_back(&edges, to));
      IGRAPH_EIT_NEXT(eit);
    }
    
    igraph_eit_destroy(&eit);
    igraph_es_destroy(&es);
    IGRAPH_FINALLY_CLEAN(2);
    
    IGRAPH_CHECK(igraph_create(&newgraph, &edges, no_of_nodes, IGRAPH_UNDIRECTED));
    IGRAPH_FINALLY(igraph_destroy, &newgraph);
    igraph_vector_destroy(&edges);
    IGRAPH_I_ATTRIBUTE_DESTROY(&newgraph);
    IGRAPH_I_ATTRIBUTE_COPY(&newgraph, graph, 1,1,1);
    IGRAPH_FINALLY_CLEAN(2);
    igraph_destroy(graph);
    *graph=newgraph;
    
  } else if (mode==IGRAPH_TO_UNDIRECTED_COLLAPSE) {
    igraph_vector_t inadj, outadj;
    long int i;
    igraph_vector_t mergeinto;
    long int actedge=0;

    if (attr) {
      IGRAPH_VECTOR_INIT_FINALLY(&mergeinto, no_of_edges);
    }

    IGRAPH_CHECK(igraph_vector_reserve(&edges, no_of_edges*2));
    IGRAPH_VECTOR_INIT_FINALLY(&inadj, 0);
    IGRAPH_VECTOR_INIT_FINALLY(&outadj, 0);
    
    for (i=0; i<no_of_nodes; i++) {
      long int n_out, n_in;
      long int p1=-1, p2=-1;
      long int e1=0, e2=0, n1=0, n2=0;
      IGRAPH_CHECK(igraph_incident(graph, &outadj, i, IGRAPH_OUT));
      IGRAPH_CHECK(igraph_incident(graph, &inadj, i, IGRAPH_IN));
      n_out=igraph_vector_size(&outadj);
      n_in=igraph_vector_size(&inadj);

#define STEPOUT() if ( (++p1) < n_out) {	\
	e1 = VECTOR(outadj)[p1];		\
	n1 = IGRAPH_TO(graph, e1);		\
      }
#define STEPIN()  if ( (++p2) < n_in) {	        \
        e2 = VECTOR(inadj )[p2];		\
	n2 = IGRAPH_FROM(graph, e2);		\
      }

      STEPOUT();
      STEPIN();

      while (p1 < n_out && n1 <= i && p2 < n_in && n2 <= i) {
	long int last;
	if (n1==n2) {
	  last=n1;
	  IGRAPH_CHECK(igraph_vector_push_back(&edges, i));
	  IGRAPH_CHECK(igraph_vector_push_back(&edges, n1));
	  if (attr) {
	    VECTOR(mergeinto)[e1]=actedge;
	    VECTOR(mergeinto)[e2]=actedge;
	    actedge++;
	  }
	  while (p1 < n_out && last==n1) { STEPOUT(); }
	  while (p2 < n_in  && last==n2) { STEPIN (); }
	} else if (n1<n2) {
	  last=n1;
	  IGRAPH_CHECK(igraph_vector_push_back(&edges, i));
	  IGRAPH_CHECK(igraph_vector_push_back(&edges, n1));
	  if (attr) {
	    VECTOR(mergeinto)[e1]=actedge;
	    actedge++;
	  }
	  while (p1 < n_out && last==n1) { STEPOUT(); }
	} else { /* n2<n1 */
	  last=n2;
	  IGRAPH_CHECK(igraph_vector_push_back(&edges, i));
	  IGRAPH_CHECK(igraph_vector_push_back(&edges, n2));
	  if (attr) {
	    VECTOR(mergeinto)[e2]=actedge;
	    actedge++;
	  }
	  while (p2 < n_in && last==n2) { STEPIN(); }
	}
      }
      while (p1 < n_out && n1 <= i) {
	IGRAPH_CHECK(igraph_vector_push_back(&edges, i));
	IGRAPH_CHECK(igraph_vector_push_back(&edges, n1));
	if (attr) {
	  VECTOR(mergeinto)[e1]=actedge;
	  actedge++;
	}
	STEPOUT();	
      }
      while (p2 < n_in && n2 <= i) {
	IGRAPH_CHECK(igraph_vector_push_back(&edges, i));
	IGRAPH_CHECK(igraph_vector_push_back(&edges, n2));
	if (attr) {
	  VECTOR(mergeinto)[e2]=actedge;
	  actedge++;
	}
	STEPIN();
      }
    }
	
#undef STEPOUT
#undef STEPIN 

    igraph_vector_destroy(&outadj);
    igraph_vector_destroy(&inadj);
    IGRAPH_FINALLY_CLEAN(2);

    IGRAPH_CHECK(igraph_create(&newgraph, &edges, no_of_nodes, IGRAPH_UNDIRECTED));
    IGRAPH_FINALLY(igraph_destroy, &newgraph);
    igraph_vector_destroy(&edges);
    IGRAPH_I_ATTRIBUTE_DESTROY(&newgraph);
    IGRAPH_I_ATTRIBUTE_COPY(&newgraph, graph, 1,1,0); /* no edge attributes */

    if (attr) {
      igraph_fixed_vectorlist_t vl;
      IGRAPH_CHECK(igraph_fixed_vectorlist_convert(&vl, &mergeinto, 
						   actedge));
      IGRAPH_FINALLY(igraph_fixed_vectorlist_destroy, &vl);
      
      IGRAPH_CHECK(igraph_i_attribute_combine_edges(graph, &newgraph, &vl.v, 
						    edge_comb));
      
      igraph_fixed_vectorlist_destroy(&vl);
      IGRAPH_FINALLY_CLEAN(1);
    }

    IGRAPH_FINALLY_CLEAN(2);
    igraph_destroy(graph);
    *graph=newgraph;
    
    if (attr) {
      igraph_vector_destroy(&mergeinto);
      IGRAPH_FINALLY_CLEAN(1);
    }
  } else if (mode==IGRAPH_TO_UNDIRECTED_MUTUAL) {
    igraph_vector_t inadj, outadj;
    long int i;
    igraph_vector_t mergeinto;
    long int actedge=0;

    if (attr) {
      IGRAPH_VECTOR_INIT_FINALLY(&mergeinto, no_of_edges);
      igraph_vector_fill(&mergeinto, -1);
    }
    
    IGRAPH_CHECK(igraph_vector_reserve(&edges, no_of_edges*2));
    IGRAPH_VECTOR_INIT_FINALLY(&inadj, 0);
    IGRAPH_VECTOR_INIT_FINALLY(&outadj, 0);

    for (i=0; i<no_of_nodes; i++) {
      long int n_out, n_in; 
      long int p1=-1, p2=-1;
      long int e1=0, e2=0, n1=0, n2=0;
      IGRAPH_CHECK(igraph_incident(graph, &outadj, i, IGRAPH_OUT));
      IGRAPH_CHECK(igraph_incident(graph, &inadj,  i, IGRAPH_IN));
      n_out=igraph_vector_size(&outadj);
      n_in=igraph_vector_size(&inadj);

#define STEPOUT() if ( (++p1) < n_out) {	\
	e1 = VECTOR(outadj)[p1];		\
	n1 = IGRAPH_TO(graph, e1);		\
      }
#define STEPIN()  if ( (++p2) < n_in) {	        \
        e2 = VECTOR(inadj )[p2];		\
	n2 = IGRAPH_FROM(graph, e2);		\
      }

      STEPOUT();
      STEPIN();

      while (p1 < n_out && n1 <= i && p2 < n_in && n2 <= i) {
	if (n1==n2) {
	  IGRAPH_CHECK(igraph_vector_push_back(&edges, i));
	  IGRAPH_CHECK(igraph_vector_push_back(&edges, n1));
	  if (attr) {
	    VECTOR(mergeinto)[e1]=actedge;
	    VECTOR(mergeinto)[e2]=actedge;
	    actedge++;
	  }
	  STEPOUT();
	  STEPIN();
	} else if (n1<n2) {
	  STEPOUT();
	} else { /* n2<n1 */
	  STEPIN();
	}
      }
    }

#undef STEPOUT
#undef STEPIN      

    igraph_vector_destroy(&outadj);
    igraph_vector_destroy(&inadj);
    IGRAPH_FINALLY_CLEAN(2);

    IGRAPH_CHECK(igraph_create(&newgraph, &edges, no_of_nodes, 
			       IGRAPH_UNDIRECTED));
    IGRAPH_FINALLY(igraph_destroy, &newgraph);
    igraph_vector_destroy(&edges);
    IGRAPH_I_ATTRIBUTE_DESTROY(&newgraph);
    IGRAPH_I_ATTRIBUTE_COPY(&newgraph, graph, 1,1,0); /* no edge attributes */

    if (attr) {
      igraph_fixed_vectorlist_t vl;
      IGRAPH_CHECK(igraph_fixed_vectorlist_convert(&vl, &mergeinto, 
						   actedge));
      IGRAPH_FINALLY(igraph_fixed_vectorlist_destroy, &vl);
      
      IGRAPH_CHECK(igraph_i_attribute_combine_edges(graph, &newgraph, &vl.v, 
						    edge_comb));
      
      igraph_fixed_vectorlist_destroy(&vl);
      IGRAPH_FINALLY_CLEAN(1);
    }

    IGRAPH_FINALLY_CLEAN(2);
    igraph_destroy(graph);
    *graph=newgraph;
    
    if (attr) {
      igraph_vector_destroy(&mergeinto);
      IGRAPH_FINALLY_CLEAN(1);
    }
  }

  return 0;
}

/**
 * \function igraph_get_stochastic
 * Stochastic adjacency matrix of a graph
 * 
 * Stochastic matrix of a graph. The stochastic matrix of a graph is
 * its adjacency matrix, normalized row-wise or column-wise, such that
 * the sum of each row (or column) is one. 
 * \param graph The input graph. 
 * \param sparsemat Pointer to an initialized matrix, the
 *    result is stored here.
 * \param column_wise Whether to normalize column-wise. For undirected
 *    graphs this argument does not have any effect.
 * \return Error code.
 * 
 * Time complexity: O(|V||V|), quadratic in the number of vertices.
 * 
 * \sa igraph_get_stochastic_sparsemat(), the sparse version of this
 * function.
 */

int igraph_get_stochastic(const igraph_t *graph, 
			  igraph_matrix_t *matrix,
			  igraph_bool_t column_wise) {
  
  int no_of_nodes=igraph_vcount(graph);
  igraph_real_t sum;
  int i, j;
  
  IGRAPH_CHECK(igraph_get_adjacency(graph, matrix, 
				    IGRAPH_GET_ADJACENCY_BOTH, /*eids=*/ 0));
  
  if (!column_wise) {
    for (i=0; i<no_of_nodes; i++) {
      sum=0.0; 
      for (j=0; j<no_of_nodes; j++) { 
	sum += MATRIX(*matrix, i, j);
      }
      for (j=0; j<no_of_nodes; j++) {
	MATRIX(*matrix, i, j) /= sum;
      }
    }
  } else {
    for (i=0; i<no_of_nodes; i++) {
      sum=0.0; 
      for (j=0; j<no_of_nodes; j++) { 
	sum += MATRIX(*matrix, j, i);
      }
      for (j=0; j<no_of_nodes; j++) {
	MATRIX(*matrix, j, i) /= sum;
      }
    }
  }

  return 0;
}

int igraph_i_normalize_sparsemat(igraph_sparsemat_t *sparsemat, 
				 igraph_bool_t column_wise) {
  igraph_vector_t sum;
  int no_of_nodes=igraph_sparsemat_nrow(sparsemat);
  int i;
  
  IGRAPH_VECTOR_INIT_FINALLY(&sum, no_of_nodes);

  if (!column_wise) {
    IGRAPH_CHECK(igraph_sparsemat_rowsums(sparsemat, &sum));
    for (i=0; i<no_of_nodes; i++) {
      if (VECTOR(sum)[i] == 0.0) {
	IGRAPH_ERROR("Zero out-degree vertices not allowed", 
		     IGRAPH_EINVAL);
      }
      VECTOR(sum)[i] = 1.0 / VECTOR(sum)[i];
    }
    IGRAPH_CHECK(igraph_sparsemat_scale_rows(sparsemat, &sum));
  } else {
    IGRAPH_CHECK(igraph_sparsemat_colsums(sparsemat, &sum));
    for (i=0; i<no_of_nodes; i++) {
      if (VECTOR(sum)[i] == 0.0) {
	IGRAPH_ERROR("Zero out-degree vertices not allowed", 
		     IGRAPH_EINVAL);
      }
      VECTOR(sum)[i] = 1.0 / VECTOR(sum)[i];
    }
    IGRAPH_CHECK(igraph_sparsemat_scale_cols(sparsemat, &sum));
  }

  igraph_vector_destroy(&sum);
  IGRAPH_FINALLY_CLEAN(1);
  
  return 0;
}

/**
 * \function igraph_get_stochastic_sparsemat
 * \brief Stochastic adjacency matrix of a graph
 * 
 * Stochastic matrix of a graph. The stochastic matrix of a graph is
 * its adjacency matrix, normalized row-wise or column-wise, such that
 * the sum of each row (or column) is one. 
 * \param graph The input graph. 
 * \param sparsemat Pointer to an uninitialized sparse matrix, the
 *    result is stored here.
 * \param column_wise Whether to normalize column-wise. For undirected
 *    graphs this argument does not have any effect.
 * \return Error code.
 * 
 * Time complexity: O(|V|+|E|), linear in the number of vertices and
 * edges.
 * 
 * \sa igraph_get_stochastic(), the dense version of this function.
 */

int igraph_get_stochastic_sparsemat(const igraph_t *graph, 
				    igraph_sparsemat_t *sparsemat,
				    igraph_bool_t column_wise) {
  
  IGRAPH_CHECK(igraph_get_sparsemat(graph, sparsemat));
  IGRAPH_FINALLY(igraph_sparsemat_destroy, sparsemat);
  IGRAPH_CHECK(igraph_i_normalize_sparsemat(sparsemat, column_wise));
  IGRAPH_FINALLY_CLEAN(1);
  
  return 0;
}
