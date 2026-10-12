#!/usr/bin/env bash
# Benchmarks build_release/benchmarks/solve_dodo on all JSON files in
# benchmarks/artificial/, with and without --bddlike and for each of -i f/t/s.
# Paths are relative to the abstracton project root.

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BIN="$ROOT/build_release/benchmarks/solve_dodo"
BENCHDIR="$ROOT/benchmarks/artificial"

IND_ALG="project"
MEM_LIMIT="32G"
SWAP_LIMIT="7G"
TIME_LIMIT=1200

COMMIT="$(git -C "$ROOT" rev-parse --short HEAD)"
OUT="$SCRIPT_DIR/results/results${COMMIT}.csv"
mkdir -p "$SCRIPT_DIR/results"

if [ -n "$(git -C "$ROOT" status --porcelain)" ]; then
  echo "WARNING: there are uncommitted changes in $ROOT:"
  git -C "$ROOT" status --short
  read -r -p "Proceed with the benchmark anyway? [y/N] " ans
  case "$ans" in
    [Yy]*) ;;
    *) echo "Aborted."; exit 1 ;;
  esac
fi

quote() { printf '"%s"' "${1//\"/\"\"}"; }
emit() {
  local out=""
  for f in "$@"; do out+="$(quote "$f"),"; done
  printf '%s\n' "${out%,}"
}

emit name path ind_alg interpretation bddlike time max_rss mem_limit swap_limit \
     time_limit alphabet_size automaton_sizes step_times separated unseparated \
     timeout oom > "$OUT"

for bddlike in no yes; do
  for I in f t s; do
    for FILE in "$BENCHDIR"/*.json; do
      NAME="$(basename "$FILE" .json)"
      RELPATH="${FILE#"$ROOT/benchmarks/"}"
      FLAG=(); [ "$bddlike" = yes ] && FLAG=(--bddlike)

      echo "== $NAME  (i=$I, bddlike=$bddlike) ==" >&2

      TMP="$(mktemp)"
      /usr/bin/time -v systemd-run --user --scope \
        -p "MemoryMax=$MEM_LIMIT" \
        -p "MemorySwapMax=$SWAP_LIMIT" \
        timeout "$TIME_LIMIT" "$BIN" --ind-alg "$IND_ALG" --measure-time -v 2 \
        --no-dot-printing "${FLAG[@]}" -i "$I" "$FILE" >"$TMP" 2>&1
      STATUS=$?
      OUTP="$(cat "$TMP")"
      rm -f "$TMP"

      USER_T="$(awk -F': ' '/User time \(seconds\)/ {print $2; exit}' <<<"$OUTP")"
      SYS_T="$(awk -F': ' '/System time \(seconds\)/ {print $2; exit}' <<<"$OUTP")"
      RSS="$(awk -F': ' '/Maximum resident set size \(kbytes\)/ {print $2; exit}' <<<"$OUTP")"
      TIME="$(awk -v a="${USER_T:-0}" -v b="${SYS_T:-0}" 'BEGIN{printf "%.3f", a+b}')"

      ALPHABET="$(sed -n 's/^alphabet size: //p' <<<"$OUTP" | head -1)"

      AUTOMATA="$(awk '
        /^automaton .*:$/ {
          name=$0; sub(/^automaton /,"",name); sub(/:$/,"",name);
          if ((getline line) > 0 && line ~ /^[0-9]+ states$/) {
            split(line, a, " "); printf "%s=%s;", name, a[1]
          }
        }' <<<"$OUTP")"
      AUTOMATA="${AUTOMATA%;}"

      STEPS="$(sed -n 's/^Time needed for \(.*\): \([0-9]*\)\[.*\]$/\1=\2/p' <<<"$OUTP")"
      STEPS="$(tr '\n' ';' <<<"$STEPS")"
      STEPS="${STEPS%;}"

      SEPARATED="$(awk '/could be separated from the initial configurations/ {getline; gsub(/^[ \t]+/,""); print; exit}' <<<"$OUTP")"
      UNSEPARATED="$(awk '/could not be separated from the initial configurations/ {getline; gsub(/^[ \t]+/,""); print; exit}' <<<"$OUTP")"

      TIMEOUT=no; OOM=no
      if [ "$STATUS" -eq 124 ]; then
        TIMEOUT=yes
      elif [ "$STATUS" -eq 137 ] || [ "$STATUS" -eq 143 ]; then
        OOM=yes
      fi

      emit "$NAME" "$RELPATH" "$IND_ALG" "$I" "$bddlike" "$TIME" "${RSS:-}" "$MEM_LIMIT" \
           "$SWAP_LIMIT" "$TIME_LIMIT" "$ALPHABET" "$AUTOMATA" "$STEPS" \
           "$SEPARATED" "$UNSEPARATED" "$TIMEOUT" "$OOM" >> "$OUT"
    done
  done
done

echo "Results written to $OUT"
