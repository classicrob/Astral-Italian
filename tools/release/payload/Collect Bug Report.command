#!/bin/sh
# Double-click after something went wrong (or felt different) in Astral Italian. It gathers the newest log, crash report,
# M/F9 capture and exact session tape plus the build identity into ONE zip on your Desktop that you send to the developers.
# Nothing is uploaded. Format: tools/release/REPORT-FORMAT.md in the source repository.
support="${ASTRAL_SUPPORT_DIR:-$HOME/Library/Application Support/Astral Italian}"
stamp=$(date +%Y%m%d-%H%M%S)
desktop="${ASTRAL_REPORT_DESKTOP:-$HOME/Desktop}"
work=$(mktemp -d "${TMPDIR:-/tmp}/astral-report.XXXXXX") || exit 1
name="AstralItalian-report-$stamp"
out="$work/$name"
mkdir -p "$out"
trap 'rm -rf "$work"' EXIT

here=$(cd "$(dirname "$0")" && pwd)
app=""
for candidate in "$ASTRAL_APP" "/Applications/Astral Italian.app" "$here/Astral Italian.app" "$HOME/Applications/Astral Italian.app"; do
    [ -n "$candidate" ] && [ -d "$candidate" ] && app=$candidate && break
done

cat <<'EOF'
Astral Italian bug report
-------------------------
This will make a zip file on your Desktop with:
  - what the game logged, its newest crash report, and your M/F9 captures,
  - the exact record of your latest play session (a "tape": your button presses and Mario's movement), which lets the
    developers replay what happened frame by frame,
  - your Mac model, macOS version, screen info, and which version of the app you ran.
It does NOT include your saves, your game disc or any game files. Nothing is sent anywhere: you send the zip yourself.
Details are in README-PRIVACY.txt inside the zip. The log files contain folder paths on your Mac, which include your user name.

EOF
note=""
if [ -t 0 ] && [ "${ASTRAL_REPORT_NONINTERACTIVE:-0}" != 1 ]; then
    echo "In a sentence or two: what felt different or went wrong? (Return to skip)"
    printf "> "
    IFS= read -r note
    echo
fi
printf '%s\n' "$note" > "$out/note.txt"

# ---- build identity ------------------------------------------------------------------------------------------------
plist_get() { /usr/libexec/PlistBuddy -c "Print :$1" "$app/Contents/Info.plist" 2>/dev/null || true; }
json_str() { printf '%s' "$1" | sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' | tr -d '\000-\037'; }
version=""; commit=""; uuid=""
if [ -n "$app" ]; then
    version=$(plist_get CFBundleShortVersionString)
    commit=$(plist_get AstralSourceCommit)
    uuid=$(plist_get AstralExecutableUUID)
    exe_uuid=$(dwarfdump --uuid "$app/Contents/MacOS/Petari" 2>/dev/null | awk '{print $2}' | head -n1)
    [ -n "$exe_uuid" ] && uuid=$exe_uuid
fi
macos="$(sw_vers -productVersion) ($(sw_vers -buildVersion))"
model=$(sysctl -n hw.model 2>/dev/null)
chip=$(sysctl -n machdep.cpu.brand_string 2>/dev/null)
memgb=$(($(sysctl -n hw.memsize 2>/dev/null || echo 0) / 1073741824))

# ---- files -------------------------------------------------------------------------------------------------------
# newest DIR N: the N most recently modified entries of DIR, one per line (names may contain spaces).
newest() { ls -1dt "$1"/* 2>/dev/null | head -n "$2"; }
copy_newest() { newest "$1" "$2" | while IFS= read -r item; do cp -R "$item" "$3/" 2>/dev/null; done; }

mkdir -p "$out/logs" "$out/captures" "$out/crashes" "$out/tapes" "$out/settings"
copy_newest "$support/logs" 3 "$out/logs"
copy_newest "$support/Captures" 3 "$out/captures"
copy_newest "$support/Crashes" 3 "$out/crashes"
# macOS's own crash reports for the app (these also say what the system saw), newest three
if [ -d "$HOME/Library/Logs/DiagnosticReports" ]; then
    # only the last 14 days, and only the app itself (Petari-... / Astral Italian-...), not other tools' reports
    find "$HOME/Library/Logs/DiagnosticReports" -maxdepth 1 -type f -mtime -14 \( -name 'Petari-*' -o -name 'Astral Italian-*' \) 2>/dev/null \
        | xargs -I{} ls -1dt "{}" 2>/dev/null | head -n 3 | while IFS= read -r f; do cp "$f" "$out/crashes/" 2>/dev/null; done
fi
for f in mods.txt settings.txt display.txt controls.txt camera.txt; do
    [ -f "$support/$f" ] && cp "$support/$f" "$out/settings/"
done

# The newest exact tape. A tape grows about 150 MB an hour before zipping; it compresses well.
tape_name=""; tape_included=false; tape_reason=""
tape=""
tape=$(newest "$support/Tapes" 20 | while IFS= read -r t; do [ -f "$t/route.tape.json" ] && { printf '%s\n' "$t"; break; }; done)
[ -z "$tape" ] && tape=$(newest "$support/Tapes" 1)
if [ -n "$tape" ]; then
    tape_name=$(basename "$tape")
    kb=$(du -sk "$tape" | awk '{print $1}')
    if [ "$kb" -gt 1500000 ]; then
        tape_reason="newest tape is $((kb / 1024)) MB, over the 1500 MB limit"
    else
        mkdir -p "$out/tapes/$tape_name"
        cp "$tape"/route.csv "$tape"/route.kpd "$tape"/route.actions "$tape"/route.tape.json "$out/tapes/$tape_name/" 2>/dev/null
        if [ -f "$out/tapes/$tape_name/route.tape.json" ]; then tape_included=true; else tape_reason="newest tape folder has no route.tape.json"; fi
    fi
else
    tape_reason="no session tape found (tapes are recorded automatically; ASTRAL_TAPE=off disables them)"
fi

cat > "$out/build.json" <<EOF
{
  "report_format": 1,
  "report_time": "$(date +%Y-%m-%dT%H:%M:%S%z)",
  "app_version": "$(json_str "$version")",
  "source_commit": "$(json_str "$commit")",
  "executable_uuid": "$(json_str "$uuid")",
  "app_path": "$(json_str "$app")",
  "macos": "$(json_str "$macos")",
  "model": "$(json_str "$model")",
  "chip": "$(json_str "$chip")",
  "memory_gb": $memgb,
  "newest_tape": "$(json_str "$tape_name")",
  "tape_included": $tape_included,
  "tape_skipped_reason": "$(json_str "$tape_reason")"
}
EOF

{
    echo "report time: $(date)"
    echo "app: ${app:-not found} version=$version commit=$commit uuid=$uuid"
    echo "macOS: $macos   model: $model   chip: $chip   memory: ${memgb} GB"
    echo "free disk: $(df -h "$HOME" | awk 'NR==2{print $4}')"
    [ -n "$tape_reason" ] && echo "tape: $tape_reason"
    system_profiler SPDisplaysDataType 2>/dev/null | sed -n '1,40p'
} > "$out/system.txt"

cat > "$out/README-PRIVACY.txt" <<'EOF'
What is in this report, and what is not
=======================================
Included:
  build.json, system.txt   which app version you ran; your Mac model, chip, memory, macOS version, display info, free disk space.
  note.txt                 the sentence you typed (empty if you skipped it).
  logs/                    the game's three newest run logs. They contain folder paths on your Mac (which include your user
                           name) and technical messages about loading, shaders and timing.
  captures/                your M/F9 captures: the last 5 seconds of inputs and Mario's movement, his state, a screenshot.
  crashes/                 the game's crash reports and macOS's own crash reports for the app (also with paths and your user name).
  tapes/                   the exact record of your newest play session: every controller/keyboard read the game made and
                           Mario's position per frame, so developers can replay it. It contains no video or audio.
  settings/                your mods, controls, display and camera settings.
NOT included: your save files, the game disc or any file extracted from it, other sessions' tapes, shader caches, anything
outside the folders named above. Nothing is uploaded; you decide who you send this zip to. Delete it afterwards if you like.
EOF

zip_path="$desktop/$name.zip"
mkdir -p "$desktop"
(cd "$work" && ditto -c -k --norsrc --noextattr --keepParent "$name" "$zip_path") || { echo "Could not create the zip."; exit 1; }
size_mb=$(( $(stat -f %z "$zip_path") / 1048576 ))
echo "Report saved: $zip_path (${size_mb} MB)"
# A smaller zip without the tape for chat/issue attachments with small limits.
if [ "$tape_included" = true ] && [ "$size_mb" -gt 20 ]; then
    rm -rf "$out/tapes"; mkdir -p "$out/tapes"
    sed -i '' -e 's/"tape_included": true/"tape_included": false/' -e 's/"tape_skipped_reason": ""/"tape_skipped_reason": "removed to make a small copy"/' "$out/build.json"
    small="$desktop/$name-without-tape.zip"
    (cd "$work" && ditto -c -k --norsrc --noextattr --keepParent "$name" "$small") && echo "Also saved a copy without the tape (small enough for most chat apps): $small"
fi
echo "Send the zip to the developers (a cloud-drive link or direct message is best; GitHub issues accept files up to 25 MB)."
[ "${ASTRAL_REPORT_NONINTERACTIVE:-0}" = 1 ] || open -R "$zip_path"
