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

/* 
Reached case 40
*/

/****************************************************
ANALYSIS.CPP contains the routines to manage the data analysis module.


The functions contained here are:

- void analysis(int *choice)
Makes an initialization and then there is the main cycle in
read_data

- void read_data(int *choice);                            
Builds the managemen window, setting all the bindings, and enters in a cycle
from which can make 8 choices:

1) Plot the plot as indicated
2) Exit from data analysis
3) Brings in foreground the plot on which title it was double clicked
4) Load a new data file
5) Sort the labels of the variables
6) Copy the selection from the list of variables to the list of the variables
to plot
7) delete the variables selected in the list of the variables to plot
9) plot the cross-section plot

- void init_canvas(void);
Simply sets the colors for any integer number

- void plot(int *choice);
Plot the plot with the variable indicated in the list of the variables
to plot. If the option says to use the automatic y scaling, it makes first
a scan of the file to spot the maximum and minimum y, then it plots. Otherwise,
it uses directly the values set by the users.

- void plot_cross(void);
Plot a plot of cross section data. The variables chosen are plotted along the times
steps chosen and, on request, they are sorted (descending or ascending) one one of
time steps. To choose which time to sort on, double-click on the time step.

- void set_cs_data(int *choice);
Interface used to determine the time steps to plot in the cross-section plots
and the possible sorting.

- void sort_cs_asc(char **s, double **v, int nv, int nt, int c);
Sort in ascending order the variables. Used in plot_cross.

- void sort_cs_desc(char **s, double **v, int nv, int nt, int c);

Sort in descending order the variables. Used in plot_cross.

Other functions used here:
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


- void plog(char *m);
print  message string m in the Log screen. It is in LSDMAIN.CPP

- void myexit(int v);
Exit function, which is customized on the operative system.

****************************************************/

#include "decl.h"

#define PI 3.141592654

bool allblack = false;
bool autom = true;
bool autom_x = true;
bool grid = false;
bool logs = false;		// log scale flag for the y-axis
bool time_cross = false;
bool xy = false;
bool watch = true;
char filename[MAX_PATH_LENGTH];
double maxy;
double miny;
double point_size = 1.0;
double truemaxy;
int cur_plot;
int file_counter=0;
int gnu = 1;			// don't use gnuplot for plots
int line_point = 1;
int max_c;
int min_c;
int num_c;
int num_var;
int pdigits = 4;   		// precision parameter for labels in y scale
int plot_l[MAX_PLOTS];
int plot_nl[MAX_PLOTS];
int res, dir;
int type_plot[MAX_PLOTS];
store *vs;


/***************************************************
ANALYSIS
****************************************************/

void analysis(int *choice)
{
char *app;

cmd( "set a [info vars c0]" );
cmd( "if {$a==\"\" } {set choice 0} {set choice 1}" );
if(*choice==0)
 init_canvas();

*choice=0;

while(*choice==0)
{read_data(choice); //Cases and Variables

}
*choice=0;
}


/***************************************************
READ_DATA
****************************************************/
void read_data(int *choice)
{
FILE *f;
int rot, i, h, j, k, l, m, n, p, q, r;
store *app_store;
char *app, *app1, *app2, dirname[MAX_PATH_LENGTH], str1[MAX_ELEM_LENGTH], str2[MAX_ELEM_LENGTH], str3[MAX_ELEM_LENGTH];
double *datum, compvalue=0;

cur_plot=0;
file_counter=0;
*choice=0;

cmd( "set daCwidth 36" );				     	// lists width (even number)

cmd( "if { ! [ info exists gpterm ] } { set gpooptions \"set ticslevel 0.0\"; set gpdgrid3d \"60,60,3\"; if { [ string equal $tcl_platform(platform) windows ] } { set gpterm windows } { set gpterm x11 } }" );

Tcl_LinkVar(inter, "cur_plot", (char *) &cur_plot, TCL_LINK_INT);

cover_browser( "Analysis of Results...", "Analysis of Results window is open", "Please exit Analysis of Results\nbefore using the Lsd Browser." );

cmd( "newtop .da \"%s%s - Lsd Analysis of Results\" { set choice 2 } \"\"", unsaved_change() ? "*" : " ", simul_name );

cmd( "menu .da.m -tearoff 0 -relief groove -bd 2" );

cmd( "set w .da.m.exit" );
cmd( ".da.m add cascade -label Exit -menu $w -underline 0" );
cmd( "menu $w -tearoff 0 -relief groove -bd 2" );
cmd( "$w add command -label \"Quit and return to Browser\" -command {set choice 2} -underline 0 -accelerator Esc" );

cmd( "set w .da.m.gp" );
cmd( ".da.m add cascade -label Gnuplot -menu $w -underline 0" );
cmd( "menu $w -tearoff 0 -relief groove -bd 2" );
cmd( "$w add command -label \"Gnuplot...\" -command {set choice 4} -underline 0 -accelerator Ctrl+G" );
cmd( "$w add command -label \"Gnuplot Options...\" -command {set choice 37} -underline 8" );

cmd( "set w .da.m.color" );
cmd( ".da.m add cascade -label Color -menu $w -underline 0" );
cmd( "menu $w -tearoff 0 -relief groove -bd 2" );
cmd( "$w add command -label \"Set Colors...\" -command {set choice 21} -underline 0" );
cmd( "$w add command -label \"Set Default Colors\" -command {set choice 22} -underline 0" );

cmd( "set w .da.m.help" );
cmd( "menu $w -tearoff 0 -relief groove -bd 2" );
cmd( ".da.m add cascade -label Help -menu $w -underline 0" );
cmd( "$w add command -label \"Help on Analysis of Result\" -command {set choice 41} -underline 0" );
cmd( "$w add command -label \"Model Report\" -command {set choice 44} -underline 0" );
cmd( "$w add separator" );
cmd( "$w add command -label \"About Lsd...\" -command { tk_messageBox -parent .da -type ok -icon info -title \"About Lsd\" -message \"Version %s (%s)\" -detail \"Platform: [ string totitle $tcl_platform(platform) ] ($tcl_platform(machine))\nOS: $tcl_platform(os) ($tcl_platform(osVersion))\nTcl/Tk: [ info patch ]\" } -underline 0", _LSD_VERSION_, _LSD_DATE_  ); 

cmd( ".da configure -menu .da.m" );

cmd( "set f .da.head" );
cmd( "frame $f" );
cmd( "label $f.lb -width [ expr $daCwidth - 3 ] -text \"Series Available\"" );
cmd( "label $f.pad -width 6" );
cmd( "label $f.ch -width [ expr $daCwidth - 3 ] -text \"Series Selected\"" );
cmd( "label $f.pl -width [ expr $daCwidth - 3 ] -text \"Plots\"" );
cmd( "pack $f.lb $f.pad $f.ch $f.pl -side left" );
cmd( "pack $f" );

cmd( "frame .da.vars" );

cmd( "set f .da.vars.lb" );
cmd( "frame $f" );
cmd( "scrollbar $f.v_scroll -command \"$f.v yview\"" );
cmd( "listbox $f.v -selectmode extended -width $daCwidth -yscroll \"$f.v_scroll set\"" );
cmd( "pack $f.v $f.v_scroll -side left -fill y" );

cmd( "bind $f.v <Return> {.da.vars.b.in invoke}" );
cmd( "bind $f.v <Double-Button-1> { event generate .da.vars.lb.v <Return> }" );
cmd( "bind $f.v <Button-2> {.da.vars.lb.v selection clear 0 end;.da.vars.lb.v selection set @%%x,%%y; set res [selection get]; set choice 30}" );
cmd( "bind $f.v <Button-3> { event generate .da.vars.lb.v <Button-2> -x %%x -y %%y }" );
cmd( "bind $f.v <KeyPress-space> {set res [.da.vars.lb.v get active]; set choice 30}; bind $f.v <KeyPress-space> {set res [.da.vars.lb.v get active]; set choice 30}" );

cmd( "bind $f.v <Shift-Button-2> {.da.vars.lb.v selection clear 0 end;.da.vars.lb.v selection set @%%x,%%y; set res [selection get]; set choice 16}" );
cmd( "bind $f.v <Shift-Button-3> { event generate .da.vars.lb.v <Shift-Button-2> -x %%x -y %%y }" );

cmd( "set f .da.vars.ch" );
cmd( "frame $f" );
cmd( "scrollbar $f.v_scroll -command \"$f.v yview\"" );
cmd( "listbox $f.v -selectmode extended -width $daCwidth -yscroll \"$f.v_scroll set\"" );
cmd( "pack $f.v $f.v_scroll -side left -fill y" );

cmd( "bind $f.v <KeyPress-o> {.da.vars.b.out invoke}; bind $f.v <KeyPress-O> {.da.vars.b.out invoke}" );
cmd( "bind $f.v <Return> {.da.b.ts invoke}" );
cmd( "bind $f.v <Double-Button-1> { event generate .da.vars.ch.v <Return> }" );
cmd( "bind $f.v <Button-2> {.da.vars.ch.v selection clear 0 end;.da.vars.ch.v selection set @%%x,%%y; set res [selection get]; set choice 33}" );
cmd( "bind $f.v <Button-3> { event generate .da.vars.ch.v <Button-2> -x %%x -y %%y }" );
cmd( "bind $f.v <KeyPress-space> {set res [.da.vars.ch.v get active]; set choice 33}; bind $f.v <KeyPress-space> {set res [.da.vars.ch.v get active]; set choice 33}" );

cmd( "set f .da.vars.pl" );
cmd( "frame $f" );
cmd( "scrollbar $f.v_scroll -command \"$f.v yview\"" );
cmd( "listbox $f.v -width $daCwidth -yscroll \"$f.v_scroll set\" -selectmode single" );
cmd( "pack $f.v $f.v_scroll -side left -fill y" );

cmd( "bind $f.v <Return> {set it [selection get];set choice 3}" );
cmd( "bind $f.v <Double-Button-1> { event generate .da.vars.pl.v <Return> }" );
cmd( "bind $f.v <Button-2> {.da.vars.pl.v selection clear 0 end; .da.vars.pl.v selection set @%%x,%%y; set it [selection get]; set n_it [.da.vars.pl.v curselection]; set choice 20}" );
cmd( "bind $f.v <Button-3> { event generate .da.vars.pl.v <Button-2> -x %%x -y %%y }" );
cmd( "bind .da <KeyPress-Delete> {set n_it [.da.vars.pl.v curselection]; if {$n_it != \"\" } {set it [selection get]; set choice 20} {}}" );

cmd( "frame .da.vars.b" );
cmd( "set f .da.vars.b" );
cmd( "button $f.in -width -6 -relief flat -overrelief groove -text \u25b6 -command {set choice 6}" );
cmd( "button $f.out -width -6 -relief flat -overrelief groove -state disabled -text \u25c0 -command {set choice 7}" );
cmd( "button $f.sort -width -6 -relief flat -overrelief groove -text \"Sort \u25b2\" -command {set choice 5} -underline 0" );
cmd( "button $f.sortdesc -width -6 -relief flat -overrelief groove -text \"Sort \u25bc\" -command {set choice 38} -underline 1" );
cmd( "button $f.sortend -width -6 -relief flat -overrelief groove -text \"Sort+\" -command {set choice 15} -underline 2" );
cmd( "button $f.unsort -width -6 -relief flat -overrelief groove -text \"Unsort\" -command {set choice 14} -underline 0" );
cmd( "button $f.search -width -6 -relief flat -overrelief groove -text Find... -command { set choice 39 } -underline 0" );
cmd( "button $f.add -width -6 -relief flat -overrelief groove -text \"Add...\" -command {set choice 24} -underline 0" );
cmd( "button $f.empty -width -6 -relief flat -overrelief groove -text Clear -command {set choice 8} -underline 0" );
cmd( "pack $f.in $f.out $f.sort $f.sortdesc $f.sortend $f.unsort $f.search $f.add $f.empty -padx 2 -pady 1 -fill y" );

cmd( "pack .da.vars.lb .da.vars.b .da.vars.ch .da.vars.pl -side left  -expand true -fill y" );
cmd( "pack .da.vars -expand true -fill y" );

cmd( "frame .da.com" );
cmd( "label .da.com.nvar -text \"Series = %d\" -width [ expr [ expr $daCwidth - 4 ] / 2 ]", num_var  );
cmd( "label .da.com.ncas -text \"Cases = %d\" -width [ expr [ expr $daCwidth - 4 ] / 2 ]", num_c  );
cmd( "label .da.com.pad -width 6" );
cmd( "label .da.com.selec -text \"Series = [ .da.vars.ch.v size ]\" -width [ expr $daCwidth - 3 ]" );
cmd( "label .da.com.plot -text \"Plots = [ .da.vars.pl.v size ]\" -width [ expr $daCwidth - 3 ]" );
cmd( "pack .da.com.nvar .da.com.ncas .da.com.pad .da.com.selec .da.com.plot -side left" );

num_c=1;
num_var=0;
if(actual_steps>0)
  insert_data_mem(root, &num_var, &num_c);

Tcl_LinkVar(inter, "auto", (char *) &autom, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "auto_x", (char *) &autom_x, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "minc", (char *) &min_c, TCL_LINK_INT);
Tcl_LinkVar(inter, "maxc", (char *) &max_c, TCL_LINK_INT);
Tcl_LinkVar(inter, "miny", (char *) &miny, TCL_LINK_DOUBLE);
Tcl_LinkVar(inter, "maxy", (char *) &maxy, TCL_LINK_DOUBLE);
Tcl_LinkVar(inter, "logs", (char *) &logs, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "allblack", (char *) &allblack, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "grid", (char *) &grid, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "point_size", (char *) &point_size, TCL_LINK_DOUBLE);
Tcl_LinkVar(inter, "tc", (char *) &time_cross, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "line_point", (char *) &line_point, TCL_LINK_INT);
Tcl_LinkVar(inter, "xy", (char *) &xy, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "pdigits", (char *) &pdigits, TCL_LINK_INT);
Tcl_LinkVar(inter, "watch", (char *) &watch, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "gnu", (char *) &gnu, TCL_LINK_INT);

min_c=1;
max_c=num_c;
miny=maxy=0;

cmd( "set numy2 2" );

cmd( "frame .da.f" );
cmd( "frame .da.f.h" );
cmd( "frame .da.f.h.v" );

cmd( "frame .da.f.h.v.ft" );

cmd( "checkbutton .da.f.h.v.ft.auto -text \"Use all cases \" -variable auto_x -command {if {$auto_x==1} {.da.f.h.v.ft.to.mxc conf -state disabled; .da.f.h.v.ft.from.mnc conf -state disabled} {.da.f.h.v.ft.to.mxc conf -state normal; .da.f.h.v.ft.from.mnc conf -state normal}}" );

cmd( "frame .da.f.h.v.ft.from" );
cmd( "label .da.f.h.v.ft.from.minc -text \"From case\"" );
cmd( "entry .da.f.h.v.ft.from.mnc -width 5 -validate focusout -vcmd { if [ string is integer %%P ] { set minc %%P; return 1 } { %%W delete 0 end; %%W insert 0 $minc; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "pack .da.f.h.v.ft.from.minc .da.f.h.v.ft.from.mnc -side left" );

cmd( "frame .da.f.h.v.ft.to" );
cmd( "label .da.f.h.v.ft.to.maxc -text \"to case\"" );
cmd( "entry .da.f.h.v.ft.to.mxc -width 5 -validate focusout -vcmd { if [ string is integer %%P ] { set maxc %%P; return 1 } { %%W delete 0 end; %%W insert 0 $maxc; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "pack  .da.f.h.v.ft.to.maxc .da.f.h.v.ft.to.mxc -side left" );

cmd( "pack .da.f.h.v.ft.auto .da.f.h.v.ft.from .da.f.h.v.ft.to -side left -ipadx 15" );

cmd( "frame .da.f.h.v.sc" );

cmd( "checkbutton .da.f.h.v.sc.auto -text \"Y self-scaling\" -variable auto -command {if {$auto==1} {.da.f.h.v.sc.max.max conf -state disabled; .da.f.h.v.sc.min.min conf -state disabled} {.da.f.h.v.sc.max.max conf -state normal; .da.f.h.v.sc.min.min conf -state normal}}" );

cmd( "frame .da.f.h.v.sc.min" );
cmd( "label .da.f.h.v.sc.min.lmin -text \"Min. Y\"" );
cmd( "entry .da.f.h.v.sc.min.min -width 10 -validate focusout -vcmd { if [ string is double %%P ] { set miny %%P; return 1 } { %%W delete 0 end; %%W insert 0 $miny; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "pack .da.f.h.v.sc.min.lmin .da.f.h.v.sc.min.min -side left" );

cmd( "frame .da.f.h.v.sc.max" );
cmd( "label .da.f.h.v.sc.max.lmax -text \"Max. Y\"" );
cmd( "entry .da.f.h.v.sc.max.max -width 10 -validate focusout -vcmd { if [ string is double %%P ] { set maxy %%P; return 1 } { %%W delete 0 end; %%W insert 0 $maxy; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "pack .da.f.h.v.sc.max.lmax .da.f.h.v.sc.max.max -side left" );

cmd( "pack .da.f.h.v.sc.auto .da.f.h.v.sc.min .da.f.h.v.sc.max -side left -ipadx 5" );

cmd( "frame .da.f.h.v.y2" );
cmd( "checkbutton .da.f.h.v.y2.logs -text \"Series in logs\" -variable logs" );
cmd( "set y2 0" );
cmd( "checkbutton .da.f.h.v.y2.y2 -text \"Y2 axis\" -variable y2 -command {if {$y2==0} {.da.f.h.v.y2.f.e conf -state disabled} {.da.f.h.v.y2.f.e conf -state normal}}" );

cmd( "frame .da.f.h.v.y2.f" );
cmd( "label .da.f.h.v.y2.f.l -text \"First series in Y2 axis\"" );
cmd( "entry .da.f.h.v.y2.f.e -width 4 -validate focusout -vcmd { if [ string is integer %%P ] { set numy2 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $numy2; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "pack .da.f.h.v.y2.f.l .da.f.h.v.y2.f.e -side left" );

cmd( "pack .da.f.h.v.y2.logs .da.f.h.v.y2.y2 .da.f.h.v.y2.f -side left -ipadx 7" );

cmd( "pack .da.f.h.v.ft .da.f.h.v.sc .da.f.h.v.y2" );

cmd( "frame .da.f.h.tc -relief groove -bd 2" );
cmd( "radiobutton .da.f.h.tc.time -text \"Time series\" -variable tc -value 0" );
cmd( "radiobutton .da.f.h.tc.cross -text \"Cross-section\" -variable tc -value 1" );
cmd( "pack .da.f.h.tc.time .da.f.h.tc.cross -anchor w" );

cmd( "frame .da.f.h.xy -relief groove -bd 2" );
cmd( "radiobutton .da.f.h.xy.seq -text \"Sequence\" -variable xy -value 0" );
cmd( "radiobutton .da.f.h.xy.xy -text \"XY plot\" -variable xy -value 1" );
cmd( "pack .da.f.h.xy.seq .da.f.h.xy.xy -anchor w" );

cmd( "pack .da.f.h.v .da.f.h.tc .da.f.h.xy -side left -padx 11" );

cmd( "frame .da.f.tit -relief groove -bd 2" );
cmd( "label .da.f.tit.l -text Title" );
cmd( "entry .da.f.tit.e -textvariable tit -width 36" );

cmd( "frame .da.f.tit.chk" );

cmd( "checkbutton .da.f.tit.chk.allblack -text \"No colors\" -variable allblack" );
cmd( "checkbutton .da.f.tit.chk.grid -text \"Grids\" -variable grid" );
cmd( "pack .da.f.tit.chk.allblack .da.f.tit.chk.grid -anchor w" );

cmd( "frame .da.f.tit.lp" );
cmd( "radiobutton .da.f.tit.lp.line -text \"Lines\" -variable line_point -value 1" );
cmd( "radiobutton .da.f.tit.lp.point -text \"Points\" -variable line_point -value 2" );
cmd( "pack .da.f.tit.lp.line .da.f.tit.lp.point -anchor w" );

cmd( "frame .da.f.tit.ps" );
cmd( "label .da.f.tit.ps.l -text \"Point size\"" );
cmd( "entry .da.f.tit.ps.e -width 4 -validate focusout -vcmd { if [ string is double %%P ] { set point_size %%P; return 1 } { %%W delete 0 end; %%W insert 0 $point_size; return 0 } } -invcmd { bell } -justify center" );
cmd( "pack .da.f.tit.ps.l .da.f.tit.ps.e" );

cmd( "frame .da.f.tit.run" );			// field for adjusting 
cmd( "checkbutton .da.f.tit.run.watch -text Watch -variable watch" );
cmd( "checkbutton .da.f.tit.run.gnu -text Gnuplot -variable gnu -onvalue 2 -offvalue 1" );
cmd( "pack .da.f.tit.run.watch .da.f.tit.run.gnu -anchor w" );

cmd( "frame .da.f.tit.pr" );			// field for adjusting y-axis precision
cmd( "label .da.f.tit.pr.l -text \"Precision\"" );
cmd( "entry .da.f.tit.pr.e -width 2 -validate focusout -vcmd { if [ string is double %%P ] { set pdigits %%P; return 1 } { %%W delete 0 end; %%W insert 0 $pdigits; return 0 } } -invcmd { bell } -justify center" );
cmd( "pack .da.f.tit.pr.l .da.f.tit.pr.e" );

cmd( "pack .da.f.tit.l .da.f.tit.e .da.f.tit.chk .da.f.tit.run .da.f.tit.pr .da.f.tit.ps .da.f.tit.lp -side left -padx 5" );

cmd( "pack .da.f.h .da.f.tit" );
cmd( "pack .da.f" );

cmd( "frame .da.b" );
cmd( "button .da.b.ts -width -9 -text Plot -command {set choice 1} -underline 0" );
cmd( "button .da.b.dump -width -9 -text \"Save Plot\" -command {set choice 11} -underline 2" );
cmd( "button .da.b.sv -width -9 -text \"Save Data\" -command {set choice 10} -underline 3" );
cmd( "button .da.b.sp -width -9 -text \"Show Data\" -command {set choice 36} -underline 5" );
cmd( "button .da.b.st -width -9 -text Statistics -command {set choice 12} -underline 1" );
cmd( "button .da.b.fr -width -9 -text Histogram -command {set choice 32} -underline 0" );
cmd( "button .da.b.lat -width -9 -text Lattice -command {set choice 23} -underline 0" );

cmd( "pack .da.b.ts .da.b.dump .da.b.sv .da.b.sp .da.b.st .da.b.fr .da.b.lat -padx 10 -pady 10 -side left -expand no -fill none" );
cmd( "pack .da.b -side right -expand no -fill none" );

// top window shortcuts binding
cmd( "bind .da <KeyPress-Escape> {set choice 2}" );	// quit
cmd( "bind .da <Control-l> {set choice 23}; bind .da <Control-L> {set choice 23}" );	// plot lattice
cmd( "bind .da <Control-h> {set choice 32}; bind .da <Control-H> {set choice 32}" );	// plot histograms
cmd( "bind .da <Control-c> {set choice 8}; bind .da <Control-C> {set choice 8}" );	// empty (clear) selected series
cmd( "bind .da <Control-a> {set choice 24}; bind .da <Control-A> {set choice 24}" );	// insert new series
cmd( "bind .da <Control-i> {set choice 34}; bind .da <Control-I> {set choice 34}" );	// sort selected in inverse order
cmd( "bind .da <Control-f> { set choice 39 }; bind .da <Control-F> { set choice 39 }" );	// search first
cmd( "bind .da <F3> { set choice 40 }; bind .da <Control-n> { set choice 40 }; bind .da <Control-N> { set choice 40 }" );	// search next
cmd( "bind .da <Control-greater> { set choice 6 }" );	// insert series
cmd( "bind .da <Control-less> { set choice 7 }" );	// remove series
cmd( "bind .da <Control-s> { set choice 5 }; bind .da <Control-S> { set choice 5 }" );	// sort up
cmd( "bind .da <Control-o> { set choice 38 }; bind .da <Control-O> { set choice 38 }" );	// sort down
cmd( "bind .da <Control-r> { set choice 15 }; bind .da <Control-R> { set choice 15 }" );	// sort up nice (end)
cmd( "bind .da <Control-u> { set choice 14 }; bind .da <Control-U> { set choice 14 }" );	// un-sort
cmd( "bind .da <Control-g> { set choice 4 }; bind .da <Control-G> { set choice 4 }" );	// launch gnuplot
cmd( "bind .da <Control-p> { set choice 1 }; bind .da <Control-P> { set choice 1 }" );	// plot
cmd( "bind .da <Control-v> { set choice 11 }; bind .da <Control-V> { set choice 11 }" );	// save plot
cmd( "bind .da <Control-e> { set choice 10 }; bind .da <Control-E> { set choice 10 }" );	// save data
cmd( "bind .da <Control-d> { set choice 36 }; bind .da <Control-D> { set choice 36 }" );	// show data
cmd( "bind .da <Control-t> { set choice 12 }; bind .da <Control-D> { set choice 12 }" );	// statistics

// create special sort procedure to keep names starting with underline at the end
cmd( "proc comp_und { n1 n2 } { \
	if [ string equal $n1 $n2 ] { \
		return 0 \
	}; \
	if { ! [ expr { [ string index $n1 0 ] == \"_\" && [ string index $n2 0 ] == \"_\" } ] } { \
		if { [ string index $n1 0 ] == \"_\" } { \
			return 1 \
		}; \
		if { [ string index $n2 0 ] == \"_\" } { \
			return -1 \
		}; \
	}; \
	set listn [ lsort -dictionary [ list $n1 $n2 ] ]; \
	if [ string equal [ lindex $listn 0 ] $n1 ] { \
		return -1 \
	} { \
		return 1 \
	}; \
}" );

// grab focus when called from LSD Debugger
Tcl_SetVar( inter, "running", running ? "1" : "0", 0 );
cmd( "if $running { showtop .da overM 0 1 } { showtop .da overM 0 1 0 }" );

if(num_var==0)
  cmd( "tk_messageBox -parent .da -type ok -title \"Analysis of Results\" -icon info -message \"There are no series available\" -detail \"Click on button 'Add...' to load series from results files.\n\nIf you were looking for data after a simulation run, please make sure you have selected the series to be saved, or have not set the objects containing them to not be computed.\"" );  

// make a copy to allow insertion of new temporary variables
cmd( "if [ info exists ModElem ] { set DaModElem $ModElem } { set DaModElem [ list ] }" );

// main loop

there:

// sort the list of available variables
cmd( "if [ info exists DaModElem ] { set DaModElem [ lsort -unique -dictionary $DaModElem ] }" );

// enable/disable the remove series button in the series toolbar
cmd( "if { [ .da.vars.ch.v size ] > 0 } { .da.vars.b.out conf -state normal } { .da.vars.b.out conf -state disabled }" );

// update entry boxes with linked variables
cmd( "write_disabled .da.f.h.v.ft.from.mnc $minc" );
cmd( "write_disabled .da.f.h.v.ft.to.mxc $maxc" );
cmd( "write_disabled .da.f.h.v.sc.min.min [ format \"%%.[ expr $pdigits ]g\" $miny ]" );
cmd( "write_disabled .da.f.h.v.sc.max.max [ format \"%%.[ expr $pdigits ]g\" $maxy ]" );
cmd( "write_disabled .da.f.h.v.y2.f.e $numy2" );
cmd( "write_any .da.f.tit.ps.e $point_size" ); 
cmd( "write_any .da.f.tit.pr.e $pdigits" ); 

// analysis command loop
while( ! *choice )
{
	try
	{
		Tcl_DoOneEvent( 0 );
	}
	catch ( std::bad_alloc& ) 	// raise memory problems
	{
		throw;
	}
	catch ( ... )				// ignore the rest
	{
		goto there;
	}
}   

// update linked variables with values in entry boxes
cmd( "set minc [ .da.f.h.v.ft.from.mnc get ]" );
cmd( "set maxc [ .da.f.h.v.ft.to.mxc get ]" );
cmd( "set miny [ .da.f.h.v.sc.min.min get ]" );
cmd( "set maxy [ .da.f.h.v.sc.max.max get ]" );
cmd( "set numy2 [ .da.f.h.v.y2.f.e get ]" );
cmd( "set point_size [ .da.f.tit.ps.e get ]" ); 
cmd( "set pdigits [ .da.f.tit.pr.e get ]" ); 

if(*choice==1 && time_cross==1 && xy==0) //Plot cross section
 *choice=9;

if(*choice==1 && time_cross==0 && xy==1) //Plot XY
 *choice=17;

if(*choice==1 && time_cross==1 && xy==1) //Plot XY Cross section
 *choice=18;

if(*choice==12 && time_cross==1) //Statistics cross section
 *choice=13;

if(pdigits<1 || pdigits>8)
	pdigits=4;

switch(*choice)
{

//exit
case 2:
cmd( "if { [ .da.vars.pl.v size ] != 0 } { set answer [ tk_messageBox -parent .da -type okcancel -title Confirmation -icon question -default ok -message \"Exit Analysis of Results?\" -detail \"All the plots and series created and not saved will be lost.\"] } { set answer ok }" );
app=(char *)Tcl_GetVar(inter, "answer",0);

cmd( "if {[string compare $answer ok] == 0} { } {set choice 0}" );
if(*choice==0)
  goto there;

delete[] vs;

cmd( "destroytop .da" );
uncover_browser( );
	
Tcl_UnlinkVar(inter, "auto");
Tcl_UnlinkVar(inter, "auto_x");
Tcl_UnlinkVar(inter, "minc");
Tcl_UnlinkVar(inter, "maxc");
Tcl_UnlinkVar(inter, "miny");
Tcl_UnlinkVar(inter, "maxy");
Tcl_UnlinkVar(inter, "logs");
Tcl_UnlinkVar(inter, "allblack");
Tcl_UnlinkVar(inter, "grid");
Tcl_UnlinkVar(inter,"point_size");
Tcl_UnlinkVar(inter,"tc");
Tcl_UnlinkVar(inter,"line_point");
Tcl_UnlinkVar(inter,"xy");
Tcl_UnlinkVar(inter, "pdigits");
Tcl_UnlinkVar(inter, "watch");
Tcl_UnlinkVar(inter, "gnu");
Tcl_UnlinkVar(inter, "cur_plot");

cmd( "catch [set a [glob -nocomplain plotxy_*]]" ); //remove directories
cmd( "catch [foreach b $a {catch [file delete -force $b]}]" );
return;
  
 
//Brutally shut down the system
case 35:
error_hard( msg, "Abort requested", "If error persists, please contact developers." );
myexit(20);


// MAIN BUTTONS OPTIONS

//Plot
case 1:
  if ( watch )
	cmd( ".da.b.ts conf -text Stop" );

  cur_plot++;
  
  plot(choice);
  
  if ( watch )
	cmd( ".da.b.ts conf -text Plot" );

  if(*choice==2) //plot aborted
   {
   cur_plot--;
   *choice=0;
   goto there;
   }

  cmd( "set exttit \"$cur_plot) $tit\"" );
  cmd( ".da.vars.pl.v insert end $exttit" );
  cmd( ".da.vars.pl.v selection clear 0 end; .da.vars.pl.v selection set end; .da.vars.pl.v activate end; .da.vars.pl.v see end" );
  
  type_plot[cur_plot]=0; //Lsd standard plot
  plot_l[cur_plot]=390; //height of plot with labels
  plot_nl[cur_plot]=325; //height of plot without labels  

  cmd( ".da.com.plot conf -text \"Plots = [ .da.vars.pl.v size ]\"" );

  *choice=0;
  goto there;

  
//Plot the cross-section plot. That is, the vars selected form
//one series for each time step selected
case 9:
   set_cs_data(choice);
   
   if(*choice==2) //plot aborted
    {
    *choice=0;
    goto there;
    }

   if ( watch )
	cmd( ".da.b.ts conf -text Stop" );

   cur_plot++;
	
   plot_cross(choice);

   if ( watch )
	cmd( ".da.b.ts conf -text Plot" );

   if(*choice==2) //plot aborted
    {
    cur_plot--;
    *choice=0;
    goto there;
    }

    cmd( "set exttit \"$cur_plot) $tit\"" );
    cmd( ".da.vars.pl.v insert end $exttit" );
	cmd( ".da.vars.pl.v selection clear 0 end; .da.vars.pl.v selection set end; .da.vars.pl.v activate end; .da.vars.pl.v see end" );
	
    type_plot[cur_plot]=0; //Lsd standard plot
    plot_l[cur_plot]=390; //height of plot with labels
    plot_nl[cur_plot]=325; //height of plot with labels  

    cmd( ".da.com.plot conf -text \"Plots = [ .da.vars.pl.v size ]\"" );
	
    *choice=0;
    goto there;

	
//Plot_gnu AND phase diagram, depending on how many series are selected
case 17:
  if ( watch )
	cmd( ".da.b.ts conf -text Stop" );

  cur_plot++;
  
  cmd( "set choice [.da.vars.ch.v size]" );
  if(*choice>1)
    plot_gnu(choice);
  else
    plot_phase_diagram(choice);

  if ( watch )
	cmd( ".da.b.ts conf -text Plot" );

  if(*choice==2)
   {
   cur_plot--;
   *choice=0;
   goto there;
   }

  cmd( "set exttit \"$cur_plot) $tit\"" );
  cmd( ".da.vars.pl.v insert end $exttit" );
  cmd( ".da.vars.pl.v selection clear 0 end; .da.vars.pl.v selection set end; .da.vars.pl.v activate end; .da.vars.pl.v see end" );
  
  type_plot[cur_plot]=1; //Gnuplot plot
  plot_l[cur_plot]=430; //height of plot with labels
  plot_nl[cur_plot]=430; //height of plot without labels  
  
  cmd( ".da.com.plot conf -text \"Plots = [ .da.vars.pl.v size ]\"" );
  
  *choice=0;
  goto there;

  
//Plot_cs_xy
case 18:
  if ( watch )
	cmd( ".da.b.ts conf -text Stop" );

  cur_plot++;

  plot_cs_xy(choice);

  if ( watch )
	cmd( ".da.b.ts conf -text Plot" );

  if(*choice==2) //plot aborted
   {
   cur_plot--;
   *choice=0;
   goto there;
   }

  cmd( "set exttit \"$cur_plot) $tit\"" );
  cmd( ".da.vars.pl.v insert end $exttit" );
  cmd( ".da.vars.pl.v selection clear 0 end; .da.vars.pl.v selection set end; .da.vars.pl.v activate end; .da.vars.pl.v see end" );
  
  type_plot[cur_plot]=1; //gnuplot standard plot
  plot_l[cur_plot]=430; //height of plot with labels
  plot_nl[cur_plot]=430; //height of plot with labels  
  
  cmd( ".da.com.plot conf -text \"Plots = [ .da.vars.pl.v size ]\"" );

  *choice=0;
  goto there;

  
/*
plot a lattice. Data must be stored on a single time step organized for lines and columns in
sequence

*/
case 23:
  if ( watch )
	cmd( ".da.b.ts conf -text Stop" );

  cur_plot++;

  plot_lattice(choice);
  
  if ( watch )
	cmd( ".da.b.ts conf -text Plot" );

  if(*choice==2) //plot aborted
   {
   cur_plot--;
   *choice=0;
   goto there;
   }
   
  cmd( "set exttit \"$cur_plot) $tit\"" );
  cmd( ".da.vars.pl.v insert end $exttit" );
  cmd( ".da.vars.pl.v selection clear 0 end; .da.vars.pl.v selection set end; .da.vars.pl.v activate end; .da.vars.pl.v see end" );
  
  type_plot[cur_plot]=3; //Lattice canvas standard plot
  plot_l[cur_plot]=-1; //height of plot with labels
  plot_nl[cur_plot]=-1; //height of plot with labels  

  cmd( ".da.com.plot conf -text \"Plots = [ .da.vars.pl.v size ]\"" );

  *choice=0;
  goto there;


// plot histograms
case 32:
  if ( watch )
	cmd( ".da.b.ts conf -text Stop" );

  cur_plot++;
  
  cmd( "set choice $tc" );
  if(*choice==0)
    histograms(choice);
  else
    histograms_cs(choice);
  
  if ( watch )
	cmd( ".da.b.ts conf -text Plot" );

  if(*choice==2) //plot aborted
   {
   cur_plot--;
   *choice=0;
   goto there;
   }
   
  cmd( "set exttit \"$cur_plot) $tit\"" );
  cmd( ".da.vars.pl.v insert end $exttit" );
  cmd( ".da.vars.pl.v selection clear 0 end; .da.vars.pl.v selection set end; .da.vars.pl.v activate end; .da.vars.pl.v see end" );
    
  type_plot[cur_plot]=0; //Lsd standard plot
  plot_l[cur_plot]=325; //height of plot with labels
  plot_nl[cur_plot]=325; //height of plot with labels  
  
  cmd( ".da.com.plot conf -text \"Plots = [ .da.vars.pl.v size ]\"" );
  
  *choice=0;
  goto there;


//Print the data series in the log window
case 36: 
   plog_series(choice);
   cmd( "wm deiconify .log" );
   cmd( "raise .log .da" );
   *choice=0;
   goto there;
   
   
//show the equation for the selected variable
case 16:
cmd( "set a [split $res]" );
cmd( "set b [lindex $a 0]" );
app=(char *)Tcl_GetVar(inter, "b",0);
show_eq(app, choice);
*choice=0;
goto there;


//Save data in the Vars. to plot listbox
case 10:
 save_datazip(choice);
 *choice=0;
 goto there;


//Statistics
case 12:
 statistics(choice);
 cmd( "wm deiconify .log" );
 cmd( "raise .log .da" );
 *choice=0;
 goto there;

 
//Statistics Cross Section
case 13:
 set_cs_data(choice);
 if(*choice==2)
  {
	*choice=0;
    goto there;
  }
 statistics_cross(choice);
 cmd( "wm deiconify .log" );
 cmd( "raise .log .da" );
 *choice=0;
 goto there;


//create the postscript for a plot
case 11:
cmd( "set choice [ .da.vars.pl.v size ]" );
if ( *choice == 0 )		 // no plots to save
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No plot available\" -detail \"Place one or more series in the Series Selected listbox and select 'Plot' to produce a plot.\"" );
	goto there;
}

do
{
	*choice=0;
	cmd( "set iti [.da.vars.pl.v curselection]" );
	cmd( "if {[string length $iti] == 0} {set choice 1}" );

	if(*choice==1)
	{
		*choice=0;
		cmd( "newtop .da.a \"Save Plot\" { set choice 2 } .da" );
		cmd( "label .da.a.l -text \"Select one plot from the \nPlots listbox before clicking 'Ok'\"" );
		cmd( "pack .da.a.l -pady 10" );
		cmd( "okcancel .da.a b { set choice 1 } { set choice 2 }" );
		cmd( "showtop .da.a centerW 0 0 0" );
		while(*choice==0)
			Tcl_DoOneEvent(0);
		cmd( "destroytop .da.a" );
	}
}
while ( *choice == 1 );

if(*choice==2)
   {*choice=0;
    goto there;
   }

*choice=0;
cmd( "set iti [.da.vars.pl.v curselection]" );
cmd( "set it [.da.vars.pl.v get $iti]" );
cmd( "scan $it %d)%s a b" );
cmd( "set choice [winfo exist .da.f.new$a]" );

if( *choice == 0 )
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"The selected plot does not exist anymore\" -detail \"If you closed the window of the plot, then re-create it.\nIf the plot is a high-quality scatter plot, use the gnuplot tools to export it (use the right button of the mouse).\"" );
	goto there;  
}

cmd( "newtop .da.file \"Save Plot\" { set choice 2 } .da" );

cmd( "frame .da.file.l -bd 2" );
cmd( "label .da.file.l.l1 -text \"Settings for plot\"" );
cmd( "label .da.file.l.l2 -fg red -text \"($a) $b\"" );
cmd( "pack .da.file.l.l1 .da.file.l.l2" );

cmd( "set cm \"color\"" );
cmd( "frame .da.file.col -relief groove -bd 2" );
cmd( "radiobutton .da.file.col.r1 -text \"Color\" -variable cm -value color" );
cmd( "radiobutton .da.file.col.r2 -text \"Grayscale\" -variable cm -value gray" );
cmd( "radiobutton .da.file.col.r3 -text \"Mono\" -variable cm -value mono" );
cmd( "pack .da.file.col.r1 .da.file.col.r2 .da.file.col.r3 -side left" );

cmd( "set res 0" );
cmd( "frame .da.file.pos -relief groove -bd 2" );
cmd( "radiobutton .da.file.pos.p1 -text \"Landscape\" -variable res -value 0" );
cmd( "radiobutton .da.file.pos.p2 -text \"Portrait\" -variable res -value 1" );
cmd( "pack .da.file.pos.p1 .da.file.pos.p2 -side left -ipadx 11" );

cmd( "set dim 270" );
cmd( "frame .da.file.dim -bd 2" );
cmd( "label .da.file.dim.l1 -text \"Dimension\"" );
cmd( "entry .da.file.dim.n -width 4 -validate focusout -vcmd { if [ string is integer %%P ] { set dim %%P; return 1 } { %%W delete 0 end; %%W insert 0 $dim; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.file.dim.n insert 0 $dim" );
cmd( "label .da.file.dim.l2 -text \"(mm@96DPI)\"" );
cmd( "pack .da.file.dim.l1 .da.file.dim.n .da.file.dim.l2 -side left" );

cmd( "set heightpost 1" );
cmd( "checkbutton .da.file.lab -text \"Include plot labels\" -variable heightpost -onvalue 1 -offvalue 0" );
cmd( "set choice $a" );
if ( plot_l[ *choice ] == plot_nl[ *choice ] )
	cmd( ".da.file.lab conf -state disabled" );

cmd( "pack .da.file.l .da.file.col .da.file.pos .da.file.dim .da.file.lab -pady 5 -padx 5" );
cmd( "okcancel .da.file b { set choice 1 } { set choice 2 }" );
cmd( "showtop .da.file centerW" );

*choice=0;
  while(*choice==0)
	Tcl_DoOneEvent(0);

cmd( "set dim [ .da.file.dim.n get ]" ); 
cmd( "destroytop .da.file" );

if(*choice==2)
 {*choice=0;
  goto there;
 }

cmd( "set fn \"$b.eps\"" );
cmd( "set fn [ tk_getSaveFile -parent .da -title \"Save Plot File\" -defaultextension .eps -initialfile $fn -filetypes { { {Encapsulated Postscript files} {.eps} } { {All files} {*} } } ]; if { [string length $fn] == 0 } { set choice 2 }" );

if(*choice==2)
 {*choice=0;
  goto there;
 }

cmd( "set dd \"\"" );
cmd( "append dd $dim m" );
cmd( "set fn [file nativename $fn]" ); //return the name in the platform specific format

cmd( "set choice $a" );
if ( plot_l[ *choice ] == plot_nl[ *choice ] )	// no labels?
{
	if ( plot_l[ *choice ] > 0 )
		cmd( "set heightpost %d", plot_l[ *choice ] );
	else
	{
		cmd( "set str [.da.f.new$a.f.plots conf -height]" );
		cmd( " scan $str \"%s %s %s %s %d\" trash1 trash2 trash3 trash4 heighpost" );
	} 
}
else
	cmd( "if { $heightpost == 1 } { set heightpost %d } { set heightpost %d }", plot_l[ *choice ], plot_nl[ *choice ] );

cmd( ".da.f.new$a.f.plots postscript -height $heightpost -pagewidth $dd -rotate $res -colormode $cm -file \"$fn\"" );
cmd( "plog \"\nPlot saved: $fn\n\"" );

*choice = 0;
goto there;


// SELECTION BOX CONTEXT MENUS

//Use right button of the mouse to select all series with a given label
case 30:
Tcl_LinkVar(inter, "compvalue", (char *) &compvalue, TCL_LINK_DOUBLE);
cmd( "set a [split $res]" );
cmd( "set b [lindex $a 0]" );
cmd( "set c [lindex $a 1]" ); //get the tag value
cmd( "set i [llength [split $c {_}]]" );
cmd( "set ntag $i" );
cmd( "set ssys 2" );
cmd( "if { ! [info exist tvar] } {set tvar $maxc}" );
cmd( "if { ! [info exist cond] } {set cond 1}" );

cmd( "newtop .da.a \"Select Series\" {set choice 2} .da" );

cmd( "frame .da.a.tit" );
cmd( "label .da.a.tit.l -text \"Select series with label\"" );
cmd( "label .da.a.tit.s -text \"$b\" -foreground red" );
cmd( "pack .da.a.tit.l .da.a.tit.s" );
cmd( "frame .da.a.q -relief groove -bd 2" );
cmd( "frame .da.a.q.f1" );
cmd( "radiobutton .da.a.q.f1.c -text \"Select all\" -variable ssys -value 2 -command {for {set x 0} {$x<$i} {incr x} {.da.a.q.f.l.e$x conf -state disabled}; .da.a.q.f2.f.e conf -state disabled; .da.a.c.v.t.e2 conf -state disabled; .da.a.c.v.c.e conf -state disabled; .da.a.c.o.eq conf -state disabled; .da.a.c.o.dif conf -state disabled; .da.a.c.o.geq conf -state disabled; .da.a.c.o.g conf -state disabled; .da.a.c.o.seq conf -state disabled; .da.a.c.o.s conf -state disabled}" );
cmd( "bind .da.a.q.f1.c <Return> {.da.a.q.f1.c invoke; focus .da.a.b.ok}" );
cmd( "bind .da.a.q.f1.c <Down> {focus .da.a.q.f.c; .da.a.q.f.c invoke}" );
cmd( "pack .da.a.q.f1.c" );
cmd( "pack .da.a.q.f1 -anchor w" );
cmd( "frame .da.a.q.f" );
cmd( "radiobutton .da.a.q.f.c -text \"Select by series' tags\" -variable ssys -value 1 -command {for {set x 0} {$x<$i} {incr x} {.da.a.q.f.l.e$x conf -state normal}; .da.a.q.f2.f.e conf -state disabled; .da.a.c.v.t.e2 conf -state disabled; .da.a.c.v.c.e conf -state disabled; .da.a.c.o.eq conf -state normal; .da.a.c.o.dif conf -state normal; .da.a.c.o.geq conf -state normal; .da.a.c.o.g conf -state normal; .da.a.c.o.seq conf -state normal; .da.a.c.o.s conf -state normal}" );
cmd( "bind .da.a.q.f.c <Up> {focus .da.a.q.f1.c; .da.a.q.f1.c invoke}" );
cmd( "bind .da.a.q.f.c <Return> {focus .da.a.q.f.l.e0; .da.a.q.f.l.e0 selection range 0 end}" );
cmd( "bind .da.a.q.f.c <Down> {focus .da.a.q.f3.s; .da.a.q.f3.s invoke}" );
cmd( "pack .da.a.q.f.c -anchor w" );
cmd( "frame .da.a.q.f.l" );
cmd( "for {set x 0} {$x<$i} {incr x} {if {$x > 0} {label .da.a.q.f.l.s$x -text \u2014}; entry .da.a.q.f.l.e$x -width 4 -textvariable v$x -justify center -state disabled}" );
cmd( "for { set x 0 } { $x < $i } { incr x } { if { $x > 0 } { pack .da.a.q.f.l.s$x -side left }; pack .da.a.q.f.l.e$x -side left; bind .da.a.q.f.l.e$x <Return> [ subst -nocommand { focus .da.a.q.f.l.e[ expr $x + 1 ]; .da.a.q.f.l.e[ expr $x + 1 ] selection range 0 end } ]; bind .da.a.q.f.l.e$x <KeyRelease> { .da.a.q.f.c invoke } }; incr x -1; bind .da.a.q.f.l.e$x <Return> { focus .da.a.b.ok }" );
cmd( "pack .da.a.q.f.l -anchor w -padx 25" );
cmd( "pack .da.a.q.f -anchor w" );
cmd( "frame .da.a.q.f3" );
cmd( "radiobutton .da.a.q.f3.s -text \"Select by series values\" -variable ssys -value 3 -command {for {set x 0} {$x<$i} {incr x} {.da.a.q.f.l.e$x conf -state disabled}; .da.a.q.f2.f.e conf -state disabled; .da.a.c.v.t.e2 conf -state normal; .da.a.c.v.c.e conf -state normal; .da.a.c.o.eq conf -state normal; .da.a.c.o.dif conf -state normal; .da.a.c.o.geq conf -state normal; .da.a.c.o.g conf -state normal; .da.a.c.o.seq conf -state normal; .da.a.c.o.s conf -state normal}" );
cmd( "bind .da.a.q.f3.s <Up> {focus .da.a.q.f.c; .da.a.q.f.c invoke}" );
cmd( "bind .da.a.q.f3.s <Return> {focus .da.a.c.v.c.e; .da.a.c.v.c.e selection range 0 end}" );
cmd( "bind .da.a.q.f3.s <Down> {focus .da.a.q.f2.s; .da.a.q.f2.s invoke}" );
cmd( "pack .da.a.q.f3.s -anchor w" );
cmd( "pack .da.a.q.f3 -anchor w" );
cmd( "frame .da.a.q.f2" );
cmd( "radiobutton .da.a.q.f2.s -text \"Select by values from another series\" -variable ssys -value 4 -command {for {set x 0} {$x<$i} {incr x} {.da.a.q.f.l.e$x conf -state disabled}; .da.a.q.f2.f.e conf -state normal; .da.a.c.v.t.e2 conf -state normal; .da.a.c.v.c.e conf -state normal; .da.a.c.o.eq conf -state normal; .da.a.c.o.dif conf -state normal; .da.a.c.o.geq conf -state normal; .da.a.c.o.g conf -state normal; .da.a.c.o.seq conf -state normal; .da.a.c.o.s conf -state normal}" );
cmd( "bind .da.a.q.f2.s <Up> {focus .da.a.q.f3.s; .da.a.q.f3.s invoke}" );
cmd( "bind .da.a.q.f2.s <Return> {focus .da.a.q.f2.f.e; .da.a.q.f2.f.e selection range 0 end}" );
cmd( "pack .da.a.q.f2.s -anchor w" );
cmd( "frame .da.a.q.f2.f" );
cmd( "label .da.a.q.f2.f.l -text \"Label\"" );
cmd( "entry .da.a.q.f2.f.e -width 17 -textvariable svar -justify center -state disabled" );
cmd( "bind .da.a.q.f2.f.e <KeyRelease> {if { %%N < 256 && [info exists DaModElem] } { set bb1 [.da.a.q.f2.f.e index insert]; set bc1 [.da.a.q.f2.f.e get]; set bf1 [lsearch -glob $DaModElem $bc1*]; if { $bf1 !=-1 } {set bd1 [lindex $DaModElem $bf1]; .da.a.q.f2.f.e delete 0 end; .da.a.q.f2.f.e insert 0 $bd1; .da.a.q.f2.f.e index $bb1; .da.a.q.f2.f.e selection range $bb1 end } } }" );
cmd( "bind .da.a.q.f2.f.e <Return> {focus .da.a.c.v.c.e; .da.a.c.v.c.e selection range 0 end}" );
cmd( "pack .da.a.q.f2.f.l .da.a.q.f2.f.e -anchor w -side left" );
cmd( "pack .da.a.q.f2.f -anchor w -padx 22" );
cmd( "pack .da.a.q.f2 -anchor w" );
cmd( "frame .da.a.c -relief groove -bd 2" );
cmd( "frame .da.a.c.o" );
cmd( "radiobutton .da.a.c.o.eq -text \"Equal (=)\" -variable cond -value 1 -state disabled" );
cmd( "radiobutton .da.a.c.o.dif -text \"Different (\u2260)\" -variable cond -value 0 -state disabled" );
cmd( "radiobutton .da.a.c.o.geq -text \"Larger or equal (\u2265)\" -variable cond -value 2 -state disabled" );
cmd( "radiobutton .da.a.c.o.g -text \"Larger (>)\" -variable cond -value 3 -state disabled" );
cmd( "radiobutton .da.a.c.o.seq -text \"Smaller or equal (\u2264)\" -variable cond -value 4 -state disabled" );
cmd( "radiobutton .da.a.c.o.s -text \"Smaller (<)\" -variable cond -value 5 -state disabled" );
cmd( "pack .da.a.c.o.eq .da.a.c.o.dif .da.a.c.o.geq .da.a.c.o.g .da.a.c.o.seq .da.a.c.o.s -anchor w" );
cmd( "frame .da.a.c.v" );
cmd( "frame .da.a.c.v.c" );
cmd( "label .da.a.c.v.c.l -text \"Comparison value\"" );
cmd( "entry .da.a.c.v.c.e -width 17 -validate focusout -vcmd { if [ string is double %%P ] { set compvalue %%P; return 1 } { %%W delete 0 end; %%W insert 0 $compvalue; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "write_any .da.a.c.v.c.e $compvalue" ); 
cmd( "bind .da.a.c.v.c.e <Return> {focus .da.a.c.v.t.e2; .da.a.c.v.t.e2 selection range 0 end }" );
cmd( "pack .da.a.c.v.c.l .da.a.c.v.c.e" );
cmd( "frame .da.a.c.v.t" );
cmd( "label .da.a.c.v.t.t -text \"Case\"" );
cmd( "entry .da.a.c.v.t.e2 -width 6 -validate focusout -vcmd { if [ string is integer %%P ] { set tvar %%P; return 1 } { %%W delete 0 end; %%W insert 0 $tvar; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "write_any .da.a.c.v.t.e2 $tvar" ); 
cmd( "bind .da.a.c.v.t.e2 <Return> {focus .da.a.b.ok}" );
cmd( "pack .da.a.c.v.t.t .da.a.c.v.t.e2" );
cmd( "pack .da.a.c.v.c .da.a.c.v.t -ipady 10" );
cmd( "pack .da.a.c.o .da.a.c.v -anchor w -side left -ipadx 5" );
cmd( "pack .da.a.tit .da.a.q .da.a.c -expand yes -fill x -padx 5 -pady 5" );

cmd( "okhelpcancel .da.a b { set choice 1 } { LsdHelp mdatares.html#batch_sel } { set choice 2 }" );
cmd( "showtop .da.a topleftW 0 0" );
cmd( "focus .da.a.q.f1.c" );

*choice=0;
while(*choice==0)
	Tcl_DoOneEvent(0);

cmd( "set tvar [ .da.a.c.v.t.e2 get ]" ); 
cmd( "set compvalue [ .da.a.c.v.c.e get ]" ); 
Tcl_UnlinkVar(inter, "compvalue");

if(*choice==2)
{
cmd( "destroytop .da.a" );
*choice=0;
goto there;
}

cmd( "if {[.da.vars.ch.v get 0] == \"\"} {set tit \"\"} {}" );
cmd( "set choice $ssys" );
if(*choice==2)
 {
 cmd( "set tot [.da.vars.lb.v get 0 end]" );
 cmd( "foreach i $tot { if { [lindex [split $i] 0] == \"$b\"} {  .da.vars.ch.v insert end \"$i\"  } {}}" );
 cmd( "if {\"$tit\" == \"\"} {set tit [.da.vars.ch.v get 0]} {}" );
 }
 
if(*choice== 1)
{
cmd( "set choice $cond" );
i=*choice;

*choice=-1;
cmd( "for {set x 0} {$x<$i} {incr x} {if {[.da.a.q.f.l.e$x get]!=\"\"} {set choice $x}}" );
if(*choice==-1)
{
cmd( "set tot [.da.vars.lb.v get 0 end]" );
cmd( "foreach i $tot { if { [lindex [split $i] 0] == \"$b\"} { .da.vars.ch.v insert end \"$i\"  } }" );
}
else
 {
cmd( "set tot [.da.vars.lb.v get 0 end]" );
cmd( "if { [info exist vcell]==1} {unset vcell} {}" );
cmd( "set choice $ntag" );
for(j=0; j<*choice; j++)
 cmd( "lappend vcell $v%d", j );

switch(i) 
{
case 0:
  cmd( "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] == [lindex [split [lindex [split $i] 1] {_}] $x] } { set c 0 } }; if $c { .da.vars.ch.v insert end \"$i\" } } }" );
 break;
case 1:
  cmd( "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] != [lindex [split [lindex [split $i] 1] {_}] $x] } { set c 0} {} }; if { $c==1 } {  .da.vars.ch.v insert end \"$i\"  } {}} {}}" );
 break;
case 2:
   cmd( "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] > [lindex [split [lindex [split $i] 1] {_}] $x] } { set c 0} {} }; if { $c==1 } {  .da.vars.ch.v insert end \"$i\"  } {}} {}}" );
 break;
case 3:
   cmd( "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] >= [lindex [split [lindex [split $i] 1] {_}] $x] } { set c 0} {} }; if { $c==1 } {  .da.vars.ch.v insert end \"$i\"  } {}} {}}" );
 break;
case 4:
   cmd( "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] < [lindex [split [lindex [split $i] 1] {_}] $x] } { set c 0} {} }; if { $c==1 } {  .da.vars.ch.v insert end \"$i\"  } {}} {}}" );
 break;
case 5:
   cmd( "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] <= [lindex [split [lindex [split $i] 1] {_}] $x] } { set c 0} {} }; if { $c==1 } {  .da.vars.ch.v insert end \"$i\"  } {}} {}}" );
 break;
} 
}
*choice=0;
}

if ( *choice == 3 || *choice == 4 )
{
l = *choice;
cmd( "set choice $cond" );
p=*choice;
cmd( "set tot [.da.vars.lb.v get 0 end]" );
cmd( "set choice [llength $tot]" );
j=*choice;

if ( l == 3 )
	app = ( char * ) Tcl_GetVar( inter, "b", 0 );
else
	app = ( char * ) Tcl_GetVar( inter, "svar", 0 );

strcpy(str3,app);

cmd( "set choice $tvar" );
h=*choice;

for(i=0; i<j; i++)
 {
  cmd( "set res [lindex $tot %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str1, str2, &l, &m, &k);
  if(h>=l && h<=m && !strcmp(str1, str3))
   {
   datum=find_data(k);
   r=0;
   if(is_finite(datum[h]))		// ignore NaNs
    switch(p)
    {
    case 0: if(datum[h]!=compvalue) r=1;
    break;
    case 1: if(datum[h]==compvalue) r=1;
    break;
    case 2: if(datum[h]>=compvalue) r=1;
     break;
    case 3: if(datum[h] > compvalue) r=1;
     break;
    case 4: if(datum[h]<=compvalue) r=1;
     break;
    case 5: if(datum[h] < compvalue) r=1;
     break;
        
    }
    if(r==1)
     { 
      cmd( "set templ $tot" );
      cmd( "set choice [lsearch $templ \"$b %s *\"]", str2 );

      while(*choice>=0)
       {
       cmd( ".da.vars.ch.v insert end [lindex $templ $choice]" );
       cmd( "set templ [lreplace $templ $choice $choice]" );
       }
     }
   }
 }
}

cmd( "destroytop .da.a" );

cmd( "if {\"$tit\" == \"\"} {set tit [.da.vars.ch.v get 0]} {}" );
cmd( ".da.com.selec conf -text \"Series = [ .da.vars.ch.v size ]\"" );

*choice=0;
goto there;


//Use right button of the mouse to remove series selected with different criteria
case 33:
cmd( "unset -nocomplain compvalue" );
Tcl_LinkVar(inter, "compvalue", (char *) &compvalue, TCL_LINK_DOUBLE);
cmd( "set a [split $res]" );
cmd( "set b [lindex $a 0]" );
cmd( "set c [lindex $a 1]" ); //get the tag value
cmd( "set i [llength [split $c {_}]]" );
cmd( "set ntag $i" );
cmd( "set ssys 2" );
cmd( "if { ! [info exist tvar] } { set tvar $maxc }" );
cmd( "if { ! [info exist cond] } { set cond 1 }" );
cmd( "if { ! [info exist selOnly] } { set selOnly 0 }" );

cmd( "newtop .da.a \"Unselect Series\" {set choice 2} .da" );

cmd( "frame .da.a.tit" );
cmd( "label .da.a.tit.l -text \"Unselect series with label\"" );
cmd( "label .da.a.tit.s -text \"$b\" -foreground red" );
cmd( "pack .da.a.tit.l .da.a.tit.s" );
cmd( "frame .da.a.q -relief groove -bd 2" );
cmd( "frame .da.a.q.f1" );
cmd( "radiobutton .da.a.q.f1.c -text \"Unselect all\" -variable ssys -value 2 -command {for {set x 0} {$x<$i} {incr x} {.da.a.q.f.l.e$x conf -state disabled}; .da.a.q.f2.f.e conf -state disabled; .da.a.c.v.t.e2 conf -state disabled; .da.a.c.v.c.e conf -state disabled; .da.a.c.o.eq conf -state disabled; .da.a.c.o.dif conf -state disabled; .da.a.c.o.geq conf -state disabled; .da.a.c.o.g conf -state disabled; .da.a.c.o.seq conf -state disabled; .da.a.c.o.s conf -state disabled}" );
cmd( "bind .da.a.q.f1.c <Return> {.da.a.q.f1.c invoke; focus .da.a.b.ok}" );
cmd( "bind .da.a.q.f1.c <Down> {focus .da.a.q.f.c; .da.a.q.f.c invoke}" );
cmd( "pack .da.a.q.f1.c" );
cmd( "pack .da.a.q.f1 -anchor w" );
cmd( "frame .da.a.q.f" );
cmd( "radiobutton .da.a.q.f.c -text \"Unselect by series' tags\" -variable ssys -value 1 -command {for {set x 0} {$x<$i} {incr x} {.da.a.q.f.l.e$x conf -state normal}; .da.a.q.f2.f.e conf -state disabled; .da.a.c.v.t.e2 conf -state disabled; .da.a.c.v.c.e conf -state disabled; .da.a.c.o.eq conf -state normal; .da.a.c.o.dif conf -state normal; .da.a.c.o.geq conf -state normal; .da.a.c.o.g conf -state normal; .da.a.c.o.seq conf -state normal; .da.a.c.o.s conf -state normal}" );
cmd( "bind .da.a.q.f.c <Up> {focus .da.a.q.f1.c; .da.a.q.f1.c invoke}" );
cmd( "bind .da.a.q.f.c <Return> {focus .da.a.q.f.l.e0; .da.a.q.f.l.e0 selection range 0 end}" );
cmd( "bind .da.a.q.f.c <Down> {focus .da.a.q.f3.s; .da.a.q.f3.s invoke}" );
cmd( "pack .da.a.q.f.c -anchor w" );
cmd( "frame .da.a.q.f.l" );
cmd( "for {set x 0} {$x<$i} {incr x} {if {$x > 0} {label .da.a.q.f.l.s$x -text \u2014}; entry .da.a.q.f.l.e$x -width 4 -textvariable v$x -justify center -state disabled}" );
cmd( "for { set x 0 } { $x < $i } { incr x } { if { $x > 0 } { pack .da.a.q.f.l.s$x -side left }; pack .da.a.q.f.l.e$x -side left; bind .da.a.q.f.l.e$x <Return> [ subst -nocommand { focus .da.a.q.f.l.e[ expr $x + 1 ]; .da.a.q.f.l.e[ expr $x + 1 ] selection range 0 end } ]; bind .da.a.q.f.l.e$x <KeyRelease> { .da.a.q.f.c invoke } }; incr x -1; bind .da.a.q.f.l.e$x <Return> { focus .da.a.b.ok }" );
cmd( "pack .da.a.q.f.l -anchor w -padx 25" );
cmd( "pack .da.a.q.f -anchor w" );
cmd( "frame .da.a.q.f3" );
cmd( "radiobutton .da.a.q.f3.s -text \"Unselect by series values\" -variable ssys -value 3 -command {for {set x 0} {$x<$i} {incr x} {.da.a.q.f.l.e$x conf -state disabled}; .da.a.q.f2.f.e conf -state disabled; .da.a.c.v.t.e2 conf -state normal; .da.a.c.v.c.e conf -state normal; .da.a.c.o.eq conf -state normal; .da.a.c.o.dif conf -state normal; .da.a.c.o.geq conf -state normal; .da.a.c.o.g conf -state normal; .da.a.c.o.seq conf -state normal; .da.a.c.o.s conf -state normal}" );
cmd( "bind .da.a.q.f3.s <Up> {focus .da.a.q.f.c; .da.a.q.f.c invoke}" );
cmd( "bind .da.a.q.f3.s <Return> {focus .da.a.c.v.c.e; .da.a.c.v.c.e selection range 0 end}" );
cmd( "bind .da.a.q.f3.s <Down> {focus .da.a.q.f2.s; .da.a.q.f2.s invoke}" );
cmd( "pack .da.a.q.f3.s -anchor w" );
cmd( "pack .da.a.q.f3 -anchor w" );
cmd( "frame .da.a.q.f2" );
cmd( "radiobutton .da.a.q.f2.s -text \"Unselect by values from another series\" -variable ssys -value 4 -command {for {set x 0} {$x<$i} {incr x} {.da.a.q.f.l.e$x conf -state disabled}; .da.a.q.f2.f.e conf -state normal; .da.a.c.v.t.e2 conf -state normal; .da.a.c.v.c.e conf -state normal; .da.a.c.o.eq conf -state normal; .da.a.c.o.dif conf -state normal; .da.a.c.o.geq conf -state normal; .da.a.c.o.g conf -state normal; .da.a.c.o.seq conf -state normal; .da.a.c.o.s conf -state normal}" );
cmd( "bind .da.a.q.f2.s <Up> {focus .da.a.q.f3.s; .da.a.q.f3.s invoke}" );
cmd( "bind .da.a.q.f2.s <Return> {focus .da.a.q.f2.f.e; .da.a.q.f2.f.e selection range 0 end}" );
cmd( "pack .da.a.q.f2.s -anchor w" );
cmd( "frame .da.a.q.f2.f" );
cmd( "label .da.a.q.f2.f.l -text \"Label\"" );
cmd( "entry .da.a.q.f2.f.e -width 17 -textvariable svar -justify center -state disabled" );
cmd( "bind .da.a.q.f2.f.e <KeyRelease> {if { %%N < 256 && [info exists DaModElem] } { set bb1 [.da.a.q.f2.f.e index insert]; set bc1 [.da.a.q.f2.f.e get]; set bf1 [lsearch -glob $DaModElem $bc1*]; if { $bf1 !=-1 } {set bd1 [lindex $DaModElem $bf1]; .da.a.q.f2.f.e delete 0 end; .da.a.q.f2.f.e insert 0 $bd1; .da.a.q.f2.f.e index $bb1; .da.a.q.f2.f.e selection range $bb1 end } } }" );
cmd( "bind .da.a.q.f2.f.e <Return> {focus .da.a.c.v.c.e; .da.a.c.v.c.e selection range 0 end}" );
cmd( "pack .da.a.q.f2.f.l .da.a.q.f2.f.e -anchor w -side left" );
cmd( "pack .da.a.q.f2.f -anchor w -padx 22" );
cmd( "pack .da.a.q.f2 -anchor w" );
cmd( "frame .da.a.c -relief groove -bd 2" );
cmd( "frame .da.a.c.o" );
cmd( "radiobutton .da.a.c.o.eq -text \"Equal (=)\" -variable cond -value 1 -state disabled" );
cmd( "radiobutton .da.a.c.o.dif -text \"Different (\u2260)\" -variable cond -value 0 -state disabled" );
cmd( "radiobutton .da.a.c.o.geq -text \"Larger or equal (\u2265)\" -variable cond -value 2 -state disabled" );
cmd( "radiobutton .da.a.c.o.g -text \"Larger (>)\" -variable cond -value 3 -state disabled" );
cmd( "radiobutton .da.a.c.o.seq -text \"Smaller or equal (\u2264)\" -variable cond -value 4 -state disabled" );
cmd( "radiobutton .da.a.c.o.s -text \"Smaller (<)\" -variable cond -value 5 -state disabled" );
cmd( "pack .da.a.c.o.eq .da.a.c.o.dif .da.a.c.o.geq .da.a.c.o.g .da.a.c.o.seq .da.a.c.o.s -anchor w" );
cmd( "frame .da.a.c.v" );
cmd( "frame .da.a.c.v.c" );
cmd( "label .da.a.c.v.c.l -text \"Comparison value\"" );
cmd( "entry .da.a.c.v.c.e -width 17 -validate focusout -vcmd { if [ string is double %%P ] { set compvalue %%P; return 1 } { %%W delete 0 end; %%W insert 0 $compvalue; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "write_any .da.a.c.v.c.e $compvalue" ); 
cmd( "bind .da.a.c.v.c.e <Return> {focus .da.a.c.v.t.e2; .da.a.c.v.t.e2 selection range 0 end }" );
cmd( "pack .da.a.c.v.c.l .da.a.c.v.c.e" );
cmd( "frame .da.a.c.v.t" );
cmd( "label .da.a.c.v.t.t -text \"Case\"" );
cmd( "entry .da.a.c.v.t.e2 -width 6 -validate focusout -vcmd { if [ string is integer %%P ] { set tvar %%P; return 1 } { %%W delete 0 end; %%W insert 0 $tvar; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "write_any .da.a.c.v.t.e2 $tvar" ); 
cmd( "bind .da.a.c.v.t.e2 <Return> {focus .da.a.b.ok}" );
cmd( "pack .da.a.c.v.t.t .da.a.c.v.t.e2" );
cmd( "pack .da.a.c.v.c .da.a.c.v.t -ipady 10" );
cmd( "pack .da.a.c.o .da.a.c.v -anchor w -side left -ipadx 5" );
cmd( "frame .da.a.s" );
cmd( "checkbutton .da.a.s.b -text \"Only mark items\" -variable selOnly" );
cmd( "pack .da.a.s.b" );
cmd( "pack .da.a.tit .da.a.q .da.a.c .da.a.s -expand yes -fill x -padx 5 -pady 5" );

cmd( "okhelpcancel .da.a b { set choice 1 } { LsdHelp mdatares.html#batch_sel } { set choice 2 }" );
cmd( "showtop .da.a topleftW 0 0" );
cmd( "focus .da.a.q.f1.c" );

*choice=0;
while(*choice==0)
	Tcl_DoOneEvent(0);

cmd( "set tvar [ .da.a.c.v.t.e2 get ]" ); 
cmd( "set compvalue [ .da.a.c.v.c.e get ]" ); 
Tcl_UnlinkVar(inter, "compvalue");

if(*choice==2)
{
cmd( "destroytop .da.a" );
*choice=0;
goto there;
}

cmd( "set choice $ssys" );
cmd( ".da.vars.ch.v selection clear 0 end" );

if(*choice==2)
 {
 cmd( "set tot [.da.vars.ch.v get 0 end]" );
 cmd( "set myc 0; foreach i $tot { if { [lindex [split $i] 0] == \"$b\"} {  .da.vars.ch.v selection set $myc  } {}; incr myc}" );
 }
 
if(*choice== 1)
{
cmd( "set choice $cond" );
i=*choice;

*choice=-1;
cmd( "for {set x 0} {$x<$i} {incr x} {if {[.da.a.q.f.l.e$x get]!=\"\"} {set choice $x}}" );
if(*choice==-1)
{
cmd( "set tot [.da.vars.ch.v get 0 end]" );
cmd( "set myc 0; foreach i $tot { if { [lindex [split $i] 0] == \"$b\"} { .da.vars.ch.v selection set $myc }; incr myc }" );
}
else
 {
cmd( "set tot [.da.vars.ch.v get 0 end]" );
cmd( "if { [info exist vcell]==1} {unset vcell} {}" );
cmd( "set choice $ntag" );
for(j=0; j<*choice; j++)
 cmd( "lappend vcell $v%d", j );

switch(i) 
{
case 0:
 cmd( "set myc 0; foreach i $tot { if { [ lindex [ split $i ] 0 ] == \"$b\" } { set c 1; for { set x 0 } { $x < $ntag } { incr x } { if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] == [ lindex [ split [ lindex [ split $i ] 1 ] {_}] $x ] } { set c 0 } }; if $c { .da.vars.ch.v selection set $myc } }; incr myc }" );  
 break;
case 1:
 cmd( "set myc 0; foreach i $tot { if { [ lindex [ split $i ] 0 ] == \"$b\" } { set c 1; for { set x 0 } { $x < $ntag } { incr x } { if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] != [ lindex [ split [ lindex [ split $i ] 1 ] {_}] $x ] } { set c 0 } }; if $c { .da.vars.ch.v selection set $myc } }; incr myc }" );  
 break;
case 2:
 cmd( "set myc 0; foreach i $tot { if { [ lindex [ split $i ] 0 ] == \"$b\" } { set c 1; for { set x 0 } { $x < $ntag } { incr x } { if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] > [ lindex [ split [ lindex [ split $i ] 1 ] {_}] $x ] } { set c 0 } }; if $c { .da.vars.ch.v selection set $myc } }; incr myc }" );  
 break;
case 3:
 cmd( "set myc 0; foreach i $tot { if { [ lindex [ split $i ] 0 ] == \"$b\" } { set c 1; for { set x 0 } { $x < $ntag } { incr x } { if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] >= [ lindex [ split [ lindex [ split $i ] 1 ] {_}] $x ] } { set c 0 } }; if $c { .da.vars.ch.v selection set $myc } }; incr myc }" );  
 break;
case 4:
 cmd( "set myc 0; foreach i $tot { if { [ lindex [ split $i ] 0 ] == \"$b\" } { set c 1; for { set x 0 } { $x < $ntag } { incr x } { if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] < [ lindex [ split [ lindex [ split $i ] 1 ] {_}] $x ] } { set c 0 } }; if $c { .da.vars.ch.v selection set $myc } }; incr myc }" );  
 break;
case 5:
 cmd( "set myc 0; foreach i $tot { if { [ lindex [ split $i ] 0 ] == \"$b\" } { set c 1; for { set x 0 } { $x < $ntag } { incr x } { if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] <= [ lindex [ split [ lindex [ split $i ] 1 ] {_}] $x ] } { set c 0 } }; if $c { .da.vars.ch.v selection set $myc } }; incr myc }" );  
 break;
} 
}
*choice=0;
}

if ( *choice == 3 || *choice == 4 )
{
l = *choice;
cmd( "set choice $cond" );
p=*choice;
cmd( "set tot [.da.vars.ch.v get 0 end]" );
cmd( "set choice [llength $tot]" );
j=*choice;

if ( l == 3 )
	app = ( char * ) Tcl_GetVar( inter, "b", 0 );
else
	app = ( char * ) Tcl_GetVar( inter, "svar", 0 );

strcpy(str3,app);

cmd( "set choice $tvar" );
h=*choice;

for(i=0; i<j; i++)
 {
  cmd( "set res [lindex $tot %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str1, str2, &l, &m, &k);
  if(h>=l && h<=m && !strcmp(str1, str3))
   {
   datum=find_data(k);
   r=0;
   if(is_finite(datum[h]))		// ignore NaNs
    switch(p)
    {
    case 0: if(datum[h]!=compvalue) r=1;
    break;
    case 1: if(datum[h]==compvalue) r=1;
    break;
    case 2: if(datum[h]>=compvalue) r=1;
     break;
    case 3: if(datum[h] > compvalue) r=1;
     break;
    case 4: if(datum[h]<=compvalue) r=1;
     break;
    case 5: if(datum[h] < compvalue) r=1;
     break;
        
    }
    if(r==1)
     { 
      cmd( "set templ $tot" );
      cmd( "set choice [lsearch $templ \"$b %s *\"]", str2 );

      while(*choice>=0)
       {
       cmd( ".da.vars.ch.v selection set $choice" );
       cmd( "set templ [lreplace $templ $choice $choice]" );
       }
     }
   }
 }
}

cmd( "destroytop .da.a" );

cmd( "if { ! $selOnly } { set steps 0; foreach i [ .da.vars.ch.v curselection ] { .da.vars.ch.v delete [ expr $i - $steps ]; incr steps} } { if { [ llength [ .da.vars.ch.v curselection ] ] > 0 } { .da.vars.ch.v see [ lindex [ .da.vars.ch.v curselection ] 0 ] } }" );

cmd( ".da.com.selec conf -text \"Series = [ .da.vars.ch.v size ]\"" );

*choice=0;
goto there;


// PLOTS LIST CONTEXT ACTIONS

//remove a plot
case 20:
cmd( "set answer [tk_messageBox -parent .da -type yesno -title Confirmation -message \"Delete plot?\" -detail \"Press 'Yes' to delete plot:\\n$tit\" -icon question -default yes]" );
cmd( "if {[string compare $answer yes] == 0} { set choice 1} {set choice 0}" );
if(*choice==0)
 goto there;
cmd( "scan $it %d)%s a b" );
cmd( "set ex [winfo exists .da.f.new$a]" );
cmd( "if { $ex == 1 } {destroy .da.f.new$a; .da.vars.pl.v delete $n_it} {.da.vars.pl.v delete $n_it}" );

cmd( ".da.com.plot conf -text \"Plots = [ .da.vars.pl.v size ]\"" );

*choice=0;
goto there;


//Raise the clicked plot
case 3:
  cmd( "scan $it %d)%s a b" );
  cmd( "set ex [winfo exists .da.f.new$a]" );
  *choice=0;
  cmd( "if { $ex == 1 } { wm deiconify .da.f.new$a; raise .da.f.new$a; focus .da.f.new$a } { set choice 1 }" );
  if(*choice==1)
   {
   getcwd(dirname, MAX_PATH_LENGTH - 1);
   cmd( "set choice $a" );
   sprintf(msg, "plotxy_%d",*choice);
   chdir(msg);
   cmd( "set choice $a" );
   show_plot_gnu(*choice, choice, 0);
   chdir(dirname);
   }

  miny=maxy=0;

  *choice=0;
  goto there;

  
// SERIES TOOLBAR OPTIONS

// Sort
case 5:
  cmd( "set a [.da.vars.lb.v get 0 end]" );
  cmd( "set choice [llength $a]" );
  if(*choice==0)
    goto there;
  cmd( "set b {}" );
  cmd( "set b [lsort -command comp_und $a]" );		// use special sort procedure to keep underscores at the end
  cmd( ".da.vars.lb.v delete 0 end" );
  cmd( "foreach i $b {.da.vars.lb.v insert end $i}" );
  *choice=0;
  goto there;
  
  
// Sort (descending order)
case 38:
  cmd( "set a [.da.vars.lb.v get 0 end]" );
  cmd( "set choice [llength $a]" );
  if(*choice==0)
    goto there;
  cmd( "set b {}" );
  cmd( "set b [lsort -decreasing -dictionary $a]" );
  cmd( ".da.vars.lb.v delete 0 end" );
  cmd( "foreach i $b {.da.vars.lb.v insert end $i}" );
  *choice=0;
  goto there;
 

//sort the selection in selected series list in inverse order
case 34:
   cmd( "set a [.da.vars.ch.v curselection]" );
   cmd( "set choice [llength $a]" );
   if(*choice==0)
    goto there;
   cmd( "set b {}" );
   cmd( "foreach i $a {lappend b [.da.vars.ch.v get $i]}" );
   cmd( "set c [lsort -decreasing -dictionary $b]" );
   cmd( "set d -1" );
   cmd( "foreach i $a {.da.vars.ch.v delete $i; .da.vars.ch.v insert $i [lindex $c [incr d]] }" );
   *choice=0;
   goto there;

   
// Unsort
case 14:
   cmd( "set a [.da.vars.lb.v get 0 end]" );
   cmd( "set choice [llength $a]" );
   if(*choice==0)
    goto there;
   cmd( ".da.vars.lb.v delete 0 end" );
   for(i=0; i<num_var; i++)
     cmd( ".da.vars.lb.v insert end \"%s %s (%d - %d) # %d\"", vs[i].label, vs[i].tag, vs[i].start, vs[i].end, vs[i].rank );
   *choice=0;
   goto there;

   
// Sort on End
case 15:
   cmd( "set a [.da.vars.lb.v get 0 end]" );
   cmd( "set choice [llength $a]" );
   if(*choice==0)
    goto there;
   app_store=new store[num_var];
   for(i=0; i<num_var; i++)
     app_store[i]=vs[i];
   sort_on_end(app_store);
   cmd( ".da.vars.lb.v delete 0 end" );
   for(i=0; i<num_var; i++)
    cmd( ".da.vars.lb.v insert end \"%s %s (%d - %d) # %d\"", app_store[i].label, app_store[i].tag, app_store[i].start, app_store[i].end, app_store[i].rank );
    delete[] app_store;
   *choice=0;
   goto there;

   
// Find first instance of series in the available series listbox
case 39:
cmd( "set a [.da.vars.lb.v get 0 end]" );
cmd( "set choice [llength $a]" );
if(*choice==0)
 goto there;

cmd( "set srchTxt \"\"; set srchInst 1" );
cmd( "newtop .da.a \"Find Series\" { set choice 2 } .da" );
cmd( "label .da.a.l -text \"Series name (or part)\"" );
cmd( "entry .da.a.e -textvariable srchTxt -width 20 -justify center" );
cmd( "bind .da.a.e <KeyRelease> {if { %%N < 256 && [info exists DaModElem] } { set bb1 [.da.a.e index insert]; set bc1 [.da.a.e get]; set bf1 [lsearch -glob $DaModElem $bc1*]; if { $bf1 !=-1 } {set bd1 [lindex $DaModElem $bf1]; .da.a.e delete 0 end; .da.a.e insert 0 $bd1; .da.a.e index $bb1; .da.a.e selection range $bb1 end } } }" );
cmd( "label .da.a.n -text \"(finds first instance only,\nuse 'F3' or 'Ctrl+N' to find others)\"" );
cmd( "pack .da.a.l .da.a.e .da.a.n -pady 5 -padx 5" );
cmd( "okcancel .da.a b  { set choice 1 } { set choice 2 }" );
cmd( "bind .da.a.e <Return> { set choice 1 }" );
cmd( "bind .da.a.e <Escape> { set choice 2 }" );
cmd( "showtop .da.a centerW" );
cmd( "focus .da.a.e" );

*choice = 0;
while ( *choice == 0 )
	Tcl_DoOneEvent( 0 );

cmd( "destroytop .da.a" );

if ( *choice == 1 )
{
	cmd( "set choice [ string length $srchTxt ]" );
	if ( *choice == 0 )
		goto there;
	
	cmd( "set srchTxt [ string tolower $srchTxt ]" );
	cmd( "set a [ .da.vars.lb.v get 0 end ]; set i 0" );
	cmd( "foreach b $a { set choice [ string first $srchTxt [ string tolower [ lindex [ split $b ] 0 ] ] ]; if { $choice != -1 } { break } { set i [ expr $i + 1 ] } }" );
	if ( *choice != -1 )
	cmd( ".da.vars.lb.v selection clear 0 end; .da.vars.lb.v selection set $i; .da.vars.lb.v see $i" );
	else
		cmd( "tk_messageBox -parent .da -type ok -title \"Error\" -icon error -default ok -message \"Series not found\" -detail \"Check if the name was spelled correctly or use just part of the name. This command is case insensitive.\"" );
}

*choice=0;
goto there;
  
  
// Find subsequent instances of a series in the available series listbox
case 40:
cmd( "set choice [ string length $srchTxt ]" );
if ( *choice == 0 )
	goto there;

cmd( "set a [ .da.vars.lb.v get 0 end ]; set i 0; set inst 0" );
cmd( "foreach b $a { set choice [ string first $srchTxt [ string tolower [ lindex [ split $b ] 0 ] ] ]; if { $choice != -1 } { set inst [ expr $inst + 1 ]; if { $inst > $srchInst } { set srchInst $inst; break } }; set i [ expr $i + 1 ] }" );
if ( *choice != -1 )
	cmd( ".da.vars.lb.v selection clear 0 end; .da.vars.lb.v selection set $i; .da.vars.lb.v see $i" );
else
	cmd( "tk_messageBox -parent .da -type ok -title \"Error\" -icon error -default ok -message \"Series not found\" -detail \"No additional instance of series found.\"" );

*choice=0;
goto there;

  
// Insert the variables selected in the list of the variables to plot
case 6:
  cmd( "set a [.da.vars.lb.v curselection]" );
  cmd( "foreach i $a {.da.vars.ch.v insert end [.da.vars.lb.v get $i]}" );
  *choice=0;
  cmd( "set tit [.da.vars.ch.v get 0]" );

  cmd( ".da.com.selec conf -text \"Series = [ .da.vars.ch.v size ]\"" );
  goto there;

  
//Remove the vars. selected from the variables to plot
case 7:

  cmd( "set steps 0" );
  cmd( "foreach i [.da.vars.ch.v curselection] {.da.vars.ch.v delete [expr $i - $steps]; incr steps}" );
  cmd( "if [.da.vars.ch.v size]==0 {$f.out conf -state disabled}" );
  *choice=0;

  cmd( ".da.com.selec conf -text \"Series = [ .da.vars.ch.v size ]\"" );
  goto there;

  
//Remove all the variables from the list of vars to plot
case 8:
  cmd( ".da.vars.ch.v delete 0 end" );
  *choice=0;

  cmd( ".da.com.selec conf -text \"Series = [ .da.vars.ch.v size ]\"" );
  goto there;


case 24:
/*
Insert new series (from disk or combining existing series).
*/
cmd( "set choice [.da.vars.ch.v size]" );
if ( *choice > 0 )
{
	cmd( "newtop .da.s \"Choose Data Source\" { set choice 2 } .da" );
	cmd( "label .da.s.l -text \"Source of additional series\"" );

	cmd( "set bidi 4" );
	cmd( "frame .da.s.i -relief groove -bd 2" );
	cmd( "radiobutton .da.s.i.c -text \"Create new series from selected\" -variable bidi -value 4" );
	cmd( "radiobutton .da.s.i.a -text \"Moving average series from selected\" -variable bidi -value 5" );
	cmd( "radiobutton .da.s.i.f -text \"File(s) of saved results\" -variable bidi -value 1" );
	cmd( "pack .da.s.i.c .da.s.i.a .da.s.i.f -anchor w" );

	cmd( "pack .da.s.l .da.s.i -expand yes -fill x -pady 5 -padx 5" );
	cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp mdatares.html#add_series } { set choice 2 }" );
	cmd( "showtop .da.s centerW" );

	*choice = 0;
	while ( *choice == 0 )
	  Tcl_DoOneEvent( 0 );

	cmd( "destroytop .da.s" );

	if ( *choice == 2 )
	{
		*choice = 0;
		goto there;
	}

	cmd( "set choice $bidi" );
	switch ( *choice )
	{
		case 4:
			*choice = 0;
			create_series( choice );
			*choice = 0;
			goto there;
		case 5:
			*choice = 0;
			create_maverag( choice );
			*choice = 0;
			goto there;
		case 1:
			*choice = 1;
	}
}
else
	*choice = 1;

if ( *choice == 1 )
{
	bool gz = false;
#ifdef LIBZ
	const char extRes[] = ".res .res.gz";
	const char extTot[] = ".tot .tot.gz";
#else
	const char extRes[] = ".res";
	const char extTot[] = ".tot";
#endif 
 
  cmd( "set lab [tk_getOpenFile -parent .da -title \"Load Results File\" -multiple yes -initialdir [pwd] -filetypes {{{Lsd result files} {%s}} {{Lsd total files} {%s}} {{All files} {*}} }]", extRes, extTot );
  
  cmd( "set choice [llength $lab]" );
  if(*choice==0 )
   {//no file selected
    goto there; 
   }
  h=*choice;
  
  for(i=0; i<h; i++)  
  {
  cmd( "set datafile [lindex $lab %d]", i );
  app=(char *)Tcl_GetVar(inter, "datafile",0);
  strcpy(filename,app);
  
#ifdef LIBZ
  if ( strlen( filename ) > 3 && ! strcmp( &filename[ strlen( filename ) - 3 ], ".gz" ) )
	  gz = true;
#endif
  
  f = fopen( filename, "r" );

  if ( f != NULL )
   {
	fclose(f);
    file_counter++;
    insert_data_file(gz, &num_var, &num_c);
    cmd( ".da.com.nvar conf -text \"Series = %d\"", num_var );
    cmd( ".da.com.ncas conf -text \"Cases = %d\"", num_c );
    }
   else
	plog( "\nError: could not open file: %s\n", "", filename );
  }
}

*choice=0;
goto there;


// MAIN MENU ACTIONS

// open Gnuplot
case 4:
	cmd( "if { $tcl_platform(platform) == \"unix\" } { \
			set answer [ catch { exec xterm -e gnuplot & } ] \
		} { \
			if { $tcl_platform(os) == \"Windows NT\" } { \
				set answer [ catch { exec wgnuplot & } ] \
			} { \
				set answer [ catch { exec start wgnuplot & } ] \
			} \
		}; \
		if { $answer != 0 } { \
			tk_messageBox \
				-parent .da \
				-type ok \
				-icon error \
				-title Error \
				-message \"Gnuplot failed to launch\" \
				-detail \"Gnuplot returned error '$answer' during setup.\nPlease check if Gnuplot is set up properly.\" \
		}" );
	*choice=0;
	goto there;

  
//set options for gnuplot
case 37: 
cmd( "newtop .da.a \"Gnuplot Options\" { set choice 2 } .da" );
cmd( "label .da.a.l -text \"Options for invoking Gnuplot\"" );

cmd( "set gptermTmp $gpterm" );
cmd( "frame .da.a.t -bd 2" );
cmd( "label .da.a.t.l -text \"Terminal\"" );
cmd( "entry .da.a.t.e -textvariable gptermTmp -width 12 -justify center" );
cmd( "pack .da.a.t.l .da.a.t.e -side left" );

cmd( "set gpdgrid3dTmp $gpdgrid3d" );
cmd( "frame .da.a.d -bd 2" );
cmd( "label .da.a.d.l -text \"3D grid configuration\"" );
cmd( "entry .da.a.d.e -textvariable gpdgrid3dTmp -width 12 -justify center" );
cmd( "pack .da.a.d.l .da.a.d.e -side left" );

cmd( "frame .da.a.o -relief groove -bd 2" );
cmd( "label .da.a.o.l -text \"Other options\"" );
cmd( "text .da.a.o.t -height 10 -width 45" );
cmd( ".da.a.o.t insert end \"$gpooptions\"" );
cmd( "pack .da.a.o.l .da.a.o.t" );

cmd( "pack .da.a.l .da.a.t .da.a.d .da.a.o -pady 5 -padx 5" );
cmd( "okXhelpcancel .da.a b  { Default } { set choice 3 } { set choice 1 } { LsdHelp mdatares.html#gpoptions } { set choice 2 }" );

cmd( "showtop .da.a centerW" );

gpoptions:
*choice=0;
while(*choice==0)
	Tcl_DoOneEvent(0);

if(*choice==3)
 {
  cmd( "set gpdgrid3dTmp \"60,60,3\"" );
  cmd( ".da.a.o.t delete 1.0 end; .da.a.o.t insert end \"set ticslevel 0.0\"" );
  cmd( "if { $tcl_platform(platform) == \"windows\"} {set gptermTmp \"windows\"} { set gptermTmp \"x11\"}" );
  goto gpoptions;
 }
 
if ( *choice == 1 )
{
	cmd( "set gpterm $gptermTmp" );
	cmd( "set gpdgrid3d $gpdgrid3dTmp" );
	cmd( "set gpooptions [.da.a.o.t get 0.0 end]" ); 
}

cmd( "destroytop .da.a" );
*choice=0;
goto there;
 
 
case 21:		//set personal colors

cmd( "newtop .da.a \"Set Colors\" { set choice 2 } .da" );

cmd( "label .da.a.l -text \"Pick the color to change\"" );

cmd( "frame .da.a.o" );

cmd( "frame .da.a.o.l1" );
for ( i = 0; i < 10; ++i )
{
	cmd( "frame .da.a.o.l1.c%d", i );
	cmd( "label .da.a.o.l1.c%d.n -text %d", i, i + 1 );
	cmd( "label .da.a.o.l1.c%d.c -text \"\u2588\u2588\u2588\u2588\" -fg $c%d", i, i );
	cmd( "bind .da.a.o.l1.c%d.c <Button-1> { set n_col %d; set col $c%d; set choice 1 }", i, i, i );
	cmd( "pack .da.a.o.l1.c%d.n .da.a.o.l1.c%d.c -side left", i, i );
	cmd( "pack .da.a.o.l1.c%d -anchor e", i );
}
 
cmd( "frame .da.a.o.l2" );
for ( i = 0; i < 10; ++i )
{
	cmd( "frame .da.a.o.l2.c%d", i );
	cmd( "label .da.a.o.l2.c%d.n -text %d", i, i + 11 );
	cmd( "label .da.a.o.l2.c%d.c -text \"\u2588\u2588\u2588\u2588\" -fg $c%d", i, i + 10 );
	cmd( "bind .da.a.o.l2.c%d.c <Button-1> { set n_col %d; set col $c%d; set choice 1 }", i, i + 10, i + 10 );
	cmd( "pack .da.a.o.l2.c%d.n .da.a.o.l2.c%d.c -side left", i, i );
	cmd( "pack .da.a.o.l2.c%d -anchor e", i );
}

cmd( "pack .da.a.o.l1 .da.a.o.l2 -side left -pady 5 -padx 5" );
cmd( "pack .da.a.l .da.a.o -pady 5 -padx 5" );

cmd( "done .da.a b { set choice 2 }" );
cmd( "showtop .da.a centerW" );

set_col:
*choice = 0;
while ( *choice == 0 )
	Tcl_DoOneEvent( 0 );

if ( *choice == 1 )
{
	cmd( "set a [ tk_chooseColor -initialcolor $col ]" );
	cmd( "if { $a != \"\" } { set choice 1 } { set choice 0 }" );
	if ( *choice == 1 )
	{
		cmd( "set c$n_col $a" );
		cmd( "if { $n_col >= 10 } { set fr 2; set n_col [expr $n_col - 10 ] } { set fr 1 }" );
		cmd( ".da.a.o.l$fr.c$n_col.c configure -fg $a" );
	}
	goto set_col;
}

cmd( "destroytop .da.a" );

*choice = 0;
goto there;


case 22:
//set default colors
init_canvas();
*choice=0;
goto there;


//help on Analysis of Result
case 41:
cmd( "LsdHelp mdatares.html" );
*choice=0;
goto there;


case 44:
//see model report

sprintf(name_rep, "report_%s.html", simul_name);
cmd( "set choice [file exists %s]", name_rep );
if(*choice == 0)
 {
  cmd( "set answer [tk_messageBox -parent .da -message \"Model report not found\" -detail \"You may create a model report file from menu Model or press 'Ok' to look for another HTML file.\" -type okcancel -title Warning -icon warning -default cancel]" );
  cmd( "if {[string compare -nocase $answer \"ok\"] == 0} {set choice 1} {set choice 0}" );
 if(*choice == 0)
  goto there;
 cmd( "set fname [tk_getOpenFile -parent .da -title \"Load Report File\" -defaultextension \".html\" -initialdir [pwd] -filetypes {{{HTML files} {.html}} {{All files} {*}} }]" );
 cmd( "if {$fname == \"\"} {set choice 0} {set choice 0}" );
 if(*choice == 0)
  goto there;

 }
else
 cmd( "set fname %s", name_rep );

if(*choice==1) //model report exists
  cmd( "LsdHtml $fname" );
  
*choice=0;
goto there;


// CANVAS OPTIONS

//Edit labels
case 26:
*choice=0;
 cmd( "toplevel .da.a" );
 cmd( "wm title .da.a \"Edit Label\"" );
 cmd( "wm geometry .da.a +$hereX+$hereY" );
 cmd( "wm transient .da.a $ccanvas" );
 cmd( "label .da.a.l -text \"New label for the selected label\"" );
 cmd( "set itext [$ccanvas itemcget selected -text]" );
 cmd( "entry .da.a.e -textvariable itext -width 30" );
 cmd( "frame .da.a.format -relief groove -bd 2" );
 cmd( "label .da.a.format.tit -text \"Font name, size, style and color\"" );
 cmd( "pack .da.a.format.tit" );
 cmd( "set ifont [lindex [$ccanvas itemcget selected -font] 0]" );
 cmd( "set idim [lindex [$ccanvas itemcget selected -font] 1]" );
 cmd( "set istyle [lindex [$ccanvas itemcget selected -font] 2]" );
 cmd( "frame .da.a.format.e" );
 cmd( "entry .da.a.format.e.font -textvariable ifont -width 30" );
 cmd( "entry .da.a.format.e.dim -textvariable idim -width 4 -justify center" );
  cmd( "entry .da.a.format.e.sty -textvariable istyle -width 10 -justify center" );
 cmd( "set icolor [$ccanvas itemcget selected -fill]" );  
 cmd( "button .da.a.format.e.color -width -9 -text Color -background white -foreground $icolor -command {set app [tk_chooseColor -initialcolor $icolor]; if { $app != \"\"} {set icolor $app} {}; .da.a.format.e.color configure -foreground $icolor; focus .da.a.format.e.color}" );

 cmd( "bind .da.a.format.e.font <Return> {.da.a.b.ok invoke}" );
 cmd( "bind .da.a.format.e.dim <Return> {.da.a.b.ok invoke}" );
 
 cmd( "pack .da.a.format.e.font .da.a.format.e.dim .da.a.format.e.sty .da.a.format.e.color -side left" );

 cmd( "frame .da.a.format.fall" );
 cmd( "set fontall 0" );
 cmd( "set colorall 0" );
 cmd( "checkbutton .da.a.format.fall.font -text \"Apply font to all text items\" -variable fontall" );
 cmd( "checkbutton .da.a.format.fall.color -text \"Apply color to all text items\" -variable colorall" );

 cmd( "pack .da.a.format.fall.font .da.a.format.fall.color -anchor w" );
 cmd( "pack .da.a.format.e .da.a.format.fall -anchor w" );

 cmd( "frame .da.a.b" );
 
 cmd( "button .da.a.b.ok -width -9 -text Ok -command {set choice 1}" );
 cmd( "button .da.a.b.esc -width -9 -text Cancel -command {set choice 2}" );
 cmd( "pack .da.a.b.ok .da.a.b.esc -side left" );
 cmd( "pack .da.a.l .da.a.e .da.a.format .da.a.b" );
 cmd( ".da.a.e selection range 0 end" );
 cmd( "focus .da.a.e" );
 cmd( "bind .da.a.e <Return> {.da.a.b.ok invoke}" );
 
  while(*choice==0)
	Tcl_DoOneEvent(0);

if(*choice==1)
 {
  cmd( " if { $itext==\"\"} {$ccanvas delete selected; set choice 0} {}" );
  if(*choice==1)
  {
   cmd( "$ccanvas itemconf selected -text \"$itext\"" );
   cmd( "set ml [list $ifont $idim $istyle]" );
   cmd( "$ccanvas itemconf selected -font \"$ml\"" );
   cmd( "$ccanvas itemconf selected -fill $icolor" );  
  }
  cmd( "set choice $fontall" );
  if(*choice==1)
   cmd( "$ccanvas itemconf text -font \"$ml\"" );
  cmd( "set choice $colorall" );
  if(*choice==1)
   cmd( "$ccanvas itemconf text -fill $icolor" );
 
 } 
 
cmd( "$ccanvas dtag selected" ); 
cmd( "destroy .da.a" ); 
cmd( "focus $ccanvas" );
 *choice=0;
 goto there;


//New labels
case 27:
*choice=0;
 cmd( "toplevel .da.a" );
 cmd( "wm geometry .da.a +$hereX+$hereY" );
 cmd( "wm title .da.a \"New Labels\"" );
 cmd( "wm transient .da.a $ncanvas" );
 cmd( "label .da.a.l -text \"New label\"" );
 cmd( "set itext \"new text\"" );
 cmd( "entry .da.a.e -textvariable itext -width 30" );
 cmd( "frame .da.a.b" );
 
 cmd( "button .da.a.b.ok -width -9 -text Ok -command {set choice 1}" );
 cmd( "button .da.a.b.esc -width -9 -text Cancel -command {set choice 2}" );
 cmd( "pack .da.a.b.ok .da.a.b.esc -side left" );
 cmd( "pack .da.a.l .da.a.e .da.a.b" );
 cmd( ".da.a.e selection range 0 end" );
 cmd( "focus .da.a.e" );
 cmd( "bind .da.a.e <Return> {.da.a.b.ok invoke}" );
 
  while(*choice==0)
	Tcl_DoOneEvent(0);
cmd( "destroy .da.a" ); 
if(*choice==2)
{

 *choice=0;
 goto there;
}

cmd( "set choice $ncanvas" );
i=*choice;
cmd( "set choice [.da.f.new%d.f.plots create text $LX $LY -text \"$itext\" -font {{Times} 10}]", i );

cmd( ".da.f.new%d.f.plots bind $choice <1> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; .da.f.new%d.f.plots raise current; set LX %%x; set LY %%y}", i,i,i,*choice,i );
cmd( ".da.f.new%d.f.plots bind $choice <ButtonRelease-1> {.da.f.new%d.f.plots dtag selected}",i,i);
cmd( ".da.f.new%d.f.plots bind $choice <B1-Motion> {.da.f.new%d.f.plots move selected [expr %%x-$LX] [expr %%y - $LY]; set LX %%x; set LY %%y}",i,i );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",i,i,i, *choice,i );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",i,i,i, *choice,i );
*choice=0;
goto there;

 
case 31:
/*
Edit line's color
*/
*choice=0;

cmd( "set icolor [$ccanvas itemcget $cline -fill]" );
cmd( "set app [tk_chooseColor -initialcolor $icolor]; if { $app != \"\"} {set icolor $app} {}" );
cmd( "$ccanvas itemconfig $cline -fill $icolor" );
cmd( "focus $ccanvas" );

goto there;
*choice=0;


case 28: 
/*insert a line item

NON ACTIVATED
*/
cmd( "set choice $ncanvas" );
cmd( "set c .da.f.new%d.f.plots", *choice );

cmd( "bind $c <B1-Motion> {$c delete selected; set ax %%x; set ay %%y; set cl [$c create line $px $py $ax $ay]; $c addtag selected withtag $cl}" );
cmd( "bind $c <ButtonRelease-1> {$c dtag selected}" );

cmd( "bind $c <1> {set px %%x; set py %%y; set ax [expr $px+1]; set ay [expr $py+1]; $c dtag selected; set cl [$c create line $px $py $ax $ay]; $c addtag selected withtag $cl }" );
*choice=0;
goto there;


default:
*choice=0;
 goto there;
}

}

/***************************************************
PLOT
****************************************************/
void plot(int *choice)
{

char *app;
char **str, **tag;
int idseries;

int logErrCnt = 0;				// log errors counter to prevent excess messages
bool stopErr = false;

int i, nv, j, k, *start, *end, done, doney2, color,  numy2;
double x1,x01, *y1, x2, x02, *y2,  cminy, cmaxy, miny2, maxy2, truemaxy2;
double step;
Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);
cmd( "set nv [.da.vars.ch.v size]" );
Tcl_UnlinkVar(inter, "nv");
double **data,**logdata;

if(nv>1000)
 {
  cmd( "set answer [tk_messageBox -parent .da -type okcancel -title \"Too Many Series\" -icon warning -default ok -message \"You selected too many ($nv) series to plot\" -detail \"So many series may cause a crash of the Lsd model program, with the loss of all data.\nIf you continue the system may become slow, please be patient.\nPress 'Ok' to continue anyway.\"]" );
  cmd( "if {[string compare $answer ok] == 0} {set choice 1 } {set choice 2}" );
  if(*choice==2)
   return;
 }
 
if ( nv == 0 )
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
	*choice = 2;
	return;
}

data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];
if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  if(autom_x ||(start[i]<=max_c && end[i]>=min_c))
   {

   data[i]=find_data(idseries);
   
   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!is_nan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
		 {
			plog( "\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d\n", "", i + 1, j );
		 }
		 else
			if ( ! stopErr )
			{
				plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
				stopErr = true;
			}
	   }
	 data[i]=logdata[i];				// replace the data series
   }
  }
 }

y1=new double[nv];
y2=new double[nv];

if(autom_x||min_c>=max_c)
{

for(i=0; i<nv; i++)
 {if(i==0)
   min_c=max_c=start[i];
  if(start[i]<min_c)
   min_c=start[i];
  if(end[i]>max_c)
   max_c=end[i]>num_c?num_c:end[i];
 }
}

cmd( "set choice $y2" );
if(*choice==1)
 {
  cmd( "set choice $numy2" );
  numy2=*choice;
  if(numy2>nv||numy2<2)
   numy2=nv+3;
 }
else
 numy2=nv+3; 
 
numy2--;
 
if(autom||miny>=maxy)
{

miny2=miny;
maxy2=maxy;
for(done=doney2=0, i=0; i<nv; i++)
 {
  if(i<numy2)
  {
  for(j=min_c; j<=max_c; j++)
   {
    if(done==0 && start[i]<=j && end[i]>=j && is_finite(data[i][j]))		// ignore NaNs
     {miny=maxy=data[i][j];
      done=1;
     }
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]<miny )		// ignore NaNs
     miny=data[i][j];
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]>maxy )		// ignore NaNs
     maxy=data[i][j];

   }
  }
  else
  {
  for(j=min_c; j<=max_c; j++)
   {
    if(doney2==0 && start[i]<=j && end[i]>=j && is_finite(data[i][j]))		// ignore NaNs
     {miny2=maxy2=data[i][j];
      doney2=1;
     }
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]<miny2 )		// ignore NaNs
     miny2=data[i][j];
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]>maxy2 )		// ignore NaNs
     maxy2=data[i][j];

   }
  }
 }

} //End for finding min-max


if(miny==maxy) //To avoid divisions by zero for constant values
 {if(miny==0)
   {maxy=0.1;
    miny=-0.1;
   }
  else
   {if(miny>0)
    {miny*=0.9;
     maxy*=1.1;
    }
    else
    {miny*=1.1;
     maxy*=0.9;
    }
   }
 }

if(miny2==maxy2) //To avoid divisions by zero for constant values
 {if(miny2==0)
   {maxy2=0.1;
    miny2=-0.1;
   }
  else
   {if(miny2>0)
    {miny2*=0.9;
     maxy2*=1.1;
    }
    else
    {miny2*=1.1;
     maxy2*=0.9;
    }
   }
 }

cmd( "write_disabled .da.f.h.v.sc.min.min $miny" );
cmd( "write_disabled .da.f.h.v.sc.max.max $maxy" );
cmd( "write_disabled .da.f.h.v.ft.from.mnc $minc" );
cmd( "write_disabled .da.f.h.v.ft.to.mxc $maxc" );

truemaxy=maxy;
maxy+=(maxy-miny)/100; //The f... Windows does not plot the very firt pixels...

if(numy2!=nv+2)
 {truemaxy2=maxy2;
  maxy2+=(maxy2-miny2)/100;
 } 
cmd( "set p .da.f.new%d", cur_plot );

cmd( "toplevel $p" );
cmd( "wm title $p $tit" );
cmd( "if {$tcl_platform(platform) != \"windows\"} {wm iconbitmap $p @$RootLsd/$LsdSrc/icons/lsd.xbm} {}" );
cmd( "wm protocol $p WM_DELETE_WINDOW { wm withdraw .da.f.new%d}", cur_plot );
cmd( "bind $p <Double-Button-1> { raise .da; focus .da.b.ts}" );
cmd( "frame $p.f -width 640 -height 430" );
cmd( "pack $p.f" );

//Reset p to point to the canvas (shit...)
cmd( "set p $p.f.plots" );

cmd( "canvas $p -width 640 -height 430 -background white -relief flat" );
cmd( "pack $p" );

cmd( "$p create line 40 300 640 300 -width 1 -fill grey50 -tag p" );

if(grid)
{
cmd( "$p create line 40 0 40 310 -fill grey60 -width 1p -tag p" );
cmd( "$p create line 190 0 190 310 -fill grey60 -width 1p -tag p" );
cmd( "$p create line 340 0 340 310 -fill grey60 -width 1p -tag p" );
cmd( "$p create line 490 0 490 310 -fill grey60 -width 1p -tag p" );
}
else
{
cmd( "$p create line 640 295 640 305  -width 1 -tag p" );
cmd( "$p create line 190 295 190 305 -width 1 -tag p" );
cmd( "$p create line 340 295 340 305 -width 1 -tag p" );
cmd( "$p create line 490 295 490 305 -width 1 -tag p" );
}


cmd( "set choice [$p create text 40 312 -font {Times 10 normal} -anchor nw -text %d -tag {p text}]", min_c );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );


cmd( "set choice [$p create text 190 312 -font {Times 10 normal} -anchor n -text %d -tag {p text}]", min_c+(max_c-min_c)/4 );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 340 312 -font {Times 10 normal} -anchor n -text %d -tag {p  text}]", min_c+(max_c-min_c)/2 );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 490 312 -font {Times 10 normal} -anchor n -text %d -tag {p text}]", min_c+(max_c-min_c)*3/4 );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );


cmd( "set choice [$p create text 635 312 -font {Times 10 normal} -anchor ne -text %d -tag {p  text}]", max_c );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "$p create line 40 0 40 300 -width 1 -fill grey50 -tag p" );

if(grid)
{
cmd( "$p create line 38 300 640 300 -fill grey60 -tag p" );
cmd( "$p create line 38 225 640 225 -fill grey60 -tag p" );
cmd( "$p create line 38 150 640 150 -fill grey60 -tag p" );
cmd( "$p create line 38 75 640 75 -fill grey60 -tag p" );
cmd( "$p create line 38 0 640 0 -fill grey60 -tag p" );
}
else
{
cmd( "$p create line 35 225 45 225  -tag p" );
cmd( "$p create line 35 150 45 150  -tag p" );
cmd( "$p create line 35 75 45 75  -tag p" );
cmd( "$p create line 35 2 45 2 -tag p" );
}

cmd( "set choice [$p create text 4 300 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,miny );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 225 -font {Times 10 normal} -anchor sw -text %.*g -tag {p  text}]", pdigits,(miny+(truemaxy-miny)/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 150 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,(miny+(truemaxy-miny)/2) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 75 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,(miny+(truemaxy-miny)*3/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 4 -font {Times 10 normal} -anchor nw -text %.*g -tag {p text}]", pdigits,(truemaxy) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

if(numy2!=nv+2)
 {
cmd( "set choice [$p create text 4 312 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,miny2 );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 237 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,(miny2+(truemaxy2-miny2)/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 162 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,(miny2+(truemaxy2-miny2)/2) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 87 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,(miny2+(truemaxy2-miny2)*3/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 16 -font {Times 10 normal} -anchor nw -text (%.*g) -tag {p text}]", pdigits,(truemaxy2) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
 
 }
cmd( "set xlabel 4" );
cmd( "set ylabel 0" );
for(i=0, j=4, k=0, color=0; i<nv && k<4; i++)
 { if(start[i]<=max_c && end[i]>=min_c)
  {
  cmd( "set app [font measure {Times 10 normal} \"%s_%s\"]", str[i],tag[i] );

  cmd( "if {[expr $xlabel + $app]>600} {set xlabel 4; set ylabel [expr $ylabel + 1]} {}" );
  cmd( "set app1 [expr 330+14*$ylabel]" );
  cmd( "$p create text $xlabel $app1 -font {Times 10 normal} -anchor nw -text %s_%s -tag {txt%d text} ", str[i], tag[i], i );
  cmd( "$p addtag p%d withtag txt%d",i,i );
  
//add the mobility and editability of the labels
  cmd( "$p bind txt%d <1> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag current; .da.f.new%d.f.plots raise current; set LX %%x; set LY %%y}",i, cur_plot, cur_plot, cur_plot );
  cmd( "$p bind txt%d <ButtonRelease-1> {.da.f.new%d.f.plots dtag selected}",i, cur_plot );
  cmd( "$p bind txt%d <B1-Motion> {.da.f.new%d.f.plots move selected [expr %%x-$LX] [expr %%y - $LY]; set LX %%x; set LY %%y}", i, cur_plot );
  cmd( "$p bind txt%d <Button-3> {set ccanvas .da.f.new%d.f.plots; .da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag txt%d; set hereX %%X ; set hereY %%y;set choice 26}", i, cur_plot, cur_plot, cur_plot,i );
  cmd( "$p bind txt%d <Button-2> {set ccanvas .da.f.new%d.f.plots; .da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag txt%d; set hereX %%X ; set hereY %%y;set choice 26}", i, cur_plot, cur_plot, cur_plot,i );
  cmd( "set xlabel [expr $xlabel+$app+4]" );
  cmd( "set choice $ylabel" );
  k=*choice;
  if(!allblack)
	cmd( "$p itemconf p%d -fill $c%d",i,color );
  color++;
  }
 }

*choice=0;

if(i<nv)
  { cmd( "set choice [$p create text $xlabel %d -font {Times 10 normal} -anchor nw -text (more) -tag {p%d text}]", 330+k*14, i );
    cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot );
    cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot );
 }

cmd( "update" );

step=600/(double)(max_c-min_c);

for(k=0; k<nv; k++)
  y2[k]=y1[k]=0;

for(x01=0,x1=40,i=min_c ;x1==40 ;i++ ) //, and record the first to be used
 {
  for(k=0; k<nv; k++)
   {if(start[k]<=i && end[k]>=i && is_finite(data[k][i]))		// ignore NaNs
      y1[k]+=data[k][i];
   }
  x1=40+(int)((double)(1+i)*step);
  x01++;
 }

x1=40;


for(k=0; k<nv; k++)
  {
  if(k<numy2)
   {
   y1[k]/=x01;
   if(y1[k]<miny)
    y1[k]=miny;
	 y1[k]=300-((y1[k]-miny)/(maxy-miny))*300;
   }
  else
   {
   y1[k]/=x01;
   if(y1[k]<miny2)
    y1[k]=miny2;
	 y1[k]=300-((y1[k]-miny2)/(maxy2-miny2))*300;
   }
  }

for(x02=0; i<=max_c; i++)
{
 x2=40+(int)((double)(1+i-x01-min_c)*step);
 x02++;                     //counter for the average when many values occupy one point
	for(k=0, color=-1; k<nv; k++)
	{
    if(k<numy2)
     {
     cminy=miny;
     cmaxy=maxy;
     }
    else
     {
     cminy=miny2;
     cmaxy=maxy2;
     }
      
    color++;
    if( (start[k]<i) && (end[k]>=i) && is_finite(data[k][i]))		// ignore NaNs
	  {y2[k]+=data[k][i];
		if(x1!=x2 )
		{y2[k]/=x02;
       if(y2[k]<cminy)
         y2[k]=cminy;
		 y2[k]=(300-((y2[k]-cminy)/(cmaxy-cminy))*300);
       if ( ! allblack && color < 1100 )
       {
       if(line_point==1)
        {
           cmd( "$p create line %d %d %d %d -width $point_size -tag p%d -fill $c%d",(int)x1,(int)y1[k],(int)x2,(int)y2[k], k, color );
        }
       else
        {
           cmd( "$p create oval %d %d %d %d -tag p%d -width 1 -fill $c%d -outline $c%d ",(int)x1 -(int)(point_size-1),(int)y1[k]-(int)(point_size-1),(int)x1+(int)(point_size-1),(int)y1[k]+(int)(point_size-1), k, color, color );
           cmd( "$p create oval %d %d %d %d -tag p%d -width 1 -fill $c%d -outline $c%d ",(int)x2-(int)(point_size-1),(int)y2[k]-(int)(point_size-1),(int)x2+(int)(point_size-1),(int)y2[k]+(int)(point_size-1), k, color, color );
        }
       }
       else
       {
		 cmd( "$p create line %d %d %d %d -tag p%d -width $point_size",(int)x1,(int)y1[k],(int)x2,(int)y2[k], k );
       }

		 y1[k]=y2[k];
		 y2[k]=0;
		 }
	  }
     else
      if(start[k]==i && is_finite(data[k][i]))		// ignore NaNs
       { //series entrying after the min_c

        y1[k]=(300-((data[k][i]-cminy)/(cmaxy-cminy))*300);
        if(x1==x2)
          y2[k]=data[k][i]*x02;
       }

   if(*choice==1) //Button STOP pressed
    {
     i=max_c;
    }
  
  }
 if(watch)
	cmd( "update" );
 if(x1!=x2 )
 { x1=x2;
	x02=0;
 }
 }

cmd( "$p create text 200 420 -font {Times 10 normal} -text \"Case num: \" -anchor w " );
if(logs)
 cmd( "$p create text 360 420 -font {Times 10 normal} -text \"log(Y) value: \" -anchor w " );
else
 cmd( "$p create text 380 420 -font {Times 10 normal} -text \"Y value: \" -anchor w " );

cmd( "bind $p <Motion> {.da.f.new%d.f.plots delete xpos%d; .da.f.new%d.f.plots delete ypos%d ;if {%%x>39 && %%y<301} {.da.f.new%d.f.plots create text 290 420 -font {Times 10 normal} -text [expr (%%x-40)*(%d-%d)/600+%d] -anchor w -tag xpos%d; .da.f.new%d.f.plots create text 490 420 -font {Times 10 normal} -text [expr double(round((10**$pdigits)*((300.0-%%y)*(%lf-%lf)/300+%lf)))/(10**$pdigits)] -tag ypos%d}}", cur_plot, cur_plot, cur_plot, cur_plot, cur_plot, max_c, min_c, min_c,cur_plot,cur_plot, maxy, miny, miny, cur_plot );

if(numy2!=nv+2)
{
cmd( "bind $p <Shift-Motion> {.da.f.new%d.f.plots delete xpos%d; .da.f.new%d.f.plots delete ypos%d ;if {%%x>39 && %%y<301} {.da.f.new%d.f.plots create text 290 420 -font {Times 10 normal} -text [expr (%%x-40)*(%d-%d)/600+%d] -anchor w -tag xpos%d; .da.f.new%d.f.plots create text 490 420 -font {Times 10 normal} -text [expr double(round((10**$pdigits)*((300.0-%%y)*(%lf-%lf)/300+%lf)))/(10**$pdigits)] -tag ypos%d}}", cur_plot, cur_plot, cur_plot, cur_plot, cur_plot, max_c, min_c, min_c,cur_plot,cur_plot, maxy2, miny2, miny2, cur_plot );
}
for(i=0; i<nv; i++)
{
cmd( "$p bind p%d <Enter> {.da.f.new%d.f.plots create text 5 420 -font {Times 10 normal} -text \"%s_%s\" -tag p%d -tag pp -anchor w}", i,cur_plot, str[i], tag[i],i );
cmd( "$p bind p%d <Leave> {.da.f.new%d.f.plots delete pp}", i,cur_plot );
cmd( "$p dtag txt%i p%d", i,i );
cmd( "$p bind txt%d <Enter> {.da.f.new%d.f.plots create text 5 420 -font {Times 10 normal} -text \"%s_%s\" -tag p%d -tag pp -anchor w}", i,cur_plot, str[i], tag[i],i );
cmd( "$p bind txt%d <Leave> {.da.f.new%d.f.plots delete pp}", i,cur_plot );

cmd( "$p bind p%d <Button-3> {set ccanvas .da.f.new%d.f.plots; set cline p%d; set hereX %%X; set hereY %%Y; set choice 31}", i,cur_plot,i );
cmd( "$p bind p%d <Button-2> {set ccanvas .da.f.new%d.f.plots; set cline p%d; set hereX %%X; set hereY %%Y; set choice 31}", i,cur_plot,i );

}
*choice=0;
cmd( "bind $p <Shift-1> {set ncanvas %d; set LX %%x; set LY %%y; set hereX %%X ; set hereY %%y; set choice 27}", cur_plot );

for(i=0; i<nv; i++)
 {delete[] str[i];
  delete[] tag[i];
  if(logs)
    delete[] logdata[i];
 }
delete[] str;
delete[] tag;
delete[] y1;
delete[] y2;

delete[] logdata;
delete[] data;
delete[] start;
delete[] end;
}


/***************************************************
PLOT_CROSS
****************************************************/
void plot_cross(int *choice)
{

int idseries;

char *app;
char **str;
char **tag;

int *start, *end, app1, app2, new_nv;
int *erase;
FILE *f;
int i, nv, j, *pos, k, nt, *list_times, h,  first;
double x1, y01, x2,  y02;
double val1,  step, **val, **data, **logdata, truemaxy;

int logErrCnt = 0;				// log errors counter to prevent excess messages
bool stopErr = false;

Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);
cmd( "set nv [.da.vars.ch.v size]" );
Tcl_UnlinkVar(inter, "nv");
if ( nv == 0 )
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
	*choice = 2;
	return;
}

Tcl_LinkVar(inter, "nt", (char *) &nt, TCL_LINK_INT);
cmd( "set nt $num_t" );
Tcl_UnlinkVar(inter, "nt");

if(nv<2 || nt==0)
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place at least two series in the Series Selected listbox and select at least one case.\"" );
	*choice = 2;
	return;
}

list_times=new int[nt];
cmd( "set k 0" );
Tcl_LinkVar(inter, "k", (char *) &k, TCL_LINK_INT);

for(i=0; i<nt; i++)    //Sets the list of cases to plot
 {
  cmd( "set k [lindex $list_times %d]", i );
  list_times[i]=k;
 }
Tcl_UnlinkVar(inter, "k");

pos=new int[nv];
val=new double *[nv];
start=new int[nv];
end=new int[nv];
erase = new int[nv];
str=new char *[nv];
tag=new char *[nv];
data=new double *[nv];
logdata=new double *[nv];
for(i=0, new_nv=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  for(k=0, erase[i]=0; k<nt; k++)
    if(list_times[k]<start[i] || list_times[k]>end[i])
        {erase[i]=1;
         k=nt;
        }
  if(erase[i]==0)
   {
    data[i]=find_data(idseries);
    if(data[i]==NULL)
      plog("\nError: invalid data\n");
   
    if(logs)			// apply log to the values to show "log scale"
    {
	  logdata[new_nv]=new double[end[i]+1];	// create space for the logged values
      for(j=start[i];j<=end[i];j++)		// log everything possible
	  if(!is_nan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	  else
	  {
		 logdata[i][j]=NAN;
		 if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
		 {
			plog( "\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d\n", "", i + 1, j );
		 }
		 else
			if ( ! stopErr )
			{
				plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
				stopErr = true;
			}
	  }
	  
	  data[i]=logdata[new_nv];				// replace the data series
    }
    val[new_nv]=new double[nt];
    new_nv++;
   }
  else
   data[i]=NULL;
 }

 for(j=0, first=1, i=0; j<nv; j++)
  {
	for(k=0; k<nt; k++)
	{if(erase[j]==0 && is_finite(data[j][list_times[k]]))		// ignore NaNs
	  {val[i][k]=data[j][list_times[k]];
     if(first==1)  //The first value is assigned to both min and max
      {miny=maxy=val[i][k];
       first=0;
      }
     if(miny>val[i][k])
      miny=val[i][k];
     if(maxy<val[i][k])
      maxy=val[i][k];

     }//End if of erase[]

	} //end for over times
    if(erase[j]==0)
     i++;
  } //End for (j on num_var)

delete[] data;

if(dir==-1)
  sort_cs_desc(str, tag, val, new_nv, nt, res);
if(dir==1)
  sort_cs_asc(str, tag, val, new_nv, nt, res);

if(miny==maxy)
 {miny*=0.75; //To avoid divisions by zero for constant values
  maxy*=1.25;
 }


cmd( "set miny %lf", miny );
cmd( "set maxy %lf", maxy );

truemaxy=maxy;
maxy+=(maxy-miny)/100; //The f... windows does not plot the very firt pixels...

cmd( "set p .da.f.new%d", cur_plot );
cmd( "toplevel $p" );
cmd( "wm title $p $tit" );
cmd( "if {$tcl_platform(platform) != \"windows\"} {wm iconbitmap $p @$RootLsd/$LsdSrc/icons/lsd.xbm} {}" );
cmd( "wm protocol $p WM_DELETE_WINDOW { wm withdraw .da.f.new%d}", cur_plot );
cmd( "bind $p <Double-Button-1> {raise .da; focus .da.b.ts}" );
cmd( "frame $p.f -width 640 -height 430" );
cmd( "pack $p.f" );

//Reset p to point to the canvas (shit...)
cmd( "set p $p.f.plots" );

cmd( "canvas $p -width 640 -height 430 -relief flat -background white" );
cmd( "pack $p" );

cmd( "$p create line 40 300 640 300 -width 1 -fill grey50 -tag p" );
if(grid)
{
cmd( "$p create line 40 0 40 310 -fill grey60 -tag p" );
cmd( "$p create line 190 0 190 310 -fill grey60 -tag p" );
cmd( "$p create line 340 0 340 310 -fill grey60 -tag p" );
cmd( "$p create line 490 0 490 310 -fill grey60 -tag p" );
}
else
{
cmd( "$p create line 640 295 640 305  -width 1 -tag p" );
cmd( "$p create line 190 295 190 305 -width 1 -tag p" );
cmd( "$p create line 340 295 340 305 -width 1 -tag p" );
cmd( "$p create line 490 295 490 305 -width 1 -tag p" );
}

cmd( "set app [$p create text 40 312 -font {Times 10 normal} -anchor nw -text \"%d series used\" -tag {p  text}]", new_nv );
cmd( "set choice $app" );

//add the mobility and editability of the labels
cmd( "$p bind %d <1> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag current; .da.f.new%d.f.plots raise current; set LX %%x; set LY %%y}", *choice, cur_plot, cur_plot, cur_plot );
cmd( "$p bind %d <ButtonRelease-1> {.da.f.new%d.f.plots dtag selected}", *choice, cur_plot );
cmd( "$p bind %d <B1-Motion> {.da.f.new%d.f.plots move selected [expr %%x-$LX] [expr %%y - $LY]; set LX %%x; set LY %%y}", *choice, cur_plot );
cmd( "$p bind %d <Button-3> {set ccanvas .da.f.new%d.f.plots; .da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set hereX %%X ; set hereY %%y;set choice 26}", *choice, cur_plot, cur_plot, cur_plot,*choice );
cmd( "$p bind %d <Button-2> {set ccanvas .da.f.new%d.f.plots; .da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set hereX %%X ; set hereY %%y;set choice 26}", *choice, cur_plot, cur_plot, cur_plot,*choice );

cmd( "$p create line 40 0 40 300 -width 1 -fill grey50 -tag p" );

if(grid)
{
cmd( "$p create line 38 300 640 300 -fill grey60 -tag p" );
cmd( "$p create line 38 225 640 225 -fill grey60 -tag p" );
cmd( "$p create line 38 150 640 150 -fill grey60 -tag p" );
cmd( "$p create line 38 75 640 75 -fill grey60 -tag p" );
cmd( "$p create line 38 0 640 0 -fill grey60 -tag p" );
}
else
{
cmd( "$p create line 35 225 45 225  -tag p" );
cmd( "$p create line 35 150 45 150  -tag p" );
cmd( "$p create line 35 75 45 75  -tag p" );
cmd( "$p create line 35 2 45 2 -tag p" );
}

cmd( "set choice [$p create text 4 300 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,miny );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 225 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,(miny+(truemaxy-miny)/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 150 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,(miny+(truemaxy-miny)/2) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 75 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,(miny+(truemaxy-miny)*3/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 4 -font {Times 10 normal} -anchor nw -text %.*g -tag {p text}]", pdigits,(truemaxy) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "update" );

step=600/(double)(new_nv-1);
for(i=0 ; i<new_nv-1; i++)
 {
  for(k=j=0; j<i || erase[k]==1; k++) //find the index for the labels
    if(erase[k]==0)
     j++;
  x1=40+(int)( (double)(i*step));
  x2=40+(int)( (double)((i+1)*step));
  for(j=0; j<nt; j++)
   {
    y01=300-((val[i][j]-miny)/(maxy-miny))*300;
    y02=300-((val[i+1][j]-miny)/(maxy-miny))*300;

   if(!allblack)
    {// draw colorful lines
    if(line_point==1)
      {
     	 cmd( "$p create line %d %d %d %d  -width $point_size -tag {p%d v%d_%d} -fill $c%d",(int)x1,(int)y01,(int)x2,(int)y02, j,i,j, j );
      }
    else
      {
     	 cmd( "$p create oval %d %d %d %d -tag {p%d v%d_%d} -fill $c%d -outline $c%d ",(int)x1,(int)y01,(int)x1,(int)y01, j,i,j, j, j );
     	 cmd( "$p create oval %d %d %d %d -tag {p%d v%d_%d} -fill $c%d -outline $c%d ",(int)x2,(int)y02,(int)x2,(int)y02, j,i,j, j, j );
      }
    }
    else
     {//draw only black lines
     if(line_point==1)
      {
     	 cmd( "$p create line %d %d %d %d  -width $point_size -tag {p%d v%d_%d}",(int)x1,(int)y01,(int)x2,(int)y02, j,i,j );
       }
    else
      {
     	 cmd( "$p create oval %d %d %d %d -tag {p%d v%d_%d}",(int)x1,(int)y01,(int)x1,(int)y01, j,i,j );
     	 cmd( "$p create oval %d %d %d %d -tag {p%d v%d_%d}",(int)x2,(int)y02,(int)x2,(int)y02, j,i,j );
      }
     }
    cmd( "$p bind v%d_%d <Enter> {.da.f.new%d.f.plots create text 70 400 -font {Times 10 normal} -text \"%s_%s in Order %d at time %d\" -tag pp -anchor w}",i,j,cur_plot, str[k], tag[k], i+1, list_times[j] );
    cmd( "$p bind v%d_%d <Leave> {.da.f.new%d.f.plots delete pp}", i, j, cur_plot );
   }

 }

 for(i=0; i<nt ; i++)
 {
  cmd( "$p bind p%d <Button-3> {set ccanvas .da.f.new%d.f.plots; set cline p%d; set hereX %%X; set hereY %%Y; set choice 31}",i,cur_plot,i );
  cmd( "$p bind p%d <Button-2> {set ccanvas .da.f.new%d.f.plots; set cline p%d; set hereX %%X; set hereY %%Y; set choice 31}",i,cur_plot,i );
 }  
 for(i=0, j=0, k=0; i<nt && i<20; i++)
 {
  cmd( "set app [$p create text %d %d -font {Times 10 normal} -anchor nw -text \"Time = [lindex $list_times %d]\" -tag {p%d text}]",4+j*100, 330+k*12, i, i );
  cmd( "set choice $app" );
  
  //add the mobility and editability of the labels
  cmd( "$p bind %d <1> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag current; .da.f.new%d.f.plots raise current; set LX %%x; set LY %%y}", *choice, cur_plot, cur_plot, cur_plot );
  cmd( "$p bind %d <ButtonRelease-1> {.da.f.new%d.f.plots dtag selected}", *choice, cur_plot );
  cmd( "$p bind %d <B1-Motion> {.da.f.new%d.f.plots move selected [expr %%x-$LX] [expr %%y - $LY]; set LX %%x; set LY %%y}", *choice, cur_plot );
  cmd( "$p bind %d <Button-3> {set ccanvas .da.f.new%d.f.plots; .da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set hereX %%X ; set hereY %%y;set choice 26}", *choice, cur_plot, cur_plot, cur_plot,*choice );
  cmd( "$p bind %d <Button-2> {set ccanvas .da.f.new%d.f.plots; .da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set hereX %%X ; set hereY %%y;set choice 26}", *choice, cur_plot, cur_plot, cur_plot,*choice );
  
  if(j<5)
	j++;
  else
	{k++;
	 j=0;
	}

  if(!allblack)
    cmd( "$p itemconf p%d -fill $c%d",i,i );
 }


cmd( "set lab \"\"" );

  cmd( "$p create text 5 400 -font {Times 10 normal} -text \"X value: \" -anchor w " );
if(logs)
  cmd( "$p create text 5 420 -font {Times 10 normal} -text \"log(Y) value: \" -anchor w " );
else
  cmd( "$p create text 5 420 -font {Times 10 normal} -text \"Y value: \" -anchor w " );

cmd( "bind $p <Motion> {.da.f.new%d.f.plots delete ypos%d ;if {%%x>39 && %%y<301} { .da.f.new%d.f.plots create text 130 420 -font {Times 10 normal} -text [expr double(round((10**$pdigits)*((300.0-%%y)*(%lf-%lf)/300+%lf)))/(10**$pdigits)] -tag ypos%d}}", cur_plot, cur_plot,cur_plot, maxy, miny, miny, cur_plot );


cmd( "bind $p <Shift-1> {set ncanvas %d; set LX %%x; set LY %%y; set hereX %%X ; set hereY %%y; set choice 27}", cur_plot );

for(i=0; i<nv; i++)
 {delete[] str[i];
  delete[] tag[i];
 }
for(i=0; i<new_nv; i++)
{
  delete[] val[i];
  if(logs)
    delete[] logdata[i];
}
delete[] str;
delete[] tag;
delete[] pos;
delete[] val;
delete[] start;
delete[] end;
delete[] erase;
delete[] logdata;
}


/***************************************************
INIT_CANVAS
****************************************************/
void init_canvas(void)
{
int i;
cmd( "set c0 black" );
cmd( "set c1 red" );
cmd( "set c2 green" );
cmd( "set c3 #d0d000" );
cmd( "set c4 #fb46bc" );
cmd( "set c5 blue" );
cmd( "set c6 DeepSkyBlue1" );
cmd( "set c7 grey40" );
cmd( "set c8 PaleTurquoise2" );
cmd( "set c9 cyan" );
cmd( "set c10 aquamarine1" );
cmd( "set c11 DarkSeaGreen1" );
cmd( "set c12 chartreuse1" );
cmd( "set c13 OliveDrab" );
cmd( "set c14 khaki3" );
cmd( "set c15 LightGoldenrod4" );
cmd( "set c16 sienna1" );
cmd( "set c17 chocolate4" );
cmd( "set c18 firebrick3" );
cmd( "set c19 orange1" );
cmd( "set c20 salmon3" );
cmd( "set c1000 white" );

for(i=21; i<1000; i++)
  cmd( "set c%d black", i );

for(i=0; i<100; i++)
  cmd( "set c1%03d grey%d", i,i );
  
cmd( "set c1000 white" );  
}

/***************************************************
SET_CS_DATA
****************************************************/
void set_cs_data(int *choice)
{

cmd( "set choice [.da.vars.ch.v size]" );
if ( *choice < 2 )
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No plot available\" -detail \"Place two or more series in the Series Selected listbox and select 'Plot' to produce a cross-section plot.\"" );
	*choice = 2;
	return;
}
	
cmd( "set bidi $maxc" );
cmd( "set res 0" );
cmd( "set dir 0" );
cmd( "set count 0" );
cmd( "set sfrom -1" );
cmd( "set sto -1" );

cmd( "toplevel .da.s" );
cmd( "wm protocol .da.s WM_DELETE_WINDOW {set choice 2}" );
cmd( "wm transient .da.s .da" );
cmd( "set p .da.s" );
cmd( "wm title $p \"Cases for Cross Section Analysis\"" );

Tcl_LinkVar(inter, "res", (char *) &res, TCL_LINK_INT);
Tcl_LinkVar(inter, "dir", (char *) &dir, TCL_LINK_INT);

cmd( "frame .da.s.i -relief groove -bd 2" );
cmd( "label .da.s.i.l -text \"Insert time steps to use\"" );
cmd( "entry .da.s.i.e -validate focusout -vcmd { if [ string is integer %%P ] { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.i.e insert 0 $bidi" ); 
cmd( "label .da.s.i.l1 -text \"Time steps selected\"" );
cmd( "listbox .da.s.i.lb" );
cmd( "pack .da.s.i.l .da.s.i.e .da.s.i.l1 .da.s.i.lb" );

cmd( "frame .da.s.fb -relief groove -bd 2" );
cmd( "set p .da.s.fb" );

cmd( "bind .da.s.i.e <KeyPress-Return> {$p.add invoke}" );
cmd( "button $p.add -width -9 -text Add -command {set bidi [ .da.s.i.e get ]; .da.s.i.lb insert end $bidi; incr count 1; focus .da.s.i.e; .da.s.i.e selection range 0 end; .da.s.i.lb selection set end }" );
cmd( "button $p.del -width -9 -text Delete -command {.da.s.i.lb delete [.da.s.i.lb curselection]; incr count -1; focus .da.s.i.e; .da.s.i.e selection range 0 end }" );
cmd( "button $p.end -width -9 -text Ok -command {set choice 1}" );
cmd( "button $p.help -width -9 -text Help -command {LsdHelp mdatares.html#crosssection}" );
cmd( "button $p.can -width -9 -text Cancel -command {set choice 2}" );
cmd( "pack $p.add $p.del $p.end $p.help $p.can -expand yes -fill x -anchor n" );

cmd( "frame .da.s.s -relief groove -bd 2" );
cmd( "label .da.s.s.l -text \"Use series in the same order as selected\"" );
cmd( "button .da.s.s.up -width -9 -text \"Sort Ascending\" -command {set res [.da.s.i.lb curselection]; .da.s.s.l config -text \"User series according to increasing values at time: [selection get]\"; set dir 1}" );
cmd( "button .da.s.s.down -width -9 -text \"Sort Descending\" -command {set res [.da.s.i.lb curselection]; .da.s.s.l config -text \"User series according to decreasing values at time: [selection get]\"; set dir -1}" );
cmd( "button .da.s.s.nosort -width -9 -text \"No Sort\" -command {.da.s.s.l config -text \"Use series in the same order as selected\"; set dir 0}" );
cmd( "pack .da.s.s.l .da.s.s.up .da.s.s.down .da.s.s.nosort -anchor n" );

cmd( "pack .da.s.i .da.s.fb .da.s.s -side left -anchor n" );
cmd( "focus .da.s.i.e; .da.s.i.e selection range 0 end" );
cmd( "set sfrom [format \"%%d\" $bidi]" );
cmd( "set sto [format \"%%d\" $bidi]" );
cmd( "set sskip 1" );

cmd( "bind .da.s <KeyPress-c> {set choice 1}; bind .da.s <KeyPress-C> {set choice 1}" );
cmd( "bind .da.s <Control-f> {set bidi [ .da.s.i.e get ]; set sfrom $bidi}; bind .da.s <Control-F> {set bidi [ .da.s.i.e get ]; set sfrom $bidi}" );
cmd( "bind .da.s <Control-t> {set bidi [ .da.s.i.e get ]; set sto $bidi}; bind .da.s <Control-T> {set bidi [ .da.s.i.e get ]; set sto $bidi}" );
cmd( "bind .da.s <Control-x> {set bidi [ .da.s.i.e get ]; set sskip $bidi}; bind .da.s <Control-X> {set bidi [ .da.s.i.e get ]; set sskip $bidi}" );
cmd( "bind .da.s <Control-z> { if { [expr $sto - $sfrom] > 0 } {for {set x $sfrom} {$x<$sto} {incr x $sskip} {	.da.s.i.lb insert end $x} } {}}; bind .da.s <Control-Z> { if { [expr $sto - $sfrom] > 0 } {for {set x $sfrom} {$x<$sto} {incr x $sskip} {	.da.s.i.lb insert end $x} } {}}" );

*choice=0;
while(*choice==0)
  Tcl_DoOneEvent(0);

if(*choice==2)
 goto end;

cmd( "if { [.da.s.i.lb size] == 0 } {tk_messageBox -parent .da.s -type ok -title Error -icon error -message \"No time step has been selected\" -detail \"No plot will be created.\";set choice 2} { }" );
cmd( "set num_t [.da.s.i.lb size]" );
cmd( "set list_times [.da.s.i.lb get 0 end]" );


end:
Tcl_UnlinkVar(inter, "res");
Tcl_UnlinkVar(inter, "dir");

cmd( "destroy .da.s" );
return;
}


/***************************************************
SORT_CS_DESC
****************************************************/
void sort_cs_desc(char **s,char **t, double **v, int nv, int nt, int c)
{
int i, j, h;
double dapp;
char sapp[MAX_ELEM_LENGTH];


for(i=nv-2; i>=0; i--)
 {
 for(j=i; j<nv-1 && v[j][c]<v[j+1][c] ; j++)
  {
   for(h=0;h<nt; h++)
    {
     dapp=v[j][h];
     v[j][h]=v[j+1][h];
     v[j+1][h]=dapp;
    }
   strcpy(sapp,s[j]);
   strcpy(s[j],s[j+1]);
   strcpy(s[j+1],sapp);

   strcpy(sapp,t[j]);
   strcpy(t[j],t[j+1]);
   strcpy(t[j+1],sapp);

//   printf("%d",j);
  }

 }
}

/***************************************************
SORT_CS_ASC
****************************************************/
void sort_cs_asc(char **s,char **t, double **v, int nv, int nt, int c)
{
int i, j, h;
double dapp;
char sapp[MAX_ELEM_LENGTH];


for(i=nv-2; i>=0; i--)
 {
 for(j=i; j<nv-1 && v[j][c]>v[j+1][c]; j++)
  {
   for(h=0;h<nt; h++)
    {
     dapp=v[j][h];
     v[j][h]=v[j+1][h];
     v[j+1][h]=dapp;
    }
   strcpy(sapp,s[j]);
   strcpy(s[j],s[j+1]);
   strcpy(s[j+1],sapp);

   strcpy(sapp,t[j]);
   strcpy(t[j],t[j+1]);
   strcpy(t[j+1],sapp);

  }

 }
}


/***************************************************
SEARCH_LAB_TIT
****************************************************/
double *search_lab_tit(object *r, char *s, char *t, int st, int en)
{
object *cur;
variable *cv;
double *res=NULL;
sprintf(msg, "%s_%s", s, t);
for(cur=r; cur!=NULL &&res==NULL; cur=cur->next)
 { for(cv=cur->v; cv!=NULL; cv=cv->next)
    {
     if(cv->save==1 && !strcmp(cv->lab_tit, msg)  && st==cv->start )
      return cv->data;
    }
  if(cur->b!=NULL)
   res=search_lab_tit(cur->b->head, s, t, st, en);
 }
if(res==NULL &&r->up==NULL)
 for(cv=cemetery; cv!=NULL &&res==NULL; cv=cv->next)
  {if(!strcmp(cv->lab_tit, msg) && st==cv->start)
     return cv->data;
  }
return res;
}


double *find_data(int id)
{
int i;

double *d, app;

return vs[id].data;

cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Series %d not found\" -detail \"To continue the very first series is returned.\"", id );
return vs[0].data;
}


/***************************************************
SEARCH_LAB_TIT_FILE
****************************************************/
double *search_lab_tit_file(char *s, char *t, int st, int en)
{
int i, done, app_st, app_en, j;
char str[MAX_ELEM_LENGTH], tag[MAX_ELEM_LENGTH];
double *d, app;
FILE *f;

for(i=0; i<num_var ; i++)
 { if(!strcmp(s, vs[i].label) && !strcmp(t, vs[i].tag) && vs[i].start==st && vs[i].end==en)
    return vs[i].data;

 }

sprintf( msg, "data not found in search_lab_tit_file" );
error_hard( msg, "Internal error", "If error persists, please contact developers." );
myexit(21);

f=fopen(filename, "r");
for(i=0, done=0; i<num_var; i++)
 {fscanf(f, "%99s %99s (%d %d)\t", str, tag, &app_st, &app_en);
  if(!strcmp(s, str) && !strcmp(tag, t ) && (app_st==st || app_st==-1))
    done=i;

 }


d=new double[num_c+1];

fscanf(f, "\n");
for(i=0; i<=num_c; i++)
  for(j=0; j<num_var; j++)
   if(j==done)
    fscanf(f,"%lf",&d[i+1]);
   else
    fscanf(f, "%lf", &app);
fclose(f);

return d;
}



/***************************************************
INSERT_LABELS_NOSAVE
****************************************************/
void insert_labels_nosave(object *r, char *lab,  int *num_v)
{
object *cur;
variable *cv;
bridge *cb;
int flag=0;

for(cur=r; cur!=NULL; cur=cur->next)
 {
  for(cv=cur->v; cv!=NULL; cv=cv->next)
   {
   if(cv->save==0 && !strcmp(cv->label,lab))
    {
     set_lab_tit(cv);
     cmd( ".da.vars.lb.v insert end \"%s %s (%d - %d) # %d\"", cv->label, cv->lab_tit, 0, 0, num_var + (*num_v) );
     cmd( ".da.vars.ch.v insert end \"%s %s (%d - %d) # %d\"", cv->label, cv->lab_tit, 0, 0, num_var + (*num_v) );

     *num_v+=1;
     flag=1;
    }
   } 

  for(cb=cur->b; cb!=NULL && cb->head!=NULL && flag==0; cb=cb->next) 
   insert_labels_nosave(cb->head, lab, num_v);
 }
}

/***************************************************
insert_data_nosave
****************************************************/
void insert_data_nosave(object *r, char *lab, int *num_v)
{
store *app;
int i;
insert_labels_nosave(r,lab, num_v);
app=new store[num_var+ *num_v];
for(i=0; i<num_var; i++)
 {app[i]=vs[i];
  strcpy(app[i].label, vs[i].label);
  strcpy(app[i].tag, vs[i].tag);
 } 
delete[] vs;
vs=app;

*num_v=num_var;
insert_store_nosave(r,lab, num_v);
num_var=*num_v;


cmd( ".da.com.nvar conf -text \"Series = %d\"", num_var );
}

/***************************************************
INSERT_STORE_NOSAVE
****************************************************/
void insert_store_nosave(object *r, char *lab, int *num_v)
{
object *cur;
variable *cv;
bridge *cb;

int flag=0;
for(cur=r; cur!=NULL; cur=cur->next)
 {for(cv=cur->v; cv!=NULL; cv=cv->next)
   if(cv->save==0 && !strcmp(cv->label, lab))
    {
     strcpy(vs[*num_v].label,cv->label);
     set_lab_tit(cv);
     strcpy(vs[*num_v].tag,cv->lab_tit);
     vs[*num_v].start=0;
     vs[*num_v].end=0;
     vs[*num_v].rank=*num_v;
     vs[*num_v].data=&(cv->val[0]);
     *num_v+=1;
     flag=1;
    }
//  if(cur->b!=NULL && cur->b->head!=NULL && flag==0)
  for(cb=cur->b; cb!=NULL && cb->head!=NULL && flag==0; cb=cb->next) 
   insert_store_nosave(cb->head,lab, num_v);

 }
}


/***************************************************
INSERT_LABELS_MEM
****************************************************/
void insert_labels_mem(object *r, int *num_v, int *num_c)
{
object *cur;
variable *cv;
bridge *cb;

for(cv=r->v; cv!=NULL; cv=cv->next)
   if(cv->save)
    {
     set_lab_tit(cv);
     cmd( ".da.vars.lb.v insert end \"%s %s (%d - %d) # %d\"", cv->label, cv->lab_tit, cv->start, cv->end, *num_v );
     if(cv->end>*num_c)
       *num_c=cv->end;
     *num_v+=1;
}

for(cb=r->b; cb!=NULL; cb=cb->next)
 {cur=cb->head;
 if(cur->to_compute==1)
   {
   for(cur=cb->head; cur!=NULL; cur=cur->next)
     insert_labels_mem(cur, num_v, num_c);
   }  
 }
if(r->up==NULL)
 for(cv=cemetery; cv!=NULL; cv=cv->next)
  {  
	 cmd( ".da.vars.lb.v insert end \"%s %s (%d - %d) # %d\"", cv->label, cv->lab_tit, cv->start, cv->end, *num_v );
     if(cv->end>*num_c)
       *num_c=cv->end;
     *num_v+=1;
  }
}

/***************************************************
insert_data_mem
****************************************************/
void insert_data_mem(object *r, int *num_v, int *num_c)
{


insert_labels_mem(r,num_v, num_c);
vs=new store[*num_v];
*num_v=0;
insert_store_mem(r, num_v);

}

/***************************************************
INSERT_STORE_MEM
****************************************************/
void insert_store_mem(object *r, int *num_v)
{
object *cur;
variable *cv;
bridge *cb;

for(cv=r->v; cv!=NULL; cv=cv->next)
 if(cv->save)
  {
   set_lab_tit(cv);
   strcpy(vs[*num_v].label,cv->label);
   strcpy(vs[*num_v].tag,cv->lab_tit);
   vs[*num_v].start=cv->start;
   vs[*num_v].end=cv->end;
   vs[*num_v].rank=*num_v;
   vs[*num_v].data=cv->data;
   *num_v+=1;
  }
for(cb=r->b; cb!=NULL; cb=cb->next)
 {cur=cb->head;
 if(cur->to_compute==1)
   {
   for(cur=cb->head; cur!=NULL; cur=cur->next)
     {
      insert_store_mem(cur, num_v);
      
     }
   }
 }    
 
if(r->up==NULL)
 for(cv=cemetery; cv!=NULL; cv=cv->next)
  {
     strcpy(vs[*num_v].label,cv->label);
     strcpy(vs[*num_v].tag,cv->lab_tit);
     vs[*num_v].start=cv->start;
     vs[*num_v].end=cv->end;
     vs[*num_v].rank=*num_v;
     vs[*num_v].data=cv->data;
     *num_v+=1;

  }
}


/***************************************************
INSERT_data_FILE
****************************************************/
void insert_data_file( bool gz, int *num_v, int *num_c )
{
FILE *f;
#ifdef LIBZ
gzFile fz;
#endif
char ch, label[MAX_ELEM_LENGTH], tag[MAX_ELEM_LENGTH], app_str[20], *tok, *linbuf;
int i, j, new_v, new_c;
bool header = false;
long linsiz = 1;
store *app;

if ( ! gz )
	f = fopen( filename, "rt" );
else
{
#ifdef LIBZ
	fz = gzopen( filename, "rt" );
#endif
}

new_v=0;
if(*num_v==0)
  plog( "\nResult data from file %s:\n", "", filename);
else
  plog( "\nAdditional data file number %d.\nResult data from file %s:\n", "", file_counter, filename);  

if ( ! gz )
	ch = ( char ) fgetc( f );
else
{
#ifdef LIBZ
	ch = ( char ) gzgetc( fz );
#endif
}

while( ch != '\n' )
{
	if ( ! gz )
		ch = ( char ) fgetc( f );
	else
	{
#ifdef LIBZ
		ch = ( char ) gzgetc( fz );
#endif
	}
	if ( ch == '\t' )
	   new_v += 1;
   
	if ( ch == '(' )		// check for header-less .tot files
		header = true;
	
	linsiz++;				// count line size
}

if ( ! header )
{
	plog( "\nError: invalid header, aborting file load.\nCheck if '.tot' files where created with the -g option.\n" );
	goto end;
}

if ( ! gz )
	fclose( f );
else
{
#ifdef LIBZ
	gzclose( fz );
#endif
}

plog( "- %d Variables\n", "",  new_v );

cmd( "update" );

if ( ! gz )
	f = fopen( filename, "rt" );
else
{
#ifdef LIBZ
	fz = gzopen( filename, "rt" );
#endif
}

new_c=-1;
while( ch != EOF )
{
	if ( ! gz )
		ch = ( char ) fgetc( f );
	else
	{
#ifdef LIBZ
		ch = ( char ) gzgetc( fz );
#endif
	}
	if ( ch == '\n' )
		new_c += 1;
}

if ( ! gz )
	fclose( f );
else
{
#ifdef LIBZ
	gzclose( fz );
#endif
}

plog( "- %d Cases\nLoading...", "", new_c );

if(*num_v==0)
 {
  vs=new store[new_v];
 }
else
 {
 app=new store[new_v+ *num_v];
 for(i=0; i<*num_v; i++)
  {app[i]=vs[i];
   strcpy(app[i].label, vs[i].label);
   strcpy(app[i].tag, vs[i].tag);
  } 
 delete[] vs;
 vs=app;
 }  

if ( ! gz )
	f = fopen( filename, "rt" );
else
{
#ifdef LIBZ
	fz = gzopen( filename, "rt" );
#endif
}

if(*num_v>-1)
 sprintf(app_str, " (file=%d)",file_counter);
else
 strcpy(app_str, "");

linsiz = max( linsiz, new_v * ( DBL_DIG + 4 ) ) + 1;
linbuf = new char[ linsiz ];
if ( linbuf == NULL )
{
	plog( "\nError: not enough memory or invalid format, aborting file load.\n" );
	goto end;
}

// read header line
if ( ! gz )
	fgets( linbuf, linsiz, f );	// buffers one entire line
else
{
#ifdef LIBZ
	gzgets( fz, linbuf, linsiz );
#endif
}

tok = strtok( linbuf , "\t" ); // prepares for parsing and get first one
for ( i = *num_v; i < new_v + *num_v; ++i )
{
  if ( tok == NULL )
  {
	  plog( "\nError: invalid header, aborting file load.\n");
	  goto end;
  }
  
  sscanf( tok, "%s %s (%d %d)", vs[i].label, vs[i].tag, &( vs[i].start ), &( vs[i].end ) );	
  strcat(vs[i].label, "F");
  sprintf(msg, "%d_%s", file_counter, vs[i].tag);
  strcpy(vs[i].tag, msg);
  vs[i].rank=i;

 if(vs[i].start!=-1)
   cmd( ".da.vars.lb.v insert end \"%s %s (%d - %d) # %d %s\"", vs[i].label, vs[i].tag, vs[i].start, vs[i].end, i, app_str );
 else
   {
	cmd( ".da.vars.lb.v insert end \"%s %s (0 - %d) # %d %s\"", vs[i].label, vs[i].tag, new_c-1, i, app_str );
    vs[i].start=0;
    vs[i].end=new_c-1;
   }
 
 cmd( "lappend DaModElem %s", vs[i].label );

 vs[i].data=new double[new_c+2];
 
 tok=strtok( NULL, "\t" );		// get next token, if any
}

// read data lines
for(j=0; j<new_c; j++)
{
 if ( ! gz )
	fgets( linbuf, linsiz, f );	// buffers one entire line
 else
 {
#ifdef LIBZ
	gzgets( fz, linbuf, linsiz );
#endif
 }
 
 tok = strtok( linbuf , "\t" ); // prepares for parsing and get first one
 for ( i = *num_v; i < new_v + *num_v; ++i )
 {
	if ( tok == NULL )
	{
	  plog( "\nError: invalid data, aborting file load.\n" );
	  *num_c += ( j > 0 ? j - 1 : 0 ) > *num_c ? ( j > 0 ? j - 1 : 0 ) : 0;
	  goto end;
	}
  
	if ( ! strcmp( tok, nonavail ) )	// it's a non-available observation
		vs[ i ].data[ j ] = NAN;
	else
		sscanf( tok,"%lf", &( vs[ i ].data[ j ] ) );
	
	tok=strtok( NULL, "\t" );		// get next token, if any
 }
}

*num_v+=new_v;
new_c--;
if(new_c > *num_c)
 *num_c=new_c;
if(new_c > max_c)
 max_c=new_c; 

plog( " Done\n" );

end:

if ( ! gz )
	fclose( f );
else
{
#ifdef LIBZ
	gzclose( fz );
#endif
}

}


/************************
STATISTICS
************************/
void statistics(int *choice)
{

int idseries;
char *app;
char **str, **tag;
char str1[50], longmsg[300];
int i, nv, j, *start, *end;

int logErrCnt = 0;				// log errors counter to prevent excess messages
bool stopErr = false;

Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);
cmd( "set nv [.da.vars.ch.v size]" );
Tcl_UnlinkVar(inter, "nv");

if ( nv == 0 )			// no variables selected
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
	return;
}

double **data,**logdata, av, var, num, ymin, ymax, sig;

data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];
if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  if(autom_x ||(start[i]<=max_c && end[i]>=min_c))
   {
   data[i]=find_data(idseries); 
   
   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!is_nan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
		 {
			plog( "\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d\n", "", i + 1, j );
		 }
		 else
			if ( ! stopErr )
			{
				plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
				stopErr = true;
			}
	   }
	 data[i]=logdata[i];				// replace the data series
   }
   }

 }

if(logs)
 cmd( ".log.text.text.internal insert end \"\n\nTime series descriptive statistics (in log)\n\" tabel" );
else
 cmd( ".log.text.text.internal insert end \"\n\nTime series descriptive statistics\n\" tabel" );

sprintf(str1, "%d Cases", max_c-min_c+1);
sprintf(longmsg, "%-20s\tAverage\tStd.Dev.\tVar.\tMin\tMax\n", str1);
cmd( ".log.text.text.internal insert end \"%s\" tabel", longmsg );

for(i=0; i<nv; i++)
{
 ymin=10000000;
 ymax=-10000000;

 for(av=var=num=0, j=min_c; j<=max_c; j++)
  {
  if(j>=start[i] && j<=end[i] && is_finite(data[i][j]))		// ignore NaNs
  {
   if(data[i][j]<ymin)
    ymin=data[i][j];
   if(data[i][j]>ymax)
    ymax=data[i][j];
   av+=data[i][j];
   num++;
   var+=data[i][j]*data[i][j];
  }
  }

 if(num>1)
 {
  av=av/num;
  var=fabs(var/num-av*av);
  sig=sqrt(var*num/(num-1));
 }
 else
  var=sig=0;
 if(num>0)
 {
 sprintf(msg, "%s %s (%.*g)", str[i], tag[i], pdigits, num);
 sprintf(str1, "%-20s\t", msg);
 cmd( ".log.text.text.internal insert end \"%s\" tabel", str1 );


 sprintf(longmsg, "%.*g\t%.*g\t%.*g\t%.*g\t%.*g\n", pdigits, av, pdigits, sig, pdigits, var, pdigits, ymin, pdigits, ymax);
 cmd( ".log.text.text.internal insert end \"%s\" tabel", longmsg );


 }
}
plog("\n");
for(i=0; i<nv; i++)
 {delete[] str[i];
  delete[] tag[i];
  if(logs)
    delete[] logdata[i];
 }
delete[] str;
delete[] tag;

delete[] logdata;
delete[] data;
delete[] start;
delete[] end;


} //end Statistics


/************************
STATISTICS_CROSS
************************/
void statistics_cross(int *choice)
{
int idseries;
char *app;
char **str, **tag;
char str1[50], longmsg[300];
int i, nv, j, *start, *end, nt, *list_times, h, k;
double **data,**logdata, av, var, num, ymin, ymax, sig;
bool first;

int logErrCnt = 0;				// log errors counter to prevent excess messages
bool stopErr = false;

Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);
cmd( "set nv [.da.vars.ch.v size]" );
Tcl_UnlinkVar(inter, "nv");

if ( nv == 0 )			// no variables selected
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
	return;
}

Tcl_LinkVar(inter, "nt", (char *) &nt, TCL_LINK_INT);
cmd( "set nt $num_t" );
Tcl_UnlinkVar(inter, "nt");

if(nv<2 || nt==0)
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place at least two series in the Series Selected listbox and select at least one case.\"" );
	return;
}

list_times=new int[nt];
cmd( "set k 0" );
Tcl_LinkVar(inter, "k", (char *) &k, TCL_LINK_INT);

for(i=0; i<nt; i++)    //Sets the list of cases to plot
 {
  cmd( "set k [lindex $list_times %d]", i );
  list_times[i]=k;
 }
Tcl_UnlinkVar(inter, "k");

data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];
if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  data[i]=find_data(idseries); 
   
   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!is_nan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
		 {
			plog( "\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d\n", "", i + 1, j );
		 }
		 else
			if ( ! stopErr )
			{
				plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
				stopErr = true;
			}
	   }
	 data[i]=logdata[i];				// replace the data series
   }
 }

if(logs)
 cmd( ".log.text.text.internal insert end \"\n\nCross-section descriptive statistics (in log)\n\" tabel" );
else
 cmd( ".log.text.text.internal insert end \"\n\nCross-section descriptive statistics\n\" tabel" );

sprintf(str1, "%d Variables",nv);
sprintf(longmsg, "%-20s\tAverage\tStd.Dev.\tVar.\tMin\tMax\n", str1);
cmd( ".log.text.text.internal insert end \"%s\" tabel", longmsg );

for(j=0; j<nt; j++)
{h=list_times[j];
 first=true;
 for(av=var=num=0, i=0; i<nv; i++)
  {
  if(h>=start[i] && h<=end[i] && is_finite(data[i][h]))		// ignore NaNs
  {
  if(first)
  {
     ymin=ymax=data[i][h];
	 first=false;
  }
   if(data[i][h]<ymin)
    ymin=data[i][h];
   if(data[i][h]>ymax)
    ymax=data[i][h];
   av+=data[i][h];
   num++;
   var+=data[i][h]*data[i][h];
  }
 }

 if(num>1)
 {av=av/num;
 var=var/num-av*av;
 sig=sqrt(var*num/(num-1));
 }
 else
  var=sig=0;
 if(num>0)
 {
 sprintf(str1, "Case %d (%.*g)\t",h, pdigits, num);
 cmd( ".log.text.text.internal insert end \"%s\" tabel", str1 );

 sprintf(longmsg, "%.*g\t%.*g\t%.*g\t%.*g\t%.*g\n", pdigits, av, pdigits, sig, pdigits, var, pdigits, ymin, pdigits, ymax);
 cmd( ".log.text.text.internal insert end \"%s\" tabel", longmsg );
 }
}
plog("\n");
delete[] list_times;
for(i=0; i<nv; i++)
 {delete[] str[i];
  delete[] tag[i];
  if(logs)
    delete[] logdata[i];
 }
delete[] str;
delete[] tag;
delete[] logdata;
delete[] data;
delete[] start;
delete[] end;
} //end Statistics_cross


void sort_on_end(store *app)
{

qsort((void *)app, num_var, sizeof(vs[0]), sort_labels_down);

}
/*
Sorting function for presenting variables' labels in a nice way.
The variables are grouped according to:
1) their label (increasing: A first z last)
	2) time of their last occurrence (decreasing: existing variable first)
   	3) time of their first occurrence (increasing: first born first)
      	4) lsd internal ID indexing system (used for the tag) (increasing)
The function is complicated for the point 4) by the fact that the tag is recorded
in the labels as a single string using the underscore '_' as joining character.

*/
int sort_labels_down(const void *a, const void *b)
{
int a_int, b_int, counter;
int diff;

// convert labels to lowercase for comparison
int a_sz = strlen( ( ( store * ) a )->label );
int b_sz = strlen( ( ( store * ) b )->label );
char *a_str = new char [ a_sz + 1 ];
char *b_str = new char [ b_sz + 1 ];
strcpy( a_str, ( ( store * ) a )->label );
strcpy( b_str, ( ( store * ) b )->label );
for ( int i = 0; i < a_sz; ++i )
	a_str[ i ] = tolower( a_str[ i ] );
for ( int i = 0; i < b_sz; ++i )
	b_str[ i ] = tolower( b_str[ i ] );
// make names started with a underscore go to the end
if ( a_str[ 0 ] == '_' )
	a_str[ 0 ] = '~';
if ( b_str[ 0 ] == '_' )
	b_str[ 0 ] = '~';

diff = strcmp( a_str, b_str );

delete [] a_str;
delete [] b_str;

if(diff!=0)
 return diff;
else
 if(((store *)a)->end != ((store *)b)->end)
  return ((store *)b)->end - ((store *)a)->end;
 else
  if(((store *)a)->start != ((store *)b)->start)
    return ((store *)a)->start - ((store *)b)->start;
  else
    {
     for(counter=0; ; )
      {
       a_int=atoi(((store *)a)->tag+counter);
       b_int=atoi(((store *)b)->tag+counter);
       if(a_int!=b_int)
        return a_int-b_int;
       while(((store *)a)->tag[counter]!='_')
        counter++;
       counter++;
      }
    }
}


/***************************************************
PLOT_GNU
Draws the XY plots, with the first series as X and the others as Y's
****************************************************/
void plot_gnu(int *choice)
{

char *app;
char **str, **tag;
char str1[50], str2[100], str3[10], dirname[MAX_PATH_LENGTH];
FILE *f, *f2;
double **data,**logdata;

int logErrCnt = 0;				// log errors counter to prevent excess messages
bool stopErr = false;

int i, nv,nanv=0, j, k, *start, *end, done, box;
int idseries, ndim, gridd;

cmd( "set choice [.da.vars.ch.v size]" );
nv=*choice;
*choice=0;
if ( nv == 0 )
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
	*choice = 2;
	return;
}

if(nv>2)
 {
 cmd( "toplevel .da.s" );
 cmd( "wm title .da.s \"Plot Type\"" );
 cmd( "wm transient .da.s .da" );

 cmd( "label .da.s.l -text \"Choose the type of plot\"" );

 cmd( "frame .da.s.d -relief groove -bd 2" );
 cmd( "set ndim 2" );
 cmd( "radiobutton .da.s.d.2d -text \"2D plot\" -variable ndim -value 2" );
 cmd( "radiobutton .da.s.d.3d -text \"3D plot \" -variable ndim -value 3" );

 cmd( "pack .da.s.d.2d .da.s.d.3d -expand yes -fill x -anchor w" );

cmd( "frame .da.s.o -relief groove -bd 2" );

cmd( "label .da.s.o.l -text \"Select 3D options\"" );

cmd( "if { [info exist box]==1} {} {set box 0}" );
cmd( "radiobutton .da.s.o.a -text \"Use 1st and 2nd vars. as plane\" -variable box -value 0 -anchor w" );
cmd( "radiobutton .da.s.o.c -text \"Use time and 1st var. as plane\" -variable box -value 2 -anchor w" );
cmd( "radiobutton .da.s.o.b -text \"Use time and rank as plane\" -variable box -value 1 -anchor w" );
cmd( "checkbutton .da.s.o.g -text \"Use gridded data\" -variable gridd -anchor w" );
cmd( "checkbutton .da.s.o.p -text \"Draw palette-mapped surface\" -variable pm3d -anchor w" );

cmd( "pack .da.s.o.l .da.s.o.a .da.s.o.c .da.s.o.b .da.s.o.g .da.s.o.p -expand yes -fill x -anchor w" );

cmd( "frame .da.s.w -relief groove -bd 2" );
cmd( "label .da.s.w.l -text \"Select window type\"" );
cmd( "radiobutton .da.s.w.g -text Lsd -variable gnu -value 1 -anchor w" );
cmd( "radiobutton .da.s.w.p -text Gnuplot -variable gnu -value 2 -anchor w" );
cmd( "pack .da.s.w.l .da.s.w.g .da.s.w.p -expand yes -fill x -anchor w" );

 cmd( "frame .da.s.b" );
 cmd( "set p .da.s.b" );
 cmd( "button $p.ok -width -9 -text Ok -command {set choice 1}" );
 cmd( "button $p.help -width -9 -text Help -command {LsdHelp mdatares.html#3dTime}" );
 cmd( "button $p.can -width -9 -text Cancel -command {set choice 2}" );
 cmd( "pack $p.ok $p.help $p.can -side left -expand yes -fill x" );
 
 cmd( "bind .da.s.b.ok <Return> {.da.s.b.ok invoke}" );

 cmd( "pack .da.s.l .da.s.d .da.s.o .da.s.w .da.s.b -expand yes -fill x" );
 cmd( "set w .da.s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w" ); 

*choice=0;
while(*choice==0)
  Tcl_DoOneEvent(0);

cmd( "destroy .da.s" );

if(*choice==2)
 return;

cmd( "set choice $ndim" );
ndim=*choice;
 }
else
 ndim=2; 

data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];
if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  if(autom_x ||(start[i]<=max_c && end[i]>=min_c))
   {

  data[i]=find_data(idseries); 

   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!is_nan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
		 {
			plog( "\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d\n", "", i + 1, j );
		 }
		 else
			if ( ! stopErr )
			{
				plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
				stopErr = true;
			}
	   }
	 data[i]=logdata[i];				// replace the data series
   }
   }
  else
   nanv++; 

 }

if(autom_x||min_c>=max_c)
{

for(i=0; i<nv; i++)
 {if(i==0)
   min_c=max_c=start[i];
  if(start[i]<min_c)
   min_c=start[i];
  if(end[i]>max_c)
   max_c=end[i]>num_c?num_c:end[i];
 }
}

if(autom||miny>=maxy)
{

for(done=0, i=1; i<nv; i++)
 {
  for(j=min_c; j<=max_c; j++)
   {
    if(done==0 && start[i]<=j && end[i]>=j && is_finite(data[i][j]))		// ignore NaNs
     {miny=maxy=data[i][j];
      done=1;
     }
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]<miny )		// ignore NaNs
     miny=data[i][j];
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]>maxy )		// ignore NaNs
     maxy=data[i][j];

   }
 }
} //End for finding min-max

   
cmd( "set dirxy plotxy_%d", cur_plot );

cmd( "file mkdir $dirxy" );

getcwd(dirname, MAX_PATH_LENGTH-1);
sprintf(msg, "plotxy_%d", cur_plot);
chdir(msg);
f=fopen("data.gp","w");
fprintf(f,"#");
if(nv>2)
{
  cmd( "set choice $box" );
  box=*choice;
  cmd( "set choice $gridd" );
  gridd=*choice;
}
else
  box=gridd=0;

if(box==0)
 {
 for(i=0; i<nv; i++)
  if(start[i]<=max_c && end[i]>=min_c)
     fprintf(f,"%s_%s\t", str[i], tag[i]);
     
 }    
else
 {
 if(gridd==0)
 {
  fprintf(f,"Time\t");
  if(box==1)
   {
  for(i=0; i<nv; i++)
   if(start[i]<=max_c && end[i]>=min_c)
     fprintf(f,"Var%d\t%s_%s\t",i, str[i], tag[i]);
   }
  else
   {
  for(i=0; i<nv; i++)
   if(start[i]<=max_c && end[i]>=min_c)
     fprintf(f,"%s_%s\t", str[i], tag[i]);   
   }
 }    
 else
 {
  fprintf(f,"Time\tRank\tVal");
 }    
 
 } 
fprintf(f,"\n");
if(box==0)
{
for(j=min_c; j<=max_c; j++)
 {
 for(i=0; i<nv; i++)
  if(start[i]<=max_c && end[i]>=min_c)
    {
    if(j>=start[i] && i<=end[i])
      fprintf(f,"%lf\t", data[i][j]);
    else
      fprintf(f,"nan\t");  
    
    }
 fprintf(f,"\n");
 }
}
else
{
//3D with time and rank
if(gridd==0)
 {//not gridded
 if(box==1)
 {
 for(j=min_c; j<=max_c; j++)
  {fprintf(f,"%d\t",j);
   for(i=0; i<nv; i++)
    if(start[i]<=max_c && end[i]>=min_c)
      {
      if(j>=start[i] && i<=end[i])
       fprintf(f,"%d\t%lf\t",i+1, data[i][j]);
      else
        fprintf(f,"%d\tnan\t", i+1);        
      }
   fprintf(f,"\n");
  }
 }
 else
 {
 for(j=min_c; j<=max_c; j++)
  {fprintf(f,"%d\t",j);
   for(i=0; i<nv; i++)
    if(start[i]<=max_c && end[i]>=min_c)
      {
      if(j>=start[i] && i<=end[i]) 
        fprintf(f,"%lf\t",data[i][j]);
      else
        fprintf(f,"nan\t");  
      } 
   fprintf(f,"\n");
  }
 
 } 
 }
else
 {//gridded
  for(i=0; i<nv; i++)
   {
    for(j=min_c; j<=max_c; j++)
    if(start[i]<=max_c && end[i]>=min_c)
     {
     if(j>=start[i] && i<=end[i])
       fprintf(f,"%d\t%d\t%lf\n",j,i+1,data[i][j]);
     else
       fprintf(f,"%d\t%d\tnan\n",j,i+1);         
     }
   }  
 } 
} 
fclose(f);

*choice=0;

f=fopen("gnuplot.lsd","w");
f2=fopen("gnuplot.gp","w");

fprintf(f, "set datafile missing \"nan\" \n");		//handle NaNs
fprintf(f2, "set datafile missing \"nan\" \n");
app=(char *)Tcl_GetVar(inter, "gpterm",0);

fprintf(f,"set term tkcanvas\n");
fprintf(f2,"set term %s\n", app);
fprintf(f,"set output 'plot.file'\n");

if(grid)
 {fprintf(f, "set grid\n");
  fprintf(f2, "set grid\n");
 }
if(line_point==2)
 {sprintf(str1,"set pointsize %lf\n", point_size);
  fprintf(f,"%s", str1);
  fprintf(f2,"%s", str1);
 }
if(ndim==2)
{
 sprintf(msg, "set yrange [%lf:%lf]\n", miny, maxy);
 fprintf(f, "%s",msg);
 fprintf(f2, "%s",msg);
} 

if(box==0)
  sprintf(msg, "set xlabel \"%s_%s\"\n", str[0], tag[0]);
else
  sprintf(msg, "set xlabel \"Time\"\n");  
fprintf(f, "%s",msg);
fprintf(f2, "%s",msg);

if(ndim>2)
{
if(box==0)
 sprintf(msg, "set ylabel \"%s_%s\"\n", str[1], tag[1]);
else
  sprintf(msg, "set ylabel \"Series\"\n"); 
 fprintf(f, "%s",msg);
 fprintf(f2, "%s",msg);
} 

if(line_point==1 && ndim==2)
 sprintf(str1, "smooth csplines");
else
 if(line_point==1 && ndim>2)
  sprintf(str1, "with lines");
 else
  strcpy(str1, ""); 

if(ndim==2)
{
if(allblack)
 sprintf(str3, " lt -1");
else
 strcpy(str3, "");
}
else
 {
  cmd( "set choice $gridd" );
 if(*choice==1)
  {
   app=(char *)Tcl_GetVar(inter, "gpdgrid3d",0);
   fprintf(f, "set dgrid3d %s\nset hidden3d\n", app);
   fprintf(f2, "set dgrid3d %s\nset hidden3d\n",app);
  }
 cmd( "set choice $pm3d" );
 if(*choice==1)
  {
   fprintf(f, "set pm3d\n");
   fprintf(f2, "set pm3d\n");
   sprintf(str1, "with pm3d ");
  }

  if(allblack)
   {
   fprintf(f, "set palette gray\n");
   fprintf(f2, "set palette gray\n");
   
   }
    
 }

app=(char *)Tcl_GetVar(inter, "gpooptions",0);
fprintf(f,"%s\n", app);
fprintf(f2,"%s\n", app);
 
if(ndim==2) 
 {
  sprintf(msg,"plot 'data.gp' using 1:2 %s t \"%s(%s)\"", str1, str[1], tag[1]);
  if(allblack)
   strcat(msg, str3);
  i=2;
 } 
else
 {

  if(box==0)
    {sprintf(msg,"splot 'data.gp' using 1:2:3 %s t \"%s(%s)\"", str1, str[2], tag[2]); 
     i=3;
    } 
  else
   {
    i=1;  
    if(box==1)
      sprintf(msg,"splot 'data.gp' using 1:2:3 %s t \"\"", str1); 
    else 
      {
       sprintf(msg,"splot 'data.gp' using 1:2:%d %s t \"\"",(nv-nanv)/2+2, str1);   
       i=2;
      } 
   } 
 } 

fprintf(f, "%s",msg);
fprintf(f2, "%s",msg);

for( ; i<nv; i++)
 if(start[i]<=max_c && end[i]>=min_c)
  {
   if(ndim==2)
    sprintf(str2,", 'data.gp' using 1:%d %s t \"%s(%s)\"",i+1, str1, str[i], tag[i]);
   else
    {if(box==0 )
      sprintf(str2,", 'data.gp' using 1:2:%d %s t \"%s(%s)\"",i+1, str1, str[i], tag[i]); 
     else
      if(gridd==0)
        {
         if(box==1)
           sprintf(str2,", 'data.gp' using 1:%d:%d %s t \"\"",2+2*i, 2*i+3, str1); 
         else
          if(i<=nv/2)
           sprintf(str2,", 'data.gp' using 1:%d:%d %s t \"\"",i+1, (nv-nanv)/2+i+1, str1); 
          else
           strcpy(str2,"");     
        } 
      else
       strcpy(str2,"");  
    }  

   if(strlen(str2)>0 && allblack)
     strcat(str2, str3);
   fprintf(f, "%s",str2);
   fprintf(f2, "%s",str2);


  }

fprintf(f, "\n");
fprintf(f2, "\n");

fprintf(f2, "pause -1 \"Close plot %d)\"\n", cur_plot);

fclose(f);
fclose(f2);

if(nv>2)
 cmd( "set choice $gnu" );
else
 *choice=0;

show_plot_gnu(cur_plot, choice, *choice);

chdir(dirname);

end:
for(i=0; i<nv; i++)
 {delete[] str[i];
  delete[] tag[i];
  if(logs)
    delete[] logdata[i];
 }

delete[] str;
delete[] tag;

delete[] logdata;
delete[] data;
delete[] start;
delete[] end;
}

/*
PLOT_CS_XY
*/
void plot_cs_xy(int *choice)
{

int idseries;
char *app;
char **str, **tag;
char str1[5*MAX_ELEM_LENGTH], str2[5*MAX_ELEM_LENGTH], str3[10], dirname[MAX_PATH_LENGTH];
FILE *f, *f2;
double **data,**logdata;
int i, nv, j, k, *start, *end, done, color;
int time_sel, block_length, ndim;

int logErrCnt = 0;				// log errors counter to prevent excess messages
bool stopErr = false;

double previous_row;
cmd( "set choice [.da.vars.ch.v size]" );
nv=*choice;
*choice=0;
if(nv==0)
 {
 cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
 *choice=2;
 return;
 }

data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];
if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  if(autom_x ||(start[i]<=max_c && end[i]>=min_c))
   {
  data[i]=find_data(idseries); 
   
   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!is_nan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
		 {
			plog( "\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d\n", "", i + 1, j );
		 }
		 else
			if ( ! stopErr )
			{
				plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
				stopErr = true;
			}
	   }
	 data[i]=logdata[i];				// replace the data series
   }
   }
 }

if(autom_x||min_c>=max_c)
{

for(i=0; i<nv; i++)
 {if(i==0)
   min_c=max_c=start[i];
  if(start[i]<min_c)
   min_c=start[i];
  if(end[i]>max_c)
   max_c=end[i]>num_c?num_c:end[i];
 }
}

if(autom||miny>=maxy)
{

for(done=0, i=1; i<nv; i++)
 {
  for(j=min_c; j<=max_c; j++)
   {
    if(done==0 && start[i]<=j && end[i]>=j && is_finite(data[i][j]))		// ignore NaNs
     {miny=maxy=data[i][j];
      done=1;
     }
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]<miny )		// ignore NaNs
     miny=data[i][j];
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]>maxy )		// ignore NaNs
     maxy=data[i][j];

   }
 }
} //End for finding min-max

cmd( "set bidi %d", end[0] );

cmd( "toplevel .da.s" );
cmd( "wm title .da.s \"Options\"" );
cmd( "wm transient .da.s .da" );
cmd( "frame .da.s.i -relief groove -bd 2" );
cmd( "label .da.s.i.l -text \"Insert time step to use\"" );
cmd( "entry .da.s.i.e -validate focusout -vcmd { if [ string is integer %%P ] { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.i.e insert 0 $bidi" ); 
cmd( "pack .da.s.i.l .da.s.i.e -expand yes -fill x" );

cmd( "frame .da.s.d -relief groove -bd 2" );

cmd( "label .da.s.d.l -text \"Select the type of plot\"" );
cmd( "radiobutton .da.s.d.2d -text \"2D plot\" -variable ndim -value 2" );
cmd( "radiobutton .da.s.d.3d -text \"3D plot\" -variable ndim -value 3" );

cmd( "pack .da.s.d.l .da.s.d.2d .da.s.d.3d -expand yes -fill x -anchor w" );

cmd( "frame .da.s.o -relief groove -bd 2" );

cmd( "label .da.s.o.l -text \"Select 3D options\"" );
cmd( "checkbutton .da.s.o.g -text \"Use gridded data\" -variable gridd -anchor w" );
cmd( "checkbutton .da.s.o.p -text \"Draw palette-mapped surface\" -variable pm3d -anchor w" );

cmd( "pack .da.s.o.l .da.s.o.g .da.s.o.p -expand yes -fill x -anchor w" );

cmd( "set ndim 2" );

cmd( "set choice $ndim" );
if(*choice==2)
 cmd( "set blength %d", (int)(nv/2) );
else
 cmd( "set blength %d", (int)(nv/3) ); 

cmd( "set numv 1" );
cmd( "frame .da.s.v -relief groove -bd 2" );
cmd( "label .da.s.v.l -text \"Number of dependent variables: \"" );
cmd( "entry .da.s.v.e -validate focusout -vcmd { if [ string is integer %%P ] { set numv %%P; return 1 } { %%W delete 0 end; %%W insert 0 $numv; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.v.e insert 0 $numv" ); 
cmd( "label .da.s.v.n -text \"Num. of points: $blength\"" );
cmd( "pack .da.s.v.l .da.s.v.e .da.s.v.n -expand yes -fill x" );
cmd( "bind .da.s.v.e <KeyRelease> {set blength [expr $nnvar / ($numv + $ndim-1)]; .da.s.v.n conf -text \"Num. of points: $blength\"}" );
cmd( "set nnvar %d", nv );
cmd( "bind .da.s.v.e <Return> {set blength [expr $nnvar / ($numv + $ndim-1)]; .da.s.v.n conf -text \"Num. of points: $blength\"}" );
cmd( "bind .da.s.v.e <Tab> {set blength [expr $nnvar / ($numv + $ndim-1)]; .da.s.v.n conf -text \"Num. of points: $blength\"}" );
cmd( "bind .da.s.d.2d <Return> {set blength [expr $nnvar / ($numv + $ndim-1)]; .da.s.v.n conf -text \"Num. of points: $blength\"}" );
cmd( "bind .da.s.d.2d <ButtonRelease-1> {set ndim 2; set blength [expr $nnvar / ($numv + $ndim-1)]; .da.s.v.n conf -text \"Num. of points: $blength\"}" );
cmd( "bind .da.s.d.3d <ButtonRelease-1> {set ndim 3; set blength [expr $nnvar / ($numv + $ndim-1)]; .da.s.v.n conf -text \"Num. of points: $blength\"}" );
cmd( "bind .da.s.d.2d <Down> {.da.s.d.3d invoke; focus .da.s.d.3d; set ndim 3; set blength [expr $nnvar / ($numv + $ndim-1)]; .da.s.v.n conf -text \"Num. of points: $blength\"}" );
cmd( "bind .da.s.d.3d <Up> {.da.s.d.2d invoke; focus .da.s.d.2d; set ndim 2; set blength [expr $nnvar / ($numv + $ndim-1)]; .da.s.v.n conf -text \"Num. of points: $blength\"}" );

cmd( "frame .da.s.l -relief groove -bd 2" );
cmd( "label .da.s.l.l -text \"Select the block length\"" );
cmd( "pack .da.s.l.l " );

cmd( "frame .da.s.w -relief groove -bd 2" );
cmd( "label .da.s.w.l -text \"Select window type\"" );
cmd( "radiobutton .da.s.w.g -text Lsd -variable gnu -value 1 -anchor w" );
cmd( "radiobutton .da.s.w.p -text Gnuplot -variable gnu -value 2 -anchor w" );
cmd( "pack .da.s.w.l .da.s.w.g .da.s.w.p -expand yes -fill x -anchor w" );

cmd( "frame .da.s.b" );
cmd( "set p .da.s.b" );
cmd( "button $p.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "button $p.help -width -9 -text Help -command {LsdHelp mdatares.html#3dCrossSection}" );
cmd( "button $p.can -width -9 -text Cancel -command {set choice 2}" );
cmd( "pack $p.ok $p.help $p.can -side left -expand yes -fill x" );

cmd( "bind .da.s.b.ok <Return> {.da.s.b.ok invoke}" );

cmd( "pack .da.s.i .da.s.d .da.s.o .da.s.v .da.s.w .da.s.b -expand yes -fill x" );
cmd( "focus .da.s.i.e; .da.s.i.e selection range 0 end" );

cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );
cmd( " .da.s.i.e selection range 0 end" );
cmd( "bind .da.s.i.e <KeyPress-Return> {if {$ndim == 2} { focus .da.s.d.2d } {focus .da.s.d.3d}}" );
cmd( "bind .da.s.d.2d <KeyPress-Return> {.da.s.v.e selection range 0 end; focus .da.s.v.e}" );
cmd( "bind .da.s.d.3d <KeyPress-Return> {.da.s.v.e selection range 0 end; focus .da.s.v.e}" );

cmd( "bind .da.s.v.e <KeyPress-Return> {focus .da.s.b.ok}" );
cmd( "set w .da.s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w" );

*choice=0;
while(*choice==0)
  Tcl_DoOneEvent(0);

cmd( "set bidi [ .da.s.i.e get ]" ); 
cmd( "set numv [ .da.s.v.e get ]" ); 
cmd( "destroy .da.s" );

if(*choice==2)
 goto end;

cmd( "set choice $ndim" );
ndim=*choice;

cmd( "set choice $bidi" );
time_sel=*choice;

cmd( "set blength [expr $nnvar / ($numv + $ndim-1)]" );
cmd( "set choice $blength" );
block_length=*choice;
*choice=0;

if(nv%block_length!=0)
 {
  cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid block length\" -detail \"It should be an exact divisor of the number of variables.\"" );
  *choice=2;
  goto end;
 }

//here we are
cmd( "set dirxy plotxy_%d", cur_plot );

cmd( "file mkdir $dirxy" );
getcwd(dirname, MAX_PATH_LENGTH-1);
sprintf(msg, "plotxy_%d",cur_plot);
chdir(msg);

f=fopen("data.gp","w");
if(start[0] == end[0])  
  previous_row=data[0][end[0]];
else
  previous_row=data[0][time_sel];  
  
for(j=0; j<block_length; j++)
 {

if(start[0] == end[0])  
 {
  if(data[j][end[j]]!=previous_row)
   {
     previous_row=data[j][end[j]];
   }
 }
else
 {
  if(data[j][time_sel]!=previous_row)
   {
    previous_row=data[j][time_sel];
   }
 }
 

 for(i=0; i<nv; i+=block_length)
  {
  if(start[i+j] == end[i+j])  
     fprintf(f,"%lf\t", data[i+j][end[i+j]]);
  else
   {
    if(start[i+j]<=max_c && end[i+j]>=min_c)
      fprintf(f,"%lf\t", data[i+j][time_sel]);
   }
  } 
 fprintf(f,"\n");
 }
fclose(f);

f=fopen("gnuplot.lsd","w");
f2=fopen("gnuplot.gp","w");

fprintf(f, "set datafile missing \"nan\" \n");		//handle NaNs
fprintf(f2, "set datafile missing \"nan\" \n");
app=(char *)Tcl_GetVar(inter, "gpterm",0);
fprintf(f2,"set term %s\n", app);
fprintf(f,"set term tkcanvas\n");
fprintf(f,"set output 'plot.file'\n");

if(grid)
 {
  fprintf(f, "set grid\n");
  fprintf(f2, "set grid\n");
 }
if(line_point==2)
 { fprintf(f, "set pointsize %lf\n", point_size);
   fprintf(f2, "set pointsize %lf\n", point_size);
   if(ndim==2)
    strcpy(str2, "");
   else
    sprintf(str2, "with points ");       
 }
else
 {
 if(ndim==2)
  sprintf(str2, "smooth csplines ");
 else
   sprintf(str2, "with lines "); 
 }

sprintf(msg, "set xlabel \"%s_%s\"\n", str[0], tag[0]);
fprintf(f, "%s",msg);
fprintf(f2, "%s",msg);

if(ndim==3)
 {
 sprintf(msg, "set ylabel \"%s_%s\"\n", str[block_length], tag[block_length]);
 fprintf(f, "%s",msg);
 fprintf(f2, "%s",msg);
 } 

if(allblack)
 sprintf(str3, " lt -1");
else
  str3[0]=0;

if(ndim==3)
 {
 cmd( "set choice $gridd" );
 if(*choice==1)
  {
   app=(char *)Tcl_GetVar(inter, "gpdgrid3d",0);
   fprintf(f, "set dgrid3d %s\nset hidden3d\n", app);
   fprintf(f2, "set dgrid3d %s\nset hidden3d\n", app);
  }
 cmd( "set choice $pm3d" );
 if(*choice==1)
  {sprintf(str2, "with pm3d ");
   fprintf(f, "set pm3d\n");
   fprintf(f2, "set pm3d\n");
  }

 if(allblack)
  {
   strcpy(str3, "");
  fprintf(f, "set palette gray\n");
  fprintf(f2, "set palette gray\n");
  } 
 }
 
if(ndim==2)
 {
  sprintf(msg,"plot 'data.gp' using 1:2 %s t \"%s_%s(%d)\"",str2, str[block_length], tag[block_length], time_sel);
  if(allblack)
   strcat(msg, str3);
  i=2; //init from the second variable
 } 
else
 {
  sprintf(msg,"splot 'data.gp' using 1:2:3 %s t \"%s_%s(%d)\"",str2, str[2*block_length], tag[2*block_length], time_sel); 
 i=3;
 }  
for(; i<nv/block_length; i++)
 {j=i*block_length;
 if(start[j]<=max_c && end[j]>=min_c)
  {
   if(ndim==2)
    {
     sprintf(str1,", 'data.gp' using 1:%d %s t \"%s_%s(%d)\"",i+1, str2, str[j], tag[j], time_sel);
     if(allblack)
      strcat(str1, str3);
    }  
   else
    sprintf(str1,", 'data.gp' using 1:2:%d %s t \"%s_%s(%d)\"",i+1, str2, str[j], tag[j], time_sel); 
   strcat(msg,str1);

  }
 }
strcat(msg,"\n");
fprintf(f, "%s",msg);
fprintf(f2, "%s",msg);

fprintf(f2, "pause -1 \"(Close plot %d)\"\n", cur_plot);
fclose(f);
fclose(f2);
cmd( "set choice $gnu" );
show_plot_gnu(cur_plot, choice, *choice);
chdir(dirname);
end:
for(i=0; i<nv; i++)
 {delete[] str[i];
  delete[] tag[i];
  if(logs)
    delete[] logdata[i];
 }

delete[] str;
delete[] tag;

delete[] logdata;
delete[] data;
delete[] start;
delete[] end;
}


/***************************************************
PLOT_PHASE_DIAGRAM
****************************************************/
void plot_phase_diagram(int *choice)
{

int idseries;
char *app;
char **str, **tag;
char str1[50], str2[100], str3[100], dirname[MAX_PATH_LENGTH];
FILE *f, *f2;
double **data,**logdata;

int i, nv, j, k, *start, *end, done, nlags;

int logErrCnt = 0;				// log errors counter to prevent excess messages
bool stopErr = false;

cmd( "set choice [.da.vars.ch.v size]" );
if(*choice!=1)
 {
 cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of series\" -detail \"One and only one series must be selected.\"" );
 *choice = 2;
 return;
 }

nv=1; //ridiculous, but I am recycling the code

data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];
if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  if(autom_x ||(start[i]<=max_c && end[i]>=min_c))
   {
   data[i]=find_data(idseries);
   
   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!is_nan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
		 {
			plog( "\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d\n", "", i + 1, j );
		 }
		 else
			if ( ! stopErr )
			{
				plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
				stopErr = true;
			}
	   }
	 data[i]=logdata[i];				// replace the data series
   }
   }
 }

if(autom_x||min_c>=max_c)
{

for(i=0; i<nv; i++)
 {if(i==0)
   min_c=max_c=start[i];
  if(start[i]<min_c)
   min_c=start[i];
  if(end[i]>max_c)
   max_c=end[i]>num_c?num_c:end[i];
 }
}

if(autom||miny>=maxy)
{

for(done=0, i=0; i<nv; i++)
 {
  for(j=min_c; j<=max_c; j++)
   {
    if(done==0 && start[i]<=j && end[i]>=j && is_finite(data[i][j]))		// ignore NaNs
     {miny=maxy=data[i][j];
      done=1;
     }
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]<miny )		// ignore NaNs
     miny=data[i][j];
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]>maxy )		// ignore NaNs
     maxy=data[i][j];

   }
 }
} //End for finding min-max

cmd( "set bidi 1" );
cmd( "toplevel .da.s" );
cmd( "wm transient .da.s .da" );
cmd( "wm title .da.s \"Lags Selection\"" );
cmd( "frame .da.s.i -relief groove -bd 2" );
cmd( "label .da.s.i.l -text \"Insert number of lags\"" );
cmd( "entry .da.s.i.e -validate focusout -vcmd { if [ string is integer %%P ] { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.i.e insert 0 $bidi" ); 
cmd( "set dia 0" );
cmd( "checkbutton .da.s.i.arrow -text \"Diagonal\" -variable dia" );
cmd( "pack .da.s.i.l .da.s.i.e .da.s.i.arrow" );

cmd( "frame .da.s.w -relief groove -bd 2" );
cmd( "label .da.s.w.l -text \"Select window type\"" );
cmd( "radiobutton .da.s.w.g -text Lsd -variable gnu -value 1 -anchor w" );
cmd( "radiobutton .da.s.w.p -text Gnuplot -variable gnu -value 2 -anchor w" );
cmd( "pack .da.s.w.l .da.s.w.g .da.s.w.p -expand yes -fill x" );

cmd( "frame .da.s.b" );
cmd( "set p .da.s.b" );
cmd( "button $p.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "button $p.help -width -9 -text Help -command {LsdHelp mdatares.html#plot}" );
cmd( "button $p.can -width -9 -text Cancel -command {set choice 2}" );
cmd( "pack $p.ok $p.help $p.can -side left -expand yes -fill x" );

cmd( "pack .da.s.i .da.s.w .da.s.b" );
cmd( "focus .da.s.i.e; .da.s.i.e selection range 0 end" );

cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );
cmd( " .da.s.i.e selection range 0 end" );
cmd( "set w .da.s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w" );

*choice=0;
while(*choice==0)
  Tcl_DoOneEvent(0);

cmd( "set bidi [ .da.s.i.e get ]" ); 
cmd( "destroy .da.s" );

if(*choice==2)
 goto end;
cmd( "set dirxy plotxy_%d", cur_plot );

cmd( "file mkdir $dirxy" );
getcwd(dirname, MAX_PATH_LENGTH-1);
sprintf(msg, "plotxy_%d",cur_plot);
chdir(msg);

cmd( "set choice $bidi" );
nlags=*choice;

f=fopen("data.gp","w");
fprintf(f,"#");
for(i=0; i<=nlags; i++)
  if(start[0]<=max_c && end[0]>=min_c)
     fprintf(f,"%s_%s(%d)\t", str[0], tag[0], i);
fprintf(f,"\n");
for(j=min_c; j<=max_c-nlags; j++)
 {
 for(i=0; i<=nlags; i++)
  if(start[0]<=max_c && end[0]>=min_c)
    fprintf(f,"%lf\t", data[0][j+i]);
 fprintf(f,"\n");
 }
fclose(f);

*choice=0;
f=fopen("gnuplot.lsd","w");
f2=fopen("gnuplot.gp","w");
fprintf(f, "set datafile missing \"nan\" \n");		//handle NaNs
fprintf(f2, "set datafile missing \"nan\" \n");
fprintf(f,"set term tkcanvas\n");
fprintf(f,"set output 'plot.file'\n");

if(grid)
 {fprintf(f, "set grid\n");
  fprintf(f2, "set grid\n");
 }
if(line_point==2)
 {sprintf(str1,"set pointsize %lf\n", point_size);
  fprintf(f,"%s", str1);
  fprintf(f2,"%s", str1);
 }
sprintf(msg, "set yrange [%lf:%lf]\n", miny, maxy);
fprintf(f, "%s",msg);
fprintf(f2, "%s",msg);
sprintf(msg, "set xlabel \"%s_%s\"\n", str[0], tag[0]);
fprintf(f, "%s",msg);
fprintf(f2, "%s",msg);

cmd( "set choice $dia" );
if(*choice==1)
 {
  fprintf(f,"set arrow from %lf,%lf to %lf,%lf lt -1\n",miny, miny, maxy, maxy);
  fprintf(f2,"set arrow from %lf,%lf to %lf,%lf lt -1\n",miny, miny, maxy, maxy);
 }
if(line_point==1)
 sprintf(str1, "smooth csplines");
else
 strcpy(str1, "");

if(allblack)
 sprintf(str3, " lt -1");
else
 str3[0]=0;

 sprintf(msg,"plot 'data.gp' using 1:2 %s t \"t+1\"", str1 );
 if(allblack)
  strcat(msg, str3);
for(i=2; i<=nlags; i++)
 if(start[0]<=max_c && end[0]>=min_c)
  {sprintf(str2,", 'data.gp' using 1:%d %s t \"t+%d\"",i+1, str1, i);
   strcat(msg,str2);
   if(allblack)
    strcat(msg, str3);

  }

 strcat(msg,"\n");
fprintf(f, "%s",msg);
fprintf(f2, "%s",msg);

fprintf(f2, "pause -1 \"Close plot %d)\"\n", cur_plot);

fclose(f);
fclose(f2);
cmd( "set choice $gnu" );
show_plot_gnu(cur_plot, choice, *choice);
chdir(dirname);
end:
for(i=0; i<nv; i++)
 {delete[] str[i];
  delete[] tag[i];
  if(logs)
    delete[] logdata[i];
 }

delete[] str;
delete[] tag;

delete[] logdata;
delete[] data;
delete[] start;
delete[] end;
}


/***************************************************
SHOW_PLOT_GNU
****************************************************/
void show_plot_gnu(int n, int *choice, int type)
{

cmd( "if {$tcl_platform(platform) == \"unix\"} {set choice 1} {if {$tcl_platform(os) == \"Windows NT\"} { if { $tcl_platform(os) == \"4.0\" } {set choice 2} {set choice 3} } {set choice 4}}" );

switch (*choice)
{
//unix
case 1: cmd( "set choice [ catch { exec gnuplot gnuplot.lsd } ]" );
break;
//Win NT
case 2: cmd( "set choice [ catch { exec cmd /c start wgnuplot gnuplot.lsd } ]" );
break;
//Win 2K
case 3: cmd( "set choice [ catch { exec wgnuplot gnuplot.lsd } ]" );
break;
//Win 95/98/ME
case 4: cmd( "set choice [ catch { exec start wgnuplot gnuplot.lsd } ]" );
break;
}

if ( *choice != 0 )			// Gnuplot failed
{
	cmd( "tk_messageBox -parent .da -type ok -icon error -title Error -message \"Gnuplot returned error '$choice' during setup\" -detail \"Please check if you have selected an adequate configuration for the plot and if Gnuplot is set up properly.\"" );
	*choice=0;
	return;
}
	
cmd( "set p .da.f.new%d", n );

cmd( "toplevel $p" );
cmd( "wm title $p $tit" );
cmd( "if {$tcl_platform(platform) != \"windows\"} {wm iconbitmap $p @$RootLsd/$LsdSrc/icons/lsd.xbm} {}" );
cmd( "wm protocol $p WM_DELETE_WINDOW { wm withdraw .da.f.new%d}", cur_plot );
cmd( "bind $p <Double-Button-1> {raise .da}" );
cmd( "frame $p.f" );
cmd( "canvas $p.f.plots -height 430 -width 640 -bg white" );

cmd( "pack $p.f.plots" );
cmd( "pack $p.f" );
cmd( "update" );
shrink_gnufile();
cmd( "file delete plot.file; file rename plot_clean.file plot.file" );

if(type==0)
{
  cmd( "set answer [tk_messageBox -parent .da -type yesno -title Option -icon question -default yes -message \"Plot may be generated with higher quality by using Gnuplot\" -detail \"Press 'Yes' to generate a low-quality plot within Lsd or 'No' to generate a high-quality plot with Gnuplot.\"]" );
  cmd( "if {[string compare -nocase $answer \"yes\"] == 0} { set choice 1} {set choice 0}" );
  if(*choice ==0)
   type=2;
  else
   type=1;
}   
   
if(type==2)
 {//plot with external gnuplot
   cmd( "destroy $p" );
   cmd( "if {$tcl_platform(platform) == \"unix\"} { set choice [ catch { exec xterm -e gnuplot gnuplot.gp & } ] } {if {$tcl_platform(os) == \"Windows NT\"} { set choice [ catch { exec wgnuplot gnuplot.gp & } ] } { set choice [ catch { exec start wgnuplot gnuplot.gp & } ] } }" );
   if ( *choice != 0 )			// Gnuplot failed
		cmd( "tk_messageBox -parent .da -type ok -icon error -title Error -message \"Gnuplot returned error '$choice' during plotting\" -detail \"Please check if you have selected an adequate configuration for the plot.\"" );
 *choice=0;
 return;
 }

cmd( "source plot.file" );

cmd( "update idletasks" );
cmd( "set choice 0" );

  cmd( "catch [gnuplot $p.f.plots]" );
cmd( "catch [rename gnuplot \"\"]" );
}


/***************************************************
PLOT_LATTICE
****************************************************/
void plot_lattice(int *choice)
{

FILE *f, *f2;
double **data;
int i, nv, j, hi, le, done, nlags, ncol, nlin, end;

cmd( "set choice [.da.vars.ch.v size]" );
nv=*choice;
if ( nv == 0 )		 // no plots to save
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No plot available\" -detail \"Place one or more series in the Series Selected listbox and select 'Plot' to produce a plot.\"" );
	*choice = 2;
	return;
}

data=new double *[nv];
if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

cmd( "set res [.da.vars.ch.v get 0]" );
cmd( "scan $res \"%s %s (%d - %d) # %d\" a b c d choice" );
end=vs[nv].end;

for(i=0; i<nv; i++)
 {
  cmd( "set res [.da.vars.ch.v get %d]", i );
  cmd( "scan $res \"%s %s (%d - %d) # %d\" a b c d choice" );
  data[i]=vs[nv].data;
 }

cmd( "set bidi 1" );
cmd( "toplevel .da.s" );
cmd( "wm title .da.s \"Lattice Definition\"" );
cmd( "wm transient .da.s .da" );
cmd( "frame .da.s.i -relief groove -bd 2" );
cmd( "label .da.s.i.l -text \"Insert number of columns\"" );
cmd( "entry .da.s.i.e -validate focusout -vcmd { if [ string is integer %%P ] { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.i.e insert 0 $bidi" ); 
cmd( "pack .da.s.i.l .da.s.i.e" );
cmd( "set time %d", end );
cmd( "frame .da.s.t -relief groove -bd 2" );
cmd( "label .da.s.t.l -text \"Insert time step to use\"" );
cmd( "entry .da.s.t.e -validate focusout -vcmd { if [ string is integer %%P ] { set time %%P; return 1 } { %%W delete 0 end; %%W insert 0 $time; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.t.e insert 0 $time" ); 
cmd( "pack .da.s.t.l .da.s.t.e" );

cmd( "set lx 400" );
cmd( "frame .da.s.x -relief groove -bd 2" );
cmd( "label .da.s.x.l -text \"Lattice width\"" );
cmd( "entry .da.s.x.e -validate focusout -vcmd { if [ string is integer %%P ] { set lx %%P; return 1 } { %%W delete 0 end; %%W insert 0 $lx; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.x.e insert 0 $lx" ); 
cmd( "pack .da.s.x.l .da.s.x.e" );

cmd( "set ly 400" );
cmd( "frame .da.s.y -relief groove -bd 2" );
cmd( "label .da.s.y.l -text \"Lattice heigth\"" );
cmd( "entry .da.s.y.e -validate focusout -vcmd { if [ string is integer %%P ] { set ly %%P; return 1 } { %%W delete 0 end; %%W insert 0 $ly; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.y.e insert 0 $ly" ); 
cmd( "pack .da.s.y.l .da.s.y.e" );

cmd( "frame .da.s.b" );
cmd( "set p .da.s.b" );
cmd( "button $p.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "button $p.help -width -9 -text Help -command {LsdHelp mdatares.html#lattice}" );
cmd( "button $p.can -width -9 -text Cancel -command {set choice 2}" );
cmd( "pack $p.ok $p.help $p.can -side left -expand yes -fill x" );

cmd( "pack .da.s.t .da.s.i .da.s.x .da.s.y .da.s.b" );
cmd( "focus .da.s.t.e; .da.s.t.e selection range 0 end" );


cmd( "bind .da.s.t.e <KeyPress-Return> {focus .da.s.i.e; .da.s.i.e selection range 0 end}" );
cmd( "bind .da.s.i.e <KeyPress-Return> {focus .da.s.x.e; .da.s.x.e selection range 0 end}" );
cmd( "bind .da.s.x.e <KeyPress-Return> {focus .da.s.y.e; .da.s.y.e selection range 0 end}" );
cmd( "bind .da.s.y.e <KeyPress-Return> {set choice 1}" );
cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );
cmd( "set w .da.s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w" );

*choice=0;
while(*choice==0)
  Tcl_DoOneEvent(0);

cmd( "set bidi [ .da.s.i.e get ]" ); 
cmd( "set time [ .da.s.t.e get ]" ); 
cmd( "set lx [ .da.s.x.e get ]" ); 
cmd( "set ly [ .da.s.y.e get ]" ); 
cmd( "destroy .da.s" );

if(*choice==2)
 goto end;

cmd( "set choice $time" );
nlags=*choice;
cmd( "set choice $bidi" );
ncol=*choice;

nlin=nv/ncol;
if(nlin*ncol!=nv)
 {
 cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of columns\"" );
 *choice=2;
 goto end;
 }

cmd( "set choice $ly" );
hi=*choice/nlin;
cmd( "set choice $lx" );
le=*choice/ncol;

cmd( "set p .da.f.new%d", cur_plot );
cmd( "toplevel $p" );
cmd( "wm title $p $tit" );
cmd( "if {$tcl_platform(platform) != \"windows\"} {wm iconbitmap $p @$RootLsd/$LsdSrc/icons/lsd.xbm} {}" );
cmd( "wm protocol $p WM_DELETE_WINDOW { wm withdraw .da.f.new%d}", cur_plot );
cmd( "bind $p <Double-Button-1> {raise .da}" );
cmd( "frame $p.f -width %d -height %d", ncol*le, nlin*hi );

cmd( "pack $p.f" );

//Reset p to point to the canvas (shit...)
cmd( "set p $p.f.plots" );

cmd( "canvas $p -width %d -height %d -background white -relief flat", ncol*le, nlin*hi );
cmd( "pack $p" );

if(!grid)
{
for(j=0; j<nlin; j++)
 for(i=0; i<ncol; i++)
  cmd( "$p create poly %d %d %d %d %d %d %d %d -fill $c%d", i*le, j*hi, i*le, (j+1)*hi, (i+1)*le, (j+1)*hi, (i+1)*le, j*hi, (int)data[ncol*j+i][nlags] );
}
else
{
for(j=0; j<nlin; j++)
 for(i=0; i<ncol; i++)
  cmd( "$p create poly %d %d %d %d %d %d %d %d -fill $c%d -outline black", i*le, j*hi, i*le, (j+1)*hi, (i+1)*le, (j+1)*hi, (i+1)*le, j*hi, (int)data[ncol*j+i][nlags] );
}

*choice=0;
end:

delete[] data;
}


/*********
WORK IN PROGRESS
object *skip_next_obj(object *t, int *count);

void add_list(object *r)
{
int i=0;
object *cur;
variable *cv;

for(cur=r; cur!=NULL; cur=skip_next_obj(object cur, &i))
 {for(cv=cur->v; cv!=NULL; cv=cv->next)
    {
     cmd( "lappend series_bylab \"%s (Obj. %s)\"",cv->label, cur->label );
     cmd( "lappend series_byobj \"(Obj. %s) %s\"", cur->label, cv->label );

    }
  if(cur->son!=NULL)
   add_list(cur->son);

 }

}

void list_series(object *r)
{
cmd( "set a [info vars series_bylab]" );
cmd( "if {$a==\"\" } {} {unset series_bylab; unset series_byobj}" );
add_list(r);
cmd( "set temp [lsort -dictionary -integer $series_bylab]" );
cmd( "unset series_bylab" );
cmd( "set series_bylab $temp" );
cmd( "set temp [lsort -dictionary -integer $series_byobj]" );
cmd( "unset series_byobj" );
cmd( "set series_byobj $temp" );

}
*********/

struct bin
{
double num;
double min;
double max;
double center;
double av;
double lowb;
double highb;
};	

/***************************************************
HISTOGRAMS
****************************************************/
void histograms(int *choice)
{

int idseries;
char *app;
char str[MAX_ELEM_LENGTH];
char tag[MAX_ELEM_LENGTH];

int start, end;
int i, num_bin, j, first, last, stat;
int x1, x2, y1,y2;
double ap, mx,mn, *data, step, a, b, s, lminy, miny2, truemaxy, truemaxy2, average, sigma, tot, totnorm;
bin *cl;

cmd( "set choice [.da.vars.ch.v size]" );
if(*choice!=1)
 {
	if ( *choice == 0 )			// no variables selected
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one series in the Series Selected listbox.\"" );
	else
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of series\" -detail \"For time series histograms select only one series.\"" );
	*choice=2;
	return;
 } 
 
cmd( "set res [.da.vars.ch.v get 0]" );
app=(char *)Tcl_GetVar(inter, "res",0);
strcpy(msg,app);
sscanf(msg, "%s %s (%d - %d) # %d", str, tag, &start, &end, &idseries);

data=find_data(idseries);
if(autom_x)
 {first=start;
  last=end;
 }
else
 {
 if(min_c>start)
   first=min_c;
 else
   first=start;
 if(max_c<end)  
   last=max_c;
 else
   last=end;
 if(first>=last) 
  {
   first=start;
   last=end;
  }
 }  

for(tot=0, i=first; i<=last; i++)	// count number of points excluding NaNs
 if(is_finite(data[i]))				// ignore NaNs
  tot++;

cmd( "set bidi %d", 100<tot?100:(int)tot );

cmd( "toplevel .da.s" );
cmd( "wm title .da.s \"Number of Classes\"" );
cmd( "wm transient .da.s .da" );
cmd( "frame .da.s.i -relief groove -bd 2" );
cmd( "label .da.s.i.l -text \"Insert the number of classes to use\"" );
cmd( "entry .da.s.i.e -validate focusout -vcmd { if [ string is integer %%P ] { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.i.e insert 0 $bidi" ); 
cmd( "set norm 0" );
cmd( "checkbutton .da.s.i.norm -text \"Interpolate a Normal\" -variable norm" );
cmd( "set stat 0" );
cmd( "checkbutton .da.s.i.st -text \"Print statistics in Log window\" -variable stat" );
cmd( "pack .da.s.i.l .da.s.i.e .da.s.i.norm .da.s.i.st -anchor w" );

cmd( "frame .da.s.b" );
cmd( "set p .da.s.b" );
cmd( "button $p.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "button $p.help -width -9 -text Help -command {LsdHelp mdatares.html#seq_xy}" );
cmd( "button $p.can -width -9 -text Cancel -command {set choice 2}" );
cmd( "pack $p.ok $p.help $p.can -side left -expand yes -fill x" );

cmd( "pack .da.s.i .da.s.b" );
cmd( "focus .da.s.i.e; .da.s.i.e selection range 0 end" );

cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );
cmd( " .da.s.i.e selection range 0 end" );
cmd( "bind .da.s.i.e <KeyPress-Return> {set choice 1}" );
cmd( "set w .da.s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w" );

*choice=0;
while(*choice==0)
  Tcl_DoOneEvent(0);

cmd( "set bidi [ .da.s.i.e get ]" ); 
cmd( "destroy .da.s" );

if(*choice==2)
 return;

cmd( "set choice $bidi" );
num_bin=*choice;
cl=new bin[num_bin];
ap=(double)num_bin;

tot=average=sigma=0;
for(i=first; i<=last; i++)
 {
 if(!is_finite(data[i]))		// ignore NaNs
  continue;
 if(i==first)
  mx=mn=data[i];
 else
  {
  if(data[i]>mx)
   mx=data[i];
  else
   if(data[i]<mn)
    mn=data[i];
  }  
  average+=data[i];
  sigma+=data[i]*data[i];
  tot++;
 }
average=average/tot;
sigma=sigma/tot-average*average;

for(i=0; i<num_bin; i++)
 {
 cl[i].num=0;
 cl[i].av=0;
 cl[i].min=0;
 cl[i].max=0;
 cl[i].center=0; 
  
 }
 
step=(mx-mn)/(ap); 
for(i=first; i<=last; i++)
 {
  if(!is_finite(data[i]))		// ignore NaNs
   continue;
  a=floor( ap*(data[i]-mn)/(mx-mn));
  s=ap*(data[i]-mn)/(mx-mn);
    
  j=(int)a;
  if(j==num_bin)
   j--;

  if(cl[j].num==0)
   {cl[j].min=cl[j].max=data[i];
   }
  else
   {
   if(cl[j].min>data[i])
    cl[j].min=data[i];
   else
   if(cl[j].max<data[i])
    cl[j].max=data[i];
   }  
  cl[j].num++;   
  cl[j].av+=data[i];
 } 

a=(mx-mn)/(ap-1);
for(i=1; i<num_bin; i++)
 {
  if(cl[i].num!=0 && cl[i-1].num!=0 && cl[i].min-cl[i-1].max<a)
    a=cl[i].min-cl[i-1].max;
 }

cmd( "set choice $stat" );
stat = *choice;

if(stat==1)
 plog("\n\n#    Boundaries(center)\t\tMin\tAve\tMax\tNum.\tFreq.");
step=(mx+a/2-(mn-a/2))/(ap);

lminy=tot;		// consider only non-NaNs
truemaxy=0;
for(i=0; i<num_bin; i++)
 {if(cl[i].num!=0)
    cl[i].av/=cl[i].num;
  cl[i].lowb=mn-a/2+(double)(i)*step;
  cl[i].highb=mn -a/2 + (double)(i+1)*step;
  cl[i].center=  cl[i].highb/2+  cl[i].lowb/2;
 if(stat==1)
 {
 plog( "\n%3d: %.*g<=X<%.*g (%.*g)\t\t%.*g\t%.*g\t%.*g\t%.*g\t%.*g", "", i+1, pdigits, mn-a/2+(double)(i)*step, pdigits, mn -a/2 + (double)(i+1)*step, pdigits, mn -a/2 +(double)(i)*step +step/2, pdigits, cl[i].min, pdigits, cl[i].av, pdigits, cl[i].max, pdigits, cl[i].num, pdigits, cl[i].num/(tot) );
 }
 if(cl[i].num<lminy)
  {
   lminy=cl[i].num;
   miny2=cl[i].num/tot;
  } 
 if(cl[i].num>truemaxy)
  {
   truemaxy=cl[i].num;
   truemaxy2=cl[i].num/tot;
  } 
 }

if(stat==1)
 cmd( "wm deiconify .log; raise .log .da" ); 
if(!autom && miny<maxy)
 {
  lminy=miny;
  miny2=lminy/tot;
  truemaxy=maxy;
  truemaxy2=maxy/tot;
 }
else
 {
  cmd( ".da.f.h.v.sc.max.max conf -state normal" );
  cmd( ".da.f.h.v.sc.min.min conf -state normal" );  
  maxy=truemaxy;
  miny=lminy;
  cmd( "update" );
  cmd( ".da.f.h.v.sc.max.max conf -state disabled" );
  cmd( ".da.f.h.v.sc.min.min conf -state disabled" );  
 }


cmd( "set p .da.f.new%d", cur_plot );

cmd( "toplevel $p" );
cmd( "wm title $p $tit" );
cmd( "if {$tcl_platform(platform) != \"windows\"} {wm iconbitmap $p @$RootLsd/$LsdSrc/icons/lsd.xbm} {}" );
cmd( "wm protocol $p WM_DELETE_WINDOW { wm withdraw .da.f.new%d}", cur_plot );
cmd( "bind $p <Double-Button-1> {raise .da; focus .da.b.ts}" );
cmd( "frame $p.f -width 640 -height 430" );
cmd( "pack $p.f" );

//Reset p to point to the canvas (shit...)
cmd( "set p $p.f.plots" );

cmd( "canvas $p -width 640 -height 430 -background white -relief flat" );
cmd( "pack $p" );

cmd( "$p create line 40 300 640 300 -width 1 -fill grey50 -tag p" );

if(grid)
{
cmd( "$p create line 40 0 40 310 -fill grey60 -width 1p -tag p" );
cmd( "$p create line 190 0 190 310 -fill grey60 -width 1p -tag p" );
cmd( "$p create line 340 0 340 310 -fill grey60 -width 1p -tag p" );
cmd( "$p create line 490 0 490 310 -fill grey60 -width 1p -tag p" );
}
else
{
cmd( "$p create line 640 295 640 305  -width 1 -tag p" );
cmd( "$p create line 190 295 190 305 -width 1 -tag p" );
cmd( "$p create line 340 295 340 305 -width 1 -tag p" );
cmd( "$p create line 490 295 490 305 -width 1 -tag p" );
}

cmd( "set choice [$p create text 40 312 -font {Times 10 normal} -anchor nw -text %.*g -tag {p text}]", pdigits,cl[0].lowb );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

i=num_bin/4;
cmd( "set choice [$p create text 190 312 -font {Times 10 normal} -anchor n -text %.*g -tag {p text}]", pdigits,cl[i].center );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

if(num_bin%2==0)
 a=cl[num_bin/2].lowb;
else
 a=cl[(num_bin-1)/2].center; 
i=num_bin/2;
cmd( "set choice [$p create text 340 312 -font {Times 10 normal} -anchor n -text %.*g -tag {p  text}]", pdigits,a );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

i=(int)((double)(num_bin)*.75);
cmd( "set choice [$p create text 490 312 -font {Times 10 normal} -anchor n -text %.*g -tag {p text}]", pdigits,cl[i].center );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );


cmd( "set choice [$p create text 635 312 -font {Times 10 normal} -anchor ne -text %.*g -tag {p  text}]", pdigits,cl[num_bin-1].highb );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "$p create line 40 0 40 300 -width 1 -fill grey50 -tag p" );

if(grid)
{
cmd( "$p create line 38 300 640 300 -fill grey60 -tag p" );
cmd( "$p create line 38 225 640 225 -fill grey60 -tag p" );
cmd( "$p create line 38 150 640 150 -fill grey60 -tag p" );
cmd( "$p create line 38 75 640 75 -fill grey60 -tag p" );
cmd( "$p create line 38 0 640 0 -fill grey60 -tag p" );
}
else
{
cmd( "$p create line 35 225 45 225  -tag p" );
cmd( "$p create line 35 150 45 150  -tag p" );
cmd( "$p create line 35 75 45 75  -tag p" );
cmd( "$p create line 35 2 45 2 -tag p" );
}

cmd( "set choice [$p create text 4 300 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,lminy );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 225 -font {Times 10 normal} -anchor sw -text %.*g -tag {p  text}]", pdigits,(lminy+(truemaxy-lminy)/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );


cmd( "set choice [$p create text 4 150 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,(lminy+(truemaxy-lminy)/2) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );


cmd( "set choice [$p create text 4 75 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,(lminy+(truemaxy-lminy)*3/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );


cmd( "set choice [$p create text 4 4 -font {Times 10 normal} -anchor nw -text %.*g -tag {p text}]", pdigits,(truemaxy) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );


cmd( "set choice [$p create text 4 312 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,miny2 );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );


cmd( "set choice [$p create text 4 237 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,(miny2+(truemaxy2-miny2)/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );


cmd( "set choice [$p create text 4 162 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,(miny2+(truemaxy2-miny2)/2) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );


cmd( "set choice [$p create text 4 87 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,(miny2+(truemaxy2-miny2)*3/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 16 -font {Times 10 normal} -anchor nw -text (%.*g) -tag {p text}]", pdigits,(truemaxy2) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

*choice=0;
for(i=0; i<num_bin; i++)
 {
  x1=40+(int)(600* (cl[i].lowb-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
  x2=40+(int)(600* (cl[i].highb-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
  y1=(int)(300-((cl[i].num-lminy)/(truemaxy-lminy))*300);
  y2=300;

  if(y1<301)
  {//in case of user-definde miny
   cmd( "$p create rect %d %d %d %d -tag p%d -width 1 -fill $c%d",x1,y1,x2,y2, i, 1 );
   cmd( "$p bind p%d <Enter> {.da.f.new%d.f.plots delete xpos; .da.f.new%d.f.plots create text 0 390 -font {Times 10 normal} -text \"Class %d, %.*g <= X < %.*g, (center=%.*g)\\nContains %.*g units (%.*g perc.). Actual values: min=%.*g, av.=%.*g, max=%.*g  \" -anchor w -tag xpos}", i,  cur_plot, cur_plot, i, pdigits, cl[i].lowb, pdigits, cl[i].highb, pdigits, cl[i].center, pdigits, cl[i].num, pdigits, 100*cl[i].num/tot, pdigits, cl[i].min, pdigits, cl[i].av, pdigits, cl[i].max  );
   cmd( "$p bind p%d <Leave> {.da.f.new%d.f.plots delete xpos}", i, cur_plot );
  }
  if(*choice==1) //Button STOP pressed
     i=num_bin;
 if(watch)
	cmd( "update" );
 }

/***
just to check that the sum is one
*/
cmd( "set choice $norm" );
if(*choice==1)
{
 totnorm=0;
 for(i=0; i<num_bin; i++)
  {
   a=cl[i].lowb;
   b=exp(-(a-average)*(a-average)/(2*sigma))/(sqrt(2*PI*sigma));
   s=cl[i].highb;
   ap=exp(-(s-average)*(s-average)/(2*sigma))/(sqrt(2*PI*sigma));
   totnorm+=(b+ap)/2;
  }
  
 /*******/
 for(i=0; i<num_bin-1; i++)
  {
   a=cl[i].center;
   b=exp(-(a-average)*(a-average)/(2*sigma))/(sqrt(2*PI*sigma));
   b=b*tot/totnorm;
   b=300-((b-lminy)/(truemaxy-lminy))*300;    
   a=cl[i+1].center;  
   s=exp(-(a-average)*(a-average)/(2*sigma))/(sqrt(2*PI*sigma));
   s=s*tot/totnorm;
   s=300-((s-lminy)/(truemaxy-lminy))*300;  
   
   x1=40+(int)(600* (cl[i].center-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
   x2=40+(int)(600* (cl[i+1].center-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
 
   cmd( "$p create line %d %d %d %d",x1,(int)b, x2, (int)s );
  }
} 
    
cmd( "bind $p <Shift-1> {set ncanvas %d; set LX %%x; set LY %%y; set hereX %%X ; set hereY %%y; set choice 27}", cur_plot );
*choice=0;

delete[] cl;
}

/***************************************************
HISTOGRAMS CS
****************************************************/
void histograms_cs(int *choice)
{

int idseries;
char *app;
char **str;
char **tag;
double **data,**logdata;
int *start, *end;
int i, num_bin, j, first, last, stat, nv, time, active_v;
int x1, x2, y1,y2;
double ap, mx,mn,  step, a, b, s, lminy, miny2, truemaxy, truemaxy2, average, sigma, tot, totnorm;
bin *cl;

int logErrCnt = 0;				// log errors counter to prevent excess messages
bool stopErr = false;

cmd( "set choice [.da.vars.ch.v size]" );
nv=*choice;
if(nv<2)
{
	if ( nv == 0 )			// no variables selected
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place two or more series in the Series Selected listbox.\"" );
	else
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of series\" -detail \"For Cross Section histograms select more than two series.\"" );
	*choice=2;
	return;
}

data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];

if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  data[i]=find_data(idseries);
  
  /*****************
  IT IS CS, SO THE TIME MUST BE DECIDED YET
  if(autom_x ||(start[i]<=max_c && end[i]>=min_c))
   {
   data[i]=find_data(idseries);
   }
  *********************/
   
   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!is_nan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
		 {
			plog( "\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d\n", "", i + 1, j );
		 }
		 else
			if ( ! stopErr )
			{
				plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
				stopErr = true;
			}
	   }
	 data[i]=logdata[i];				// replace the data series
   }
 }

cmd( "set bidi %d", 100<nv?100:nv );

cmd( "set time %d", end[0] );

cmd( "toplevel .da.s" );
cmd( "wm title .da.s \"Number of Classes\"" );
cmd( "wm transient .da.s .da" );
cmd( "frame .da.s.t -relief groove -bd 2" );
cmd( "label .da.s.t.l -text \"Insert the time step to use\"" );
cmd( "entry .da.s.t.e -validate focusout -vcmd { if [ string is integer %%P ] { set time %%P; return 1 } { %%W delete 0 end; %%W insert 0 $time; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.t.e insert 0 $time" ); 
cmd( "bind .da.s.t.e <Return> {focus .da.s.i.e; .da.s.i.e selection range 0 end}" );
cmd( "pack .da.s.t.l .da.s.t.e -anchor w" );

cmd( "frame .da.s.i -relief groove -bd 2" );
cmd( "label .da.s.i.l -text \"Insert the number of classes to use\"" );
cmd( "entry .da.s.i.e -validate focusout -vcmd { if [ string is integer %%P ] { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.i.e insert 0 $bidi" ); 
cmd( "set norm 0" );
cmd( "checkbutton .da.s.i.norm -text \"Interpolate a Normal\" -variable norm" );
cmd( "set stat 0" );
cmd( "checkbutton .da.s.i.st -text \"Print statistics in Log window\" -variable stat" );
cmd( "pack .da.s.i.l .da.s.i.e .da.s.i.norm .da.s.i.st -anchor w" );

cmd( "frame .da.s.b" );
cmd( "set p .da.s.b" );
cmd( "button $p.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "button $p.help -width -9 -text Help -command {LsdHelp mdatares.html#seq_xy}" );
cmd( "button $p.can -width -9 -text Cancel -command {set choice 2}" );
cmd( "pack $p.ok $p.help $p.can -side left -expand yes -fill x" );

cmd( "pack .da.s.t .da.s.i .da.s.b" );
cmd( "focus .da.s.t.e; .da.s.t.e selection range 0 end" );

cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );

cmd( "bind .da.s.i.e <KeyPress-Return> {set choice 1}" );
cmd( "set w .da.s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w" );

*choice=0;
while(*choice==0)
  Tcl_DoOneEvent(0);

cmd( "set bidi [ .da.s.i.e get ]" ); 
cmd( "set time [ .da.s.t.e get ]" ); 
cmd( "destroy .da.s" );

if(*choice==2)
 {
 delete[] start;
 delete[] end;
 delete[] data;
 for(i=0; i<nv; i++)
  {
  delete[] str[i];
  delete[] tag[i];
  
  if(logs)
    delete[] logdata[i]; 
  }
 delete[] logdata;
 delete[] str;
 delete[] tag; 
 
 return;
 }

cmd( "set choice $bidi" );
num_bin=*choice;
cmd( "set choice $time" );
time=*choice;

cl=new bin[num_bin];
ap=(double)num_bin;

tot=average=sigma=0;
active_v=0;

if(autom)
{//min and max fixed automatically
for(i=0; i<nv; i++)
 {
 if(start[i]<=time && end[i]>=time && is_finite(data[i][time]))		// ignore NaNs
 {
 if(active_v==0)
  mx=mn=data[i][time];
 else
  {
  if(data[i][time]>mx)
   mx=data[i][time];
  else
   if(data[i][time]<mn)
    mn=data[i][time];
  }  
  average+=data[i][time];
  sigma+=data[i][time]*data[i][time];
  tot++;
  active_v++;
  }
 }
}
else
{//min and max set by the user
mx=maxy;
mn=miny;
for(i=0; i<nv; i++)
 {
 if(start[i]<=time && end[i]>=time && is_finite(data[i][time]) && data[i][time]>=mn && data[i][time]<=mx)		// ignore NaNs
 {
  average+=data[i][time];
  sigma+=data[i][time]*data[i][time];
  tot++;
  active_v++;
  }
 }
}

if(tot==0)
	tot=1;
average=average/tot;
sigma=sigma/tot-average*average;

for(i=0; i<num_bin; i++)
 {
 cl[i].num=0;
 cl[i].av=0;
 cl[i].min=0;
 cl[i].max=0;  
 cl[i].center=0; 
 }
 
step=(mx-mn)/(ap); 
for(i=0; i<nv; i++)
 {
  if(start[i]<=time && end[i]>=time && is_finite(data[i][time]) && data[i][time]>=mn && data[i][time]<=mx)		// ignore NaNs
  {
  a=floor( ap*(data[i][time]-mn)/(mx-mn));
  s=ap*(data[i][time]-mn)/(mx-mn);
    
  j=(int)a;
  if(j==num_bin)
   j--;

  if(cl[j].num==0)
   {cl[j].min=cl[j].max=data[i][time];
   }
  else
   {
   if(cl[j].min>data[i][time])
    cl[j].min=data[i][time];
   else
   if(cl[j].max<data[i][time])
    cl[j].max=data[i][time];
   }  
  cl[j].num++;   
  cl[j].av+=data[i][time];
  
  }
 } 

a=(mx-mn)/(ap-1);
for(i=1; i<num_bin; i++)
 {
  if(cl[i].num!=0 && cl[i-1].num!=0 && cl[i].min-cl[i-1].max<a)
    a=cl[i].min-cl[i-1].max;
 }

cmd( "set choice $stat" );
stat = *choice;

if(stat==1)
 plog("\n\n#    Boundaries(center)\t\tMin\tAve\tMax\tNum.\tFreq.");
step=(mx+a/2-(mn-a/2))/(ap);

lminy=active_v;
truemaxy=0;
for(i=0; i<num_bin; i++)
 {
 if(cl[i].num!=0)
    cl[i].av/=cl[i].num;
  cl[i].lowb=mn-a/2+(double)(i)*step;
  cl[i].highb=mn -a/2 + (double)(i+1)*step;
  cl[i].center=  cl[i].highb/2+  cl[i].lowb/2;
 if(stat==1)
 {
 plog( "\n%3d %.*g<=X<%.*g (%.*g)\t\t%.*g\t%.*g\t%.*g\t%.*g\t%.*g", "", i+1, pdigits, mn-a/2+(double)(i)*step, pdigits, mn -a/2 + (double)(i+1)*step, pdigits, mn -a/2 +(double)(i)*step +step/2, pdigits, cl[i].min, pdigits, cl[i].av, pdigits, cl[i].max, pdigits, cl[i].num, pdigits, cl[i].num/(tot) );
 }
 if(cl[i].num<lminy)
  {
   lminy=cl[i].num;
   miny2=cl[i].num/tot;
  } 
 if(cl[i].num>truemaxy)
  {
   truemaxy=cl[i].num;
   truemaxy2=cl[i].num/tot;
  } 
 }

if(stat==1)
 cmd( "wm deiconify .log; raise .log .da" ); 
if(!autom && miny<maxy)
 {
  lminy=miny;
  miny2=lminy/tot;
  truemaxy=maxy;
  truemaxy2=maxy/tot;
 }
else
 {
  cmd( ".da.f.h.v.sc.max.max conf -state normal" );
  cmd( ".da.f.h.v.sc.min.min conf -state normal" );  
  maxy=truemaxy;
  miny=lminy;
  cmd( "update" );
  cmd( ".da.f.h.v.sc.max.max conf -state disabled" );
  cmd( ".da.f.h.v.sc.min.min conf -state disabled" );  
 }

cmd( "set p .da.f.new%d", cur_plot );

cmd( "toplevel $p" );
cmd( "wm title $p $tit" );
cmd( "if {$tcl_platform(platform) != \"windows\"} {wm iconbitmap $p @$RootLsd/$LsdSrc/icons/lsd.xbm} {}" );
cmd( "wm protocol $p WM_DELETE_WINDOW { wm withdraw .da.f.new%d}", cur_plot );
cmd( "bind $p <Double-Button-1> {raise .da; focus .da.b.ts}" );
cmd( "frame $p.f -width 640 -height 430" );
cmd( "pack $p.f" );

//Reset p to point to the canvas (shit...)
cmd( "set p $p.f.plots" );

cmd( "canvas $p -width 640 -height 430 -background white -relief flat" );
cmd( "pack $p" );

cmd( "$p create line 40 300 640 300 -width 1 -fill grey50 -tag p" );

if(grid)
{
cmd( "$p create line 40 0 40 310 -fill grey60 -width 1p -tag p" );
cmd( "$p create line 190 0 190 310 -fill grey60 -width 1p -tag p" );
cmd( "$p create line 340 0 340 310 -fill grey60 -width 1p -tag p" );
cmd( "$p create line 490 0 490 310 -fill grey60 -width 1p -tag p" );
}
else
{
cmd( "$p create line 640 295 640 305  -width 1 -tag p" );
cmd( "$p create line 190 295 190 305 -width 1 -tag p" );
cmd( "$p create line 340 295 340 305 -width 1 -tag p" );
cmd( "$p create line 490 295 490 305 -width 1 -tag p" );
}

cmd( "set choice [$p create text 40 312 -font {Times 10 normal} -anchor nw -text %.*g -tag {p text}]", pdigits,cl[0].lowb );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

i=num_bin/4;
cmd( "set choice [$p create text 190 312 -font {Times 10 normal} -anchor n -text %.*g -tag {p text}]", pdigits,cl[i].center );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

if(num_bin%2==0)
 a=cl[num_bin/2].lowb;
else
 a=cl[(num_bin-1)/2].center; 
i=num_bin/2;
cmd( "set choice [$p create text 340 312 -font {Times 10 normal} -anchor n -text %.*g -tag {p  text}]", pdigits,a );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

i=(int)((double)(num_bin)*.75);
cmd( "set choice [$p create text 490 312 -font {Times 10 normal} -anchor n -text %.*g -tag {p text}]", pdigits,cl[i].center );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 635 312 -font {Times 10 normal} -anchor ne -text %.*g -tag {p  text}]", pdigits,cl[num_bin-1].highb );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "$p create line 40 0 40 300 -width 1 -fill grey50 -tag p" );

if(grid)
{
cmd( "$p create line 38 300 640 300 -fill grey60 -tag p" );
cmd( "$p create line 38 225 640 225 -fill grey60 -tag p" );
cmd( "$p create line 38 150 640 150 -fill grey60 -tag p" );
cmd( "$p create line 38 75 640 75 -fill grey60 -tag p" );
cmd( "$p create line 38 0 640 0 -fill grey60 -tag p" );
}
else
{
cmd( "$p create line 35 225 45 225  -tag p" );
cmd( "$p create line 35 150 45 150  -tag p" );
cmd( "$p create line 35 75 45 75  -tag p" );
cmd( "$p create line 35 2 45 2 -tag p" );
}

cmd( "set choice [$p create text 4 300 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,lminy );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 225 -font {Times 10 normal} -anchor sw -text %.*g -tag {p  text}]", pdigits,(lminy+(truemaxy-lminy)/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 150 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,(lminy+(truemaxy-lminy)/2) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 75 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,(lminy+(truemaxy-lminy)*3/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 4 -font {Times 10 normal} -anchor nw -text %.*g -tag {p text}]", pdigits,(truemaxy) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 312 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,miny2 );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 237 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,(miny2+(truemaxy2-miny2)/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 162 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,(miny2+(truemaxy2-miny2)/2) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 87 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,(miny2+(truemaxy2-miny2)*3/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 16 -font {Times 10 normal} -anchor nw -text (%.*g) -tag {p text}]", pdigits,(truemaxy2) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

*choice=0;
for(i=0; i<num_bin; i++)
 {
  x1=40+(int)(600* (cl[i].lowb-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
  x2=40+(int)(600* (cl[i].highb-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
  y1=(int)(300-((cl[i].num-lminy)/(truemaxy-lminy))*300);
  y2=300;

  if(y1<301)
  {//in case of user-definde miny
  cmd( "$p create rect %d %d %d %d -tag p%d -width 1 -fill $c%d",x1,y1,x2,y2, i, 1 );
  cmd( "$p bind p%d <Enter> {.da.f.new%d.f.plots delete xpos; .da.f.new%d.f.plots create text 0 390 -font {Times 10 normal} -text \"Class %d, %.*g <= X < %.*g, (center=%.*g)\\nContains %.*g units (%.*g perc.). Actual values: min=%.*g, av.=%.*g, max=%.*g  \" -anchor w -tag xpos}",i,  cur_plot, cur_plot, i, pdigits, cl[i].lowb, pdigits, cl[i].highb, pdigits, cl[i].center, pdigits, cl[i].num, pdigits, 100*cl[i].num/tot, pdigits, cl[i].min, pdigits, cl[i].av, pdigits, cl[i].max );
  cmd( "$p bind p%d <Leave> {.da.f.new%d.f.plots delete xpos}",i, cur_plot );
  }
  if(*choice==1) //Button STOP pressed
     i=num_bin;
 if(watch)
	cmd( "update" );
 }

/***
just to check that the sum is one
*/
cmd( "set choice $norm" );
if(*choice==1)
{
 totnorm=0;
 for(i=0; i<num_bin; i++)
  {
   a=cl[i].lowb;
   b=exp(-(a-average)*(a-average)/(2*sigma))/(sqrt(2*PI*sigma));
   s=cl[i].highb;
   ap=exp(-(s-average)*(s-average)/(2*sigma))/(sqrt(2*PI*sigma));
   totnorm+=(b+ap)/2;
  }
  
 /*******/
 for(i=0; i<num_bin-1; i++)
  {
   a=cl[i].center;
   b=exp(-(a-average)*(a-average)/(2*sigma))/(sqrt(2*PI*sigma));
   b=b*tot/totnorm;
   b=300-((b-lminy)/(truemaxy-lminy))*300;    
   a=cl[i+1].center;  
   s=exp(-(a-average)*(a-average)/(2*sigma))/(sqrt(2*PI*sigma));
   s=s*tot/totnorm;
   s=300-((s-lminy)/(truemaxy-lminy))*300;  
   
   x1=40+(int)(600* (cl[i].center-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
   x2=40+(int)(600* (cl[i+1].center-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
 
   cmd( "$p create line %d %d %d %d",x1,(int)b, x2, (int)s );
  }
} 
    
cmd( "bind $p <Shift-1> {set ncanvas %d; set LX %%x; set LY %%y; set hereX %%X ; set hereY %%y; set choice 27}", cur_plot );

*choice=0;

delete[] cl;
delete[] start;
delete[] end;
delete[] data;
for(i=0; i<nv; i++)
 {
 delete[] str[i];
 delete[] tag[i];
 if(logs)
   delete[] logdata[i];
 }
delete[] logdata;
delete[] str;
delete[] tag; 
}


/***************************************************
CREATE_SERIES
****************************************************/
void create_series(int *choice)
{
int i, nv, j, k, *start, *end, idseries, flt;
double nmax, nmin, nmean, nvar, nn, thflt, confi;
double step;
bool first;
char *lapp, **str, **tag;
store *app;

Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);
cmd( "set nv [.da.vars.ch.v size]" );
Tcl_UnlinkVar(inter, "nv");

if ( nv == 0 )
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
	return;
}

double **data;

if(logs)
  cmd( "tk_messageBox -parent .da -type ok -icon warning -title Warning -message \"Series in logs not allowed\" -detail \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"" );

Tcl_LinkVar(inter, "thflt", (char *) &thflt, TCL_LINK_DOUBLE);
Tcl_LinkVar(inter, "confi", (char *) &confi, TCL_LINK_DOUBLE);

thflt=0;
confi=1.96;
cmd( "set flt 0" );
cmd( "set bido 1" );
cmd( "set bidi 1" );

cmd( "toplevel .da.s" );
cmd( "wm title .da.s \"Elaboration Selection\"" );
cmd( "wm transient .da.s .da" );

cmd( "frame .da.s.o -relief groove -bd 2" );
cmd( "label .da.s.o.l -text \"Type of scanning\" -fg red" );
cmd( "pack .da.s.o.l" );
cmd( "radiobutton .da.s.o.m -text \"Compute over series (same # of cases)\" -variable bido -value 1" );
cmd( "radiobutton .da.s.o.f -text \"Compute over cases (# cases = # of series)\" -variable bido -value 2" );
cmd( "pack .da.s.o.m .da.s.o.f -anchor w" );

cmd( "pack .da.s.o" );

cmd( "frame .da.s.f -relief groove -bd 2" );
cmd( "label .da.s.f.l -text \"Filtering\" -fg red" );
cmd( "pack .da.s.f.l" );
cmd( "radiobutton .da.s.f.n -text \"Use all the data\" -variable flt -value 0" );
cmd( "radiobutton .da.s.f.s -text \"Ignore small values\" -variable flt -value 1" );
cmd( "radiobutton .da.s.f.b -text \"Ignore large values\" -variable flt -value 2" );
cmd( "entry .da.s.f.th -width 21 -validate focusout -vcmd { if [ string is double %%P ] { set thflt %%P; return 1 } { %%W delete 0 end; %%W insert 0 $thflt; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.f.th insert 0 $thflt" ); 
cmd( "pack .da.s.f.n .da.s.f.s .da.s.f.b .da.s.f.th  -anchor w" );

cmd( "pack .da.s.f" );

cmd( "frame .da.s.i -relief groove -bd 2" );
/**/
cmd( "label .da.s.i.l -text \"Type of series to create\" -fg red" );

cmd( "radiobutton .da.s.i.m -text \"Average\" -variable bidi -command {set headname \"Av\"; set vname $headname$basename; .da.s.nv selection range 0 end} -value 1" );
cmd( "radiobutton .da.s.i.z -text \"Sum\" -variable bidi -command {set headname \"Sum\"; set vname $headname$basename; .da.s.nv selection range 0 end} -value 5" );
cmd( "radiobutton .da.s.i.f -text \"Maximum\" -variable bidi -command {set headname \"Max\"; set vname $headname$basename; .da.s.nv selection range 0 end} -value 2" );
cmd( "radiobutton .da.s.i.t -text \"Minimum\" -variable bidi -command {set headname \"Min\"; set vname $headname$basename; .da.s.nv selection range 0 end} -value 3" );
cmd( "radiobutton .da.s.i.c -text \"Variance\" -variable bidi -command {set headname \"Var\"; set vname $headname$basename; .da.s.nv selection range 0 end} -value 4" );
cmd( "frame .da.s.i.ci" );
cmd( "radiobutton .da.s.i.ci.c -text \"StdDev\" -variable bidi -command {set headname \"CI\"; set vname $headname$basename; .da.s.nv selection range 0 end} -value 6" );
cmd( "label .da.s.i.ci.x -text \"x\"" );
cmd( "entry .da.s.i.ci.p -width 4 -validate focusout -vcmd { if [ string is double %%P ] { set confi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $confi; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.i.ci.p insert 0 $confi" ); 
cmd( "pack .da.s.i.ci.c .da.s.i.ci.x .da.s.i.ci.p -side left" );

cmd( "radiobutton .da.s.i.n -text \"Count\" -variable bidi -command {set headname \"Num\"; set vname $headname$basename; .da.s.nv selection range 0 end} -value 7" );

cmd( "pack .da.s.i.l .da.s.i.m .da.s.i.z .da.s.i.f .da.s.i.t .da.s.i.c .da.s.i.ci .da.s.i.n -anchor w" );
/**/
cmd( "pack .da.s.i" );

cmd( "set a [.da.vars.ch.v get 0]" );
cmd( "set basename [lindex [split $a] 0]" );
cmd( "set headname \"Av\"" );
cmd( "set vname $headname$basename" );

cmd( "label .da.s.lnv -text \"New series label\" -fg red" );
cmd( "entry .da.s.nv -width 40 -textvariable vname" );
cmd( "pack .da.s.lnv .da.s.nv" );

cmd( "set vtag 1" );
cmd( "label .da.s.tnv -text \"New series tag\" -fg red" );
cmd( "entry .da.s.tv -width 20 -textvariable vtag" );
cmd( "pack .da.s.tnv .da.s.tv" );

cmd( "button .da.s.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "button .da.s.help -width -9 -text Help -command {LsdHelp mdatares.html#create_series}" );
cmd( "button .da.s.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "pack .da.s.i .da.s.ok .da.s.help .da.s.esc" );
cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );
cmd( "set w .da.s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w" );

cmd( "focus .da.s.nv" );
cmd( ".da.s.nv selection range 0 end" );
 
*choice=0;
while(*choice==0)
	Tcl_DoOneEvent(0);

cmd( "set thflt [ .da.s.f.th get ]" ); 
cmd( "set confi [ .da.s.i.ci.p get ]" ); 
Tcl_UnlinkVar(inter,"thflt");
Tcl_UnlinkVar(inter,"confi");
cmd( "destroy .da.s" );

if(*choice==2)
 {*choice=0;
  return;
 }

data=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];

 app=new store[1+ num_var];
 for(i=0; i<num_var; i++)
  {app[i]=vs[i];
   strcpy(app[i].label, vs[i].label);
   strcpy(app[i].tag, vs[i].tag);
  } 
 delete[] vs;
 vs=app;

if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  lapp=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,lapp);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  if(autom_x ||(start[i]<=max_c && end[i]>=min_c))
   {

   data[i]=find_data(idseries);
   }
 }

if(autom_x||min_c>=max_c)
{

for(i=0; i<nv; i++)
 {if(i==0)
   min_c=max_c=start[i];
  if(start[i]<min_c)
   min_c=start[i];
  if(end[i]>max_c)
   max_c=end[i]>num_c?num_c:end[i];
 }
}

cmd( "set choice $flt" );
flt=*choice;

cmd( "set choice $bido" );

if(*choice==1)
{//compute over series
vs[num_var].data = new double[max_c+2];
lapp=(char *)Tcl_GetVar(inter, "vname",0);
strcpy(msg,lapp);
strcpy(vs[num_var].label, msg);
vs[num_var].end=max_c;
vs[num_var].start=min_c;
lapp=(char *)Tcl_GetVar(inter, "vtag",0);
strcpy(msg,lapp);
strcpy(vs[num_var].tag, msg);
vs[num_var].rank=num_var;

cmd( "set choice $bidi" );

for(i=min_c; i<=max_c; i++)
 {nmean=nn=nvar=0;
  first=true;
  for(j=0; j<nv; j++)
   {
    if(i>=start[j] && i<=end[j] && is_finite(data[j][i]) && (flt==0 || (flt==1 && data[j][i]>thflt) || (flt==2 && data[j][i]<thflt) ))		// ignore NaNs
    {
    nmean+=data[j][i];
    nvar+=data[j][i]*data[j][i];
    nn++;
    if(first)
	{
     nmin=nmax=data[j][i];
	 first=false;
	}
    else
     {
     if(nmin>data[j][i])
      nmin=data[j][i];
     if(nmax<data[j][i])
      nmax=data[j][i];
      
     } 
    }
   }

   if(nn==0)	// not a single valid value?
   {
		nn=nmean=nvar=nmin=nmax=NAN;
   }
   else
   {
		nmean/=nn;
		nvar/=nn;
		nvar-=nmean*nmean;
   }
   
if(*choice==1)
 vs[num_var].data[i]=nmean;
if(*choice==5)
 vs[num_var].data[i]=nmean*nn;
 
if(*choice==2)
 vs[num_var].data[i]=nmax;
if(*choice==3)
 vs[num_var].data[i]=nmin;
if(*choice==4)
 vs[num_var].data[i]=nvar;
if(*choice==7)
 vs[num_var].data[i]=nn;
if(*choice==6)
 vs[num_var].data[i]=sqrt(nvar)*confi;


 }
cmd( ".da.vars.lb.v insert end \"%s %s (%d - %d) # %d (created)\"", vs[num_var].label, vs[num_var].tag, min_c, max_c, num_var ); 

cmd( "lappend DaModElem %s", vs[num_var].label  );
} 
else
{
//compute over cases
vs[num_var].data = new double[nv];
lapp=(char *)Tcl_GetVar(inter, "vname",0);
strcpy(msg,lapp);
strcpy(vs[num_var].label, msg);
vs[num_var].end=nv-1;
vs[num_var].start=0;
lapp=(char *)Tcl_GetVar(inter, "vtag",0);
strcpy(msg,lapp);
strcpy(vs[num_var].tag, msg);
vs[num_var].rank=num_var;

cmd( "set choice $bidi" );

for(j=0; j<nv; j++)
 {nmean=nn=nvar=0;
  first=true;
  for(i=min_c; i<=max_c; i++)
   {
    if(i>=start[j] && i<=end[j] && is_finite(data[j][i]) && (flt==0 || (flt==1 && data[j][i]>thflt) || (flt==2 && data[j][i]<thflt) ) )
    {
    nmean+=data[j][i];
    nvar+=data[j][i]*data[j][i];
    nn++;
    if(first)
	{
     nmin=nmax=data[j][i];
	 first=false;
	}
    else
     {
     if(nmin>data[j][i])
      nmin=data[j][i];
     if(nmax<data[j][i])
      nmax=data[j][i];
      
     } 
    }
   }

   if(nn==0)	// not a single valid value?
   {
		nn=nmean=nvar=nmin=nmax=NAN;
   }
   else
   {
		nmean/=nn;
		nvar/=nn;
		nvar-=nmean*nmean;
   }
      
if(*choice==1)
 vs[num_var].data[j]=nmean;
if(*choice==5)
 vs[num_var].data[j]=nmean*nn;
if(*choice==2)
 vs[num_var].data[j]=nmax;
if(*choice==3)
 vs[num_var].data[j]=nmin;
if(*choice==4)
 vs[num_var].data[j]=nvar;
if(*choice==7)
 vs[num_var].data[j]=nn;
if(*choice==6)
 vs[num_var].data[j]=sqrt(nvar)*confi;

 }

cmd( ".da.vars.lb.v insert end \"%s %s (%d - %d) # %d (created)\"", vs[num_var].label, vs[num_var].tag, 0, nv-1, num_var ); 

cmd( "lappend DaModElem %s", vs[num_var].label  );
}
cmd( ".da.vars.lb.v see end" );
num_var++; 
cmd( ".da.com.nvar conf -text \"Series = %d\"", num_var );

delete[] start;
delete[] end;
delete[] data;
for(i=0; i<nv; i++)
 {
 delete[] str[i];
 delete[] tag[i];
 }
delete[] str;
delete[] tag; 
}


void create_maverag(int *choice)
{
int i, nv, j, h, *start, *end, flt, idseries;
double xapp;
double step;

char *lapp, **str, **tag;
store *app;

Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);
cmd( "set nv [.da.vars.ch.v size]" );
Tcl_UnlinkVar(inter, "nv");

if ( nv == 0 )
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
	return;
}

double **data;

if(logs)
  cmd( "tk_messageBox -parent .da -type ok -icon warning -title Warning -message \"Series in logs not allowed\" -detail \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"" );

cmd( "toplevel .da.s" );
cmd( "wm title .da.s \"Mov. Average Range\"" );
cmd( "wm transient .da.s .da" );

cmd( "frame .da.s.o -relief groove -bd 2" );
cmd( "label .da.s.o.l -text \"Set # of (odd) periods\" -fg red" );
cmd( "pack .da.s.o.l" );
cmd( "set bido 10" );
cmd( "entry .da.s.o.th -width 6 -validate focusout -vcmd { if [ string is integer %%P ] { set bido %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bido; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.o.th insert 0 $bido" ); 
cmd( "pack .da.s.o.th" );

cmd( "pack .da.s.o" );

cmd( "button .da.s.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "button .da.s.help -width -9 -text Help -command {LsdHelp mdatares.html#create_maverag}" );
cmd( "button .da.s.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "pack .da.s.ok .da.s.help .da.s.esc" );
cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );
cmd( "set w .da.s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w" );

*choice=0;
cmd( "focus .da.s.o.th" );
cmd( ".da.s.o.th selection range 0 end" );
  while(*choice==0)
	Tcl_DoOneEvent(0);

cmd( "set bido [ .da.s.o.th get ]" ); 
cmd( "destroy .da.s" );

if(*choice==2)
 {*choice=0;
  return;
 }

data=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];

 app=new store[nv + num_var];
 for(i=0; i<num_var; i++)
  {app[i]=vs[i];
   strcpy(app[i].label, vs[i].label);
   strcpy(app[i].tag, vs[i].tag);
  } 
 delete[] vs;
 vs=app;

if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

cmd( "set choice $bido" );
flt=*choice;
if((flt%2)==0)
 flt++;

if(flt<2)
  {*choice=0;
  return;
 }


for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  lapp=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,lapp);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  
  sprintf(msg, "MA%d_%s", flt, str[i]);
  strcpy(vs[num_var+i].label, msg);
  strcpy(vs[num_var+i].tag,tag[i]);
  vs[num_var+i].start=start[i];
  vs[num_var+i].end=end[i];
  vs[num_var+i].rank=num_var+i;
  vs[num_var+i].data = new double[max_c+2];
  if(autom_x ||(start[i]<=max_c && end[i]>=min_c))
   {
   data[i]=find_data(idseries);
   xapp=0;

   for(h=0, j=start[i]; j<start[i]+flt;j++)
	   if(is_finite(data[i][j]))		// not a NaN?
	   {
		   xapp+=data[i][j];
		   h++;
	   }
	if(h==0)	// no observation?
		xapp=NAN;
	else
		xapp/=(double)h;
	
   for(j=start[i]; j<start[i]+(flt-1)/2; j++)
     vs[num_var+i].data[j]=xapp;

   for(   ; j<end[i]-(flt-1)/2; j++)
    {
	 if(is_finite(data[i][j-(flt-1)/2]) && is_finite(data[i][j+(flt-1)/2]))
		xapp=xapp-data[i][j-(flt-1)/2]/(double)flt+data[i][j+(flt-1)/2]/(double)flt;
	 else
		xapp=NAN;
     vs[num_var+i].data[j]=xapp;
    }
   for(   ; j<end[i]; j++)
     vs[num_var+i].data[j]=xapp;     
   }
  cmd( ".da.vars.lb.v insert end \"%s %s (%d - %d) # %d (created)\"", vs[num_var+i].label, vs[num_var+i].tag, vs[num_var+i].start, vs[num_var+i].end, num_var+i ); 
  
  cmd( "lappend DaModElem %s", vs[num_var+i].label );
 }

cmd( ".da.vars.lb.v see end" );
num_var+=nv; 
cmd( ".da.com.nvar conf -text \"Series = %d\"", num_var );

delete[] start;
delete[] end;
delete[] data;
for(i=0; i<nv; i++)
 {
 delete[] str[i];
 delete[] tag[i];
 }
delete[] str;
delete[] tag; 
}

/************************
SAVE_DATAzip
************************/
void save_datazip(int *choice)
{
int idseries, memstep;
char *app;
char **str, **tag;
char str1[100], delimiter[10], misval[10];
int i, nv, j, *start, *end, typelab, numcol, del, fr, gp, type_res;
double **data, *dd, *ddstart;
const char *descr, *ext;
const char descrRes[] = "Lsd Result File";
const char descrTxt[] = "Text File";
const char extResZip[] = ".res.gz";
const char extTxtZip[] = ".txt.gz";
const char extRes[] = ".res";
const char extTxt[] = ".txt";
FILE *fsave;
#ifdef LIBZ
gzFile fsavez;
#else
FILE *fsavez; 
#endif 

Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);
cmd( "set nv [.da.vars.ch.v size]" );
Tcl_UnlinkVar(inter, "nv");

*choice=0;

if ( nv == 0 )			// no variables selected
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
	return;
}

if(logs)
  cmd( "tk_messageBox -parent .da -type ok -icon warning -title Warning -message \"Series in logs not allowed\" -detail \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"" );

data=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];

max_c=min_c=0;

for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);

  data[i]=find_data(idseries);
  if(max_c<end[i])
   max_c=end[i];
 }

Tcl_LinkVar(inter, "typelab", (char *) &typelab, TCL_LINK_INT);
Tcl_LinkVar(inter, "dozip", (char *)&dozip, TCL_LINK_INT);
Tcl_LinkVar(inter, "deli", (char *) &del, TCL_LINK_INT);
Tcl_LinkVar(inter, "numcol", (char *) &numcol, TCL_LINK_INT);
Tcl_LinkVar(inter, "fr", (char *) &fr, TCL_LINK_INT);

//Variables' Name in first column
strcpy(misval,nonavail);
fr=1;
typelab=3;
cmd( "toplevel .da.lab" );
cmd( "wm title .da.lab \"Saving Data\"" );
cmd( "wm transient .da.lab .da " );
cmd( "label .da.lab.l -text \"File format\"" );
cmd( "frame .da.lab.f -relief groove -bd 2" );
cmd( "radiobutton .da.lab.f.lsd -text \"Lsd results file\" -variable typelab -value 3" );
cmd( "radiobutton .da.lab.f.nolsd -text \"Text file\" -variable typelab -value 4" );
cmd( "checkbutton .da.lab.dozip -text \"Generate zipped file\" -variable dozip" );

cmd( "button .da.lab.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "button .da.lab.help -width -9 -text Help -command {LsdHelp mdatares.html#save}" );
cmd( "button .da.lab.esc -width -9 -text Cancel -command {set choice 2}" );

cmd( "pack .da.lab.f.lsd .da.lab.f.nolsd -anchor w" );
#ifdef LIBZ
cmd( "pack .da.lab.l .da.lab.f .da.lab.dozip .da.lab.ok .da.lab.help .da.lab.esc" );
#else
cmd( "pack .da.lab.l .da.lab.f .da.lab.ok .da.lab.help .da.lab.esc" );
#endif
cmd( "bind .da.lab <Return> {.da.lab.ok invoke}" );
cmd( "bind .da.lab <Escape> {.da.lab.esc invoke}" );
cmd( "focus .da.lab" );
cmd( "set w .da.lab; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w" );

while(*choice==0)
 Tcl_DoOneEvent(0);

if(*choice==2)
 goto end;
type_res=typelab;

*choice=0;
cmd( "destroy .da.lab" );

if(typelab==4)
{
typelab=1;
cmd( "toplevel .da.lab" );
cmd( "wm title .da.lab \"Saving Data\"" );
cmd( "wm transient .da.lab .da " );
cmd( "frame .da.lab.f -relief groove -bd 2" );
cmd( "label .da.lab.f.tit -text \"Labels to use\" -foreground red" );

cmd( "radiobutton .da.lab.f.orig -text Original -variable typelab -value 1" );
cmd( "radiobutton .da.lab.f.new -text \"New names\" -variable typelab -value 2" );
cmd( "set newlab \"\"" );
cmd( "entry .da.lab.f.en -textvariable newlab" );
cmd( "set gp 0" );
cmd( "checkbutton .da.lab.f.gp -text \"Add #\" -variable gp" );
cmd( "bind .da.lab.f.en <FocusIn> {.da.lab.f.new invoke}" );
cmd( "pack .da.lab.f.tit .da.lab.f.orig .da.lab.f.new .da.lab.f.en .da.lab.f.gp -anchor w" );
cmd( "frame .da.lab.d -relief groove -bd 2" );
cmd( "label .da.lab.d.tit -text \"Columns delimiter\" -foreground red" );

cmd( "frame .da.lab.d.r" );
del=1;
cmd( "radiobutton .da.lab.d.r.tab -text \"Tab delimited\" -variable deli -value 1" );
cmd( "radiobutton .da.lab.d.r.oth -text \"Other delimiter\" -variable deli -value 2" );
cmd( "set delimiter \"\"" );
cmd( "entry .da.lab.d.r.del -textvariable delimiter -justify center" );
cmd( "bind .da.lab.d.r.del <FocusIn> {.da.lab.d.r.oth invoke}" );

cmd( "radiobutton .da.lab.d.r.col -text \"Fixed length columns\" -variable deli -value 3" );
numcol=16;
cmd( "entry .da.lab.d.r.ecol -validate focusout -vcmd { if [ string is double %%P ] { set numcol %%P; return 1 } { %%W delete 0 end; %%W insert 0 $numcol; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.lab.d.r.ecol insert 0 $numcol" ); 
cmd( "bind .da.lab.d.r.ecol <FocusIn> {.da.lab.d.r.col invoke}" );

cmd( "pack .da.lab.d.r.tab .da.lab.d.r.oth .da.lab.d.r.del .da.lab.d.r.col .da.lab.d.r.ecol -anchor w" );
cmd( "pack .da.lab.d.tit .da.lab.d.r -anchor w" );

cmd( "frame .da.lab.gen -relief groove -bd 2" );
cmd( "label .da.lab.gen.tit -text \"General Options\" -foreground red" );
cmd( "checkbutton .da.lab.gen.fr -text \"Names in first row\" -variable fr" );
cmd( "label .da.lab.gen.miss -text \"Missing values\"" );
cmd( "set misval \"n/a\"" );
cmd( "entry .da.lab.gen.mis_val -textvariable misval -justify center" );
cmd( "pack .da.lab.gen.tit .da.lab.gen.fr .da.lab.gen.miss .da.lab.gen.mis_val -anchor w" );
cmd( "button .da.lab.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "button .da.lab.help -width -9 -text Help -command {LsdHelp mdatares.html#save}" );
cmd( "button .da.lab.esc -width -9 -text Cancel -command {set choice 2}" );

cmd( "pack .da.lab.f .da.lab.d .da.lab.gen .da.lab.ok .da.lab.help .da.lab.esc -fill x" );
*choice=0;
cmd( "focus .da.lab" );
cmd( "bind .da.lab <KeyPress-Return> {.da.lab.ok invoke}" );
cmd( "set w .da.lab; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w" );

while(*choice==0)
 Tcl_DoOneEvent(0);

cmd( "set numcol [ .da.lab.d.r.ecol get ]" ); 

if(*choice==2)
 goto end;

cmd( "set choice $gp" );
gp=*choice;

*choice=0;

app=(char *)Tcl_GetVar(inter, "misval", 0);
strcpy(misval,app);
}

if ( type_res == 4 )
{
	descr = descrTxt;
	if ( ! dozip )
		ext = extTxt;
	else
		ext = extTxtZip;
}
else
{
	descr = descrRes;
	if ( ! dozip )
		ext = extRes;
	else
		ext = extResZip;
}

cmd( "set bah [tk_getSaveFile -parent .da -title \"Save Data File\" -initialdir [pwd] -defaultextension \"%s\" -filetypes {{{%s} {%s}} {{All files} {*}} }]", ext, descr, ext );
app=(char *)Tcl_GetVar(inter, "bah",0);
strcpy(msg, app);

if(strlen(msg)==0)
 goto end;
if(dozip==1) 
 {
 #ifdef LIBZ
  fsavez=gzopen(msg, "wt");
 #endif 
 } 
else
 fsave=fopen(msg, "wt");  // use text mode for Windows better compatibility

if(del!=3) //Delimited files
{if(del==2)
 {app=(char *)Tcl_GetVar(inter, "delimiter",0);
  if(strlen(app)==0)
    strcpy(delimiter,"\t");
  else
    strcpy(delimiter,app);
 }
 else
  strcpy(delimiter, "\t");
}
if(fr==1)
{
if(del!=3)
{
switch(typelab)
{
case 1:   //Original labels
if(gp==1)
 {if(dozip==1) 
    {
     #ifdef LIBZ
     gzprintf(fsavez, "#");
     #endif
    } 
  else
    fprintf(fsave, "#");
 }   
for(i=0; i<nv; i++)
  {
   if(dozip==1) 
    {
    #ifdef LIBZ 
     gzprintf(fsavez, "%s_%s%s", str[i], tag[i], delimiter);
    #endif 
    } 
   else
    fprintf(fsave, "%s_%s%s", str[i], tag[i], delimiter); 
  }  
break;

case 2: //New names for labels
  app=(char *)Tcl_GetVar(inter, "newlab",0);
  if(strlen(app)==0)
    strcpy(msg,"Var");
  else
    strcpy(msg,app);
if(gp==1)
 {if(dozip==1) 
   {
    #ifdef LIBZ
    gzprintf(fsavez, "#");
    #endif
   } 
  else
   fprintf(fsave, "#");
 }  
 for(i=0; i<nv; i++)
   {if(dozip==1) 
      {
       #ifdef LIBZ
       gzprintf(fsavez, "%s%d%s", msg, i, delimiter);
       #endif
      } 
    else
      fprintf(fsave, "%s%d%s", msg, i, delimiter);  
   }   
 break;

case 3:
for(i=0; i<nv; i++) //Lsd result files
  {if(dozip==1) 
     {
     #ifdef LIBZ 
      gzprintf(fsavez, "%s %s (%d %d)\t", str[i], tag[i], start[i], end[i]);
     #endif 
     } 
   else
     fprintf(fsave, "%s %s (%d %d)\t", str[i], tag[i], start[i], end[i]);  
  }   
break;
 } //end switch delimiter
} //and of delimiter header writing
else //header for Column text files
  {
if(gp==1)
 {
  if(dozip==1) 
    {
    #ifdef LIBZ 
     gzprintf(fsavez, "#");
    #endif 
    } 
  else
    fprintf(fsave, "#");  
 }    

  strcpy(str1, "                                                                                ");
   for(i=0; i<nv; i++)
    {sprintf(msg, "%s_%s", str[i], tag[i]);
     if(strlen(msg)<=numcol)
      strcat(msg, str1);
     msg[numcol]=(char)NULL;
    if(dozip==1) 
     {
      #ifdef LIBZ
      gzprintf(fsavez,"%s", msg);
      #endif
     } 
    else
     fprintf(fsave,"%s", msg); 
    }
  }

if(dozip==1) 
  {
  #ifdef LIBZ 
   gzprintf(fsavez,"\n");
  #endif 
  } 
else
  fprintf(fsave,"\n");  
}//end of header writing

if(del!=3) //data delimited writing
{

if(dozip==1)
{
#ifdef LIBZ
for(j=min_c; j<=max_c; j++)
 {
  for(i=0; i<nv; i++)
   {if(j>=start[i] && j<=end[i] && !is_nan(data[i][j]))		// write NaN as n/a
      gzprintf(fsavez, "%g%s", data[i][j], delimiter);
    else
      gzprintf(fsavez, "%s%s", misval, delimiter);
      
   }
  gzprintf(fsavez,"\n");
 }
#endif 
}
else
{
for(j=min_c; j<=max_c; j++)
 {for(i=0; i<nv; i++)
   {if(j>=start[i] && j<=end[i] && !is_nan(data[i][j]))		// write NaN as n/a
      fprintf(fsave, "%g%s", data[i][j], delimiter);  
    else
      fprintf(fsave, "%s%s", misval, delimiter);  
   }
   fprintf(fsave,"\n");  
 }
}
}
else //column data writing
{ strcpy(str1, "00000000000000000000000000000000000000000000000000000000000000000000000000000000");
for(j=min_c; j<=max_c; j++)
 {for(i=0; i<nv; i++)
   {
   if(j>=start[i] && j<=end[i] && !is_nan(data[i][j]))		// write NaN as n/a
   {
      sprintf(msg, "%.10lf", data[i][j]);
	  strcat(msg, str1);
	  msg[numcol]=(char)NULL;
   }
   else
      sprintf(msg, "%s", misval);
   if(dozip==1) 
     {
     #ifdef LIBZ
      gzprintf(fsavez, "%s", msg);
     #endif 
     }
   else
     fprintf(fsave, "%s", msg);  
   }
  if(dozip==1) 
    {
     #ifdef LIBZ
     gzprintf(fsavez,"\n");
     #endif
    } 
   else
    fprintf(fsave,"\n");  
 }

}
if(dozip==1) 
  {
  #ifdef LIBZ
  gzclose(fsavez);
  #endif
  }
else
  fclose(fsave);  

end:
cmd( "destroy .da.lab" );
Tcl_UnlinkVar(inter, "typelab");
Tcl_UnlinkVar(inter, "dozip");
Tcl_UnlinkVar(inter, "numcol");
Tcl_UnlinkVar(inter, "deli");
Tcl_UnlinkVar(inter, "fr");

for(i=0; i<nv; i++)
 {delete[] str[i];
  delete[] tag[i];
 }
delete[] str;
delete[] tag;
delete[] data;
delete[] start;
delete[] end;
*choice=0;

} //end of save_datazip


void plog_series(int *choice)
{
int i, nv, j, k, *start, *end, idseries, flt;
double nmax, nmin, nmean, nvar, nn, thflt;
double step;

char *lapp, **str, **tag;
store *app;

Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);
cmd( "set nv [.da.vars.ch.v size]" );
Tcl_UnlinkVar(inter, "nv");

if ( nv == 0 )			// no variables selected
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
	return;
}

double **data;

if(logs)
  cmd( "tk_messageBox -parent .da -type ok -icon warning -title Warning -message \"Series in logs not allowed\" -detail \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"" );

data=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];

 app=new store[1+ num_var];
 for(i=0; i<num_var; i++)
  {app[i]=vs[i];
   strcpy(app[i].label, vs[i].label);
   strcpy(app[i].tag, vs[i].tag);
  } 
 delete[] vs;
 vs=app;

if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  lapp=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,lapp);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  if(autom_x ||(start[i]<=max_c && end[i]>=min_c))
   {
   
   data[i]=find_data(idseries);
   }
 }

if(autom_x||min_c>=max_c)
{

for(i=0; i<nv; i++)
 {if(i==0)
   min_c=max_c=start[i];
  if(start[i]<min_c)
   min_c=start[i];
  if(end[i]>max_c)
   max_c=end[i]>num_c?num_c:end[i];
 }
}

plog( "\n\nTime series data\n" );
plog( "t\t%s_%s", "series", str[0], tag[0]);

for(i=1; i<nv; i++)
  plog( "\t%s_%s", "series", str[i], tag[i]);
plog( "\n" );

for(i=min_c; i<=max_c; i++)
 {
 if ( ! is_nan( data[0][i] ) && start[0] <= i )
	plog( "%d\t%.*g", "series", i, pdigits, data[0][i] );
 else
	plog( "%d\t%s", "series", i, nonavail );		// write NaN as n/a

 for(j=1; j<nv; j++)
   {
   if ( ! is_nan( data[j][i] ) && start[j] <= i )
	plog( "\t%.*g", "series", pdigits, data[j][i] );
   else
	plog( "\t%s", "series", nonavail );		// write NaN as n/a
   }
 plog( "\n" );
 }

delete[] start;
delete[] end;
delete[] data;
for(i=0; i<nv; i++)
 {
 delete[] str[i];
 delete[] tag[i];
 }
delete[] str;
delete[] tag; 
}    
