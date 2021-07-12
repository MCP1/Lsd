/*************************************************************

	LSD 8.0 - May 2021
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD
	
 *************************************************************/

/*************************************************************
GETLIMITS.CPP
Executes the lsd_getlimits command line utility.

Lists all initial values ranges and configuration.
*************************************************************/

#include "decl.h"

// limits and description for simulation settings
#define MIN_STEP 10
#define MAX_STEP 10000
#define DESC_STEP "Number of time steps to perform the simulation"
#define MIN_RUNS 1
#define MAX_RUNS 100
#define DESC_RUNS "Number of times to repeat the simulation (Monte Carlo experiment)"
#define MIN_SEED 1
#define MAX_SEED 999999
#define DESC_SEED "First seed to be used to initialize the pseudorandom number generator"


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
bool save_ok = true;		// control if saving model configuration is possible
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
char name_rep[ MAX_PATH_LENGTH + 1 ] = "";	// documentation report file name
char path_rep[ MAX_PATH_LENGTH + 1 ] = "";	// documentation report file path
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

// constant string arrays
const char *signal_names[ REG_SIG_NUM ] = REG_SIG_NAME;
const int signals[ REG_SIG_NUM ] = REG_SIG_CODE;


char *out_file = NULL;		// output .csv file, if any

// command line strings
const char lsdCmdMsg[ ] = "This is the LSD Initial Values Range Reader.";
const char lsdCmdDsc[ ] = "It reads a LSD configuration file (.lsd) and a LSD sensitivity analysis file\n(.sa) and shows the ranges used for variables/parameters being analyzed,\noptionally saving them in a comma separated text file (.csv).\n";
const char lsdCmdHlp[ ] = "Command line options:\n'-f FILENAME.lsd' the configuration file to use\n'-s FILENAME.sa' the sensitivity analysis file to use\n'-o OUTPUT.csv' name for the comma separated output text file\n";


/*********************************
 LSDMAIN
 *********************************/
int lsdmain( int argn, char **argv )
{
	int i, confs;
	char *sep;
	FILE *f;

	path = new char[ strlen( "" ) + 1 ];
	strcpy( path, "" );

	findex = 1;

	if ( argn < 5 )
	{
		fprintf( stderr, "\n%s\n%s\n%s\n", lsdCmdMsg, lsdCmdDsc, lsdCmdHlp );
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
			// read -s parameter : sensitivity file name
			if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 's' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
			{
				sens_file = new char[ strlen( argv[ 1 + i ] ) + 1 ];
				strcpy( sens_file, argv[ 1 + i ] );
				continue;
			}
			// read -o parameter : output file name
			if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'o' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
			{
				out_file = new char[ strlen( argv[ 1 + i ] ) + 1 ];
				strcpy( out_file, argv[ 1 + i ] );
				continue;
			}

			fprintf( stderr, "\nOption '%c%c' not recognized.\n%s\n%s\n", argv[ i ][ 0 ], argv[ i ][ 1 ], lsdCmdMsg, lsdCmdHlp );
			myexit( 2 );
		}
	}

	if ( struct_file == NULL )
	{
		fprintf( stderr, "\nNo original configuration file provided.\n%s\nSpecify a -f FILENAME.lsd to use for reading the saved variables (if any).\n\n", lsdCmdMsg );
		myexit( 3 );
	}

	f = fopen( struct_file, "r" );
	if ( f == NULL )
	{
		fprintf( stderr, "\nFile '%s' not found.\n%s\nSpecify an existing -f FILENAME.lsd configuration file.\n\n", struct_file, lsdCmdMsg );
		myexit( 4 );
	}
	fclose( f );

	root = new object;
	root->init( NULL, "Root" );
	add_description( "Root" );
	reset_blueprint( NULL );

	if ( load_configuration( true ) != 0 )
	{
		fprintf( stderr, "\nFile '%s' is invalid.\n%s\nCheck if the file is a valid LSD configuration or regenerate it using the LSD Browser.\n\n", struct_file, lsdCmdMsg );
		myexit( 5 );
	}

	if ( sens_file == NULL )
	{
		fprintf( stderr, "\nNo sensitivity analysis file provided.\n%s\nSpecify a -s FILENAME.sa to use for reading the values limits (if any).\n\n", lsdCmdMsg );
		myexit( 6 );
	}

	// read sensitivity file
	f = fopen( sens_file, "rt" );
	if ( f == NULL )
	{
		fprintf( stderr, "\nFile '%s' not found.\n%s\nSpecify an existing -s FILENAME.sa sensitivity analysis file.\n\n", sens_file, lsdCmdMsg );
		myexit( 7 );
	}

	if ( load_sensitivity( f ) != 0 )
	{
		fprintf( stderr, "\nFile '%s' is invalid.\n%s\nCheck if the file is a valid LSD sensitivity analysis or regenerate it using the LSD Browser.\n\n", sens_file, lsdCmdMsg  );
		fclose( f );
		myexit( 8 );
	}

	fclose( f );

	if ( out_file != NULL && strlen( out_file ) != 0 )
	{
		f = fopen( out_file, "wt" );
		if ( f == NULL )
		{
			fprintf( stderr, "\nFile '%s' cannot be saved.\n%s\nCheck if the drive or the file is set READ-ONLY, change file name or\nselect a drive with write permission and try again.\n\n", out_file, lsdCmdMsg  );
			myexit( 9 );
		}

		sep = new char [ strlen( CSV_SEP ) + 1 ];
		strcpy( sep, CSV_SEP );

		// write .csv header
		fprintf( f, "Name%sType%sLag%sFormat%sValue%sMinimum%sMaximum%sDescription\n", sep, sep, sep, sep, sep, sep, sep );
		
		// write all parameters and initial conditions
		get_sa_limits( root, f, sep );
		
		// write simulation setting
		fprintf( f, "_timeSteps_%ssetting%s0%sinteger%s%d%s%d%s%d%s%s\n", 
				 sep, sep, sep, sep, max_step, sep, MIN_STEP, sep, MAX_STEP, sep, DESC_STEP );
		fprintf( f, "_numRuns_%ssetting%s0%sinteger%s%d%s%d%s%d%s%s\n", 
				 sep, sep, sep, sep, sim_num, sep, MIN_RUNS, sep, MAX_RUNS, sep, DESC_RUNS );
		fprintf( f, "_rndSeed_%ssetting%s0%sinteger%s%d%s%d%s%d%s%s\n", 
				 sep, sep, sep, sep, seed, sep, MIN_SEED, sep, MAX_SEED, sep, DESC_SEED );
		
		fclose( f );
	}
	else	// send to stdout
	{
		get_sa_limits( root, stdout, "\t" );
		fprintf( stdout, "_timeSteps_%ssetting%s0%sinteger%s%d%s%d%s%d%s%s\n", 
				 sep, sep, sep, sep, max_step, sep, MIN_STEP, sep, MAX_STEP, sep, DESC_STEP );
		fprintf( stdout, "_numRuns_%ssetting%s0%sinteger%s%d%s%d%s%d%s%s\n", 
				 sep, sep, sep, sep, sim_num, sep, MIN_RUNS, sep, MAX_RUNS, sep, DESC_RUNS );
		fprintf( stdout, "_rndSeed_%ssetting%s0%sinteger%s%d%s%d%s%d%s%s\n", 
				 sep, sep, sep, sep, seed, sep, MIN_SEED, sep, MAX_SEED, sep, DESC_SEED );
	}

	empty_sensitivity( rsense );
	empty_blueprint( );
	empty_description( );
	root->delete_obj( );
	delete [ ] out_file;
	delete [ ] simul_name;

	return 0;
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
