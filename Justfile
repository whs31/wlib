default:
    @just --list

# Mirror WLib to Git, or replace an rcommon checkout when given a path.
mirror destination="https://github.com/whs31/wlib":
    wkit x export "{{destination}}"
