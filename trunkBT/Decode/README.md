# This is the dir with the core of the packet: the tools to decode binary files, clusterize raw file and output ROOT files. The code contain also the code to create high level objects like tracks and charge measurements

Some decoding examples:

## AMS-L0

- ### AMS-LO USB-LF+LINF and version >=5
```
./Decode --rawdata "/path/to/BLOCKS/PG/TEST_LINF" --caldata "./CalData" --l0 -c --calrunstart 0001717 --calrunstop 0001717 0001718 0001718
```
```
./PlotCalib --l0 --caldir="/CalData" 0001717
```

- ### AMS-L0 LEF directly to USB-LEF (not USB-LF, I think)
```
./Decode --rawdata "/path/to/BLOCKS/PG/202212_AMSL0_data" --caldata "/path/to/BLOCKS/PG/202212_AMSL0_data" --l0old -c --calrunstart 0009870 --calrunstop 0009870 0009876 0009880
```
```
./PlotCalib --l0 --caldir="/path/to/BLOCKS/PG/202212_AMSL0_data" 0009870
```

## AMS

## OCA

## FOOT

Some logger examples:

## AMS-L0

- ### AMS-LO USB-LF+LINF and version >=5
```
./AMSL0filelogger --datadir="/path/to/BLOCKS/CH/USBLF_PCGSC17" --runstart 0000435 --runstop 0000479
```
***

# Size and speed optimization
- ## baseline:
```
./Decode --rawdata "./USBLF_PCGSC17/C42_DDC-C4C_DE5/" --l0 -c --calrunstart 0008860 --calrunstop 0008861 0008861 0008862
Processed 30407 events
Event processing took 567636ms
```
```
$> ls -lh RootData/run_008861.root | awk '{ print $5 " " $9}'
2.5G RootData/run_008861.root
```
- ## `CalSigma`, `CalStatus` and `CalPed` as `static`:
```
./Decode --rawdata "./USBLF_PCGSC17/C42_DDC-C4C_DE5/" --l0 -c --calrunstart 0008860 --calrunstop 0008861 0008861 0008862
Processed 30407 events
Event processing took 86506ms
```
```
$> ls -lh RootData/run_008861.root | awk '{ print $5 " " $9}'
426M RootData/run_008861.root
```
- ## `_eventkind` as `static`:
```
./Decode --rawdata "./USBLF_PCGSC17/C42_DDC-C4C_DE5/" --l0 -c --calrunstart 0008860 --calrunstop 0008861 0008861 0008862
Processed 30407 events
Event processing took 87869ms
```
```
$> ls -lth RootData/run_008861.root | awk '{ print $5 " " $9}'
426M RootData/run_008861.root
```
- ## `ROOT::CompressionSettings(ROOT:kLZMA, 8)` instead of `ROOT::CompressionSettings(ROOT::kZLIB, 2)`
```
./Decode --rawdata "./USBLF_PCGSC17/C42_DDC-C4C_DE5/" --l0 -c --calrunstart 0008860 --calrunstop 0008861 0008861 0008862
Processed 30407 events
Event processing took 328712ms
```
```
$> ls -lh RootData/run_008861.root | awk '{ print $5 " " $9}'
249M RootData/run_008861.root
```
- ## `ROOT::CompressionSettings(ROOT:kLZMA, 7)` instead of `ROOT::CompressionSettings(ROOT::kZLIB, 2)`
```
./Decode --rawdata "./USBLF_PCGSC17/C42_DDC-C4C_DE5/" --l0 -c --calrunstart 0008860 --calrunstop 0008861 0008861 0008862
Processed 30407 events
Event processing took 319037ms
```
```
$> ls -lh RootData/run_008861.root | awk '{ print $5 " " $9}'
249M RootData/run_008861.root
```
- ## `ROOT::CompressionSettings(ROOT:kLZMA, 6)` instead of `ROOT::CompressionSettings(ROOT::kZLIB, 2)`
```
./Decode --rawdata "./USBLF_PCGSC17/C42_DDC-C4C_DE5/" --l0 -c --calrunstart 0008860 --calrunstop 0008861 0008861 0008862
Processed 30407 events
Event processing took 325122ms
```
```
$> ls -lh RootData/run_008861.root | awk '{ print $5 " " $9}'
249M RootData/run_008861.root
```
- ## `ROOT::CompressionSettings(ROOT:kLZMA, 2)` instead of `ROOT::CompressionSettings(ROOT::kZLIB, 2)`
```
./Decode --rawdata "./USBLF_PCGSC17/C42_DDC-C4C_DE5/" --l0 -c --calrunstart 0008860 --calrunstop 0008861 0008861 0008862
Processed 30407 events
Event processing took 322143ms
```
```
$> ls -lh RootData/run_008861.root | awk '{ print $5 " " $9}'
249M RootData/run_008861.root
```
- ## back to `ROOT::CompressionSettings(ROOT::kZLIB, 2)` and `TTree::Branch("cluster_branch", dd->EventClassname(), &(dd->ev), 32000, 99)` instead of `TTree::Branch("cluster_branch", dd->EventClassname(), &(dd->ev), 64000, 2)`
```
./Decode --rawdata "./USBLF_PCGSC17/C42_DDC-C4C_DE5/" --l0 -c --calrunstart 0008860 --calrunstop 0008861 0008861 0008862
Processed 30407 events
Event processing took 87577ms
```
```
$> ls -lh RootData/run_008861.root | awk '{ print $5 " " $9}'
426M RootData/run_008861.root
```
--> no change but should be faster when reading...
- ## compression at branch level
#### `TBranch *branch = t4->GetBranch("cluster_branch");`
#### `if (branch)`
#### `    branch->SetCompressionLevel(6);`
```
./Decode --rawdata "./USBLF_PCGSC17/C42_DDC-C4C_DE5/" --l0 -c --calrunstart 0008860 --calrunstop 0008861 0008861 0008862
Processed 30407 events
Event processing took 91075ms
```
```
$> ls -lh RootData/run_008861.root | awk '{ print $5 " " $9}'
426M RootData/run_008861.root
```
- ## filling `ev->CalSigma`, `ev->CalPed` and `ev->CalStatus` only first time, since are `static`
```
./Decode --rawdata "./USBLF_PCGSC17/C42_DDC-C4C_DE5/" --l0 -c --calrunstart 0008860 --calrunstop 0008861 0008861 0008862
Processed 30407 events
Event processing took 92939ms
```
```
$> ls -lh RootData/run_008861.root | awk '{ print $5 " " $9}'
426M RootData/run_008861.root
```
- ## streaming on disk `RawSignal` via
#### `template <typename T, size_t N1, size_t N2, size_t N3> using Map3 = std::map<size_t, std::map<size_t, std::map<size_t, T>>>;`
## instead of
#### `template <typename T, size_t N1, size_t N2, size_t N3> using Array3 = T[N1][N2][N3];`
```
./Decode --rawdata "./USBLF_PCGSC17/C42_DDC-C4C_DE5/" --l0 -c --calrunstart 0008860 --calrunstop 0008861 0008861 0008862
Processed 30407 events
Event processing took 612979ms
```
```
$> ls -lh RootData/run_008861.root | awk '{ print $5 " " $9}'
1.8G RootData/run_008861.root
```
- ## streaming on disk `RawSignal` via
#### `template <typename T, size_t N1, size_t N2, size_t N3> using Map3 = std::map<size_t, std::map<size_t, std::array<T, N3>>>;`
## instead of
#### `template <typename T, size_t N1, size_t N2, size_t N3> using Array3 = T[N1][N2][N3];`
```
./Decode --rawdata "./USBLF_PCGSC17/C42_DDC-C4C_DE5/" --l0 -c --calrunstart 0008860 --calrunstop 0008861 0008861 0008862
Processed 30407 events
Event processing took 611241ms
```
```
$> ls -lh RootData/run_008861.root | awk '{ print $5 " " $9}'
1.8G RootData/run_008861.root
```
and however there's a worring:
```
Error in <TGenCollectionProxy>: Unknown fundamental type short[1024]
Error in <Pair Emulation Building>: short[1024] is not yet supported in pair emulation
```
- ## streaming on disk `RawSignal` via
#### `template <typename T, size_t N1, size_t N2, size_t N3> using Map3 = std::map<size_t, std::map<size_t, T[N3]>>;`
## instead of
#### `template <typename T, size_t N1, size_t N2, size_t N3> using Array3 = T[N1][N2][N3];`
```
./Decode --rawdata "./USBLF_PCGSC17/C42_DDC-C4C_DE5/" --l0 -c --calrunstart 0008860 --calrunstop 0008861 0008861 0008862
Processed 30407 events
Event processing took 597869ms
```
```
$> ls -lh RootData/run_008861.root | awk '{ print $5 " " $9}'
1.8G RootData/run_008861.root
```
and however there's a worring:
```
Error in <TGenCollectionProxy>: Unknown fundamental type short[1024]
Error in <Pair Emulation Building>: short[1024] is not yet supported in pair emulation
```
- ## going back to
#### `template <typename T, size_t N1, size_t N2, size_t N3> using Array3 = T[N1][N2][N3];`
## but setting
####  `using EventAMSL0 = GenericEvent<2, 9, 64, 8, 16, 0>;`
## instead
#### `using EventAMSL0 = GenericEvent<4, 9, 64, 8, 16, 0>;`
```
./Decode --rawdata "./USBLF_PCGSC17/C42_DDC-C4C_DE5/" --l0 -c --calrunstart 0008860 --calrunstop 0008861 0008861 0008862
Processed 30407 events
Event processing took 85121ms
```
```
$> ls -lh RootData/run_008861.root | awk '{ print $5 " " $9}'
421M RootData/run_008861.root
```
