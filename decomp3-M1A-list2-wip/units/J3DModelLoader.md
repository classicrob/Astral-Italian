# J3DModelLoader

Untested loader_name_field_ref changes only five nonexact count/name consumers to a typed field reference. loader_shape_branch_store is an independent alternative on baseline, duplicating the same count store across exhaustive paths before calls. Baselines: readJoint 99.649124% (228 B), readShape 99.72603% (292 B), readMaterialTable and v21 99.74683% (316 B each), readMaterialDL 99.76636% (428 B). Do not combine patches initially; no changed pointer-width representation.

See sibling .data.txt and .diff.txt for cached pre-quiet evidence. No candidate compile or native check was run during quiet.
