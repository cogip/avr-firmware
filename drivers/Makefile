# absolute path of current Makefile
abs-path			:= drivers

# all subdir with a Makefile
src-subdirs-y			:=
src-subdirs-y			+= $(abs-path)/actuators
src-subdirs-y			+= $(abs-path)/sensors
src-subdirs-y			+= $(abs-path)/system

# Replace current 'abs-path' dir in 'src-dirs' with the list of src-subdirs
src-dirs			:= $(patsubst $(abs-path),$(src-subdirs-y),$(src-dirs))

# include subdirs Makefiles
-include $(patsubst %,%/Makefile,$(src-subdirs-y))
