#-------------------------------------------------------------------------------
# following https://www.gnu.org/software/make/manual/html_node/Syntax-of-Functions.html#Syntax-of-Functions
comma:= ,
empty:=
space:= $(empty) $(empty)

#-------------------------------------------------------------------------------
# LISTS
# transforms a list of space separated into a list of comma separated
to_clist = $(subst $(space),$(comma),$(strip $(1)))
# transforms a list of comma separated into a list of space separated
to_slist = $(subst $(space),$(comma),$(strip $(1)))

#-------------------------------------------------------------------------------
# COPY
copy_list = $(foreach file,$(1),$(shell cp $(file) $(2)))