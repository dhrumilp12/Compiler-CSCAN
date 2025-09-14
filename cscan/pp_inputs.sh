#!/usr/bin/bash

mkdir -p pp

for infile in inputs/*.c; do
    base=$(basename "$infile" .c)
    outfile="pp/${base}.pp.c"
    clang -E -C "$infile" -o "$outfile"
	  sed -i '/#/d' "$outfile"
done

