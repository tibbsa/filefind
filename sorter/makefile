.AUTODEPEND
.SILENT
.PATH.obj = OBJ

#		*Translator Definitions*
CC = bcc +util.CFG
TASM = TASM /t /Mx
TLIB = tlib
TLINK = tlink
LIBPATH = e:\bc\LIB
INCLUDEPATH = e:\bc\INCLUDE;H;\source\ULIB\H;H;e:\source\RESOURCE\BBS\SQUISH\H;e:\source\RESOURCE\SNIP


#		*Implicit Rules*
.c.obj:
  $(CC) -c {$< }

.cpp.obj:
  $(CC) -c {$< }

#		*List Macros*


EXE_dependencies =  \
   filefind.obj   \
   process.obj     \
   fcopy.obj

sortarea.exe: util.cfg $(EXE_dependencies)
  $(TLINK) /c /L$(LIBPATH) @&&|
c0l.obj+
obj\filefind.obj+
obj\process.obj+
obj\fcopy.obj
sortarea
      # no map file
emu.lib+
\source\xutil.lib+
mathl.lib+
noehl.lib+
cl.lib
|



#		*Individual File Dependencies*
filefind.obj: util.cfg src\filefind.c

sort.obj: util.cfg src\sort.c

process.obj: util.cfg src\process.c

fcopy.obj: util.cfg src\fcopy.c

#		*Compiler Configuration File*
util.cfg: makefile.
  copy &&|
-ml
-Ff=24
-C
-N
-G
-O
-Z
-H=obj\UTIL.SYM
-w-ret
-w-nci
-w-inl
-wpin
-wamb
-wamp
-Tt
-Tmx
-w-par
-wasm
-w-eff
-wcln
-w-cpt
-wdef
-w-dup
-w-pia
-wsig
-wnod
-w-ill
-w-sus
-wstv
-w-ucp
-w-use
-w-ext
-w-ias
-w-ibc
-w-pre
-w-nst
-nOBJ
-I$(INCLUDEPATH)
-L$(LIBPATH)
| util.cfg


