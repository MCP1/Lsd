/*************************************************************

	LSD 7.3 - December 2020
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD
	
 *************************************************************/

/*************************************************************
CONFGEN.CPP
Execute the lsd_confgen command line utility.

Generates new configurations from a base one.
*************************************************************/

#include "decl.h"

#define SEP	",;\t"			// column separators to use


bool ignore_eq_file = true;	// flag to ignore equation file in configuration file
bool message_logged = false;// new message posted in log window
bool no_more_memory = false;// memory overflow when setting data save structure
bool no_ptr_chk = false;	// disable user pointer checking
bool no_saved = true;		// disable the usage of saved values as lagged ones
bool no_search;				// disable the standard variable search mechanism
bool no_zero_instance = true;// flag to allow deleting last object instance
bool on_bar;				// flag to indicate bar is being draw in log window
bool parallel_mode;			// parallel mode (multithreading) status
bool running = false;		// simulation is running
bool struct_loaded = false;	// a valid configuration file is loaded
bool unsavedData = false;	// flag unsaved simulation results
bool unsavedSense = false;	// control for unsaved changes in sensitivity data
bool user_exception = false;// flag indicating exception was generated by user code
bool use_nan;				// flag to allow using Not a Number value
char *config_file = NULL;	// name of text configurations file
char *eq_file = NULL;		// equation file content
char *exec_path = NULL;		// path of executable file
char *path = NULL;			// path of current configuration
char *sens_file = NULL;		// current sensitivity analysis file
char *simul_name = NULL;	// name of current simulation configuration
char *struct_file = NULL;	// name of current configuration file
char equation_name[ MAX_PATH_LENGTH ] = "";	// equation file name
char lsd_eq_file[ MAX_FILE_SIZE + 1 ] = "";	// equations saved in configuration file
char msg[ TCL_BUFF_STR ] = "";				// auxiliary Tcl buffer
char name_rep[ MAX_PATH_LENGTH ] = "";		// documentation report file name
char nonavail[ ] = "NA";	// string for unavailable values (use R default)
int actual_steps = 0;		// number of executed time steps
int debug_flag = false;		// debug enable control (bool)
int fast_mode = 1;			// flag to hide LOG messages & runtime plot
int findex = 1;				// current multi configuration job
int findexSens = 0;			// index to sequential sensitivity configuration filenames
int max_step = 100;			// default number of simulation runs
int prof_aggr_time = false;	// show aggregate profiling times
int prof_min_msecs = 0;		// profile only variables taking more than X msecs.
int prof_obs_only = false;	// profile only observed variables
int quit = 0;				// simulation interruption mode (0=none)
int t;						// current time step
int series_saved = 0;		// number of series saved
int sim_num = 1;			// simulation number running
int stack;					// LSD stack call level
int when_debug;				// next debug stop time step (0 for none)
int wr_warn_cnt;			// invalid write operations warning counter
long nodesSerial = 1;		// network node's serial number global counter
unsigned seed = 1;			// random number generator initial seed
description *descr = NULL;	// model description structure
lsdstack *stacklog = NULL;	// LSD stack
object *blueprint = NULL;	// LSD blueprint (effective model in use)
object *root = NULL;		// LSD root object
object *wait_delete = NULL;	// LSD object waiting for deletion
o_setT obj_list;			// set with all existing LSD objects
sense *rsense = NULL;		// LSD sensitivity analysis structure
variable *cemetery = NULL;	// LSD saved data series (from last simulation run)
variable *last_cemetery = NULL;	// LSD last saved data from deleted objects


char **vars = NULL;			// array of variables/parameters names
double **values = NULL;		// array of configuration values
int *lags = NULL;			// array of variables lags
int num_confs = 0;			// total configurations to produce
int num_vars = 0;			// total variables/parameters to change

// constant string arrays
const char *signal_names[ REG_SIG_NUM ] = REG_SIG_NAME;
const int signals[ REG_SIG_NUM ] = REG_SIG_CODE;


bool change_configuration( object *root, int findex );
int load_confs_csv( char *config );


/*********************************
LSD MAIN
*********************************/
int lsdmain( int argn, char **argv )
{
	int i, confs;
	FILE *f;

	path = new char[ strlen( "" ) + 1 ];
	strcpy( path, "" );

	findex = 1;

	if ( argn < 7 )
	{
		fprintf( stderr, "\nThis is LSD Configuration Generator.\nIt creates new LSD configuration file(s) (.lsd) based on changed parameters\nor variables initial values described in a comma separated text file (.csv).\nEach changed element should take one line. First column must contain the\nparameter or variable name. Second (and additional) column(s) must contain\nthe values to apply in the new configuration. First line (header) is required\nand considered for the number of columns only. One configuration is generated\nfor each column with values, sequentially numbered.\n\nCommand line options:\n'-f FILENAME.lsd' the original configuration file to use as base\n'-c CONFIG.csv' comma separated text file with new configuration values\n'-o FILE_BASE_NAME' base name (no extension) to save new configuration file(s)\n\n" );
		myexit( 1 );
	}
	else
	{
		for ( i = 1; i < argn; i += 2 )
		{
			// read -f parameter : original configuration file
			if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'f' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
			{
				struct_file = new char[ strlen( argv[ 1 + i ] ) + 1 ];
				strcpy( struct_file, argv[ 1 + i ] );
				continue;
			}
			// read -c parameter : text configuration file name
			if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'c' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
			{
				config_file = new char[ strlen( argv[ 1 + i ] ) + 1 ];
				strcpy( config_file, argv[ 1 + i ] );
				continue;
			}
			// read -o parameter : output base name
			if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'o' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
			{
				simul_name = new char[ strlen( argv[ 1 + i ] ) + 1 ];
				strcpy( simul_name, argv[ 1 + i ] );
				continue;
			}

			fprintf( stderr, "\nOption '%c%c' not recognized.\nThis is LSD Configuration Generator.\n\nCommand line options:\n'-f FILENAME.lsd' the original configuration file to use as base\n'-c CONFIG.csv' comma separated text file with new configuration values\n'-o FILE_BASE_NAME' base name (no extension) to save new configuration file(s)\n\n", argv[ i ][ 0 ], argv[ i ][ 1 ] );
			myexit( 2 );
		}
	}

	if ( struct_file == NULL )
	{
		fprintf( stderr, "\nNo original configuration file provided.\nThis is LSD Configuration Generator.\nSpecify a -f FILENAME.lsd to use as a base for the new configuration files.\n\n" );
		myexit( 3 );
	}

	f = fopen( struct_file, "r" );
	if ( f == NULL )
	{
		fprintf( stderr, "\nFile '%s' not found.\nThis is LSD Configuration Generator.\nSpecify an existing -f FILENAME.lsd base configuration file.\n\n", struct_file );
		myexit( 4 );
	}
	fclose( f );

	// default config file name
	if ( config_file == NULL )
	{
		config_file = new char[ strlen( struct_file ) + 1 ];
		strcpy( config_file, struct_file );
		strcpy( config_file + strlen( config_file ) - 4, ".csv" ); // change extension
	}

	f = fopen( config_file, "r" );
	if ( f == NULL )
	{
		fprintf( stderr, "\nFile '%s' not found.\nThis is LSD Configuration Generator.\nSpecify an existing -c CONFIG.csv to use as the new configuration values.\n\n", config_file );
		myexit( 4 );
	}
	fclose( f );

	// default output base name
	if ( simul_name == NULL )
	{
		simul_name = new char[ strlen( struct_file ) + 1 ];
		strcpy( simul_name, struct_file );
		simul_name[ strlen( simul_name ) - 4 ] = '\0'; // remove extension
	}

	root = new object;
	root->init( NULL, "Root" );
	add_description( "Root", "Object", "(no description available)" );
	reset_blueprint( NULL );

	if ( load_configuration( true ) != 0 )
	{
		fprintf( stderr, "\nFile '%s' is invalid.\nThis is LSD Configuration Generator.\nCheck if the file is a valid LSD configuration or regenerate it using the LSD Browser.\n\n", struct_file );
		myexit( 5 );
	}

	confs = load_confs_csv( config_file );
	if ( confs == 0 )
	{
		fprintf( stderr, "\nFile '%s' is invalid.\nThis is LSD Configuration Generator.\nSpecify a -c CONFIG.csv with a valid comma separated format.\n\n", config_file );
		myexit( 6 );
	}

	for ( i = 1; i <= confs; ++i )
	{
		if ( ! change_configuration( root, i ) )
		{
			fprintf( stderr, "\nInvalid parameter or variable name.\nThis is LSD Configuration Generator.\nCheck if the spelling of the names of parameters and variables is exactly the\nsame as in the original configuration.\n\n" );
			myexit( 7 );
		}

		if ( ! save_configuration( confs == 1 ? 0 : i ) )
		{
			fprintf( stderr, "\nFile '%s.lsd' cannot be saved.\nThis is LSD Configuration Generator.\nCheck if the drive or the file is set READ-ONLY, change file name or\nselect a drive with write permission and try again.\n\n", simul_name  );
			myexit( 8 );
		}
	}

	for ( i = 0; i < num_vars; ++i )
	{
		delete [ ] vars[ i ];
		delete [ ] values[ i ];
	}
	empty_blueprint( );
	empty_description( );
	root->delete_obj( );
	delete [ ] struct_file;
	delete [ ] config_file;
	delete [ ] simul_name;
	delete [ ] vars;
	delete [ ] values;
	delete [ ] lags;

	return 0;
}


/*********************************
LOAD_CONFS_CSV
*********************************/
int load_confs_csv( char *config )
{
	int i, j, lag;
	double value;
	char buf[ MAX_LINE_SIZE + 1 ], var[ MAX_ELEM_LENGTH + 1 ], *line, *tok;
	FILE *f = fopen( config, "r" );
	set< string > existing;

	if ( f == NULL )
		return 0;

	// free existing arrays
	for ( i = 0; i < num_vars; ++i )
	{
		delete [ ] vars[ i ];
		delete [ ] values[ i ];
	}
	delete [ ] vars;
	delete [ ] values;
	delete [ ] lags;
	vars = NULL;
	values = NULL;
	lags = NULL;

	// determine the number of columns
	fgets( buf, MAX_LINE_SIZE, f );
	line = buf;
	j = 0;
	do			// count columns
	{
		tok = strtok( line, SEP );
		line = NULL;
		if ( tok != NULL )			// finished?
			++j;
	}
	while ( tok != NULL );
	num_confs = j - 1;

	// determine the number of non-blank lines after header
	i = 0;
	do
	{
		strcpy( buf, "" );
		fgets( buf, MAX_LINE_SIZE, f );
		if ( strcmp( buf, "" ) )
		{
			tok = strtok( buf, SEP );
			sscanf( tok, " %s", var );		// remove spaces
			if ( ! strcmp( var, "" ) )
				continue;					// no name, go next line
			// check if name already exists and abort if so
			if ( ! existing.insert( var ).second )
			{
				fprintf( stderr, "Duplicated parameter/variable name: %s\n", var );
				return 0;
			}
			++i;
		}
	}
	while ( ! feof( f ) );
	num_vars = i;

	if ( num_confs < 1 || num_vars < 1 )
		return 0;							// nothing to do

	vars = new char * [ num_vars ];			// array of string pointers
	values = new double * [ num_vars ];		// array of double arrays
	lags = new int [ num_vars ];			// array of integer arrays

	// reread the file, populating the arrays
	i = 0;
	rewind( f );
	fgets( buf, MAX_LINE_SIZE, f );			// discard header
	do
	{
		strcpy( buf, "" );
		fgets( buf, MAX_LINE_SIZE, f );
		if ( strcmp( buf, "" ) )
		{
			lag = -1;
			tok = strtok( buf, SEP );
			sscanf( tok, " %s %u", var, & lag );	// get name & lags
			if ( ! strcmp( var, "" ) )
				continue;					// no name, go next line

			// save name & lag
			vars[ i ] = new char [ strlen( var ) + 1 ];
			strcpy( vars[ i ], var );
			lags[ i ] = lag;

			// save configuration values
			values[ i ] = new double [ num_confs ];
			for ( j = 0; j < num_confs; ++j )
			{
				tok = strtok( NULL, SEP );
				if ( tok != NULL && sscanf( tok, " %lf", & value ) > 0 )
					values[ i ][ j ] = value;
				else
					values[ i ][ j ] = NAN;
			}
			++i;
		}
	}
	while ( ! feof( f ) );

	return num_confs;
}


/*********************************
CHANGE_CONFIGURATION
*********************************/
bool change_configuration( object *root, int findex )
{
	int i, lag;
	char *lab;
	variable *cv;
	object *cur;

	if ( root == NULL || findex < 1 || findex > num_vars )
		return false;

	for ( i = 0; i < num_vars; ++i )
	{
		// handle pseudo-parameters
		if ( ! strcmp( vars[ i ], "_timeSteps_" ) )
		{
			max_step = ( int ) max( 0, round( values[ i ][ findex - 1 ] ) );
			continue;
		}
		if ( ! strcmp( vars[ i ], "_numRuns_" ) )
		{
			sim_num = ( int ) max( 0, round( values[ i ][ findex - 1 ] ) );
			continue;
		}
		if ( ! strcmp( vars[ i ], "_rndSeed_" ) )
		{
			seed = ( int ) max( 0, round( values[ i ][ findex - 1 ] ) );
			continue;
		}

		// it's a regular parameter/variable
		cv = root->search_var( NULL, vars[ i ] );	// get first instance
		if ( cv == NULL )
		{
			fprintf( stderr, "Parameter/variable not found: %s\n", vars[ i ] );
			return false;
		}
		lab = cv->up->label;						// container object label
		for ( cur = cv->up; cur != NULL; cur = cur->hyper_next( lab ) )
		{											// update all instances
			cv = cur->search_var( NULL, vars[ i ] );
			if ( cv == NULL )
			{
				fprintf( stderr, "Corrupted LSD configuration file.\n" );
				return false;
			}
			if ( values[ i ][ findex - 1 ] == values[ i ][ findex - 1 ] )	// test NAN
			{
				if ( cv->param == 1 )
					lag = 0;						// parameters have no lag
				else
				{
					if ( lags[ i ] < 0 )
						lag = 0;					// fix invalid/undefined (default=1) lags
					else
						lag = lags[ i ] - 1;

					if ( lag > cv->num_lag - 1 )
					{
						fprintf( stderr, "Variable not lagged or invalid lag: %s:%d\n", vars[ i ], lag + 1 );
						return false;
					}
				}
				cv->data_loaded = '+';
				cv->val[ lag ] = values[ i ][ findex - 1 ];
			}
		}
	}

	return true;
}


/*********************************
FUN
Dummy for linking
*********************************/
double variable::fun( object* r ) { return NAN; }


/*********************************
ALLOC_SAVE_VAR
*********************************/
bool alloc_save_var( variable *v ) { return true; }
