#*******************************************************************************
#
# --------------- Elementary Effects DoE analysis (Morris 1991) ---------------
#
#	Several files are required to run:
#		folder/baseName_XX_YY.csv		    : DoE specification from LSD
#		folder/baseName_XX_YY_WWW.res	  : DoE response from R
#		folder/baseName.lsd				      : LSD configuration with default values
#		folder/baseName.sa		          : factor sensitivity test ranges from LSD
#
#*******************************************************************************

library( LSDsensitivity )


folder   <- "sa-morris"               # data files folder
baseName <- "morris"                  # data files base name (same as .lsd file)
varName <- "gM"                       # variable to perform the sensitivity analysis on
iniDrop <- 0                          # initial time steps to drop from analysis (0=none)
nKeep <- -1                           # number of time steps to keep (-1=all)

plotRows <- 1                         # number of plots per row in a page
plotCols <- 1  	                      # number of plots per column in a page
plotW <- 12                           # plot window width
plotH <- 8                            # plot window height
raster <- FALSE                       # raster or vector plots
res <- 600                            # resolution of raster mode (in dpi)


# ==== LSD variables to use (all, the ones to be used in log and new) ====

lsdVars <- c( "Q", "g", "l", "m", "J" )
logVars <- c( )
newVars <- c( "gM" )


# ====== Function to extend the data set ======

eval.vars <- function( dataSet, allVars ) {
  tsteps <- nrow( dataSet )        # number of timesteps in simulated data set

  # ---- Calculate values of new variables (added to LSD dataset) ----

  if ( dataSet[ tsteps, "Q" ] > 0 )
    dataSet[ , "gM" ] <- log( dataSet[ tsteps, "Q" ] ) / tsteps
  else
    dataSet[ , "gM" ] <- 0

  return( dataSet )
}


# ==== Process LSD set of result files (an experimental data set) ====

dataSet <- read.doe.lsd( folder,                 # data files relative folder
                         baseName,               # data files base name (same as .lsd file)
                         varName,			  	       # variable name to perform the sensitivity analysis
                         iniDrop = iniDrop,      # initial time steps to drop from analysis (0=none)
                         nKeep = nKeep,          # number of time steps to keep (-1=all)
                         saveVars = lsdVars,     # LSD variables to keep in dataset
                         addVars = newVars,      # new variables to add to the LSD dataset
                         eval.vars = eval.vars,  # function to evaluate/adjust/expand the dataset
                         rm.temp = FALSE,        # remove temporary speedup files?
                         rm.outl = FALSE,        # remove outliers from dataset
                         lim.outl = 10 )         # limit non-outlier deviation (number of std. devs.)


# ====== Do Elementary Effects analysis ======

eeSA <- elementary.effects.lsd( dataSet,         # LSD experimental data set
                                p = 4,           # number of levels of the design (as set in LSD)
                                jump = 2 )       # number of jumps per level (as set in LSD)



#*******************************************************************************
#
# ----------------------- Analysis results presentation -----------------------
#
#*******************************************************************************

library( gplots )           # package for plotting tables


tryCatch( {    # enter error handling mode so PDF can be closed in case of error/interruption

  # create a daily output directory
  outDir <- format( Sys.time(), "%Y-%m-%d" )
  if( ! dir.exists( paste0( folder, "/", outDir ) ) )
    dir.create( paste0( folder, "/", outDir ) )

  cat( paste( "\nSaving results and data to:", paste0( folder, "/", outDir ), "\n" ) )

  # Select type of output
  if( raster ){
    # Open PNG (bitmap) files for output
    png( paste0( folder, "/", outDir, "/", baseName, "_ee_%d.png" ),
         width = plotW, height = plotH, units = "in", res = res )

  } else {
    # Open PDF plot file for output
    pdf( paste0( folder, "/", outDir, "/", baseName, "_ee_plots.pdf" ),
         width = plotW, height = plotH )
    par( mfrow = c ( plotRows, plotCols ) )             # define plots per page
  }

  # ====== Present sensitivity analysis for the EE model ======

  textplot( signif( eeSA$table, 3 ), cex = 1.0 )
  title( main = paste0( "Elementary effects distributions statistics ( ", dataSet$saVarName, " )" ),
         sub = paste0( "All variables rescaled to [ 0, 1 ] / H0: mu.star = 0\nLevels (p) = ", 4,
                       ", delta = ", signif( 2 / ( 4 - 1 ) , 3 ),
                       ", trajectories (r) = ", eeSA$r,
                       ", samples = ", eeSA$samples ) )

  plot( eeSA )
  title( main = paste0( "Elementary effects composition ( ", dataSet$saVarName, " )" ),
         sub = "mu.star: overall effects / sigma: non-linear/non-additive effects" )

  cat( "\nDone...\n" )

#******************************************************************
#
# ------------- Exception handling code (tryCatch) -------------
#
#******************************************************************

}, interrupt = function( ex ) {
  cat( "An interrupt was detected.\n" )
  print( ex )
  textplot( "Report incomplete due to interrupt." )
}, error = function( ex ) {
  cat( "An error was detected.\n" )
  print( ex )
  textplot( "Report incomplete due to processing error." )
}, finally = {
  options( warn = 0 )
  # Close PDF plot file
  dev.off( )
} )
