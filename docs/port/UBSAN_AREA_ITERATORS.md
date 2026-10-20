# U003/U017/U018: empty arrays and area-search sentinels

Native `AssignableArray` and `Vector` forward begin/end accessors return the
stored pointer without binding an element reference or adding zero to null
for an unallocated empty range. Allocated ranges retain their exact base and
logical end; FixedArray-backed vectors retain their fixed storage.

Native `AreaObjMgr::find_in` searches backward from end to begin, decrementing
only when the range is nonempty. It never forms `begin - 1`. The Wii branch is
preserved verbatim. The DOL at `80070D50..80070E2C` establishes:

- The search caches its original begin/end and visits last to first.
- A successful predicate stops the search; unsuccessful predicates advance it.
- After the predicates, it reloads the manager's end and compares the selected
  cell against it. It also reloads the result cell before returning its value.

The native code preserves those read epochs, including effects of virtual
predicates on the container's count or cells. It does not assume a const
predicate has no side effects. The original valid-storage requirements remain;
this is not a repair for reallocating/freeing storage during a query.

Run `python3 native/tests/area_iterators_native_test.py`. It uses the actual
Array.hpp and extracts the actual production query. Its 521 ASan/UBSan checks
cover empty/unallocated/allocated arrays, all reverse-search truth masks up to
eight elements, call order, post-predicate end reload and result-cell reload.
Two independently selected legacy branches reproduce the null-reference and
before-begin errors. The test predicate is a controlled virtual test object,
not a substitute game provider. Legacy reverse-pointer APIs elsewhere are
unchanged; this query no longer needs them.

Fresh AreaObj Wii objects compare equal with `tools/team/objsame.py`. Targeted
native checks include AreaObj and AreaObjContainer. No whole-game runtime or
whole-header-consumer sweep is claimed.
