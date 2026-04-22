# Validation Notes

Minimal validation rules for later milestones:

- exactly one global clock domain
- positive tempo
- valid meter `[numerator, denominator]`
- denominator must be one of `1, 2, 4, 8, 16, 32, 64`
- no cyclic clock-domain dependencies
- unique module IDs
- unique clock-domain IDs
- referenced clock domains must exist

Milestone 11 implements a minimal engine-owned validation pass that reports explicit findings over the localhost protocol.
