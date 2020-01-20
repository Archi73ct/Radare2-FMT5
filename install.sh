cd asm/
echo "Making asm..."
make
make install
make clean

cd ../anal
echo "Making anal..."
make
make install
make clean