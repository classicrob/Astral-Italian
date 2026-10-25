// JXA through osascript: orderly quit to one owned PID, no AX or controller input.
ObjC.import('AppKit');
function run(argv) {
    var pid = Number(argv[0]), expectedPath = argv[1];
    var expectedId = argv[2] || 'dev.petari.Petari.perfm1b';
    if (!/^dev\.petari\.Petari\.perf[A-Za-z0-9_-]+$/.test(expectedId))
        throw new Error('Dedicated perf bundle ID required');
    if (!Number.isInteger(pid) || pid <= 1) throw new Error('Invalid owned PID');
    var app = $.NSRunningApplication.runningApplicationWithProcessIdentifier(pid);
    if (!app || app.isTerminated) throw new Error('Owned app is not running');
    var bundle = ObjC.unwrap(app.bundleIdentifier), path = ObjC.unwrap(app.bundleURL.path);
    if (bundle !== expectedId || path !== expectedPath)
        throw new Error('Refusing a PID outside the exact B perf app');
    var sent = Boolean(app.terminate);
    if (!sent) throw new Error('Orderly termination request was rejected');
    return JSON.stringify({pid: pid, bundle_id: bundle, app: path, orderly_quit_requested: sent});
}
