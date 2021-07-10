#!/bin/bash

gcc -o graph_ranker graph_ranker.c

./graph_ranker < $1
