#!/usr/bin/env bash

set -e

EXECUTABLE=../cmake-build-debug/transport_catalogue

if [ ! -f "$EXECUTABLE" ]; then
  echo "Executable $EXECUTABLE not found! Build it first!"
  exit 1
fi

DATA_DIR="./data"

#cases=( "tsA_case1" "tsA_case2" )
cases=( "tsB_case1" "tsB_case2" )

for case in "${cases[@]}"; do
  echo "=== Running $case ==="

  INPUT="${DATA_DIR}/${case}_input.txt"
  EXPECTED="${DATA_DIR}/${case}_output.txt"
  OUTPUT="${DATA_DIR}/result_${case}.txt"

  $EXECUTABLE < "$INPUT" > "$OUTPUT"

  if diff -q "$OUTPUT" "$EXPECTED"; then
    echo "[OK] $case passed ✅"
  else
    echo "[FAIL] $case failed ❌"
    echo "=== Diff ==="
    diff "$OUTPUT" "$EXPECTED"
    exit 1
  fi
done

echo "=== All integration tests passed 🎉 ==="
