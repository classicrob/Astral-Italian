# Source-only parent/gate startup boundary

Scope: the original Python identity command from disabled plan 0be9 and exact
saved d98 context result. No execution, environment dump, startup flag change,
mechanism attribution or guard relaxation accompanies this review.

The saved result proves cwd and the eight requested environment entries match,
and one additional entry exists before identity publication. Its name/value is
UNKNOWN in that recording. The new source permits bounded differing names only;
it does not retrospectively recover the missing name.

## Directly established by inspected source

- `dispatch_context_check.py` clears/updates `os.environ` from the pinned context
  before entering FullBudgetJob. It requests only the original identity payload.
- `dispatch_full_budget.py:run` requires the parent's Python environment mapping
  to equal that context, pins launch tools, creates the immutable request, then
  calls Popen with the resolved interpreter, gate, request path and request SHA.
  The call has **no env argument**: default inheritance applies. There is no shell
  in this argv. The sole preexec callback restores the prior signal mask.
- The installed pinned Python's `subprocess.py:2068-2075` constructs an explicit
  `env_list` when env is supplied; otherwise sets it to None, with the source
  comment “Use execv instead of execve.” The native spawn/exec implementation and
  interpreter/loader startup are not reconstructed by this Python-level review.
- Gate imports its listed standard-library modules before `main`, request pin
  checking and `check_request`. The first context snapshot is in check_request,
  after gate/interpreter byte pins pass, before target/identity/ack/payload exec.
  The gate does not change os.environ in this inspected path.
- Installed `os.py:795-826` creates its environment mapping from the imported
  platform environment representation. This is not proof that every native
  environment operation is necessarily reflected at every earlier Python check.
- Installed `site.py:1062-1102` describes default startup processing, site packages
  and startup entries, then sitecustomize and conditionally usercustomize. Merely
  reading this source does NOT establish which such files, if any, executed in
  the failed child, nor that any of them augmented its environment.

## Unproven possibilities and remaining gap

The unchecked boundary is between the parent's successful mapping equality and
child snapshot: native inherited environment, native spawn/exec/interpreter or
loader startup, and imports/startup processing precede that snapshot. These are
possible boundaries only. No specific key, library, Python locale behavior,
launcher behavior, customization file or security policy is identified as cause.
The source trace gives no basis to widen the allowlist, drop a key, change env
inheritance, alter interpreter flags or admit a mismatched child.

Installed source byte hashes and exact source-file pins are sealed in the
adjacent review packet's `startup-source-pins.json`; they are evidence, not proof
of the actual child import/startup path. Prior recordings remain immutable.
