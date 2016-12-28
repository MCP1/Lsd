/***************************************************
****************************************************
LSD 7.0 - August 2015
written by Marco Valente
Universita' dell'Aquila

Copyright Marco Valente
Lsd is distributed according to the GNU Public License

Comments and bug reports to marco.valente@univaq.it
****************************************************
****************************************************/


/****************************************************
LSD_MAIN.CPP contains:
- early initialization (namely, of the Log windows)
- the main cycle: browse a model, run simulation, return to the browser.


The functions contained here are:

- void run(object *r)
Run the simulation model whose root is r. Running is not only the actual
simulation run, but also the initialization of result files. Of course, it has
also to manage the messages from user and from the model at run time.

- void print_title(object *root);
Prepare variables to store saved data.

- Tcl_Interp *InterpInitWin(char *tcl_dir);
A function that manages to initialize the tcl interpreter. Guess the standard
functions are actually bugged, because of the difficulty to retrive the directory
for the tk library. Why is difficult only for tk and not for tcl, don't know.
But is a good thing, so that I can actually copy the tcl directory and make
the modifications

- void plog(char *m);
print  message string m in the Log screen.

Other functions used here, and the source files where are contained:

- object *create( object *r);
manage the browser. Its code is in INTERF.CPP

- object *skip_next_obj(object *t, int *count);
Contained in UTIL.CPP. Counts how many types of objects equal to t are in this
group. count returns such value, and the whole function returns the next object
after the last of the series.

- object *go_brother(object *c);
Contained in UTIL.CPP. returns: c->next, if it is of the same type of c (brother).
Returns NULL otherwise. It is safe to use even when c or c->next are NULL.


- void cmd(char *cc);
Contained in UTIL.CPP. Standard routine to send the message string cc to the interp
Basically it makes a simple Tcl_Eval, but controls also that the interpreter
did not issue an error message.

- void myexit(int v);
Exit function, which is customized on the operative system.

- FILE *search_str(char *name, char *str);
UTIL.CPP given a string name, returns the file corresponding to name, and the current
position of the file is just after str.


****************************************************/

// launch TkCon as an auxiliary window for debugging (comment to disable)
//#define TKCON

#include "decl.h"

#ifndef NO_WINDOW			// global Tcl interpreter in LSD
Tcl_Interp *inter;
#endif

#ifdef LIBZ
bool dozip = true;			// compressed results file flag
#else
bool dozip = false;
#endif

// some program defaults
bool add_to_tot = true;		// flag to append results to existing totals file
bool grandTotal = false;	// flag to produce or not grand total in batch processing
bool ignore_eq_file = true;	// flag to ignore equation file in configuration file
bool overwConf = true;		// overwrite configuration on run flag
bool strWindowOn = true;	// control the presentation of the model structure window
char nonavail[] = "NA";		// string for unavailable values (use R default)
char tabs[] = "5c 7.5c 10c 12.5c 15c 17.5c 20c";	// Log window tabs
int lattice_type = 0;		// default lattice type (infrequent cells changes)
int max_step = 100;			// default number of simulation runs
int seed = 1;				// random number generator initial seed

bool batch_sequential = false;	// no-window multi configuration job running
bool debug_flag = false;	// debug enable control
bool firstRes = true;		// mark first results file (init grand total file)
bool message_logged = false;// new message posted in log window
bool no_more_memory = false;// memory overflow when setting data save structure	
bool no_res = false;		// do not produce .res results files
bool no_window = false;		// no-window command line job
bool pause_run;				// pause running simulation
bool redrawRoot = true;		// control for redrawing root window (.)
bool running = false;		// simulation is running
bool save_alt_path = false;	// alternate save path flag
bool scroll;				// scroll state in current runtime plot
bool struct_loaded = false;	// a valid configuration file is loaded
bool tk_ok = false;			// control for tk_ready to operate
bool unsavedData = false;	// flag unsaved simulation results
bool unsavedSense = false;	// control for unsaved changes in sensitivity data
char *alt_path = NULL;		// alternative output path
char *eq_file=NULL;			// equation file content
char *equation_name = NULL;	// equation file name
char *exec_file = NULL;		// name of executable file
char *exec_path = NULL;		// path of executable file
char *lsdroot = NULL;		// path of Lsd root directory
char *path = NULL;			// path of current configuration
char *sens_file = NULL;		// current sensitivity analysis file
char *simul_name = NULL;	// name of current simulation configuration
char *struct_file = NULL;	// name of current configuration file
char lsd_eq_file[ MAX_FILE_SIZE + 1 ];// equations saved in configuration file
char msg[ TCL_BUFF_STR ];	// auxiliary Tcl buffer
char name_rep[ MAX_PATH_LENGTH ] = "";	// documentation report file name
char tcl_dir[ MAX_PATH_LENGTH ] = "";	// Tcl/Tk directory
description *descr = NULL;	// model description structure
int actual_steps=0;			// number of executed time steps
int choice;					// Tcl menu control variable (main window)
int choice_g;               // Tcl menu control variable (structure window)
int cur_plt;				// current graph plot number
int cur_sim;				// current simulation run
int done_in;				// Tcl menu control variable (log window)
int fend;					// last multi configuration job to run
int findex;					// current multi configuration job
int findexSens=0;			// index to sequential sensitivity configuration filenames
int quit=0;					// simulation interruption mode (0=none)
int series_saved;			// number of series saved
int sim_num=1;				// simulation number running
int stack;					// Lsd stack call level
int stackinfo_flag=0;		// Lsd stack control
int t;						// current time step
int total_obj=0;			// total objects in model
int total_var=0;			// total variables/parameters in model
int when_debug;				// next debug stop time step (0 for none)
long nodesSerial = 0;		// network node's serial number global counter
lsdstack *stacklog = NULL;	// Lsd stack
object *blueprint = NULL;	// Lsd blueprint (effective model in use)
object *root = NULL;		// Lsd root object
sense *rsense = NULL;		// Lsd sensitivity analysis structure
variable *cemetery = NULL;	// Lsd saved data series (from last simulation run)


/*********************************
LSD MAIN
*********************************/
int lsdmain(int argn, char **argv)
{
char *str;
int i, len, done;
FILE *f;

path=new char[strlen("")+1];
simul_name=new char[strlen("Sim1")+1];
equation_name=new char[strlen("fun.cpp")+1];

strcpy(path, "");
strcpy(tcl_dir, "");
strcpy(simul_name, "Sim1");
strcpy(equation_name,"fun.cpp");
exec_file=clean_file(argv[0]);	// global pointer to the name of executable file
exec_path=clean_path(getcwd(NULL, 0));	// global pointer to path of executable file

root=new object;
root->init(NULL, "Root");
add_description("Root", "Object", "(no description available)");
blueprint=new object;
blueprint->init(NULL, "Root");

#ifdef NO_WINDOW

no_window = true;
findex=1;
fend=0;		// no file number limit

if(argn<3)
 {
  fprintf( stderr, "\nThis is the No Window version of Lsd. Command line options:\n'-f FILENAME.lsd' to run a single configuration file\n'-f FILE_BASE_NAME -s FIRST_NUM [-e LAST_NUM]' for batch sequential mode\n'-o PATH' to save result file(s) to a different subdirectory\n'-r' for skipping the generation of intermediate result file(s)\n'-g' for the generation of a single grand total file\n'-z' for preventing the generation of compressed result file(s)\n" );
  myexit( 1 );
 }
else
 {
 for(i=1; i<argn; i+=2)
 {
 if(argv[i][0]=='-' && argv[i][1]=='f' )
  {
   delete[] simul_name;
   simul_name=new char[strlen(argv[1+i])+1];
   strcpy(simul_name,argv[1+i]);
   continue;
  }
 if(argv[i][0]=='-' && argv[i][1]=='s' )
  {
	findex=atoi(argv[i+1]);
	batch_sequential = true;   
	continue;
  }
 if(argv[i][0]=='-' && argv[i][1]=='e' )	// read -e parameter : last sequential file to process
  {
 	fend=atoi(argv[i+1]);
	continue;
  }
 if( argv[i][0] == '-' && argv[i][1] == 'r' )	// read -r parameter : do not produce intermediate .res files
 {
	i--; 	// no parameter for this option
	no_res = true;
    continue;
 }
 if( argv[i][0] == '-' && argv[i][1] == 'g' )	// read -g parameter : create grand total file (batch only)
 {
	i--; 	// no parameter for this option
	grandTotal = true;
	printf( "Grand total file requested ('-g'), please don't run another instance of Lsd_gnuNW in this folder!\n" );
	continue;
 }
 if( argv[i][0] == '-' && argv[i][1] == 'z' )	// read -g parameter : don't create compressed result files
 {
	i--; 	// no parameter for this option
	dozip = false;
	continue;
 }
 if( argv[i][0] == '-' && argv[i][1] == 'o' )	// change the path for the output of result files
 {
	results_alt_path( argv[ 1 + i ] );
	continue;
 }
  
  fprintf( stderr, "\nOption '%c%c' not recognized.\nThis is the No Window version of Lsd. Command line options:\n'-f FILENAME.lsd' to run a single configuration file\n'-f FILE_BASE_NAME -s FIRST_NUM [-e LAST_NUM]' for batch sequential mode\n'-o PATH' to save result file(s) to a different subdirectory\n'-r' for skipping the generation of intermediate result file(s)\n'-g' for the generation of a single grand total file\n'-z' for preventing the generation of compressed result file(s)\n", argv[i][0], argv[i][1] );
  myexit( 2 );
  }
 } 

if ( ! batch_sequential )
 {
 struct_file=new char[strlen(simul_name)+1];
 sprintf(struct_file, "%s", simul_name);
 simul_name[strlen(simul_name)-4]='\0';
 } 
else
 {
  sprintf(msg, "%s_%d.lsd", simul_name,findex);
  struct_file=new char[strlen(msg)+1];
  strcpy(struct_file,msg);
 }
 
f=fopen(struct_file, "r");
if(f==NULL)
 {
  fprintf( stderr, "\nFile %s not found.\nThis is the no window version of Lsd. Specify a -f filename.lsd to run a simulation or -f simul_name -s 1 for batch sequential simulation mode (requires configuration files: simul_name_1.lsd, simul_name_2.lsd, etc).\n", struct_file );
  myexit( 3 );
 }
fclose(f);
struct_loaded = true;

if ( load_configuration( root, false ) != 0 )
{
	fprintf( stderr, "\nFile %s is invalid.\nThis is the no window version of Lsd. Check if the file is a valid Lsd configuration or regenerate it using the Lsd Browser.\n", struct_file );
	myexit( 4 );
}

#else 
	
for(i=1; argv[i]!=NULL; i++)
{if(argv[i][0]!='-' || (argv[i][1]!='f' && argv[i][1]!='i') )
  {
	log_tcl_error( "Command line parameters", "Invalid option, available options: -i tcl_directory / -f model_name" );
	myexit( 1 );
  }
 if(argv[i][1]=='f')
	{delete[] simul_name;
	 simul_name=new char[strlen(argv[i+1])+3];
	 strcpy(simul_name,argv[i+1]);
    len=strlen(simul_name);
    if(len>4 && !strcmp(".lsd",simul_name+len-4) )
     *(simul_name+len-4)=(char)NULL;
    i++;
	}
 if(argv[i][1]=='i')
	{
   strcpy(tcl_dir,argv[i+1]+2);
   i++;
  } 
}

// initialize the tcl interpreter
Tcl_FindExecutable( argv[0] ); 
inter = Tcl_CreateInterp( );
done = Tcl_Init( inter );
if ( done != TCL_OK )
{
	sprintf( msg, "Tcl initialization directories not found, check the Tcl/Tk installation  and configuration or reinstall Lsd\nTcl Error = %d : %s", done,  Tcl_GetStringResult( inter ) );
	log_tcl_error( "Create Tcl interpreter", msg );
	myexit( 5 );
}

// set variables and links in TCL interpreter
Tcl_LinkVar(inter, "choice", (char *) &choice, TCL_LINK_INT);
Tcl_LinkVar(inter, "debug_flag", (char *) &debug_flag, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "when_debug", (char *) &when_debug, TCL_LINK_INT);

// test Tcl interpreter
cmd( "set choice 1234567890" );
if ( choice != 1234567890 )
{
	log_tcl_error( "Test Tcl", "Tcl failed, check the Tcl/Tk installation and configuration or reinstall Lsd" );
	myexit( 6 );
}
	
// initialize & test the tk application
choice = 1;
done = Tk_Init( inter );
if ( done == TCL_OK )
	cmd( "if { ! [ catch { package present Tk 8.5 } ] && [ winfo exists . ] } { set choice 0 } { set choice 1 }" );
if ( choice )
{
	sprintf( msg, "Tk failed, check the Tcl/Tk installation (version 8.5+) and configuration or reinstall Lsd\nTcl Error = %d : %s", done,  Tcl_GetStringResult( inter ) );
	log_tcl_error( "Start Tk", msg );
	myexit( 7 );
}
tk_ok = true;
cmd( "tk appname browser" );

cmd( "if { [string first \" \" \"[pwd]\" ] >= 0  } {set choice 1} {set choice 0}" );
if ( choice )
 {
 cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"Spaces in file path\" -detail \"The directory containing the model is:\n[pwd]\nIt appears to include spaces. This will make impossible to compile and run Lsd model. The Lsd directory must be located where there are no spaces in the full path name.\nMove all the Lsd directory and delete the 'system_options.txt' file from the \\src directory.\n\nLsd is aborting now.\"" );
 log_tcl_error( "Path check", "Lsd directory path includes spaces, move all the Lsd directory in another directory without spaces in the path" );
 myexit( 8 ); 
 }

// check if LSDROOT already exists and use it if so, if not, search the current directory tree
cmd( "if [ info exists env(LSDROOT) ] { set RootLsd $env(LSDROOT); if { ! [ file exists \"$RootLsd/src/interf.cpp\" ] } { unset RootLsd } }" );
cmd( "if { ! [ info exists RootLsd ] } { set here [ pwd ]; while { ! [ file exists \"src/interf.cpp\" ] && [ string length [ pwd ] ] > 3 } { cd .. }; if [ file exists \"src/interf.cpp\" ] { set RootLsd [ pwd ] } { set RootLsd \"\" }; cd $here; set env(LSDROOT) $RootLsd }" );
str = ( char * ) Tcl_GetVar( inter, "RootLsd", 0 );
if ( str == NULL || strlen( str ) == 0 )
 {
 cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"LSDROOT not set\" -detail \"Please make sure the environment variable LSDROOT points to the directory where Lsd is installed.\n\nLsd is aborting now.\"" );
 log_tcl_error( "LSDROOT check", "LSDROOT not set, make sure the environment variable LSDROOT points to the directory where Lsd is installed" );
 myexit( 9 );
 }
lsdroot = new char[ strlen( str ) + 1 ];
strcpy( lsdroot, str );
len = strlen( lsdroot );
for ( i = 0; i < len; ++i )
	if ( lsdroot[ i ] == '\\' )
		lsdroot[ i ] = '/';
cmd( "set RootLsd \"%s\"", lsdroot );

cmd( "set choice [file exist $RootLsd/lmm_options.txt]" );
if ( choice )
 {
  cmd( "set f [open $RootLsd/lmm_options.txt r]" );
  cmd( "gets $f Terminal" );
  cmd( "gets $f HtmlBrowser" );
  cmd( "gets $f fonttype" );
  cmd( "gets $f wish" );
  cmd( "gets $f LsdSrc" );
  cmd( "close $f" );
 }
else
 { 
  cmd( "tk_messageBox -parent . -title Warning -icon warning -type ok -message \"Could not locate LMM system options\" -detail \"It may be impossible to open help files and compare the equation files. Any other functionality will work normally. When possible set in LMM the system options in menu File.\"" );
 }

choice = 0;
// load native Tcl procedures for windows management
cmd( "if [ file exists $RootLsd/$LsdSrc/align.tcl ] { if { [ catch { source $RootLsd/$LsdSrc/align.tcl } ] != 0 } { set choice [ expr $choice + 1 ] } } { set choice [ expr $choice + 2 ] }" );

// load native Tcl procedures for external files handling
cmd( "if [ file exists $RootLsd/$LsdSrc/ls2html.tcl ] { if { [ catch { source $RootLsd/$LsdSrc/ls2html.tcl } ] != 0 } { set choice [ expr $choice + 1 ] } } { set choice [ expr $choice + 2 ] }" );

if ( choice != 0 )
{
	cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"Source file(s) missing\" -detail \"Required Tcl/Tk source file(s) is(are) missing.\nCheck the installation of Lsd or reinstall Lsd if the problem persists.\n\nLsd is aborting now.\"" );
	log_tcl_error( "Source files check", "Required Tcl/Tk source file(s) is(are) missing, check the installation of Lsd or reinstall Lsd if the problem persists" );
	myexit( 200 + choice );
}

#ifdef TKCON
// launch TkCon as an auxiliary window for debugging
cmd( "if [ file exists $RootLsd/$LsdSrc/tkcon.tcl ] { if { [ catch { source $RootLsd/$LsdSrc/tkcon.tcl } ] == 0 } { package require tkcon; set tkcon::PRIV(showOnStartup) 0; set tkcon::PRIV(root) .console; set tkcon::PRIV(protocol) {tkcon hide}; set tkcon::OPT(exec) \"\"; tkcon::Init; tkcon title \"Tcl/Tk Debug Console\"; tkcon show } { tk_messageBox -parent . -type ok -icon error -title Error -message \"File 'src/tkcon.tcl' missing or corrupted\" -detail \"Lsd is continuing with no debug console.\" } }" );
#endif

// create a Tcl command that calls the C discard_change function before killing Lsd
Tcl_CreateCommand( inter, "discard_change", Tcl_discard_change, NULL, NULL );

// create Tcl commands that get and set LSD variable properties
Tcl_CreateCommand( inter, "get_var_conf", Tcl_get_var_conf, NULL, NULL );
Tcl_CreateCommand( inter, "set_var_conf", Tcl_set_var_conf, NULL, NULL );

// set main window
cmd( "wm title . \"Lsd Browser\"" );
cmd( "wm protocol . WM_DELETE_WINDOW { if [ string equal [ discard_change ] ok ] { exit } }" ); 
cmd( ". configure -menu .m" );		// define here to avoid redimensining the window
cmd( "icontop . lsd" );
cmd( "sizetop lsd" );

cmd( "label .l -text \"Starting Lsd\"" );
cmd( "pack .l" );
cmd( "update" );

create_logwindow( );

// use exec_path to change to the model directory
delete [] path;
path = new char[ strlen( exec_path ) + 1 ];
strcpy( path, exec_path );
cmd( "set path \"%s\"; cd \"$path\"", path );

struct_file = new char[ strlen( simul_name ) + 5 ];
sprintf( struct_file, "%s.lsd", simul_name );

eq_file = upload_eqfile();
strcpy( lsd_eq_file, "" );
sprintf( name_rep, "report_%s.html", simul_name );

grandTotal = true;				// not in parallel mode: use .tot headers

cmd( "destroy .l" );
#endif

stacklog = new lsdstack;
stacklog->next=NULL;
stacklog->prev=NULL;
stacklog->ns=0;
stacklog->vs=NULL;
strcpy(stacklog->label, "Lsd Simulation Manager");

#ifndef NO_WINDOW

while(1)
{
root=create( root);
no_more_memory = false;
series_saved=0;

try 
{
	run(root);
}
catch( int p )			// handle this here
{
	while(stacklog->prev!=NULL)
		stacklog=stacklog->prev;
	stack=0; 
}
catch ( ... )			// send the rest upward
{
	throw;
}
}

#else

run(root);

#endif 

empty_cemetery();
blueprint->empty();
root->empty();
delete blueprint;
delete root;
delete stacklog;
delete [ ] struct_file;
delete [ ] equation_name;
delete [ ] path;
delete [ ] simul_name;
delete [ ] lsdroot;

return 0;
}


/*********************************
RUN
*********************************/
void run(object *root)
{
int i, j, done=0;
bool batch_sequential_loop = false; // second or higher iteration of a batch
char ch[MAX_PATH_LENGTH];
FILE *f;
result *rf;					// pointer for results files (may be zipped or not)
double app=0;
double refresh=1.01;		// runtime refresh
clock_t start, end;

done_in=done=0;
quit=0;
 
#ifndef NO_WINDOW
Tcl_LinkVar(inter, "done_in", (char *) &done_in, TCL_LINK_INT);
Tcl_SetVar(inter, "done", "0", 0);

cover_browser( "Running...", "The simulation is being executed", "Use the Lsd Log window buttons to interact during execution:\n\n'Stop' :  stops the simulation\n'Pause' :  pauses and resumes the simulation\n'Fast' :  accelerates the simulation by hiding information\n'Observe' :  presents more run time information\n'Debug' :  trigger the debugger at a flagged variable" );
cmd( "wm deiconify .log; raise .log; focus .log" );

#else
plog( "\nProcessing configuration file %s ...\n", "", struct_file );
#endif

for(i=1; i<=sim_num && quit!=2; i++)
{
cur_sim = i;	 //Update the global variable holding information on the current run in the set of runs
empty_cemetery(); //ensure that previous data are not erroneously mixed (sorry Nadia!)

#ifndef NO_WINDOW
prepare_plot(root, i);
#endif

plog( "\nSimulation %d running...", "", i );

// if new batch configuration file, reload all
if ( batch_sequential_loop )
{
	if ( load_configuration( root, false ) != 0 )
	{
#ifndef NO_WINDOW 
		cmd( "tk_messageBox -parent . -type ok -icon error -title Error -message \"Configuration file cannot be loaded\" -detail \"Check if Lsd still has WRITE access to the model directory.\nLsd will close now.\"" );
		log_tcl_error( "Load configuration", "Configuration file not found or corrupted" );	
#else
		fprintf( stderr, "\nFile '%s' not found or corrupted.\n", struct_file );	
#endif
		myexit( 10 );
	}
	batch_sequential_loop = false;
}

// if just another run seed, reload just structure & parameters
if ( i > 1 )
	if ( load_configuration( root, true ) != 0 )
	{
#ifndef NO_WINDOW 
		cmd( "tk_messageBox -parent . -type ok -icon error -title Error -message \"Configuration file cannot be reloaded\" -detail \"Check if Lsd still has WRITE access to the model directory.\nLsd will close now.\"" );
		log_tcl_error( "Load configuration", "Configuration file not found or corrupted" );	
#else
		fprintf( stderr, "\nFile '%s' not found or corrupted.\n", struct_file );
#endif
		myexit( 10 );
	}

strcpy(ch, "");
series_saved=0;

print_title(root);
if ( no_more_memory )
 {
#ifndef NO_WINDOW 
 cmd( "tk_messageBox -parent . -type ok -icon error -title Error -message \"Not enough memory\" -detail \"Too many series saved for the available memory. Memory insufficient for %d series over %d time steps. Reduce series to save and/or time steps.\nLsd will close now.\"", series_saved, max_step );
 log_tcl_error( "Memory allocation", "Not enough memory, too many series saved for the memory available" );
#else
 fprintf( stderr, "\nNot enough memory. Too many series saved for the memory available.\nMemory insufficient for %d series over %d time steps.\nReduce series to save and/or time steps.\n", series_saved, max_step );
#endif
 myexit( 11 );
 }
 
//new random routine' initialization
init_random(seed);

seed++;
stack=0;

scroll = false;
pause_run = false;
running = true;
debug_flag = false;
done_in = 0;
actual_steps = 0;
start = clock();

for(t=1; quit==0 && t<=max_step;t++)
{
	
// adjust "clock" backwards if simulation is paused
if ( pause_run )
	t--;

#ifndef NO_WINDOW 
if(when_debug==t)
{
  debug_flag=true;
  cmd( "if [ winfo exists .deb ] { wm deiconify .deb; raise .deb; focus -force .deb; update idletasks }" );
}
#endif

cur_plt=0;

// only update if simulation not paused
if ( ! pause_run )
	root->update();

#ifndef NO_WINDOW 
switch( done_in )
{
case 0:
 if ( ! pause_run && ! fast )
	if ( cur_plt == 0 )
		plog( "\nSim. %d step %d done", "", i, t );
break;

case 1:			// Stop button in Log window / s/S key in Runtime window
  if ( pause_run )
	cmd( "wm title .log \"$origLogTit\"" );
  plog( "\nSimulation stopped at t = %d", "", t );
  quit=2;
break;

case 2:			// Fast button in Log window / f/F key in Runtime window
 fast = true;
 debug_flag=false;
 cmd( "set a [split [winfo children .] ]" );
 cmd( " foreach i $a {if [string match .plt* $i] {wm withdraw $i}}" );
 cmd( "if { [winfo exist .plt%d]} {.plt%d.c.yscale.go conf -state disabled} {}", i, i );
 cmd( "if { [winfo exist .plt%d]} {.plt%d.c.yscale.shift conf -state disabled} {}", i, i );
break;

case 3:			// Debug button in Log window / d/D key in Runtime window
if ( ! pause_run )
{
	debug_flag=true;
	cmd( "if [ winfo exists .deb ] { wm deiconify .deb; raise .deb; focus -force .deb }" );
}
else			// if paused, just call the data browser
{
	double useless = 0;
	deb( root, NULL, "Paused by User", &useless );
}
	
break;

case 4:			// Observe button in Log window / o/O key in Runtime window
 fast = false;
 cmd( "set a [split [winfo children .] ]" );
 cmd( " foreach i $a {if [string match .plt* $i] {wm deiconify $i; raise $i}}" );
 cmd( "if { [winfo exist .plt%d]} {.plt%d.c.yscale.go conf -state normal} {}", i, i );
 cmd( "if { [winfo exist .plt%d]} {.plt%d.c.yscale.shift conf -state normal} {}", i, i );
break;
 
// plot window DELETE_WINDOW button handler
case 5:
 if ( pause_run )
	cmd( "wm title .log \"$origLogTit\"" );
 cmd( "if { [winfo exist .plt%d]} {destroytop .plt%d} {}", i, i );
 plog( "\nSimulation stopped at t = %d", "", t );
 quit=2;
break;

case 6:
 Tcl_LinkVar(inter, "app_refresh", (char *) &app, TCL_LINK_DOUBLE);
 cmd( "set app_refresh $refresh" );
 Tcl_UnlinkVar(inter, "app_refresh");
 if(app<2 && app > 1)
  refresh=app;
break; 

// runtime plot events
case 7:  		// Center button
 cmd( "set newpos [expr %lf - [expr 250 / %lf]]", (double)t/(double)max_step, (double)max_step );
 cmd( "if { [winfo exist .plt%d]} {$activeplot.c.c.cn xview moveto $newpos} {}", i );
break;

case 8: 		// Scroll checkbox
 scroll = ! scroll;
break;

case 9: 		// Pause simulation
 pause_run = ! pause_run;
 if ( pause_run )
 {
	cmd( "set origLogTit [ wm title .log ]; wm title .log \"$origLogTit (PAUSED)\"" );
	plog( "\nSimulation paused at t = %d", "", t );
	cmd( ".log.but.pause conf -text Resume" );
 }
 else
 {
	cmd( "wm title .log \"$origLogTit\"" );
	plog( "\nSimulation resumed" );
	cmd( ".log.but.pause conf -text Pause" );
 }
break;

case 35:
 cmd( "tk_messageBox -parent . -type ok -icon error -title Error -message \"Unexpected termination\" -detail \"Please try again.\"" );
 log_tcl_error( "Unexpected termination", "Please try again" );
 myexit( 12 );
break;

default:
break;
}

// perform scrolling if enabled
if ( ! pause_run && scroll )
{
	cmd( "if { [winfo exist .plt%d]} {$activeplot.c.c.cn xview scroll 1 units} {}", i );
}

done_in = 0;
cmd( "update" );
#endif
}//end of for t

actual_steps=t-1;
unsavedData = true;						// flag unsaved simulation results
running = false;
end=clock();

if(quit==1) 			//For multiple simulation runs you need to reset quit
 quit=0;

plog( "\nSimulation %d finished (%2g sec.)\n", "",i,(float)(end - start) /CLOCKS_PER_SEC);

#ifndef NO_WINDOW 
cmd( "update" );
// allow for run time plot window destruction
cmd( "if [ winfo exists .plt%d ] { wm protocol .plt%d WM_DELETE_WINDOW \"\"; .plt%d.c.yscale.go conf -state disabled; .plt%d.c.yscale.shift conf -state disabled }", i, i, i, i  );
#endif

close_sim();
reset_end(root);
root->emptyturbo();

if ( sim_num > 1 || no_window ) //Save results for multiple simulation runs
{

// remove existing path, if any, from name in case of alternative output path
char *alt_name = clean_file( simul_name );

if ( ! no_res )
{
if ( ! batch_sequential )
  sprintf( msg, "%s%s%s_%d.res", save_alt_path ? alt_path : path, strlen( save_alt_path ? alt_path : path ) > 0 ? "/" : "", save_alt_path ? alt_name : simul_name, seed - 1 );
else
  sprintf( msg, "%s%s%s_%d_%d.res", save_alt_path ? alt_path : path, strlen( save_alt_path ? alt_path : path ) > 0 ? "/" : "", save_alt_path ? alt_name : simul_name, findex, seed - 1 );

sprintf( ch, "Saving results in file %s%s... ", msg, dozip ? ".gz" : "" );
plog ( ch );

rf = new result( msg, "wt", dozip );			// create results file object
rf->title( root, 1 );							// write header
rf->data( root, 0, actual_steps );				// write all data
delete rf;										// close file and delete object

plog("Done\n");
}

if ( ! grandTotal || batch_sequential )			// generate partial total files?
{
	if ( ! batch_sequential )
	  sprintf( msg, "%s%s%s_%d_%d.tot", save_alt_path ? alt_path : path, strlen( save_alt_path ? alt_path : path ) > 0 ? "/" : "", save_alt_path ? alt_name : simul_name, seed - i, seed - 1 + sim_num - i );
	else
	  sprintf( msg, "%s%s%s_%d_%d_%d.tot", save_alt_path ? alt_path : path, strlen( save_alt_path ? alt_path : path ) > 0 ? "/" : "", save_alt_path ? alt_name : simul_name, findex, seed - i, seed - 1 + sim_num - i );
}
else											// generate single grand total file
{
	sprintf( msg, "%s%s%s.tot", save_alt_path ? alt_path : path, strlen( save_alt_path ? alt_path : path ) > 0 ? "/" : "", save_alt_path ? alt_name : simul_name );
}

if ( i == sim_num )								// print only for last
	plog( "\nSaving totals in file %s%s... ", "", msg, dozip ? ".gz" : "" );

if ( ( i == 1 && ! add_to_tot ) || ( grandTotal && firstRes ) )
{
	rf = new result( msg, "wt", dozip );		// create results file object
	rf->title( root, 0 );						// write header
	firstRes = false;
}
else
	rf = new result( msg, "a", dozip );			// add results object to existing file

rf->data( root, actual_steps );					// write current data data
delete rf;										// close file and delete object

if ( i == sim_num )								// print only for last
	plog( "Done\n" );

if ( batch_sequential && i == sim_num)  		// last run of current batch file?
 {
   findex++;									// try next file
   sprintf(msg, "%s_%d.lsd",simul_name,findex);
   delete[] struct_file;
   struct_file=new char[strlen(msg)+1];
   strcpy(struct_file,msg);
   f=fopen(struct_file, "r");			
   if(f==NULL || (fend!=0 && findex>fend))  	// no more file to process
   {
	 if(f!=NULL) 
		 fclose(f);
     plog( "\nFinished processing %s.\n", "", simul_name );
     break;
   }
   plog( "\nProcessing configuration file %s ...\n", "", struct_file );
   fclose(f);  									// process next file
   struct_loaded = true;
   i = 0;   									// force restarting run count
   batch_sequential_loop = true;				// force reloading configuration
 } 
}
}

#ifndef NO_WINDOW 
uncover_browser( );
cmd( "wm deiconify .log; raise .log; focus .log" );
Tcl_UnlinkVar(inter, "done_in");
#endif
quit=0;
}


/*********************************
PRINT_TITLE
*********************************/

void print_title(object *root)
{
object *c, *cur;
variable *var;
int num=0, multi, toquit;
bridge *cb;


toquit=quit;
//for each variable set the data saving support
for(var=root->v; var!=NULL; var=var->next)
 {
 var->last_update=0;

	if( (var->save || var->savei) && ! no_more_memory )
	 {
     if(var->num_lag>0 || var->param==1)
       var->start=0;
     else
       var->start=1;
     var->end=max_step;
     if(var->data!=NULL)
      delete[] var->data;
  
     try 
	 {
      var->data=new double[max_step+1];
     }
     catch( std::bad_alloc& ) 
	 {
	 	set_lab_tit(var);
         plog( "\nNot enough memory.\nData for %s and subsequent series will not be saved.\n", "", var->lab_tit );
         var->save=0;
         no_more_memory = true;
	 	throw;
     }
   
     series_saved++;
     if(var->num_lag>0  || var->param==1)
      var->data[0]=var->val[0];
    }
   else
    {
     if ( no_more_memory )
      var->save=0;
    }
	if(var->data_loaded=='-')
	  {
		plog( "\nData for %s in object %s not loaded\n", "", var->label, root->label );
		plog( "Use the Initial Values editor to set its values\n" );
     #ifndef NO_WINDOW   
     if(var->param==1)
       cmd( "tk_messageBox -parent . -type ok -icon error -title Error -message \"Run aborted\" -detail \"The simulation cannot start because parameter:\n'%s' (object '%s')\nhas not been initialized.\nUse the browser to show object '%s' and choose menu 'Data'/'Initìal Values'.\"", var->label, root->label, root->label );
     else
       cmd( "tk_messageBox -parent . -type ok -icon error -title Error -message \"Run aborted\" -detail \"The simulation cannot start because a lagged value for variable:\n'%s' (object '%s')\nhas not been initialized.\nUse the browser to show object '%s' and choose menu 'Data'/'Init.Values'.\"", var->label, root->label, root->label );  
     #endif
		toquit=2;
	  }
 }

for(cb=root->b; cb!=NULL; cb=cb->next)
 {for(cur=cb->head; cur!=NULL && quit!=2; cur=go_brother(cur))
   {
   print_title(cur);

   }
  }
if(quit!=2)
 quit=toquit;
}


/*********************************
PLOG
has some problems, because the log window tends to interpret the message
as tcl/tk commands
The optional tag parameter has to correspond to the log window existing tags
*********************************/
#define NUM_TAGS 4
const char *tags[ NUM_TAGS ] = { "", "highlight", "tabel", "series" };

void plog( char const *cm, char const *tag, ... )
{
	char buffer[ TCL_BUFF_STR ];
	va_list argptr;
	
	va_start( argptr, tag );
	int maxSz = TCL_BUFF_STR - 40 - strlen( tag );
	int reqSz = vsnprintf( buffer, maxSz, cm, argptr );
	va_end( argptr );
	
	if ( reqSz >= maxSz )
		plog( "\nWarning: message truncated\n" );

#ifdef NO_WINDOW 
	printf( "%s", buffer );
	fflush( stdout );
#else
	bool ok = false;

	for ( int i = 0; i < NUM_TAGS; ++i )
		if ( ! strcmp( tag, tags[ i ] ) )
			ok = true;
	
	if ( ok )
	{
		cmd( ".log.text.text.internal insert end \"%s\" %s", buffer, tag );
		cmd( ".log.text.text.internal see end" );
		cmd( "update" );
	}
	else
		plog( "\nError: invalid tag, message ignored:\n%s\n", "", buffer );
#endif 
	message_logged = true;
}


void reset_end(object *r)
{
object *cur;
variable *cv;
bridge *cb;

for(cv=r->v; cv!=NULL; cv=cv->next)
  { if(cv->save)
      {
       cv->end=t-1;
      } 
   if(cv->savei==1)
       save_single(cv);

  } 

for(cb=r->b; cb!=NULL; cb=cb->next)
 {cur=cb->head;
 if(cur->to_compute==1)   
   {
   for(; cur!=NULL;cur=go_brother(cur) )
     reset_end(cur);
   }  
 }
}


/*********************************
CREATE_LOG_WINDOW
*********************************/

#ifndef NO_WINDOW

void create_logwindow(void)
{
cmd( "newtop .log \"Lsd Log\" { if { [ discard_change ] == \"ok\" } { exit } { } } \"\"" );

cmd( "set w .log.text" );
cmd( "frame $w" );
cmd( "scrollbar $w.scroll -command \"$w.text yview\"" );
cmd( "scrollbar $w.scrollx -command \"$w.text xview\" -orient hor" );
cmd( "text $w.text -relief sunken -yscrollcommand \"$w.scroll set\" -xscrollcommand \"$w.scrollx set\" -wrap none" );
cmd( "$w.text configure -tabs {%s}", tabs  );

// Log window tags
cmd( "$w.text tag configure highlight -foreground red" );
cmd( "$w.text tag configure tabel" );
cmd( "$w.text tag configure series -tabs {2c 5c 8c}" );

cmd( "pack $w.scroll -side right -fill y" );
cmd( "pack $w.text -expand yes -fill both" );
cmd( "pack $w.scrollx -side bottom -fill x" );
cmd( "pack $w -expand yes -fill both" );

cmd( "set w .log.but" );
cmd( "frame $w" );
cmd( "button $w.stop -width -9 -text Stop -command {set done_in 1} -underline 0" );
cmd( "button $w.pause -width -9 -text Pause -command {set done_in 9} -underline 0" );
cmd( "button $w.speed -width -9 -text Fast -command {set done_in 2} -underline 0" );
cmd( "button $w.obs -width -9 -text Observe -command {set done_in 4} -underline 0" );
cmd( "button $w.deb -width -9 -text Debug -command {set done_in 3} -underline 0" );
cmd( "button $w.help -width -9 -text Help -command {LsdHelp Log.html} -underline 0" );
cmd( "button $w.copy -width -9 -text Copy -command {tk_textCopy .log.text.text} -underline 0" );

cmd( "pack $w.stop $w.pause $w.speed $w.obs $w.deb $w.copy $w.help -padx 10 -pady 10 -side left" );
cmd( "pack $w -side right" );

cmd( "sizetop log" );
cmd( "showtop .log current 1 1 0" );
set_shortcuts_log( ".log" );

// replace text widget default insert, delete and replace bindings, preventing the user to change it
cmd( "rename .log.text.text .log.text.text.internal" );
cmd( "proc .log.text.text { args } { switch -exact -- [lindex $args 0] { insert { } delete { } replace { } default { return [ eval .log.text.text.internal $args] } } }" );

// a Tcl/Tk version of plog
cmd( "proc plog cm { .log.text.text.internal insert end $cm; .log.text.text.internal see end }" );
}

void set_shortcuts_log( const char *window )
{
	cmd( "bind %s <KeyPress-s> {.log.but.stop invoke}; bind %s <KeyPress-S> {.log.but.stop invoke}", window, window );
	cmd( "bind %s <KeyPress-p> {.log.but.pause invoke}; bind %s <KeyPress-P> {.log.but.pause invoke}", window, window );
	cmd( "bind %s <KeyPress-r> {.log.but.pause invoke}; bind %s <KeyPress-R> {.log.but.pause invoke}", window, window );
	cmd( "bind %s <KeyPress-f> {.log.but.speed invoke}; bind %s <KeyPress-F> {.log.but.speed invoke}", window, window );
	cmd( "bind %s <KeyPress-o> {.log.but.obs invoke}; bind %s <KeyPress-O> {.log.but.obs invoke}", window, window );
	cmd( "bind %s <KeyPress-d> {.log.but.deb invoke}; bind %s <KeyPress-D> {.log.but.deb invoke}", window, window );
	cmd( "bind %s <KeyPress-h> {.log.but.help invoke}; bind %s <KeyPress-H> {.log.but.help invoke}", window, window );
	cmd( "bind %s <KeyPress-c> {.log.but.copy invoke}; bind %s <KeyPress-C> {.log.but.copy invoke}", window, window );
	cmd( "bind %s <Control-c> {.log.but.copy invoke}; bind %s <Control-C> {.log.but.copy invoke}", window, window );
	cmd( "bind %s <KeyPress-Escape> {focus -force .}", window );
}


/*********************************
COVER_BROWSER
*********************************/
bool brCovered = false;

void cover_browser( const char *text1, const char *text2, const char *text3 )
{
	if ( brCovered )		// ignore if already covered
		return;
		
	cmd( "if [ winfo exists .str ] { wm withdraw .str }" );
	cmd( "disable_window \"\" m bbar l" );		// disable main window
	cmd( "set origMainTit [ wm title . ]; wm title . \"$origMainTit (DISABLED)\"" );
	cmd( "newtop .t [ wm title . ]" );
	cmd( "label .t.l1 -font {-weight bold} -text \"%s\"", text1  );
	cmd( "label .t.l2 -text \"\n%s\"", text2  );
	cmd( "label .t.l3 -fg red -text \"\nInteraction with the Lsd Browser is now disabled\"" );
	cmd( "label .t.l4 -justify left -text \"\n%s\"", text3  );
	cmd( "pack .t.l1 .t.l2 .t.l3 .t.l4 -expand yes -fill y" );
	cmd( "showtop .t coverW no no no" );
	cmd( "update" );
	
	brCovered = true;
}


/*********************************
UNCOVER_BROWSER
*********************************/

void uncover_browser( void )
{
	if ( ! brCovered || running )	// ignore if not covered or running
		return;

	cmd( "if [ winfo exist .t ] { destroytop .t }" );
	cmd( "wm title . $origMainTit" );
	cmd( "enable_window \"\" m bbar l" );	// enable main window
	cmd( "if { [ string equal [ wm state . ] normal ] && [ winfo exist .str ] && ! [ string equal [ wm state .str ] normal ] } { wm deiconify .str; lower .str }" );
	cmd( "update" );
	
	brCovered = false;
}
#endif


/*********************************
RESULTS_ALT_PATH
*********************************/
//Simple tool to allow changing where results are saved.
void results_alt_path( const char *altPath )
{
	if ( save_alt_path )
		delete [ ] alt_path;

	if ( strlen( altPath ) == 0 )
	{
		save_alt_path = false;
		return;
	}
	  
	alt_path = new char[ strlen( altPath ) + 1 ];
	if ( sprintf( alt_path, "%s", altPath ) > 0 )
	{
		int lstChr = strlen( alt_path ) - 1;
		if ( alt_path[ lstChr ] == '\\' || alt_path[ lstChr ] == '/' )
			alt_path[ lstChr ] = '\0';
		
		struct stat sb;
		if ( stat( alt_path, &sb ) == 0 && S_ISDIR( sb.st_mode ) )
		{
			save_alt_path = true;
			return;
		}
	}
	
	delete [ ] alt_path;
	save_alt_path = false;
	plog( "\nWarning: could not open directory '%s', ignoring '-o' option.\n", "", altPath );
}


/*********************************
SAVE_SINGLE
*********************************/

void save_single(variable *vcv)
{
FILE *f;
int i;

set_lab_tit(vcv);
sprintf(msg, "%s_%s-%d_%d_seed-%d.res", vcv->label, vcv->lab_tit, vcv->start,vcv->end,seed-1);
f=fopen(msg, "wt");  // use text mode for Windows better compatibility

fprintf(f, "%s %s (%d %d)\t\n",vcv->label, vcv->lab_tit, vcv->start, vcv->end);

for(i=0; i<=t-1; i++)
 {
  if(i>=vcv->start && i <=vcv->end && !is_nan(vcv->data[i]))		// save NaN as n/a
    fprintf(f,"%lf\t\n",vcv->data[i]);
  else
    fprintf(f,"%s\t\n", nonavail);
  }
  
fclose(f); 
}


/*********************************
CLEAN_FILE
*********************************/
// remove any path prefixes to filename, if present
char *clean_file(char *filename)
{
	if(strchr(filename, '/') != NULL)
		return strrchr(filename, '/') + 1;
	if(strchr(filename, '\\') != NULL)
		return strrchr(filename, '\\') + 1;
	return filename;
}


/*********************************
CLEAN_PATH
*********************************/
// remove cygwin path prefix, if present, and replace \ with /
char *clean_path(char *filepath)
{
	int i, len=strlen("/cygdrive/");
	if(!strncmp(filepath, "/cygdrive/", len))
	{
		char *temp=new char[strlen(filepath) + 1];
		temp[0]=toupper(filepath[len]);				// copy drive letter
		temp[1]=':';								// insert ':'
		strcpy(temp + 2, filepath + len + 1);		// copy removing prefix
		strcpy(filepath, temp);
		delete[] temp;
	}
	
	len=strlen(filepath);
	for(i=0; i<len; i++)
		if(filepath[i]=='\\')	// replace \ with /
			filepath[i]='/';
			
	return filepath;
}
