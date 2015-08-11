#!/bin/sh
wget https://launchpad.net/intelhex/1.0/1.4/+download/intelhex-1.4.tar.gz
tar -xf intelhex-1.4.tar.gz 
cd intelhex-1.4
sudo python setup.py install
cd ..
