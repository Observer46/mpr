#!/bin/bash
for i in {3..10}
do
    hdfs dfs -put data/gutenberg-500M.txt data-input
done