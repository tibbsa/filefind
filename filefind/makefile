.AUTODEPEND
.SILENT
.PATH.obj = OBJ

#		*Translator Definitions*
CC = bcc +util.CFG
TASM = TASM /t /Mx
TLIB = tlib
TLINK = tlink
LIBPATH = e:\bc\LIB
INCLUDEPATH = e:\bc\INCLUDE;H;\source\ULIB\H;H;E:\source\RESOURCE\BBS\SQUISH\H;\source\resource\snip


#		*Implicit Rules*
.c.obj:
  $(CC) -c -DNDEBUG {$< }

.cpp.obj:
  $(CC) -c -DNDEBUG {$< }

#		*List Macros*


EXE_dependencies =  \
   filefind.obj   \
   process.obj    \
   w_wrap.obj     \
   crc.obj

filefind.exe: util.cfg $(EXE_dependencies)
  $(TLINK) /c /L$(LIBPATH) @&&|
c0l.obj+
obj\filefind.obj+
obj\process.obj+
obj\w_wrap.obj+
obj\crc.obj
filefind
      # no map file
emu.lib+
\source\xutil.lib+
mathl.lib+
bc_dos_l.lib+
noehl.lib+
cl.lib
|

#		*Individual File Dependencies*
filefind.obj: util.cfg src\filefind.c

process.obj: util.cfg src\process.c

w_wrap.obj: util.cfg src\w_wrap.c

crc.obj: util.cfg src\crc.c


#		*Compiler Configuration File*
util.cfg: makefile.
  copy &&|
-ml
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


