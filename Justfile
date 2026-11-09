default:
    @just --list

# Replace an rcommon checkout with the namespace-adjusted WLib tree.
mirror target:
    wkit x export "{{target}}"
