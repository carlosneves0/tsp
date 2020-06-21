#!/usr/bin/env bash
set -euo pipefail

n="${1:-3}"

printf "$n\n"
for i in $(seq 0 "$((n - 1))"); do
  for j in $(seq 0 "$((n - 1))"); do
    # echo >&2 -e "i=$i\tj=$j\ti+1=$((i+1))\t(i+1)%n=$(((i+1) %  n))"
    if test "$i" == "$j"; then
      printf 0
    elif test "$j" == "$(((i+1) %  n))"; then
      printf 1
    else
      printf "$((2 + RANDOM % 8))"
    fi
    if test "$j" != "$n"; then
      printf ' '
    fi
  done
  printf '\n'
done
