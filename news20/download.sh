#!/bin/bash
wget http://nlp.stanford.edu/data/glove.6B.zip
#wget http://www.cs.cmu.edu/afs/cs.cmu.edu/project/theo-20/www/data/news20.tar.gz
wget http://qwone.com/~jason/20Newsgroups/20news-bydate.tar.gz
mkdir glove.6B
cd glove.6B
tar -xvf ../glove.6B.zip
cd ..
tar -xvf news20.tar.gz
