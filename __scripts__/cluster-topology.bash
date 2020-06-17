#!/usr/bin/env bash
set -euo pipefail

mpiexec='mpiexec --hostfile nodes.txt --map-by ppr:1:node --tag-output'

# DEBUG="${DEBUG:-}"
# if test "$DEBUG" != ''; then
#   mpiexec="$mpiexec --report-bindings"
# fi

nodes=()
for line in $($mpiexec hostname); do
  rank="$(echo "$line" | sed -E 's/^\[[0-9]+,([0-9]+)\].*$/\1/')"
  nodes[$rank]="$(echo "$line" | sed -E 's/^\[[0-9]+,[0-9]+\]<stdout>:(.*)$/\1/')"
done

cpuinfo_bash='cat /proc/cpuinfo | grep "model name" | uniq | cut -d: -f2 | xargs echo'
cpuinfos="$($mpiexec bash -c "$cpuinfo_bash")"
cpus=()
OLDIFS=$IFS; IFS=$'\n'
for line in $cpuinfos; do
  rank="$(echo "$line" | sed -E 's/^\[[0-9]+,([0-9]+)\].*$/\1/')"
  cpus[$rank]="$(echo "$line" | sed -E 's/^\[[0-9]+,[0-9]+\]<stdout>:(.*)$/\1/')"
done
IFS=$OLDIFS

ncores=()
for line in $($mpiexec nproc); do
  rank="$(echo "$line" | sed -E 's/^\[[0-9]+,([0-9]+)\].*$/\1/')"
  ncores[$rank]="$(echo "$line" | sed -E 's/^\[[0-9]+,[0-9]+\]<stdout>:(.*)$/\1/')"
done

meminfo_bash="cat /proc/meminfo | grep MemTotal | cut -d: -f2 | awk '$2==\"kB\"{if ($1>1024^2){$1=$1/1024^2;$2=\"GB\";} else if ($2>1024){$1=$1/1024;$2=\"MB\";}} 1'"
meminfos="$($mpiexec bash -c "$meminfo_bash")"
mems=()
OLDIFS=$IFS; IFS=$'\n'
for line in $meminfos; do
  rank="$(echo "$line" | sed -E 's/^\[[0-9]+,([0-9]+)\].*$/\1/')"
  mems[$rank]="$(echo "$line" | sed -E 's/^\[[0-9]+,[0-9]+\]<stdout>:(.*)$/\1/')"
done
IFS=$OLDIFS

echo -e "RANK\tNODE\tCPU\t\t\t\t\tNCORES\tRAM"
for rank in $(seq 0 12); do
  echo -e "$(printf %02d "$rank")\t${nodes[$rank]}\t${cpus[$rank]}\t${ncores[$rank]}\t${mems[$rank]}"
done
