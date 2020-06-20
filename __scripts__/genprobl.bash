#!/usr/bin/env bash
set -euo pipefail

N=${1:-4}
for i in $(seq 1 "$N"); do
  for j in $(seq 1 "$N"); do
    if test "$i" == "$j"; then
      printf '0'
    else
      printf '%d' "$((1 + RANDOM % 9))"
    fi
    if test "$j" != "$N"; then
      printf ' '
    fi
  done
  printf '\n'
done
