#!/bin/bash
sudo apt-get update
sudo apt-get install openssl -y
sudo apt-get install libssl-dev -y
mkdir -p ~/imperium/bin
cp imperium.sh ~/imperium
cd ..
make
cd ~/imperium/bin || echo "error"
chmod +x main
cd ..
if grep -q "source $PWD/imperium.sh" "$PWD/../.bashrc" ; then
echo 'already installed bash source';
else
echo "source $PWD/imperium.sh" >> ~/.bashrc;
fi
