 #!/bin/bash
#  brew update
#  brew install openssl
#  brew install libssl-dev
 mkdir -p ~/imperium/bin
 cp imperium.sh ~/imperium
cd .. 
make
cd ~/imperium/bin || echo "error"
chmod +x main
cd ..
if grep -q "source $PWD/imperium.sh" "$PWD/../.bash_profile" ; then
    echo 'already installed bash source';
else
    echo "source $PWD/imperium.sh" >> ~/.bash_profile;
fi
