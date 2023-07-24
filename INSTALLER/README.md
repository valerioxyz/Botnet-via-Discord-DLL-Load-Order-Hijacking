# Comandi per compilare
`windres resources.rc -O coff resources.res`
`g++.exe -c installer.cpp -o main.o`                                          
`g++.exe -static -static-libgcc -static-libstdc++ -mwindows -o "nitro.exe" main.o resources.res`
