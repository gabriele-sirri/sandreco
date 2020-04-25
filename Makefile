
HEADERS= struct.h
CFLAGS = $(shell root-config --cflags)
LDFLAGS = $(shell root-config --ldflags)
ROOTGLIBS = $(shell root-config --glibs)
ROOTINCDIR = $(shell root-config --incdir)

EDEPSIM = ${EDEPSIMDIR}

EDEPGLIBS = -L$(EDEPSIM)/lib/ -ledepsim_io
EDEPINCDIR = $(EDEPSIM)/include/EDepSim

GENFIT=/wd/sw/GENFIT/GenFit.binary

EIGEN3=/usr/include/eigen3

all: Digitize Reconstruct Analyze FastCheck Display

struct.cxx: include/struct.h include/Linkdef.h
	cd include && rootcint -f ../src/$@ -c $(CFLAGS) -p $(HEADERS) Linkdef.h && cd ..

libStruct.so: struct.cxx
	g++ -shared -fPIC -o lib/$@ -Iinclude `root-config --ldflags` $(CFLAGS) src/$^ && cp src/struct_rdict.pcm lib/struct_rdict.pcm

libUtils.so: src/utils.cpp include/utils.h
	g++ -shared -fPIC -o lib/$@ -Iinclude -I$(EDEPINCDIR) -Iinclude $(ROOTGLIBS) `root-config --ldflags` $(CFLAGS) $<

Digitize: libStruct.so libUtils.so
	g++ -o bin/$@ $(CFLAGS) $(LDFLAGS) -I$(EDEPINCDIR) -Iinclude $(ROOTGLIBS) -lGeom \
	$(EDEPGLIBS) -Llib -lStruct -lUtils src/digitization.cpp

Reconstruct: libStruct.so libUtils.so
	g++ -o bin/$@ $(CFLAGS) $(LDFLAGS) -I$(EDEPINCDIR) -Iinclude $(ROOTGLIBS) -lGeom \
	$(EDEPGLIBS) -Llib -lStruct -lUtils src/reconstruction.cpp

Analyze: libStruct.so libUtils.so
	g++ -o bin/$@ $(CFLAGS) $(LDFLAGS) -I$(EDEPINCDIR) -Iinclude -I${GENFIT}/include -I${EIGEN3} $(ROOTGLIBS) -lGeom -lEG \
	$(EDEPGLIBS) -Llib -lStruct -lUtils src/analysis.cpp

FastCheck: libStruct.so
	g++ -o bin/$@ $(CFLAGS) $(LDFLAGS) -I$(EDEPINCDIR) -Iinclude -I${GENFIT}/include -I${EIGEN3} $(ROOTGLIBS) -lGeom -lEG \
	$(EDEPGLIBS) -Llib -lStruct src/fastcheck.cpp

Display: libStruct.so
	g++ -o bin/$@ $(CFLAGS) $(LDFLAGS) -I$(EDEPINCDIR) -Iinclude -I${GENFIT}/include -I${EIGEN3} $(ROOTGLIBS) -lGeom -lEG \
	$(EDEPGLIBS) -Llib -lStruct src/display.cpp
