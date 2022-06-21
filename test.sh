#preklad a pojmenovani vystupu na "pms"
mpic++ --prefix /usr/local/share/OpenMPI -o pms pms.cpp

#generovani souboru o 16 cislech
dd if=/dev/random bs=1 count=16 of=numbers > /dev/null 2>&1

#spusteni s 5 procesory pro 16 cisel
mpirun --prefix /usr/local/share/OpenMPI --oversubscribe -np 5 pms

#smazani vytvorenych souboru
rm -f pms numbers