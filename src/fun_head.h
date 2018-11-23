/*************************************************************

	LSD 7.1 - December 2018
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License
	
 *************************************************************/

/*************************************************************
FUN_HEAD.H
This file contains all the macros required by the
model's equation file.
*************************************************************/

#define FUN												// comment this line to access internal LSD functions

#if defined( EIGENLIB ) && __cplusplus >= 201103L		// required C++11
#include <Eigen/Eigen>									// Eigen linear algebra library
using namespace Eigen;
#endif

#include "decl.h"										// LSD classes

// create and set fast lookup flag
#if ! defined FAST_LOOKUP || ! defined CPP11
bool fast_lookup = false;
void init_map( ) { };
#else
bool fast_lookup = true;
#endif

// set pointers to NULL to protect users (small overhead) if not disabled
#if defined FAST_LOOKUP && ! defined NO_POINTER_INIT
bool no_ptr_chk = false;
#define INIT_POINTERS \
	cur = cur1 = cur2 = cur3 = cur4 = cur5 = cur6 = cur7 = cur8 = cur9 = cyccur = cyccur2 = cyccur3 = NULL; \
	curl = curl1 = curl2 = curl3 = curl4 = curl5 = curl6 = curl7 = curl8 = curl9 = NULL; \
	f = NULL;
#define CHK_PTR_CHR( O ) chk_ptr ( O ) ? bad_ptr_chr( O, __FILE__, __LINE__ ) :
#define CHK_PTR_DBL( O ) chk_ptr ( O ) ? bad_ptr_dbl( O, __FILE__, __LINE__ ) :
#define CHK_PTR_LNK( O ) chk_ptr ( O ) ? bad_ptr_lnk( O, __FILE__, __LINE__ ) :
#define CHK_PTR_OBJ( O ) chk_ptr ( O ) ? bad_ptr_obj( O, __FILE__, __LINE__ ) :
#define CHK_PTR_VOID( O ) chk_ptr ( O ) ? bad_ptr_void( O, __FILE__, __LINE__ ) :
#define CHK_LNK_DBL( O ) O == NULL ? nul_lnk_dbl( __FILE__, __LINE__ ) :
#define CHK_LNK_OBJ( O ) O == NULL ? nul_lnk_obj( __FILE__, __LINE__ ) :
#define CHK_LNK_VOID( O ) O == NULL ? nul_lnk_void( __FILE__, __LINE__ ) :
#define CHK_NODE_CHR( O ) O->node == NULL ? no_node_chr( O->label, __FILE__, __LINE__ ) :
#define CHK_NODE_DBL( O ) O->node == NULL ? no_node_dbl( O->label, __FILE__, __LINE__ ) :
#else
bool no_ptr_chk = true;
#define INIT_POINTERS
#define CHK_PTR_CHR( O )
#define CHK_PTR_DBL( O )
#define CHK_PTR_LNK( O )
#define CHK_PTR_OBJ( O )
#define CHK_PTR_VOID( O )
#define CHK_LNK_DBL( O )
#define CHK_LNK_OBJ( O )
#define CHK_LNK_VOID( O )
#define CHK_NODE_CHR( O )
#define CHK_NODE_DBL( O )
#endif

// user defined variables for all equations (to be defined in equation file)
#ifndef EQ_USER_VARS
#define EQ_USER_VARS
#endif

#define EQ_BEGIN \
	double res = def_res; \
	object *p = var->up, *c = caller, app; \
	int h, i, j, k; \
	double v[ USER_D_VARS ]; \
	object *cur, *cur1, *cur2, *cur3, *cur4, *cur5, *cur6, *cur7, *cur8, *cur9, *cyccur, *cyccur2, *cyccur3; \
	netLink *curl, *curl1, *curl2, *curl3, *curl4, *curl5, *curl6, *curl7, *curl8, *curl9; \
	FILE *f; \
	INIT_POINTERS \
	EQ_USER_VARS

#define EQ_NOT_FOUND \
	char msg[ TCL_BUFF_STR ]; \
	sprintf( msg, "equation not found for variable '%s'", label ); \
	error_hard( msg, "equation not found", "check your configuration (variable name) or\ncode (equation name) to prevent this situation\nPossible problems:\n- There is no equation for this variable\n- The equation name is different from the variable name (case matters!)" ); \
	return res;
	
#define EQ_TEST_RESULT \
	if ( quit == 0 && ( ( ! use_nan && is_nan( res ) ) || is_inf( res ) ) ) \
	{ \
		char msg[ TCL_BUFF_STR ]; \
		sprintf( msg, "equation for '%s' produces the invalid value '%lf' at time %d", label, res, t ); \
		error_hard( msg, "invalid equation result", "check your equation code to prevent invalid math operations\nPossible problems:\n- Illegal math operation (division by zero, log of negative number etc.)\n- Use of too-large/small value in calculation\n- Use of non-initialized temporary variable in calculation", true ); \
		debug_flag = true; \
		debug = 'd'; \
	}

#ifndef NO_WINDOW
#define DEBUG_CODE \
	if ( debug_flag ) \
	{ \
		for ( int n = 0; n < USER_D_VARS; ++n ) \
			d_values[ n ] = v[ n ]; \
		i_values[ 0 ] = i; \
		i_values[ 1 ] = j; \
		i_values[ 2 ] = h; \
		i_values[ 3 ] = k; \
		o_values[ 0 ] = cur; \
		o_values[ 1 ] = cur1; \
		o_values[ 2 ] = cur2; \
		o_values[ 3 ] = cur3; \
		o_values[ 4 ] = cur4; \
		o_values[ 5 ] = cur5; \
		o_values[ 6 ] = cur6; \
		o_values[ 7 ] = cur7; \
		o_values[ 8 ] = cur8; \
		o_values[ 9 ] = cur9; \
		n_values[ 0 ] = curl; \
		n_values[ 1 ] = curl1; \
		n_values[ 2 ] = curl2; \
		n_values[ 3 ] = curl3; \
		n_values[ 4 ] = curl4; \
		n_values[ 5 ] = curl5; \
		n_values[ 6 ] = curl6; \
		n_values[ 7 ] = curl7; \
		n_values[ 8 ] = curl8; \
		n_values[ 9 ] = curl9; \
	};
#else
#define DEBUG_CODE
#endif

// handle fast equation look-up if enabled and C++11 is available
#if ! defined FAST_LOOKUP || ! defined CPP11
// use standard chain method for look-up
#define MODELBEGIN \
	double variable::fun( object *caller ) \
	{ \
		if ( quit == 2 ) \
			return def_res; \
		variable *var = this; \
		EQ_BEGIN
		
#define MODELEND \
		EQ_NOT_FOUND \
		end: \
		EQ_TEST_RESULT \
		DEBUG_CODE \
		return res; \
	}

#define EQUATION( X ) \
	if ( ! strcmp( label, X ) ) { 

#define RESULT( X ) \
		res = X; \
		goto end; \
	}

#define END_EQUATION( X ) \
	{ \
		res = X; \
		goto end; \
	}

#define EQUATION_DUMMY( X, Y ) \
	if ( ! strcmp( label, X ) ) { \
		if ( strlen( Y ) > 0 ) \
		{ \
			var->dummy = true; \
			p->cal( p, ( char * ) Y, 0 ); \
		} \
		res = var->val[ 0 ]; \
		goto end; \
	}

#else
// use fast map method for equation look-up
#define MODELBEGIN \
	double variable::fun( object *caller ) \
	{ \
		double res = def_res; \
		if ( quit == 2 ) \
			return res; \
		if ( eq_func == NULL ) \
		{ \
			auto eq_it = eq_map.find( label ); \
			if ( eq_it != eq_map.end( ) ) \
				eq_func = eq_it->second; \
			else \
			{ \
				EQ_NOT_FOUND \
			} \
		} \
		res = ( eq_func )( caller, this ); \
		EQ_TEST_RESULT \
		return res; \
	} \
	void init_map( ) \
	{ \
		eq_map = \
		{

#define MODELEND \
		}; \
	}
			
#define EQUATION( X ) \
	{ string( X ), [ ]( object *caller, variable *var ) \
		{ \
			EQ_BEGIN
		
#define RESULT( X ) \
			; \
			res = X; \
			DEBUG_CODE \
			return res; \
		} \
	},

#define END_EQUATION( X ) \
	{ \
		res = X; \
		DEBUG_CODE \
		return res; \
	}

#define EQUATION_DUMMY( X, Y ) \
	{ string( X ), [ ]( object *caller, variable *var ) \
		{ \
			if ( strlen( Y ) > 0 ) \
			{ \
				var->dummy = true; \
				var->up->cal( var->up, ( char * ) Y, 0 ); \
			} \
			return var->val[ 0 ]; \
		} \
	},

#endif

// redefine as macro to avoid conflicts with C++ version in <cmath.h>
#define abs( X ) _abs( X )
#define pi M_PI

#define ABORT { quit = 1; }
#define DEBUG_START deb_log( true )
#define DEBUG_START_AT( X ) deb_log( true, X )
#define DEBUG_STOP deb_log( false )
#define DEBUG_STOP_AT( X ) deb_log( false, X )
#define DEFAULT_RESULT( X ) { def_res = X; }
#define FAST set_fast( 1 )
#define FAST_FULL set_fast( 2 )
#define OBSERVE set_fast( 0 )
#define NO_NAN { use_nan = false; }
#define USE_NAN { use_nan = true; }
#define NO_POINTER_CHECK build_obj_list( false )
#define USE_POINTER_CHECK build_obj_list( true )
#define NO_SEARCH { no_search = true; }
#define USE_SEARCH { no_search = false; }
#define PARAMETER { var->param = 1; }
#define RND_GENERATOR( X ) { ran_gen = X; }
#define RND_SETSEED( X ) { seed = ( unsigned ) X; init_random( seed ); }
#define SLEEP( X ) msleep( ( unsigned ) X )

#define CONFIG ( ( const char * ) simul_name )
#define PATH ( ( const char * ) path )
#define CURRENT ( var->val[ 0 ] )
#define RND_SEED ( ( double ) seed - 1 )
#define T ( ( double ) t )
#define LAST_T ( ( double ) max_step )

#define LOG( ... ) \
{ \
	if ( ! fast ) \
	{ \
		char msg[ TCL_BUFF_STR ]; \
		sprintf( msg, __VA_ARGS__ ); \
		plog( msg ); \
	} \
}
#define PLOG( ... ) \
{ \
	if ( fast_mode < 2 ) \
	{ \
		char msg[ TCL_BUFF_STR ]; \
		sprintf( msg, __VA_ARGS__ ); \
		plog( msg ); \
	} \
}

#define V( X ) ( p->cal( p, ( char * ) X, 0 ) )
#define VL( X, Y ) ( p->cal( p, ( char * ) X, Y ) )
#define VS( O, X ) ( CHK_PTR_DBL( O ) O->cal( O, ( char * ) X, 0 ) )
#define VLS( O, X, Y ) ( CHK_PTR_DBL( O ) O->cal( O, ( char * ) X, Y ) )
#define SUM( X ) ( p->sum( ( char * ) X, 0 ) )
#define SUML( X, Y ) ( p->sum( ( char * ) X, Y ) )
#define SUMS( O, X ) ( CHK_PTR_DBL( O ) O->sum( ( char * ) X, 0 ) )
#define SUMLS( O, X, Y ) ( CHK_PTR_DBL( O ) O->sum( ( char * ) X, Y ) )
#define MAX( X ) ( p->overall_max( ( char * ) X, 0 ) )
#define MAXL( X, Y ) ( p->overall_max( ( char * ) X, Y ) )
#define MAXS( O, X ) ( CHK_PTR_DBL( O ) O->overall_max( ( char * ) X, 0 ) )
#define MAXLS( O, X, Y ) ( CHK_PTR_DBL( O ) O->overall_max( ( char * ) X, Y ) )
#define MIN( X ) ( p->overall_min( ( char * ) X, 0 ) )
#define MINL( X, Y ) ( p->overall_min( ( char * ) X, Y ) )
#define MINS( O, X ) ( CHK_PTR_DBL( O ) O->overall_min( ( char * ) X, 0 ) )
#define MINLS( O, X, Y ) ( CHK_PTR_DBL( O ) O->overall_min( ( char * ) X, Y ) )
#define AVE( X ) ( p->av( ( char * ) X, 0 ) )
#define AVEL( X, Y ) ( p->av( ( char * ) X, Y ) )
#define AVES( O, X ) ( CHK_PTR_DBL( O ) O->av( ( char * ) X, 0 ) )
#define AVELS( O, X, Y ) ( CHK_PTR_DBL( O ) O->av( ( char * ) X, Y ) )
#define WHTAVE( X, Y ) ( p->whg_av( ( char * ) Y, ( char * ) X, 0 ) )
#define WHTAVEL( X, Y, Z ) ( p->whg_av( ( char * ) Y, ( char * ) X, Z ) )
#define WHTAVES( O, X, Y ) ( CHK_PTR_DBL( O ) O->whg_av( ( char * ) Y, ( char * ) X, 0 ) )
#define WHTAVELS( O, X, Y, Z ) ( CHK_PTR_DBL( O ) O->whg_av( ( char * ) Y, ( char * ) X, Z ) )
#define SD( X ) ( p->sd( ( char * ) X, 0 ) )
#define SDL( X, Y ) ( p->sd( ( char * ) X, Y ) )
#define SDS( O, X ) ( CHK_PTR_DBL( O ) O->sd( ( char * ) X, 0 ) )
#define SDLS( O, X, Y ) ( CHK_PTR_DBL( O ) O->sd( ( char * ) X, Y ) )
#define COUNT( X ) ( p->count( ( char * ) X ) )
#define COUNTS( O, X ) ( CHK_PTR_DBL( O ) O->count( ( char * ) X ) )
#define COUNT_ALL( X ) ( p->count_all( ( char * ) X ) )
#define COUNT_ALLS( O, X ) ( CHK_PTR_DBL( O ) O->count_all( ( char * ) X ) )
#define STAT( X ) ( p->stat( ( char * ) X, v ) )
#define STATS( O, X ) ( CHK_PTR_DBL( O ) O->stat( ( char * ) X, v ) )
#define INTERACT( X, Y ) ( p->interact( ( char * ) X, Y, v, i, j, h, k, \
	cur, cur1, cur2, cur3, cur4, cur5, cur6, cur7, cur8, cur9, \
	curl, curl1, curl2, curl3, curl4, curl5, curl6, curl7, curl8, curl9 ) )
#define INTERACTS( O, X, Y ) ( CHK_PTR_DBL( O ) O->interact( ( char * ) X, Y, v, i, j, h, k, \
	cur, cur1, cur2, cur3, cur4, cur5, cur6, cur7, cur8, cur9, \
	curl, curl1, curl2, curl3, curl4, curl5, curl6, curl7, curl8, curl9 ) )
#define SEARCH( X ) ( p->search( ( char * ) X ) )
#define SEARCHS( O, X ) ( CHK_PTR_OBJ( O ) O->search( ( char * ) X ) )
#define SEARCH_CND( X, Y ) ( p->search_var_cond( ( char * ) X, Y, 0 ) )
#define SEARCH_CNDL( X, Y, Z ) ( p->search_var_cond( ( char * ) X, Y, Z ) )
#define SEARCH_CNDS( O, X, Y ) ( CHK_PTR_OBJ( O ) O->search_var_cond( ( char * ) X, Y, 0 ) )
#define SEARCH_CNDLS( O, X, Y, Z ) ( CHK_PTR_OBJ( O ) O->search_var_cond( ( char * ) X, Y, Z ) )
#define SEARCH_INST( X ) ( p->search_inst( X ) )
#define SEARCH_INSTS( O, X ) ( CHK_PTR_DBL( O ) O->search_inst( X ) )
#define RNDDRAW( X, Y ) ( p->draw_rnd( ( char * ) X, ( char * ) Y, 0 ) )
#define RNDDRAWL( X, Y, Z ) ( p->draw_rnd( ( char * ) X, ( char * ) Y, Z ) )
#define RNDDRAWS( O, X, Y ) ( CHK_PTR_OBJ( O ) O->draw_rnd( ( char * ) X, ( char * ) Y, 0 ) )
#define RNDDRAWLS( O, X, Y, Z ) ( CHK_PTR_OBJ( O ) O->draw_rnd( ( char * ) X, ( char * ) Y, Z ) )
#define RNDDRAW_FAIR( X ) ( p->draw_rnd( ( char * ) X ) )
#define RNDDRAW_FAIRS( O, X ) ( CHK_PTR_OBJ( O ) O->draw_rnd( ( char * ) X ) )
#define RNDDRAW_TOT( X, Y, Z ) ( p->draw_rnd( ( char * ) X, ( char * ) Y, 0, Z ) )
#define RNDDRAW_TOTL( X, Y, Z, W ) ( p->draw_rnd( ( char * ) X, ( char * ) Y, Z, W ) )
#define RNDDRAW_TOTS( O, X, Y, Z ) ( CHK_PTR_OBJ( O ) O->draw_rnd( ( char * ) X, ( char * ) Y, 0, Z ) )
#define RNDDRAW_TOTLS( O, X, Y, Z, W ) ( CHK_PTR_OBJ( O ) O->draw_rnd( ( char * ) X, ( char * ) Y, Z, W ) )
#define WRITE( X, Y ) ( p->write( ( char * ) X, Y, t ) )
#define WRITEL( X, Y, Z ) ( p->write( ( char * ) X, Y, Z ) )
#define WRITELL( X, Y, Z, W ) ( p->write( ( char * ) X, Y, Z, W ) )
#define WRITES( O, X, Y ) ( CHK_PTR_DBL( O ) O->write( ( char * ) X, Y, t ) )
#define WRITELS( O, X, Y, Z ) ( CHK_PTR_DBL( O ) O->write( ( char * ) X, Y, Z ) )
#define WRITELLS( O, X, Y, Z, W ) ( CHK_PTR_DBL( O ) O->write( ( char * ) X, Y, Z, W ) )
#define INCR( X, Y ) ( p->increment( ( char * ) X, Y ) )
#define INCRS( O, X, Y ) ( CHK_PTR_DBL( O ) O->increment( ( char * ) X, Y ) )
#define MULT( X, Y ) ( p->multiply( ( char * ) X, Y ) )
#define MULTS( O, X, Y ) ( CHK_PTR_DBL( O ) O->multiply( ( char * ) X, Y ) )
#define ADDOBJ( X ) ( p->add_n_objects2( ( char * ) X, 1 ) )
#define ADDOBJL( X, Y ) ( p->add_n_objects2( ( char * ) X, 1, Y ) )
#define ADDOBJS( O, X ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( ( char * ) X, 1 ) )
#define ADDOBJLS( O, X, Y ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( ( char * ) X, 1, Y ) )
#define ADDNOBJ( X, Y ) ( p->add_n_objects2( ( char * ) X, Y ) )
#define ADDNOBJL( X, Y, Z ) ( p->add_n_objects2( ( char * ) X, Y, Z ) )
#define ADDNOBJS( O, X, Y ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( ( char * ) X, Y ) )
#define ADDNOBJLS( O, X, Y, Z ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( ( char * ) X, Y, Z ) )
#define ADDOBJ_EX( X, Y ) ( p->add_n_objects2( ( char * ) X, 1, Y ) )
#define ADDOBJ_EXL( X, Y, Z ) ( p->add_n_objects2( ( char * ) X, 1, Y, Z ) )
#define ADDOBJ_EXS( O, X, Y ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( ( char * ) X, 1, Y ) )
#define ADDOBJ_EXLS( O, X, Y, Z ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( ( char * ) X, 1, Y, Z ) )
#define ADDNOBJ_EX( X, Y, Z ) ( p->add_n_objects2( ( char * ) X, Y, Z ) )
#define ADDNOBJ_EXL( X, Y, Z, W ) ( p->add_n_objects2( ( char * ) X, Y, Z, W ) )
#define ADDNOBJ_EXS( O, X, Y, Z ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( ( char * ) X, Y, Z ) )
#define ADDNOBJ_EXLS( O, X, Y, Z, W ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( ( char * ) X, ( int ) Y, Z, W ) )
#define DELETE( O ) ( CHK_PTR_VOID( O ) O->delete_obj( ) )
#define SORT( X, Y, Z ) ( p->lsdqsort( ( char * ) X, ( char * ) Y, ( char * ) Z ) )
#define SORTS( O, X, Y, Z ) ( CHK_PTR_OBJ( O ) O->lsdqsort( ( char * ) X, ( char * ) Y, ( char * ) Z ) )
#define SORT2( X, Y, Z, W ) ( p->lsdqsort( ( char * ) X, ( char * ) Y, ( char * ) Z, ( char * ) W ) )
#define SORT2S( O, X, Y, Z, W ) ( CHK_PTR_OBJ( O ) O->lsdqsort( ( char * ) X, ( char * ) Y, ( char * ) Z, ( char * ) W ) )
#define DOWN_LAT ( p->lat_down( ) )
#define DOWN_LATS( O ) ( CHK_PTR_OBJ( O ) O->lat_down( ) )
#define LEFT_LAT ( p->lat_left( ) )
#define LEFT_LATS( O ) ( CHK_PTR_OBJ( O ) O->lat_left( ) )
#define RIGHT_LAT ( p->lat_right( ) )
#define RIGHT_LATS( O ) ( CHK_PTR_OBJ( O ) O->lat_right( ) )
#define UP_LAT ( p->lat_up( ) )
#define UP_LATS( O ) ( CHK_PTR_OBJ( O ) O->lat_up( ) )
#define INIT_LAT( ... ) init_lattice( __VA_ARGS__ )
#define DELETE_LAT close_lattice( )
#define V_LAT( X, Y ) read_lattice( X, Y )
#define WRITE_LAT( X, ... ) update_lattice( X, __VA_ARGS__ )
#define SAVE_LAT( ... ) save_lattice( __VA_ARGS__ )
#define V_NODEID ( CHK_NODE_DBL( p ) p->node->id )
#define V_NODEIDS( O ) ( CHK_PTR_DBL( O ) CHK_NODE_DBL( O ) O->node->id )
#define V_NODENAME ( CHK_NODE_CHR( p ) p->node->name )
#define V_NODENAMES( O ) ( CHK_PTR_CHR( O ) CHK_NODE_CHR( O ) O->node->name )
#define V_LINK( L ) ( CHK_LNK_DBL( L ) L->weight )
#define STAT_NET( X ) ( p->stats_net( ( char * ) X, v ) )
#define STAT_NETS( O, X ) ( CHK_PTR_DBL( O ) O->stats_net( ( char * ) X, v ) )
#define STAT_NODE ( CHK_NODE_DBL( p ) p->node->nLinks )
#define STAT_NODES( O ) ( CHK_PTR_DBL( O ) CHK_NODE_DBL( O ) O->node->nLinks )
#define SEARCH_NODE( X, Y ) ( p->search_node_net( ( char * ) X, Y ) )
#define SEARCH_NODES( O, X, Y ) ( CHK_PTR_OBJ( O ) O->search_node_net( ( char * ) X, Y ) )
#define SEARCH_LINK( X ) ( p->search_link_net( X ) )
#define SEARCH_LINKS( O, X ) ( CHK_PTR_LNK( O ) O->search_link_net( X ) )
#define RNDDRAW_NODE( X ) ( p->draw_node_net( ( char * ) X ) )
#define RNDDRAW_NODES( O, X ) ( CHK_PTR_OBJ( O ) O->draw_node_net( ( char * ) X ) )
#define RNDDRAW_LINK ( p->draw_link_net( ) )
#define RNDDRAW_LINKS( O ) ( CHK_PTR_LNK( O ) O->draw_link_net( ) )
#define DRAWPROB_NODE( X ) ( CHK_NODE_DBL( p ) p->node->prob = X )
#define DRAWPROB_NODES( O, X ) ( CHK_PTR_DBL( O ) CHK_NODE_DBL( O ) O->node->prob = X )
#define DRAWPROB_LINK( L, X ) ( CHK_LNK_DBL( L ) L->probTo = X )
#define LINKTO( L ) ( CHK_LNK_OBJ( L ) L->ptrTo )
#define LINKFROM( L ) ( CHK_LNK_OBJ( L ) L->ptrFrom )
#define WRITE_NODEID( X ) ( CHK_NODE_DBL( p ) p->node->id = X )
#define WRITE_NODEIDS( O, X ) ( CHK_PTR_DBL( O ) CHK_NODE_DBL( O ) O->node->id = X )
#define WRITE_NODENAME( X ) ( p->name_node_net( ( char * ) X ) )
#define WRITE_NODENAMES( O, X ) ( CHK_PTR_VOID( O ) O->name_node_net( ( char * ) X ) )
#define WRITE_LINK( L, X ) ( CHK_LNK_DBL( L ) L->weight = X )
#define INIT_NET( X, ... ) ( p->init_stub_net( ( char * ) X, __VA_ARGS__ ) )
#define INIT_NETS( O, X, ... ) ( CHK_PTR_DBL( O ) O->init_stub_net( ( char * ) X, __VA_ARGS__ ) )
#define LOAD_NET( X, Y ) ( p->read_file_net( ( char * ) X, "", ( char * ) Y, seed - 1, "net" ) )
#define LOAD_NETS( O, X, Y ) ( CHK_PTR_DBL( O ) O->read_file_net( ( char * ) X, "", ( char * ) Y, seed - 1, "net" ) )
#define SAVE_NET( X, Y ) ( p->write_file_net( ( char * ) X, "", ( char * ) Y, seed - 1, false ) )
#define SAVE_NETS( O, X, Y ) ( CHK_PTR_DBL( O ) O->write_file_net( ( char * ) X, "", ( char * ) Y , seed - 1, false ) )
#define SNAP_NET( X, Y ) ( p->write_file_net( ( char * ) X, "", ( char * ) Y, seed - 1, true ) )
#define SNAP_NETS( O, X, Y ) ( CHK_PTR_DBL( O ) O->write_file_net( ( char * ) X, "", ( char * ) Y, seed - 1, true ) )
#define ADDNODE( X, Y ) ( p->add_node_net( X, Y, false ) )
#define ADDNODES( O, X, Y ) ( CHK_PTR_OBJ( O ) O->add_node_net( X, Y, false ) )
#define ADDLINK( X ) ( p->add_link_net( X, 0 , 1 ) )
#define ADDLINKW( X, Y ) ( p->add_link_net( X, Y, 1 ) )
#define ADDLINKS( O, X ) ( CHK_PTR_LNK( O ) O->add_link_net( X, 0 , 1 ) )
#define ADDLINKWS( O, X, Y ) ( CHK_PTR_LNK( O ) O->add_link_net( X, Y, 1 ) )
#define DELETE_NET( X ) ( p->delete_net( ( char * ) X ) )
#define DELETE_NETS( O, X ) ( CHK_PTR_VOID( O ) O->delete_net( ( char * ) X ) )
#define DELETE_NODE ( p->delete_node_net( ) )
#define DELETE_NODES( O ) ( CHK_PTR_VOID( O ) O->delete_node_net( ) )
#define DELETE_LINK( L ) ( CHK_LNK_VOID( L ) L->ptrFrom->delete_link_net( L ) )
#define SHUFFLE_NET( X ) ( p->shuffle_nodes_net( ( char * ) X ) )
#define SHUFFLE_NETS( O, X ) ( CHK_PTR_OBJ( O ) O->shuffle_nodes_net( ( char * ) X ) )
#define RECALC( X ) ( p->recal( ( char * ) X ) )
#define RECALCS( O, X ) ( CHK_PTR_DBL( O ) O->recal( ( char * ) X ) )
#define INIT_TSEARCH( X ) ( p->initturbo( ( char * ) X, 0 ) )
#define INIT_TSEARCHT( X, Y ) ( p->initturbo( ( char * ) X, Y ) )
#define INIT_TSEARCHS( O, X ) ( CHK_PTR_DBL( O ) O->initturbo( ( char * ) X, 0 ) )
#define INIT_TSEARCHTS( O, X, Y ) ( CHK_PTR_DBL( O ) O->initturbo( ( char * ) X, Y ) )
#define TSEARCH( X, Y ) ( p->turbosearch( ( char * ) X, 0, Y ) )
#define TSEARCHS( O, X, Y ) ( CHK_PTR_OBJ( O ) O->turbosearch( ( char * ) X, 0, Y ) )
#define INIT_TSEARCH_CND( X ) ( p->initturbo_cond( ( char * ) X ) )
#define INIT_TSEARCH_CNDS( O, X ) ( CHK_PTR_DBL( O ) O->initturbo_cond( ( char * ) X ) )
#define TSEARCH_CND( X, Y ) ( p->turbosearch_cond( ( char * ) X, Y ) )
#define TSEARCH_CNDS( O, X, Y ) ( CHK_PTR_OBJ( O ) O->turbosearch_cond( ( char * ) X, Y ) )
#define V_CHEAT( X, Y ) ( p->cal( Y, ( char * ) X, 0 ) )
#define V_CHEATL( X, Y, Z ) ( p->cal( Z, ( char * ) X, Y ) )
#define V_CHEATS( O, X, Y ) ( CHK_PTR_DBL( O ) O->cal( Y, ( char * ) X, 0 ) )
#define V_CHEATLS( O, X, Y, Z ) ( CHK_PTR_DBL( O ) O->cal( Z, ( char * ) X, Y ) )
#define ADDEXT( X ) { if ( p->cext != NULL ) DELETE_EXT( X ); p->cext = reinterpret_cast< void * >( new X ); }
#define ADDEXTS( O, X ) { if ( O->cext != NULL ) DELETE_EXTS( O, X ); O->cext = reinterpret_cast< void * >( new X ); }
#define DELETE_EXT( X ) { delete reinterpret_cast< X * >( p->cext ); p->cext = NULL; }
#define DELETE_EXTS( O, X ) { delete reinterpret_cast< X * >( O->cext ); O->cext = NULL; }
#define DO_EXT( X, Y, ... ) ( P_EXT( X ) -> Y( __VA_ARGS__ ) )
#define DO_EXTS( O, X, Y, ... ) ( P_EXTS( O, X ) -> Y( __VA_ARGS__ ) )
#define EXT( X ) ( * P_EXT( X ) )
#define EXTS( O, X ) ( * P_EXTS( O, X ) )
#define EXEC_EXT( X, Y, Z, ... ) ( P_EXT( X ) -> Y.Z( __VA_ARGS__ ) )
#define EXEC_EXTS( O, X, Y, Z, ... ) ( P_EXTS( O, X ) -> Y.Z( __VA_ARGS__ ) )
#define P_EXT( X ) ( reinterpret_cast< X * >( p->cext ) )
#define P_EXTS( O, X ) ( reinterpret_cast< X * >( O->cext ) )
#define V_EXT( X, Y ) ( P_EXT( X ) -> Y )
#define V_EXTS( O, X, Y ) ( P_EXTS( O, X ) -> Y )
#define WRITE_EXT( X, Y, Z ) ( P_EXT( X ) -> Y = Z )
#define WRITE_EXTS( O, X, Y, Z ) ( P_EXTS( O, X ) -> Y = Z )

#define CYCLE( X, Y ) for ( X = get_cycle_obj( p, ( char * ) Y, "CYCLE" ); X != NULL; X = go_brother( X ) )
#define CYCLE_SAFE( X, Y ) for ( X = get_cycle_obj( p, ( char * ) Y, "CYCLE_SAFE" ), \
							  cyccur = go_brother( X ); X != NULL; X = cyccur, \
							  cyccur != NULL ? cyccur = go_brother( cyccur ) : cyccur = cyccur )
#define CYCLE2_SAFE( X, Y ) for ( X = get_cycle_obj( p, ( char * ) Y, "CYCLE_SAFE" ), \
							  cyccur2 = go_brother( X ); X != NULL; X = cyccur2, \
							  cyccur2 != NULL ? cyccur2 = go_brother( cyccur2 ) : cyccur2 = cyccur2 )
#define CYCLE3_SAFE( X, Y ) for ( X = get_cycle_obj( p, ( char * ) Y, "CYCLE_SAFE" ), \
							  cyccur3 = go_brother( X ); X != NULL; X = cyccur3, \
							  cyccur3 != NULL ? cyccur3 = go_brother( cyccur3 ) : cyccur3 = cyccur3 )
#define CYCLES( O, X, Y ) for ( X = get_cycle_obj( O, ( char * ) Y, "CYCLES" ); X != NULL; X = go_brother( X ) )
#define CYCLE_SAFES( O, X, Y ) for ( X = get_cycle_obj( O, ( char * ) Y, "CYCLE_SAFES" ), \
								 cyccur = go_brother( X ); X != NULL; X = cyccur, \
								 cyccur != NULL ? cyccur = go_brother( cyccur ) : cyccur = cyccur )
#define CYCLE2_SAFES( O, X, Y ) for ( X = get_cycle_obj( O, ( char * ) Y, "CYCLE_SAFES" ), \
								 cyccur2 = go_brother( X ); X != NULL; X = cyccur2, \
								 cyccur2 != NULL ? cyccur2 = go_brother( cyccur2 ) : cyccur2 = cyccur2 )
#define CYCLE3_SAFES( O, X, Y ) for ( X = get_cycle_obj( O, ( char * ) Y, "CYCLE_SAFES" ), \
								 cyccur3 = go_brother( X ); X != NULL; X = cyccur3, \
								 cyccur3 != NULL ? cyccur3 = go_brother( cyccur3 ) : cyccur3 = cyccur3 )
								 
#ifdef NO_POINTER_INIT
#define CYCLE_LINK( O ) for ( O = p->node->first; O != NULL; O = O->next )
#define CYCLE_LINKS( C, O ) for ( O = C->node->first; O != NULL; O = O->next )
#else
#define CYCLE_LINK( X ) if ( p->node == NULL ) \
		no_node_dbl( p->label, __FILE__, __LINE__ ); \
	else \
		for ( X = p->node->first; X != NULL; X = X->next )
#define CYCLE_LINKS( O, X ) if ( O == NULL ) \
		bad_ptr_dbl( O, __FILE__, __LINE__ ); \
	else if ( O->node == NULL ) \
		no_node_dbl( O->label, __FILE__, __LINE__ ); \
	else \
		for ( X = O->node->first; X != NULL; X = X->next )
#endif

#define CYCLE_EXT( X, Y, Z ) for ( X = EXEC_EXT( Y, Z, begin ); X != EXEC_EXT( Y, Z, end ); ++X )
#define CYCLE_EXTS( O, X, Y, Z ) for ( X = EXEC_EXTS( O, Y, Z, begin ); X != EXEC_EXTS( O, Y, Z, end ); ++X )

// DEPRECATED MACRO COMPATIBILITY DEFINITIONS
// enabled only when directly including fun_head.h (and not fun_head_fast.h)
#ifndef FAST_LOOKUP

double init_lattice( double pixW = 0, double pixH = 0, double nrow = 100, double ncol = 100, 
					 char const lrow[ ] = "y", char const lcol[ ] = "x", char const lvar[ ] = "", 
					 object *p = NULL, int init_color = -0xffffff );
double poidev( double xm, long *idum_loc = NULL );
int deb( object *r, object *c, char const *lab, double *res, bool interact = false );
void cmd( const char *cm, ... );
#define FUNCTION( X ) EQUATION( X )
#define UNIFORM( X, Y ) uniform( X, Y )
#define rnd_integer( X, Y ) uniform_int( X, Y )
#define VL_CHEAT( X, Y, C ) V_CHEATL( X, Y, C )
#define VS_CHEAT( X, Y, C ) V_CHEATS( X, Y, C )
#define VLS_CHEAT( X, Y, Z, C ) V_CHEATLS( X, Y, Z, C )
#define ADDOBJL_EX( X, Y, Z ) ADDOBJ_EXL( X, Y, Z )
#define ADDOBJS_EX( O, X, Y ) ADDOBJ_EXS( O, X, Y )
#define ADDOBJLS_EX( O, X, Y, Z ) ADDOBJ_EXLS( O, X, Y, Z )
#define ADDNOBJL_EX( X, Y, Z, W ) ADDNOBJ_EXL( X, Y, Z, W )
#define ADDNOBJS_EX( O, X, Y, Z ) ADDNOBJ_EXS( O, X, Y, Z )
#define ADDNOBJLS_EX( O, X, Y, Z, W ) ADDNOBJ_EXLS( O, X, Y, Z, W )
#define TSEARCH_INI( X ) INIT_TSEARCH( X )
#define TSEARCHS_INI( O, X ) INIT_TSEARCHS( O, X )
#define TSEARCHT_INI( X, Y ) INIT_TSEARCHT( X, Y)	// the number of objects no longer required
#define TSEARCHT( X, Y, Z ) TSEARCH( X, Z )			// when calling turbo search, as it is already
#define TSEARCHTS( O, X, Y, Z ) TSEARCHS( O, X, Z )	// stored in the bridge in faster log form
#define SORTS2( O, X, Y, L, Z ) SORT2S( O, X, Y, L, Z )
#define RNDDRAWFAIR( X ) RNDDRAW_FAIR( X )
#define RNDDRAWFAIRS(Z, X ) RNDDRAW_FAIRS(Z, X )
#define RNDDRAWTOT( X, Y,T ) RNDDRAW_TOT( X, Y,T )
#define RNDDRAWTOTL( X, Y, Z, T ) RNDDRAW_TOTL( X, Y, Z, T )
#define RNDDRAWTOTS(Z, X, Y,T ) RNDDRAW_TOTS(Z, X, Y,T )
#define RNDDRAWTOTLS( O, X, Y, Z, T ) RNDDRAW_TOTLS( O, X, Y, Z, T )
#define NETWORK_INI( X, Y, Z, ... ) INIT_NET( X, Y, Z, __VA_ARGS__ )
#define NETWORKS_INI( O, X, Y, Z, ... ) INIT_NETS( O, X, Y, Z, __VA_ARGS__ )
#define NETWORK_LOAD( X, Y, Z ) ( p->read_file_net( ( char * ) X, ( char * ) Y, ( char * ) Z, seed-1, "net" ) )
#define NETWORKS_LOAD( O, X, Y, Z ) ( O == NULL ? 0. : O->read_file_net( ( char * ) X, ( char * ) Y, ( char * ) Z, seed-1, "net" ) )
#define NETWORK_SAVE( X, Y, Z ) ( p->write_file_net( ( char * ) X, ( char * ) Y, ( char * ) Z, seed-1, false ) )
#define NETWORKS_SAVE( O, X, Y, Z ) ( O == NULL ? 0. : O->write_file_net( ( char * ) X, ( char * ) Y, ( char * ) Z , seed-1, false ) )
#define STATS_NET( O, X ) STAT_NETS( O, X )
#define SHUFFLE( X ) SHUFFLE_NET( X )
#define SHUFFLES( O, X ) SHUFFLE_NETS( O, X )
#define RNDDRAW_NET( X ) RNDDRAW_NODE( X )
#define RNDDRAWS_NET( O, X ) RNDDRAW_NODES( O, X )
#define SEARCH_NET( X, Y ) SEARCH_NODE( X, Y )
#define SEARCHS_NET( O, X, Y ) SEARCH_NODES( O, X, Y )
#define VS_NODEID( O ) V_NODEIDS( O )
#define VS_NODENAME( O ) V_NODENAMES( O )
#define WRITES_NODEID( O, X ) WRITE_NODEIDS( O, X )
#define WRITES_NODENAME( O, X ) WRITE_NODENAMES( O, X )
#define STATS_NODE( O ) STAT_NODES( O )
#define DELETELINK( O ) DELETE_LINK( O )
#define SEARCHS_LINK( O, X ) SEARCH_LINKS( O, X )
#define VS_WEIGHT( O ) V_LINK( O )
#define WRITES_WEIGHT( O, X ) WRITE_LINK( O, X )
#define CYCLES_LINK( C, O ) CYCLE_LINKS( C, O )
#define ADD_EXT( CLASS ) ADDEXT( CLASS )
#define ADDS_EXT( O, CLASS ) ADDEXTS( O, CLASS )
#define DELETES_EXT( O, CLASS ) DELETE_EXTS( O, CLASS )
#define PS_EXT( O, CLASS ) P_EXTS( O, CLASS )
#define VS_EXT( O, CLASS, OBJ ) V_EXTS( O, CLASS, OBJ )
#define WRITES_EXT( O, CLASS, OBJ, VAL ) WRITE_EXTS( O, CLASS, OBJ, VAL )
#define EXECS_EXT( O, CLASS, OBJ, METHOD, ... ) EXEC_EXTS( O, CLASS, OBJ, METHOD, __VA_ARGS__ )
#define CYCLES_EXT( O, ITER, CLASS, OBJ ) CYCLE_EXTS( O, ITER, CLASS, OBJ )

#define DEBUG \
	f = fopen( "log.txt", "a" ); \
	fprintf( f, "t=%d\t%s\t(cur=%g)\n", t, var->label, var->val[0] ); \
	fclose( f );
 
#define DEBUG_AT( X ) \
	if ( t >= X ) \
	{ \
		DEBUG \
	};

#endif
