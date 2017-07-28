#!/bin/bash

find . -name "*.[ch]"   | sudo tee -a cscope.files
sudo cscope -bkq -i cscope.files
sudo ctags -R .
