#!/bin/bash
for i in {11..20}
do
    hdfs dfs -put data/gutenberg-500M.txt data-input/$i.txt
done