#!/usr/bin/env bash
#
# publish-latest-benchmarks.sh
#
# Generate and publish the newest STCP Raspberry Pi benchmark results.
#
# Usage:
#   ./publish-latest-benchmarks.sh udp
#   ./publish-latest-benchmarks.sh tcp
#   ./publish-latest-benchmarks.sh both
#
# Optional:
#   RESULT_DIR=/path/to/full-... ./publish-latest-benchmarks.sh udp
#   PUBLISH=0 ./publish-latest-benchmarks.sh both
#   ALLOW_FAILED=1 ./publish-latest-benchmarks.sh udp
#

set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT="${ROOT:-$SCRIPT_DIR}"

MODE="${1:-both}"
RESULTS_ROOT="${RESULTS_ROOT:-$ROOT/benchmark/results}"
RESULT_DIR="${RESULT_DIR:-}"

GENERATOR_DIR="${GENERATOR_DIR:-$ROOT/benchmark/stcp-raspberry-tcp-generator}"
TCP_SCRIPT="${TCP_SCRIPT:-$GENERATOR_DIR/generate-and-publish-tcp.sh}"
UDP_SCRIPT="${UDP_SCRIPT:-$GENERATOR_DIR/generate-and-publish-udp.sh}"

WEB_ROOT="${WEB_ROOT:-$ROOT/web/benchmarks/raspberry-pi}"
PUBLISH="${PUBLISH:-1}"
ALLOW_FAILED="${ALLOW_FAILED:-0}"
DRY_RUN="${DRY_RUN:-0}"

LOG_DIR="${LOG_DIR:-$ROOT/benchmark/pipeline-logs}"
STAMP="${STAMP:-$(date +%Y%m%d-%H%M%S)}"
LOG_FILE="${LOG_FILE:-$LOG_DIR/publish-latest-$MODE-$STAMP.log}"

mkdir -p "$LOG_DIR"
exec > >(tee -a "$LOG_FILE") 2>&1

log()  { printf '[INFO] %s\n' "$*"; }
ok()   { printf '[ OK ] %s\n' "$*"; }
warn() { printf '[WARN] %s\n' "$*" >&2; }
die()  { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

need_cmd() {
    command -v "$1" >/dev/null 2>&1 || die "Missing command: $1"
}

usage() {
    cat <<EOF
Usage:
  $0 udp
  $0 tcp
  $0 both

Environment:
  RESULT_DIR=/path/to/full-...   Use a specific result directory
  PUBLISH=0                     Generate only, do not publish
  ALLOW_FAILED=1                Allow publishing result sets with failed cases
  DRY_RUN=1                     Show actions without running generators
EOF
}

resolve_mode() {
    case "$MODE" in
        udp|tcp|both) ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            usage >&2
            die "Unknown mode: $MODE"
            ;;
    esac
}

latest_result_with_carrier() {
    local carrier="$1"

    find "$RESULTS_ROOT" \
        -mindepth 1 \
        -maxdepth 1 \
        -type d \
        -name 'full-*' \
        -printf '%T@ %p\n' 2>/dev/null |
    sort -nr |
    cut -d' ' -f2- |
    while IFS= read -r dir; do
        [[ -d "$dir/$carrier" ]] || continue

        if find "$dir/$carrier" \
            -maxdepth 1 \
            -type f \
            -regextype posix-extended \
            -regex ".*/(${carrier}|tls|stcp-${carrier})-c[0-9]+-p[0-9]+-q[0-9]+\.json" \
            -print -quit |
            grep -q .
        then
            printf '%s\n' "$dir"
            break
        fi
    done
}

resolve_result_dir() {
    if [[ -n "$RESULT_DIR" ]]; then
        RESULT_DIR="$(cd "$RESULT_DIR" 2>/dev/null && pwd)" ||
            die "Result directory not found: $RESULT_DIR"
        return
    fi

    case "$MODE" in
        udp)
            RESULT_DIR="$(latest_result_with_carrier udp)"
            ;;
        tcp)
            RESULT_DIR="$(latest_result_with_carrier tcp)"
            ;;
        both)
            RESULT_DIR="$(
                find "$RESULTS_ROOT" \
                    -mindepth 1 \
                    -maxdepth 1 \
                    -type d \
                    -name 'full-*' \
                    -printf '%T@ %p\n' 2>/dev/null |
                sort -nr |
                cut -d' ' -f2- |
                while IFS= read -r dir; do
                    [[ -d "$dir/udp" && -d "$dir/tcp" ]] || continue

                    udp_file="$(
                        find "$dir/udp" -maxdepth 1 -type f \
                            -regextype posix-extended \
                            -regex '.*/(udp|tls|stcp-udp)-c[0-9]+-p[0-9]+-q[0-9]+\.json' \
                            -print -quit
                    )"

                    tcp_file="$(
                        find "$dir/tcp" -maxdepth 1 -type f \
                            -regextype posix-extended \
                            -regex '.*/(tcp|tls|stcp-tcp)-c[0-9]+-p[0-9]+-q[0-9]+\.json' \
                            -print -quit
                    )"

                    if [[ -n "$udp_file" && -n "$tcp_file" ]]; then
                        printf '%s\n' "$dir"
                        break
                    fi
                done
            )"
            ;;
    esac

    [[ -n "$RESULT_DIR" ]] ||
        die "No matching full-* result directory found for mode '$MODE'"

    RESULT_DIR="$(cd "$RESULT_DIR" && pwd)"
}


validate_carrier() {
    local carrier="$1"
    local dir="$RESULT_DIR/$carrier"
    local file
    local count=0
    local failures=0
    local malformed=0
    local failed_file="$RESULT_DIR/FAILED-${carrier^^}-CASES.tsv"
    local malformed_file="$RESULT_DIR/MALFORMED-${carrier^^}-CASES.txt"

    [[ -d "$dir" ]] || die "Missing result directory: $dir"

    : >"$failed_file"
    : >"$malformed_file"

    while IFS= read -r -d '' file; do
        count=$((count + 1))

        if ! jq -e 'type == "object"' "$file" >/dev/null 2>&1; then
            malformed=$((malformed + 1))
            printf '%s\n' "$file" >>"$malformed_file"
            continue
        fi

        errors="$(jq -r '(.errors // 0) | tonumber? // 0' "$file")"
        operations="$(jq -r '(.operations // 0) | tonumber? // 0' "$file")"
        details="$(jq -r '(.error_details // []) | length' "$file")"

        if (( errors != 0 || operations <= 0 || details != 0 )); then
            failures=$((failures + 1))

            printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
                "$file" \
                "$(jq -r '.clients // "?"' "$file")" \
                "$(jq -r '.payload_bytes // "?"' "$file")" \
                "$(jq -r '.pipeline // "?"' "$file")" \
                "$operations" \
                "$errors" \
                "$(jq -r '(.error_details // []) | join("; ")' "$file")" \
                >>"$failed_file"
        fi
    done < <(
        if [[ "$carrier" == "udp" ]]; then
            find "$dir" -maxdepth 1 -type f \
                -regextype posix-extended \
                -regex '.*/(udp|tls|stcp-udp)-c[0-9]+-p[0-9]+-q[0-9]+\.json' \
                -print0
        else
            find "$dir" -maxdepth 1 -type f \
                -regextype posix-extended \
                -regex '.*/(tcp|tls|stcp-tcp)-c[0-9]+-p[0-9]+-q[0-9]+\.json' \
                -print0
        fi
    )

    (( count > 0 )) || die "No $carrier benchmark case JSON files found"

    if (( malformed > 0 )); then
        die "$carrier contains $malformed malformed JSON files; see $malformed_file"
    fi

    if (( failures > 0 )); then
        if [[ "$ALLOW_FAILED" == "1" ]]; then
            warn "$carrier contains $failures failed cases; publishing allowed by ALLOW_FAILED=1"
        else
            die "$carrier contains $failures failed cases; see $failed_file"
        fi
    else
        rm -f "$failed_file"
    fi

    rm -f "$malformed_file"
    ok "Validated $count $carrier benchmark case files"
}

run_carrier() {
    local carrier="$1"
    local script
    local output_dir="$WEB_ROOT/$carrier"

    if [[ "$carrier" == "udp" ]]; then
        script="$UDP_SCRIPT"
    else
        script="$TCP_SCRIPT"
    fi

    [[ -f "$script" ]] || die "Generator script missing: $script"

    log "Generating $carrier from: $RESULT_DIR/$carrier"
    log "Output directory: $output_dir"
    log "Publish: $PUBLISH"

    if [[ "$DRY_RUN" == "1" ]]; then
        warn "DRY_RUN=1; would run: $script"
        return 0
    fi

    mkdir -p "$output_dir"

    if [[ "$carrier" == "udp" ]]; then
        RESULT_DIR="$RESULT_DIR" \
        RESULTS_DIR="$RESULT_DIR" \
        UDP_RESULT_DIR="$RESULT_DIR/udp" \
        UDP_RESULTS_DIR="$RESULT_DIR/udp" \
        INPUT_DIR="$RESULT_DIR/udp" \
        OUTPUT_DIR="$output_dir" \
        WEB_DIR="$output_dir" \
        PUBLISH="$PUBLISH" \
        AUTO_PUBLISH="$PUBLISH" \
            bash "$script" "$RESULT_DIR/udp" "$output_dir"
    else
        RESULT_DIR="$RESULT_DIR" \
        RESULTS_DIR="$RESULT_DIR" \
        TCP_RESULT_DIR="$RESULT_DIR/tcp" \
        TCP_RESULTS_DIR="$RESULT_DIR/tcp" \
        INPUT_DIR="$RESULT_DIR/tcp" \
        OUTPUT_DIR="$output_dir" \
        WEB_DIR="$output_dir" \
        PUBLISH="$PUBLISH" \
        AUTO_PUBLISH="$PUBLISH" \
            bash "$script" "$RESULT_DIR/tcp" "$output_dir"
    fi

    ok "$carrier generation/publication finished"
}

main() {
    cd "$ROOT"

    for cmd in bash find jq tee; do
        need_cmd "$cmd"
    done

    resolve_mode
    resolve_result_dir

    log "Selected mode: $MODE"
    log "Result set:    $RESULT_DIR"

    case "$MODE" in
        udp)
            validate_carrier udp
            run_carrier udp
            ;;
        tcp)
            validate_carrier tcp
            run_carrier tcp
            ;;
        both)
            validate_carrier udp
            validate_carrier tcp
            run_carrier udp
            run_carrier tcp
            ;;
    esac

    cat <<EOF

Latest benchmark publication finished.

Mode:       $MODE
Results:    $RESULT_DIR
Web output: $WEB_ROOT
Published:  $PUBLISH
Log:        $LOG_FILE

EOF
}

main "$@"
