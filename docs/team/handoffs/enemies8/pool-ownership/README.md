# Flip and Skate literal-pool ownership

MarioFlip now owns .data 806DCE80..806DCF20 (160 bytes), and MarioSkate owns 806DDD78..806DDF18 (416 bytes). Their ends are unchanged. Previously each split claimed only its 72-byte vtable; the complete preceding auto allocations contain exactly the corresponding literal prefix (88 and 344 bytes).

Both complete source data regions already equal the original DOL after all 16 ADDR32 identities per unit are resolved. No source or payload changes are required. The four neighboring Faint, Flow, SideStep and Slider allocations retain every byte and pointer identity.

Skate strict objdiff improves 1/14 to 2/14 (+76 bytes); Flip remains 0/8. Matched report bytes are unchanged, and all-unit guard reports no losses. Both original pool rows remain REAL under the unchanged default checker at returned/partial pointer provenance. No proof waiver is published.

The shared-header rebuild renumbers generated local scalar labels by two. All allocated source bytes, function identities and effective relocations remain identical; the preservation check compares exact object bounds rather than incidental generated names.
