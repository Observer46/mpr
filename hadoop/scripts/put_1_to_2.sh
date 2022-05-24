#!/bin/bash
for i in {1..2}
do
    hdfs dfs -put data/gutenberg-500M.txt data-input/$i.txt
done