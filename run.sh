#!/bin/sh
echo "           *******This is test1.c*******"
./parser test1.c code1.ir
echo " "
echo "           *******This is test2.c*******"
./parser test2.c code2.ir
echo " "
echo "           *******This is test3.c*******"
./parser test3.c code3.ir
echo " "
echo "           *******This is test4.c*******"
./parser test4.c code4.ir
echo " "
echo "           *******This is test5.c*******"
./parser test5.c code5.ir
echo " "
python irsim.pyc
