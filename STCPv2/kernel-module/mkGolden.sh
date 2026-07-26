#!/usr/bin/env bash
set -uo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="${PROJECT_ROOT:-$SCRIPT_DIR}"

TAG_PREFIX="${TAG_PREFIX:-stcp-golden}"
TAG="${TAG:-${TAG_PREFIX}-$(date +%Y%m%d-%H%M%S)}"
MESSAGE="${MESSAGE:-STCP golden baseline}"
COMMIT_MESSAGE="${COMMIT_MESSAGE:-golden: ${MESSAGE}}"
TAG_MESSAGE="${TAG_MESSAGE:-${MESSAGE}}"

REMOTE="${REMOTE:-origin}"
PUSH="${PUSH:-0}"
ALLOW_DIRTY_AFTER_COMMIT="${ALLOW_DIRTY_AFTER_COMMIT:-0}"
ALLOW_EMPTY_COMMIT="${ALLOW_EMPTY_COMMIT:-0}"
GIT_PATHS="${GIT_PATHS:-.}"

log() { printf '[%s] %s\n' "$(date '+%F %T')" "$*"; }
die() { log "ERROR: $*"; exit 1; }

command -v git >/dev/null 2>&1 || die "git is not installed"

GIT_ROOT="$(git -C "$PROJECT_ROOT" rev-parse --show-toplevel 2>/dev/null)" || \
    die "Not inside a Git repository: $PROJECT_ROOT"

BRANCH="$(git -C "$GIT_ROOT" symbolic-ref --quiet --short HEAD 2>/dev/null || true)"
[[ -n "$BRANCH" ]] || die "Detached HEAD is not allowed"

if git -C "$GIT_ROOT" rev-parse -q --verify "refs/tags/$TAG" >/dev/null; then
    die "Tag already exists: $TAG"
fi

read -r -a PATHS <<<"$GIT_PATHS"
((${#PATHS[@]} > 0)) || PATHS=(".")

log "Repository: $GIT_ROOT"
log "Branch:     $BRANCH"
log "Tag:        $TAG"
log "Paths:      ${PATHS[*]}"

BEFORE_COMMIT="$(git -C "$GIT_ROOT" rev-parse HEAD)"

log "Staging changes"
git -C "$GIT_ROOT" add -A -- "${PATHS[@]}" || die "git add failed"

if git -C "$GIT_ROOT" diff --cached --quiet; then
    if [[ "$ALLOW_EMPTY_COMMIT" == "1" ]]; then
        log "No staged changes; creating an empty golden commit"
        git -C "$GIT_ROOT" commit --allow-empty -m "$COMMIT_MESSAGE" || \
            die "git commit failed"
    else
        log "No staged changes; tagging current HEAD"
    fi
else
    log "Creating commit"
    git -C "$GIT_ROOT" commit -m "$COMMIT_MESSAGE" || die "git commit failed"
fi

COMMIT="$(git -C "$GIT_ROOT" rev-parse HEAD)"
SHORT_COMMIT="$(git -C "$GIT_ROOT" rev-parse --short HEAD)"

if [[ "$ALLOW_DIRTY_AFTER_COMMIT" != "1" ]]; then
    REMAINING="$(git -C "$GIT_ROOT" status --porcelain)"
    if [[ -n "$REMAINING" ]]; then
        echo "$REMAINING" >&2
        die "Working tree is still dirty after commit; tag was not created"
    fi
fi

HOST_NAME="$(hostname 2>/dev/null || echo unknown)"
KERNEL="$(uname -r 2>/dev/null || echo unknown)"
LATEST_RESULTS="$(
    find "$GIT_ROOT/benchmark/results" \
        -maxdepth 1 -type d -name 'full-*' -printf '%T@ %p\n' 2>/dev/null |
    sort -nr |
    awk 'NR==1 {$1=""; sub(/^ /, ""); print}'
)"

ANNOTATION="$(cat <<EOF
${TAG_MESSAGE}

Branch: ${BRANCH}
Commit: ${COMMIT}
Host: ${HOST_NAME}
Kernel: ${KERNEL}
Created: $(date --iso-8601=seconds)
Previous HEAD: ${BEFORE_COMMIT}
Latest results: ${LATEST_RESULTS:-not found}
EOF
)"

log "Creating annotated tag"
git -C "$GIT_ROOT" tag -a "$TAG" -m "$ANNOTATION" || die "git tag failed"

if [[ "$PUSH" == "1" ]]; then
    log "Pushing branch '$BRANCH' to '$REMOTE'"
    git -C "$GIT_ROOT" push "$REMOTE" "$BRANCH" || die "branch push failed"

    log "Pushing tag '$TAG' to '$REMOTE'"
    git -C "$GIT_ROOT" push "$REMOTE" "$TAG" || die "tag push failed"
else
    log "PUSH=0: commit and tag remain local"
fi

cat <<EOF

Golden checkpoint created successfully.

Repository: $GIT_ROOT
Branch:     $BRANCH
Commit:     $COMMIT
Short SHA:  $SHORT_COMMIT
Tag:        $TAG
Pushed:     $PUSH

Inspect:
  git show --stat $COMMIT
  git show $TAG

Push later:
  git push $REMOTE $BRANCH
  git push $REMOTE $TAG

EOF
