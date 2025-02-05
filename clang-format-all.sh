# clang-format
# find ./src ./include -type f -not -name '*.cmd' -and -name '*.c*' -or -name '*.h*' | xargs clang-format -style="{BasedOnStyle: Google, BreakBeforeBraces: Linux, DerivePointerAlignment: false}"
find ./src ./include -type f -not -name '*.cmd' -and -name '*.c*' -or -name '*.h*' | xargs clang-format -i --style=file

# refresh line endings
git add --renormalize .

