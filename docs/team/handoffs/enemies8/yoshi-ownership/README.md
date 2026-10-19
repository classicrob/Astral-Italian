# Yoshi animation-table and model-name ownership

The OfsInfo/Aux table has 58 live records and one empty-name sentinel, each 20 bytes. Its correct size is 0x49C; the previous 0x4A8 extent included the next model-name literal. Yoshi now owns that 12-byte `Yoshi\0` plus padding prefix at 806E9FE4. All table addresses, records, pointers, and neighboring bytes remain unchanged.

The attached bounded target evidence binds four complete consumers and all 118 relocations to the original DOL. Their actual strides and empty-name stop tests establish all six native table extents. The 72 bytes between Groups and Aux are three complete Shift-JIS strings, not records.

Groups' true native extent is 806E39B0..806E9B00 (346 records × 72 bytes, 0x6150). Its previous exporter metadata split at 806E70C4 used a synthetic continuation symbol, not an independent native allocation. Inventory found no consumers of `lbl_806E70C4` in source, headers, or configuration apart from its symbol line. The Player owner approved removing that line and recording the full canonical extent; the interior address remains Groups+0x3714. No source alias or storage is introduced.

DTK 1.8.3 infers a positive allocation for both zero-size objects and labels, then rejects their overlap with the full canonical extent. Three bounded alias-preserving metadata trials failed at this guard. The canonical-owner correction avoids retaining an unused synthetic allocation.

No Player source, header, table contents, or canonical names change. No code-match gain is claimed by this ownership correction.
