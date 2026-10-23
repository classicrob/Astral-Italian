#!/bin/sh
# Double-click after something went wrong in Astral Italian. Gathers the newest log, playtest capture and session
# tape plus basic system information into one zip on your Desktop that you can attach to a GitHub issue.
# Nothing is uploaded. The zip contains your Mac's model and macOS version and the file paths in the log
# (which include your user name); look inside before sharing if that matters to you.
support="$HOME/Library/Application Support/Astral Italian"
stamp=$(date +%Y%m%d-%H%M%S)
work=$(mktemp -d "${TMPDIR:-/tmp}/astral-report.XXXXXX") || exit 1
out="$work/AstralItalian-report-$stamp"
mkdir -p "$out"
trap 'rm -rf "$work"' EXIT

app=""
for candidate in "/Applications/Astral Italian.app" "$(dirname "$0")/Astral Italian.app"; do
    [ -d "$candidate" ] && app=$candidate && break
done
{
    echo "report time: $(date)"
    echo "macOS: $(sw_vers -productVersion) ($(sw_vers -buildVersion))"
    echo "model: $(sysctl -n hw.model)  chip: $(sysctl -n machdep.cpu.brand_string)  memory: $(($(sysctl -n hw.memsize) / 1073741824)) GB"
    system_profiler SPDisplaysDataType 2>/dev/null | sed -n '1,30p'
    if [ -n "$app" ]; then
        for key in CFBundleShortVersionString CFBundleVersion AstralSourceCommit AstralBuildDate; do
            echo "$key: $(/usr/libexec/PlistBuddy -c "Print :$key" "$app/Contents/Info.plist" 2>/dev/null)"
        done
    else
        echo "app: not found next to this script or in /Applications"
    fi
    echo "free disk: $(df -h "$HOME" | awk 'NR==2{print $4}')"
} > "$out/system.txt"

# newest DIR N: the N most recently modified entries of DIR, one per line (names may contain spaces).
newest() { ls -1dt "$1"/* 2>/dev/null | head -n "$2"; }
# copy_newest DIR N DEST: copy those entries into DEST
copy_newest() { newest "$1" "$2" | while IFS= read -r item; do cp -R "$item" "$3/" 2>/dev/null; done; }

mkdir -p "$out/logs" "$out/captures" "$out/tapes"
copy_newest "$support/logs" 3 "$out/logs"
copy_newest "$support/Captures" 3 "$out/captures"
mkdir -p "$out/crashes"
copy_newest "$support/Crashes" 3 "$out/crashes"
# The newest session tape lets the developers replay your session frame by frame; skip it when it is huge.
tape=$(newest "$support/Tapes" 1)
if [ -n "$tape" ]; then
    kb=$(du -sk "$tape" | awk '{print $1}')
    if [ "$kb" -lt 300000 ]; then cp -R "$tape" "$out/tapes/"; else echo "newest tape skipped ($((kb / 1024)) MB): $tape" >> "$out/system.txt"; fi
fi
[ -f "$support/app.cfg" ] && cp "$support/app.cfg" "$out/"
cp "$support/mods.txt" "$support/settings.txt" "$support/display.txt" "$out/" 2>/dev/null
ls -1t "$HOME/Library/Logs/DiagnosticReports" 2>/dev/null | grep -i "petari\|astral" | head -n 3 | while IFS= read -r name; do
    mkdir -p "$out/crashes"; cp "$HOME/Library/Logs/DiagnosticReports/$name" "$out/crashes/"
done

zip_path="$HOME/Desktop/AstralItalian-report-$stamp.zip"
(cd "$work" && ditto -c -k --norsrc --noextattr --keepParent "AstralItalian-report-$stamp" "$zip_path") || { echo "Could not create the zip."; exit 1; }
echo "Report saved: $zip_path"
echo "Please attach it to your GitHub issue (https://github.com/classicrob/Astral-Italian/issues/new/choose)."
open -R "$zip_path"
