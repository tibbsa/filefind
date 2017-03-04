.SILENT
.AUTODEPEND

.PATH.obj = obj

#               *Translator Definitions*
CC = bcc +NEW.CFG
TASM = TASM
TLIB = tlib
TLINK = tlink
LIBPATH = e:\bc\LIB
INCLUDEPATH = e:\bc\INCLUDE


#               *Implicit Rules*
.c.obj:
  $(CC) -c {$< }

.cpp.obj:
  $(CC) -c {$< }

#               *List Macros*


EXE_dependencies =  \
 max2ff.obj     \
 max3.obj      \
 max2.obj

#               *Explicit Rules*
convert.exe: new.cfg $(EXE_dependencies)
  $(TLINK) /c/L$(LIBPATH) @&&|
c0l.obj+
obj\max2ff.obj+
obj\max3.obj+
obj\max2.obj
convert
		# no map file
emu.lib+
mathl.lib+
noehl.lib+
cl.lib
|


#               *Individual File Dependencies*
max2ff.obj: new.cfg max2ff.c 
   $(CC) -c -DNDEBUG max2ff.c

max3.obj: new.cfg max3.c
   $(CC) -c -DNDEBUG max3.c

max2.obj: new.cfg max2.c
   $(CC) -c -DNDEBUG max2.c

#               *Compiler Configuration File*
new.cfg: makefile.
  copy &&|
-ml
-C
-N
-G
-O
-Z
-Ff=32
-w-ret
-w-nci
-w-inl
-wpin
-wamb
-wamp
-w-par
-wasm
-wcln
-DDEBUG
-w-cpt
-wdef
-w-dup
-w-pia
-wsig
-wnod
-w-ill
-w-sus
-wstv
-wucp
-wuse
-w-ext
-w-ias
-w-ibc
-w-pre
-w-nst
-nobj
-I$(INCLUDEPATH)
-L$(LIBPATH)
| new.cfg


