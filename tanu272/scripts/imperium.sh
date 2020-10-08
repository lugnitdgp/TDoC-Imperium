#!/bin/bash

function imperium(){
DIR=$PWD
export dir=$DIR
cd ~/imperium/bin || echo "Error"
./main "$@"
cd "$DIR" || echo "Error"
}