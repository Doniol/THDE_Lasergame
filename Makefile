#############################################################################
#
# Project Makefile
#
# (c) Wouter van Ooijen (www.voti.nl) 2016
#
# This file is in the public domain.
# 
#############################################################################

# source files in this project (main.cpp is automatically assumed)
SOURCES := 

# header files in this project
HEADERS := IR.hpp display_taak.hpp invoer_taak.hpp run_game_taak.hpp decoder_taak.hpp pulse_meter_taak.hpp init_game_taak.hpp send_taak.hpp game_parameters_taak.hpp transfer_hits_taak.hpp

# other places to look for files for this project
SEARCH  := 

# set RELATIVE to the next higher directory 
# and defer to the appropriate Makefile.* there
RELATIVE := $(RELATIVE)..
include $(RELATIVE)/Makefile.due