Build
- make sure you have *root-config* in your ${PATH} environment variable
- open Makefile and customize the location of *Edep-sim* installation
- Then:

```
$ make
```

Before running application
- To have dictionaties of the structs loaded at run time:
```
$ source scripts/env.sh
```

Using with ROOT
- To load dictionaries of structs:
```
root [0] gSystem->Load("libStruct.so")
```

Digitization
- Create digits of STT and cells of calorimeter

```
$ Digitize <MC file> <digit file>
```

Reconstruction
- Track find and fit of STT track
- Clustering of calorimeter cells

```
$ Reconstruct <MC file> <digit file>
```

Analysis
- Evaluate parameters of particles
- Evaluate neutrino energy

```
Analyze <MC file> <digit file>
```

The description of the data format can be found [here](https://baltig.infn.it/dune/kloe-simu/-/wikis/Data-Model)

The code format can be find [here](https://baltig.infn.it/dune/kloe-simu/-/wikis/Code-Format)
